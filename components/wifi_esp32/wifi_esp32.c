#include <string.h>
#define DEBUG_MODULE  "WIFI_UDP"

#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "debug_cf.h"
#include  "queuemonitor.h"
//#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "lwip/api.h"
#include "tcpip_adapter.h"
#include "wifi_esp32.h"
#include "esp32_bridge.h"

#define UDP_SERVER_PORT         2390  //remotPort=2399
#define UDP_SERVER_PORT2        2392  //remotPort=2399
#define UDP_REMOTE_PORT         2399
#define UDP_REMOTE_ADDR         "192.168.43.43"
#define UDP_SERVER_RX_BUFSIZE   128

struct netconn *udp_server_netconn;
struct netconn *udp_server_netconn2;
static ip_addr_t server_ipaddr;

//#define WIFI_SSID      "Udp Server"
static char WIFI_SSID[32] = "ESPLANE";
static char WIFI_PWD[64] = "12345678" ;
#define MAX_STA_CONN (1)

static xQueueHandle udpDataRx;
static xQueueHandle udpDataTx;
static UDPPacket inPacket;
static UDPPacket inPacket2;
static UDPPacket outPacket;

static bool isInit = false;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static uint8_t calculate_cksum(void* data, size_t len)
{
  unsigned char* c = data;
  int i;
  unsigned char cksum=0;
  
  for (i=0; i<len; i++)
    cksum += *(c++);

  return cksum;
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_AP_STACONNECTED:
        //DEBUG_PRINT_LOCAL( "station:"MACSTR" join, AID=%d",MAC2STR(event->event_info.sta_connected.mac),event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        //DEBUG_PRINT_LOCAL( "station:"MACSTR" leave, AID=%d",MAC2STR(event->event_info.sta_disconnected.mac),event->event_info.sta_disconnected.aid);
        break;
    default:
        break;
    }
    return ESP_OK;
}



bool wifiTest(void)
{
    return isInit;
};

bool wifiGetDataBlocking(UDPPacket *in)
{

    while (xQueueReceive(udpDataRx, in, portMAX_DELAY) != pdTRUE){ //command receive step 02
        //vTaskDelay(20);
    }; // Don't return until we get some data on the UDP
    return true;
};

bool wifiSendData(uint32_t size, uint8_t *data)
{
    static UDPPacket outStage;
    outStage.size = size;
    memcpy(outStage.data, data, size);
    // Dont' block when sending
    return (xQueueSend(udpDataTx, &outStage, M2T(100)) == pdTRUE);
};

static esp_err_t udp_server_create(void *arg)
{
    err_t err = ERR_OK;
    udp_server_netconn = netconn_new(NETCONN_UDP);  //创建socket
    udp_server_netconn->recv_timeout = 10;
    if(udp_server_netconn == NULL) return ESP_FAIL;

    err = netconn_bind(udp_server_netconn, &server_ipaddr, UDP_SERVER_PORT);//绑定IP地址和端口号
    if (err != ERR_OK)
    {
        netconn_close(udp_server_netconn);
        netconn_delete(udp_server_netconn);
        return ESP_FAIL;
    }

    /*udp_server_netconn2 = netconn_new(NETCONN_UDP);  //创建socket
    udp_server_netconn2->recv_timeout = 10;
    if(udp_server_netconn2 == NULL) return ESP_FAIL;

    err = netconn_bind(udp_server_netconn2, &server_ipaddr, UDP_SERVER_PORT2);//绑定IP地址和端口号
    if (err != ERR_OK)
    {
        netconn_close(udp_server_netconn2);
        netconn_delete(udp_server_netconn2);
        return ESP_FAIL;
    }*/

    return ESP_OK;
}

//static uint32_t remote_addr;
static void udp_server_rx_task(void *pvParameters)
{
    struct pbuf *q = NULL;
    
    uint8_t cksum = 0;
    struct netbuf *recvbuf = NULL;
    //char recvbuffTemp[64] = {0};
    while (true)
    {
        if (netconn_recv(udp_server_netconn, &recvbuf) == ERR_OK)
        {
            for (q = recvbuf->p; q != NULL; q = q->next)
            {
                if (q->len > WIFI_RX_TX_PACKET_SIZE-4)
                {
                    //TODO:
                    DEBUG_PRINTW( "Received data LENGTH = %d > 64",q->len); 
                }
                else
                {
                    memcpy(inPacket.data, q->payload, q->len); //一部分数据
                    cksum = inPacket.data[q->len - 1];
                    inPacket.size = q->len - 1; //去掉cksum,不属于CRTP
                    if(cksum == calculate_cksum(inPacket.data,q->len - 1)){
                        xQueueSend(udpDataRx, &inPacket, M2T(2)); //command receive step 01
                    }else{
                        DEBUG_PRINTW( "udp packet cksum unmatched");
                    }
                    
#ifdef DEBUG_UDP
                    DEBUG_PRINT_LOCAL( "1.Received data LENGTH = %d  %02X \n cksum = %02X",q->len,inPacket.data[0],cksum); 
                    for (size_t i = 0; i < q->len; i++)
                    {
                       DEBUG_PRINT_LOCAL( " data[%d] = %02X ",i,inPacket.data[i]); 
                    }
#endif                                   

                }
            }
        }
        if(recvbuf != NULL) netbuf_delete(recvbuf);
    }
    vTaskDelete(NULL);
}

