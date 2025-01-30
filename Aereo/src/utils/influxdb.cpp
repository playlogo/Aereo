#include "../secrets.h"

#include <WiFiMulti.h>
WiFiMulti wifiMulti;

#include <InfluxDbClient.h>
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

#include "neopixel.h"

void wifiConnect()
{
    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

    bool ledBlinkOdd = false;

    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);

        if (ledBlinkOdd)
        {
            setColor(0, 0, 40);
        }
        else
        {
            setColor(0, 0, 0);
        }

        ledBlinkOdd = !ledBlinkOdd;
    }

    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());

    setColor(0, 100, 0);
    delay(1000);
}

void influxDBSetup()
{
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
}