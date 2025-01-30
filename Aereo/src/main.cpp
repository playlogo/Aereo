#include <Arduino.h>

// Sensors
#include "sensors/pms5003.h"
#include "sensors/aht20.h"
#include "sensors/s8.h"

// Utils
#include "utils/influxdb.h"

// Neopixel
#include "utils/neopixel.h"

// Setup
void setup()
{
    Serial.begin(115200);

    neopixelSetup();

    wifiConnect();
    influxDBSetup();

    // Sensors
    bool enable_PMS5003 = setupPMS5003();
    bool enable_AHT20AndENS160 = setupAHT20AndENS160();
    bool enable_S8 = setupS8();

    // FreeRTOS
    if (enable_PMS5003)
    {
        xTaskCreate(loopPMS5003, "loopPMS5003", 10000, NULL, 1, NULL);
    }

    if (enable_AHT20AndENS160)
    {
        xTaskCreate(loopAHT20, "loopAHT20", 10000, NULL, 1, NULL);
        xTaskCreate(loopENS160, "loopENS160", 10000, NULL, 1, NULL);
    }

    if (enable_S8)
    {
        xTaskCreate(loopS8, "loopS8", 10000, NULL, 1, NULL);
    }

    setColor(enable_PMS5003 ? 0 : 40, enable_AHT20AndENS160 ? 0 : 40, enable_S8 ? 0 : 40);
}

void loop()
{
    delay(1000);
}
