#include "aht20.h"

#include "../utils/common.h"
#include "../utils/influxdb.h"

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ScioSense_ENS160.h>

ScioSense_ENS160 ens160(ENS160_I2CADDR_1);
Adafruit_AHTX0 aht;

int tempC;
int humidity;

Point Sensor_AHT20("aht20");
Point Sensor_ENS160("ens160");

void setupAHT20()
{
    if (!aht.begin())
    {
        Serial.println("AHT20 sensor not found!");
    }
    else
    {
        Serial.println("AHT20 sensor found!");
    }
}

void setupENS160()
{
    // ENS160
    ens160.begin();

    if (ens160.available())
    {
        Sensor_ENS160.addTag("major_rev", String(ens160.getMajorRev()));
        Sensor_ENS160.addTag("minor_rev", String(ens160.getMinorRev()));
        Sensor_ENS160.addTag("build", String(ens160.getBuild()));

        // Set standard mode
        ens160.setMode(ENS160_OPMODE_STD);

        Serial.print("ENS160 Sensor initialized");
    }
    else
    {
        Serial.println("ENS160 sensor not found!");
    }
}

void loopAHT20(void *parameter)
{
    while (1)
    {
        if (xSemaphoreTake(access_mutex, portMAX_DELAY) == pdTRUE)
        {
            sensors_event_t humidity1, temp;
            aht.getEvent(&humidity1, &temp);
            tempC = (temp.temperature);
            humidity = (humidity1.relative_humidity);

            Sensor_AHT20.clearFields();
            Sensor_AHT20.addField("temperature", tempC);
            Sensor_AHT20.addField("humidity", humidity);

            // If no Wifi signal, try to reconnect it
            if (wifiMulti.run() != WL_CONNECTED)
            {
                Serial.println("Wifi connection lost");
            }

            client.writePoint(Sensor_AHT20);

            Serial.println("AHT20 Data sent to InfluxDB");
            xSemaphoreGive(access_mutex);
        }

        delay(1265);
    }
}

void loopENS160(void *parameter)
{
    while (1)
    {
        if (xSemaphoreTake(access_mutex, portMAX_DELAY) == pdTRUE)
        {
            if (ens160.available())
            {
                ens160.set_envdata(tempC, humidity);

                ens160.measure(true);
                ens160.measureRaw(true);

                Sensor_ENS160.clearFields();
                Sensor_ENS160.addField("aqi", ens160.getAQI());
                Sensor_ENS160.addField("tvoc", ens160.getTVOC());
                Sensor_ENS160.addField("eco2", ens160.geteCO2());

                // If no Wifi signal, try to reconnect it
                if (wifiMulti.run() != WL_CONNECTED)
                {
                    Serial.println("Wifi connection lost");
                }

                client.writePoint(Sensor_ENS160);

                Serial.println("ENS160 Data sent to InfluxDB");
            }
            xSemaphoreGive(access_mutex);
        }

        delay(878);
    }
}
