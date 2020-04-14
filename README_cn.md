
# Introduction

## ESPlane2.0

**Drones powered by ESP32&ESP\_IDF&Crazyflie**

### 一、简介

ESPlane2.0 是基于乐鑫 ESP32 开发的小型无人机解决方案，可使用手机 APP 或游戏手柄通过 Wi-Fi 网络进行连接和控制，目前已支持自稳定飞行、定高飞行、定点飞行等多种模式。该方案硬件结构简单，代码架构清晰完善,方便功能扩展，可用于STEAM教育等领域。控制系统代码来自 Crazyflie 开源工程，使用GPL3.0开源协议。
项目wiki：[https://qljz1993.gitbook.io/esplane/](https://qljz1993.gitbook.io/esplane/)

![ESPlane](https://img-blog.csdnimg.cn/20191030202043361.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzIwNTE1NDYx,size_16,color_FFFFFF,t_70)

![A swarm of drones exploring the environment, avoiding obstacles and each other. \(Guus Schoonewille, TU Delft\)](https://img-blog.csdnimg.cn/20191030202634944.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzIwNTE1NDYx,size_16,color_FFFFFF,t_70)

### 二、已实现功能

1. 自稳定模式
2. 定高模式（目前只支持手柄，APP不支持）
3. 定点模式（目前只支持手柄，APP不支持）
4. 对接cfclient上位机 
5. espilot APP控制


### 三、配置表

#### 传感器

| Sensor  | Interface | Comment |
|--|--|--|
| MPU6050 | I2C0 | must |
| VL53L1X | I2C0 | altitude hold  |
| ~~HMC5883L~~  | AUX_I2C | MPU6050 slave |
| ~~MS5611~~  | AUX_I2C | MPU6050 slave |
|PMW3901|	HSPI | | 

#### 指示灯


```
#define LINK_LED         LED_BLUE
//#define CHG_LED          LED_RED
#define LOWBAT_LED       LED_RED
//#define LINK_DOWN_LED  LED_BLUE
#define SYS_LED          LED_GREEN 
#define ERR_LED1         LED_RED
#define ERR_LED2         LED_RED
```

| State | LED | Action |
|--|--|--|
|SENSORS READY|BLUE|SOLID|
|SYSTEM READY|BLUE|SOLID|
|UDP_RX|GREEN|BLINK|


#### 引脚配置

| 引脚 | 功能 | 备注 |
| :---: | :---: | :---: |
| GPIO21 | I2C0 SDA | MPU6050 专用|
| GPIO22 | I2C0 SCL | MPU6050 专用|
| GPIO12 | MISO/SRV\_1 | PMW3901 HSPI |
| GPIO13 | MOSI | PMW3901 HSPI  |
| GPIO14 | SCLK/SRV\_2 | PMW3901 HSPI |
| GPIO15 | CS0* | PMW3901 HSPI  |
| GPIO16 | I2C1 SDA|VL53L1X|
| GPIO17 | I2C1 SCL |VL53L1X|
| GPIO19 | interrupt | MPU6050 interrupt |
| GPIO27 | SRV\_3 | ~~BUZZ+~~ |
| GPIO26 | SRV\_4 | ~~BUZZ-~~|
| GPIO23 | LED\_RED | LED\_1 |
| GPIO5 | LED\_GREEN | LED\_2 |
| GPIO18 | LED\_BLUE | LED\_3 |
| GPIO4 | MOT\_1 |  |
| GPIO33 | MOT\_2 |  |
| GPIO32 | MOT\_3 |  |
| GPIO25 | MOT\_4 |  |
| TXD0 |  |  |
| RXD0 |  |  |
| GPIO35 | ADC\_7\_BAT | VBAT/2 |

防止上电时 IO12 触发 flash 电压切换 ，可使用`espefuse.py`将flash电压固定到 3.3v

`espefuse.py --port /dev/ttyUSB0 set_flash_voltage 3.3V`

```note * Only the first device attaching to the bus can use CS0 pin.```

### 四、 esp_idf 版本

|esplane版本|esplane_commitID|esp_idf版本|esp_idf_commitID|
| :---: | :---: | :---: | :---: |
|master||release/v3.3 update20200306|6f9a7264ce20c6132fbd8309112630d0eb490fe4|
|release/v0.1||release/v3.3|46b12a560a29fa6ade07800a4abe12a026183988|
|release/v0.2||release/v3.3|46b12a560a29fa6ade07800a4abe12a026183988|
|dev_position_hold_oldversion||release/v3.3|46b12a560a29fa6ade07800a4abe12a026183988|


### 五、感谢/THANKS

1. 感谢Bitcraze开源组织提供很棒的[Crazyflie](https://www.bitcraze.io/%20)无人机项目代码
2. 感谢Espressif提供ESP32和[ESP-IDF操作系统](https://docs.espressif.com/projects/esp-idf/en/latest/index.html)
3. 感谢WhyEngineer提供的stm32 dsp移植库[esp-dsp](https://github.com/whyengineer/esp32-lin/tree/master/components/dsp_lib)

