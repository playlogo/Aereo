#include "s8.h"
#include "../utils/common.h"
#include "../utils/influxdb.h"

#include "s8_uart.h"

#include <InfluxDbClient.h>
#include <HardwareSerial.h>

S8_UART *sensor_S8;
S8_sensor sensor;

HardwareSerial S8_serial(1);
Point Sensor_S8("s8");

void setupS8()
{
    S8_serial.begin(S8_BAUDRATE, SERIAL_8N1, 4, 5);
    sensor_S8 = new S8_UART(S8_serial);

    sensor_S8->get_firmware_version(sensor.firm_version);
    int len = strlen(sensor.firm_version);
    if (len == 0)
    {
        Serial.println("SenseAir S8 CO2 sensor not found!");
    }
    else
    {
        Sensor_S8.addTag("firmware_version", String(sensor_S8->get_sensor_ID()));
        Sensor_S8.addTag("sensor_id", String(sensor.sensor_id, HEX));
    }

    Serial.println("SenseAir S8 CO2 sensor initialized");
}

void loopS8(void *parameter)
{
    while (1)
    {
        if (xSemaphoreTake(access_mutex, portMAX_DELAY) == pdTRUE)
        {
            Sensor_S8.clearFields();

            sensor.co2 = sensor_S8->get_co2();
            Sensor_S8.addField("co2", sensor.co2);

            // If no Wifi signal, try to reconnect it
            if (wifiMulti.run() != WL_CONNECTED)
            {
                Serial.println("Wifi connection lost");
            }

            client.writePoint(Sensor_S8);

            Serial.println("S8 Data sent to InfluxDB");

            xSemaphoreGive(access_mutex);
        }

        delay(10000);
    }
}