//static uint32_t remote_addr;
static void udp_server_rx2_task(void *pvParameters)
{
    struct pbuf *q = NULL;

    uint8_t cksum = 0;
    struct netbuf *recvbuf = NULL;
    //char recvbuffTemp[64] = {0};
    while (true)
    {
        if (netconn_recv(udp_server_netconn2, &recvbuf) == ERR_OK)
        {
            for (q = recvbuf->p; q != NULL; q = q->next)
            {
                if (q->len > (sizeof(UDPPacket) + 1))
                {
                    //TODO:
                }
                else
                {
                    memcpy(inPacket2.data, q->payload, q->len); //一部分数据
                    cksum = inPacket2.data[q->len - 1];
                    inPacket2.size = q->len - 1; //去掉cksum,不属于CRTP
                    xQueueSend(udpDataRx, &inPacket2, 0);
#ifdef DEBUG_UDP
                    DEBUG_PRINT_LOCAL( "1.Received data LENGTH = %d  %02X \n cksum = %02X",q->len,inPacket2.data[0],cksum); 
                    for (size_t i = 0; i < q->len; i++)
                    {
                       DEBUG_PRINT_LOCAL( " data[%d] = %02X ",i,inPacket2.data[i]); 
                    } 
#endif
                }
            }
        }
        netbuf_delete(recvbuf);
    }
    vTaskDelete(NULL);
}

static void udp_server_tx_task(void *pvParameters)
{
    struct netbuf *sendbuf = NULL;
    uint8_t sendbuffTemp[64] = {0};
    portBASE_TYPE xTaskWokenByReceive = pdFALSE;
    while (TRUE)
    {
        //发送数据
        if (xQueueReceive(udpDataTx, &outPacket, &xTaskWokenByReceive) == pdTRUE)
        {
            memcpy(sendbuffTemp, outPacket.data, outPacket.size); //一部分数据
            sendbuffTemp[outPacket.size + 1] = '\0';
#ifdef DEBUG_UDP
            DEBUG_PRINTD("udpDataTx get QUEUE size = %d data = %02x",outPacket.size,outPacket.data[0]);
#endif
            sendbuffTemp[outPacket.size] =  calculate_cksum(sendbuffTemp,outPacket.size);
            sendbuf = netbuf_new();
            ip4addr_aton(UDP_REMOTE_ADDR,&(sendbuf->addr.u_addr.ip4));
            sendbuf->port = UDP_REMOTE_PORT;
            netbuf_alloc(sendbuf, outPacket.size+1);
            sendbuf->p->payload = sendbuffTemp;
            if ( true) 
            {
                if(netconn_sendto(udp_server_netconn, sendbuf, &sendbuf->addr, UDP_REMOTE_PORT) == ERR_OK);
#ifdef DEBUG_UDP
                DEBUG_PRINT_LOCAL( "Send data to");
                for (size_t i = 0; i < outPacket.size+1; i++)
                {
                    DEBUG_PRINT_LOCAL( " data_send[%d] = %02X ", i, sendbuffTemp[i]);
                }
#endif
            }
        }
        netbuf_delete(sendbuf);
    }
    vTaskDelete(NULL);
}


void wifiInit(void)
{
    if(isInit)
    return;
    s_wifi_event_group = xEventGroupCreate();
    uint8_t mac[6];
    static wifi_country_t wifi_country = {.cc="JP", .schan=1, .nchan=14, .policy=WIFI_COUNTRY_POLICY_MANUAL};

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL))
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country)); // set locales for RF and channels
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_AP, mac));
    sprintf(WIFI_SSID, "ESPLANE_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    wifi_config_t wifi_config;
    memcpy(wifi_config.ap.ssid ,WIFI_SSID,strlen(WIFI_SSID)+1) ;
    wifi_config.ap.ssid_len = strlen(WIFI_SSID);
    memcpy(wifi_config.ap.password ,WIFI_PWD,strlen(WIFI_PWD)+1) ;
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.channel  = 13;

    if (strlen(WIFI_PWD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());


    tcpip_adapter_ip_info_t ip_info = {        
        .ip.addr = ipaddr_addr("192.168.43.42"),        
        .netmask.addr = ipaddr_addr("255.255.255.0"),        
        .gw.addr      = ipaddr_addr("192.168.43.42"),    
    };    
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));    
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info));    
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

    DEBUG_PRINT_LOCAL( "wifi_init_softap complete.SSID:%s password:%s",WIFI_SSID, WIFI_PWD);

    // This should probably be reduced to a CRTP packet size
    udpDataRx = xQueueCreate(5, sizeof(UDPPacket)); /* Buffer packets (max 64 bytes) */
    DEBUG_QUEUE_MONITOR_REGISTER(udpDataRx);
    udpDataTx = xQueueCreate(1, sizeof(UDPPacket)); /* Buffer packets (max 64 bytes) */
    DEBUG_QUEUE_MONITOR_REGISTER(udpDataTx);

    if (udp_server_create(NULL) == ESP_FAIL)
    {
        DEBUG_PRINT_LOCAL( "UDP server create socket failed!!!");
    }
    else
    {
        DEBUG_PRINT_LOCAL( "UDP server create socket succeed!!!");
        xTaskCreate(udp_server_tx_task, UDP_TX_TASK_NAME, UDP_TX_TASK_STACKSIZE, NULL, UDP_TX_TASK_PRI, NULL);
        xTaskCreate(udp_server_rx_task, UDP_RX_TASK_NAME, UDP_RX_TASK_STACKSIZE, NULL, UDP_RX_TASK_PRI, NULL);
        //xTaskCreate(udp_server_rx2_task, UDP_RX2_TASK_NAME, UDP_RX2_TASK_STACKSIZE, NULL, UDP_RX2_TASK_PRI, NULL);
        isInit = true;   
    }

}