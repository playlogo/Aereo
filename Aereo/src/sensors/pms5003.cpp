#include "pms5003.h"

#include <InfluxDbClient.h>
#include <HardwareSerial.h>

#include "../utils/common.h"
#include "../utils/influxdb.h"

HardwareSerial pmsSerial(0);
struct pms5003data data;

Point Sensor_PMS5003("pms5003");

void setupPMS5003()
{
    pmsSerial.begin(9600, SERIAL_8N1, 2, 3);
    Serial.println("Serial connection with PMS5003 started");
}

bool readPMSdata(Stream *s)
{
    if (!s->available())
    {
        return false;
    }

    // Read a byte at a time until we get to the special '0x42' start-byte
    if (s->peek() != 0x42)
    {
        s->read();
        return false;
    }

    // Now read all 32 bytes
    if (s->available() < 32)
    {
        return false;
    }

    uint8_t buffer[32];
    uint16_t sum = 0;
    s->readBytes(buffer, 32);

    // get checksum ready
    for (uint8_t i = 0; i < 30; i++)
    {
        sum += buffer[i];
    }

    // The data comes in endian'd, this solves it so it works on all platforms
    uint16_t buffer_u16[15];
    for (uint8_t i = 0; i < 15; i++)
    {
        buffer_u16[i] = buffer[2 + i * 2 + 1];
        buffer_u16[i] += (buffer[2 + i * 2] << 8);
    }

    // put it into a nice struct :)
    memcpy((void *)&data, (void *)buffer_u16, 30);

    if (sum != data.checksum)
    {
        Serial.println("Checksum failure");
        return false;
    }
    // success!
    return true;
}

void loopPMS5003(void *parameter)
{
    while (1)
    {
        if (readPMSdata(&pmsSerial))
        {
            if (xSemaphoreTake(access_mutex, portMAX_DELAY) == pdTRUE)
            {
                Sensor_PMS5003.clearFields();

                Sensor_PMS5003.addField("pm10_standard", data.pm10_standard);
                Sensor_PMS5003.addField("pm25_standard", data.pm25_standard);
                Sensor_PMS5003.addField("pm100_standard", data.pm100_standard);
                Sensor_PMS5003.addField("pm10_env", data.pm10_env);
                Sensor_PMS5003.addField("pm25_env", data.pm25_env);
                Sensor_PMS5003.addField("pm100_env", data.pm100_env);
                Sensor_PMS5003.addField("particles_03um", data.particles_03um);
                Sensor_PMS5003.addField("particles_05um", data.particles_05um);
                Sensor_PMS5003.addField("particles_10um", data.particles_10um);
                Sensor_PMS5003.addField("particles_25um", data.particles_25um);
                Sensor_PMS5003.addField("particles_50um", data.particles_50um);
                Sensor_PMS5003.addField("particles_100um", data.particles_100um);

                // If no Wifi signal, try to reconnect it
                if (wifiMulti.run() != WL_CONNECTED)
                {
                    Serial.println("Wifi connection lost");
                }

                client.writePoint(Sensor_PMS5003);

                Serial.println("PMS5003 Data sent to InfluxDB");
                xSemaphoreGive(access_mutex);
            }
        }

        delay(5);
    }
}