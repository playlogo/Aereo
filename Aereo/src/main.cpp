#include <Arduino.h>

// Sensors
#include "sensors/pms5003.h"
#include "sensors/aht20.h"
#include "sensors/s8.h"

// Utils
#include "utils/influxdb.h"

// Setup
void setup()
{
    Serial.begin(115200);

    wifiConnect();
    influxDBSetup();

    // Sensors
    setupPMS5003();
    setupAHT20AndENS160();
    setupS8();

    // FreeRTOS
    xTaskCreate(loopS8, "loopS8", 10000, NULL, 1, NULL);
    xTaskCreate(loopPMS5003, "loopPMS5003", 10000, NULL, 1, NULL);
    xTaskCreate(loopAHT20, "loopAHT20", 10000, NULL, 1, NULL);
    xTaskCreate(loopENS160, "loopENS160", 10000, NULL, 1, NULL);
}

void loop()
{
    delay(1000);
}
