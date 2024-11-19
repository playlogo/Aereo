#include <Arduino.h>

#include "./secrets.h"

// Wifi
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

// InfluxDB
#include <InfluxDbClient.h>
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// PMS5003
#include <HardwareSerial.h>
HardwareSerial pmsSerial(0);

struct pms5003data
{
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
};

struct pms5003data data;

Point Sensor_PMS5003("pms5003");

// AHT
#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

int tempC;
int humidity;

Point Sensor_AHT20("aht20");

// ENS160
#include "ScioSense_ENS160.h"              // ENS160 library
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // 0x53..ENS160+AHT21

Point Sensor_ENS160("ens160");

// Senseair S8
#include "s8_uart.h"

HardwareSerial S8_serial(1);

S8_UART *sensor_S8;
S8_sensor sensor;

Point Sensor_S8("s8");

// Setup
SemaphoreHandle_t access_mutex = NULL;

bool readPMSdata(Stream *s);

void readPMS5003(void *parameter);
void readAHT20(void *parameter);
void readS8(void *parameter);
void readENS160(void *parameter);

void setup()
{
    Serial.begin(115200);

    access_mutex = xSemaphoreCreateMutex();

    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();

    // InfluxDB
    if (client.validateConnection())
    {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(client.getServerUrl());
    }
    else
    {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
    }

    // PMS
    pmsSerial.begin(9600, SERIAL_8N1, 2, 3);

    // ENS160
    ens160.begin();
    Serial.println(ens160.available() ? "ens160 done." : "ens160 failed!");

    if (ens160.available())
    {
        Sensor_ENS160.addTag("major_rev", String(ens160.getMajorRev()));
        Sensor_ENS160.addTag("minor_rev", String(ens160.getMinorRev()));
        Sensor_ENS160.addTag("build", String(ens160.getBuild()));

        // Set standard mode
        ens160.setMode(ENS160_OPMODE_STD);
    }

    // AHT
    if (!aht.begin())
    {
        Serial.println("Could not find AHT20 sensor!");
    }

    // S8
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

    // FreeRTOS
    xTaskCreate(readS8, "readS8", 10000, NULL, 1, NULL);
    xTaskCreate(readPMS5003, "readPMS5003", 10000, NULL, 1, NULL);
    xTaskCreate(readAHT20, "readAHT20", 10000, NULL, 1, NULL);
    xTaskCreate(readENS160, "readENS160", 10000, NULL, 1, NULL);
}

void loop()
{
}

void readS8(void *parameter)
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

void readPMS5003(void *parameter)
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

void readAHT20(void *parameter)
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

void readENS160(void *parameter)
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

// Lib
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

    /* debugging
    for (uint8_t i=2; i<32; i++) {
      Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
    */

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