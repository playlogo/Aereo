#ifndef INFLUXDB_H
#define INFLUXDB_H

// Wifi
#include <WiFiMulti.h>
extern WiFiMulti wifiMulti;

// InfluxDB
#include <InfluxDbClient.h>
extern InfluxDBClient client;

void wifiConnect();
void influxDBSetup();

#endif
