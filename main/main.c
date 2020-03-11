// =========================================================================
// Released under the MIT License
// Copyright 2017-2018 Natanael Josue Rabello. All rights reserved.
// For the license information refer to LICENSE file in root directory.
// =========================================================================

/**
 * @file mpu_i2c.cpp
 * Example on how to setup MPU through I2C for basic usage.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define DEBUG_MODULE "APP_MAIN"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32_bridge.h"
#include "freertos/portmacro.h"
#include "i2cdev.h"
#include "platform.h"
#include "system.h"
#include "nvs_flash.h"
#include "debug_cf.h"

void app_main()
{
     esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); //wifi need this

    //Initialize the platform.
    if (platformInit() == false)
    {
        // The firmware is running on the wrong hardware. Halt
        while (1);
    }

    //Launch the system task that will initialize and start everything
    systemLaunch();


    while (true)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
