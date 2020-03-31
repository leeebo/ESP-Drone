# Introduction

## ESPlane2.0

**Drones powered by ESP32&ESP-IDF&Crazyflie**

### Introduction

**ESPlane2.0** is an open source **drone solution** based on espressif **ESP32** Wi-Fi chip, which can be controlled through **Wi-Fi** network using mobile APP or gamepad. ESPlane2.0 supports multiple fly modes, `stabilize`, `height-hold`, `position-hold` and more. ESPlane2.0 solution has **simple hardware structure**,**clear and extendible code architecture**, can be used in **STEAM education** and other fields. The main code ported from **Crazyflie** open source project, using the **GPL3.0** open source protocol.

**For User**: [01-ESPlane2.0 Operate Guide](esplane2.0-kai-fa-bi-ji/00esplane-shang-wei-ji-an-zhuang-zhi-yin.md)

**For Developer**: [01-ESPlane2.0 Develop Guide](esplane2.0-kai-fa-bi-ji/00esplane-kai-fa-zhi-yin.md)

![ESPlane](https://img-blog.csdnimg.cn/20191030202043361.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzIwNTE1NDYx,size_16,color_FFFFFF,t_70)

![A swarm of drones exploring the environment, avoiding obstacles and each other. \(Guus Schoonewille, TU Delft\)](https://img-blog.csdnimg.cn/20191030202634944.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzIwNTE1NDYx,size_16,color_FFFFFF,t_70)

### Implemented Features

1. Stabilize mode
2. Height-hold mode (through cfcilent)
3. position-hold mode (through cfcilent)
4. cfclient supported
5. ESPilot supported

### Configuration

#### Sensor

| Sensor | Interface | Comment |
| :--- | :--- | :--- |
| MPU6050 | I2C0 | must |
| VL53L1X | I2C0 | altitude hold  |
| ~~HMC5883L~~  | AUX_I2C | MPU6050 slave |
| ~~MS5611~~  | AUX_I2C | MPU6050 slave |
|PMW3901|	HSPI | |

#### LED

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


#### PIN

| Pin | Function | Remarks |
| :---: | :---: | :---: |
| GPIO21 | I2C0 SDA | MPU6050 dedicated|
| GPIO22 | I2C0 SCL | MPU6050 dedicated|
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

>note * Only the first device attaching to the bus can use CS0 pin.

Note: Please pay attention to the flash voltage switch when using GPIO12.

You can use `espefuse.py` to fix the flash voltage to 3.3v
```
espefuse.py --port /dev/ttyUSB0 set_flash_voltage 3.3V
```

####  ESP-IDF Version

|ESPlane|CommitID| ESP-IDF|CommitID|
| :---: | :---: | :---: | :---: |
|master||release/v3.3 update20200306|6f9a7264ce20c6132fbd8309112630d0eb490fe4|
|release/v0.1||release/v3.3|46b12a560a29fa6ade07800a4abe12a026183988|
|release/v0.2||release/v3.3|46b12a560a29fa6ade07800a4abe12a026183988|
|dev_position_hold_oldversion||release/v3.3|46b12a560a29fa6ade07800a4abe12a026183988|


### THANKS

1. Thanks to the Bitcraze for the great [Crazyflie project](https://www.bitcraze.io/%20)
2. Thanks to Espressif for the powerful [ESP-IDF environment](https://docs.espressif.com/projects/esp-idf/en/latest/index.html)
3. Thanks to WhyEngineer for the useful [ESP-DSP lib](https://github.com/whyengineer/esp32-lin/tree/master/components/dsp_lib)

