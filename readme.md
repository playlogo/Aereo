# Aereo - CO2 & PM2.5 Air Sensor with custom PCB

A DIY IoT air-quality sensor using a `Senseair S8` for CO2 and a `PMS5003` for particulate matter.

Features:

- ESP32c3 firmware to publish measurements to influxDB
- Custom PCB for easy assembly
- Grafana dashboard to display data

|<img src="https://raw.githubusercontent.com/playlogo/Aereo/refs/heads/main/docs/Aereo_1.jpg" height="350" width="350" style="object-fit: cover;" alt="Picture of assembeld Aereo PCB"/>|<img src="https://raw.githubusercontent.com/playlogo/Aereo/refs/heads/main/docs/grafana.png" height="350" width="675" style="object-fit: cover;" alt="Screenshot of the grafana dashboard"/>|
|-|-|
|<img src="https://raw.githubusercontent.com/playlogo/Aereo/refs/heads/main/docs/pcb.png" height="350" width="350" style="object-fit: cover;" alt="Picture of PCB design in KiCAD"/>|<img src="https://raw.githubusercontent.com/playlogo/Aereo/refs/heads/main/docs/schematic.png" height="350" width="675" style="object-fit: cover;" alt="Screenshot of the grafana dashboard"/>|

## Tutorial

### Installing InfluxDB & Grafana using docker

1. Clone this repo: `git clone https://github.com/playlogo/Aereo.git`
2. Run `docker compose up -d`

3. Setup Grafana at: `http://<ip>:3000`
4. Setup InfluxDB at: `http://<ip>:8086`

5. Follow [this guide](https://www.influxdata.com/blog/getting-started-influxdb-grafana/) to add InfluxDB as a Datasource to Grafana
6. Import the Grafana dashboard:
    - Create a new empty dashboard: `Home` -> `Dashboards` -> `New`
    - Click the little gear icon to get to the `Dashboard Settings`
    - Go to the `JSON Model` tab
    - Copy the contents of the `grafana_dashboard.json` file and paste them into the editor
    - Hit `Save Changes` and reload - voilà (you might still need to change the Datasource of each Graph to the one you just created)

Optional: Create your own dashboard following the guide linked above (don't forget to hit save!)

### Flashing the ESP32c3

1. Open `.vscode/Aereo.code-workspace` using VSCode, make sure you have [PlatformIO](https://platformio.org/install/ide?install=vscode) installed and setup
2. Create `Aereo/src/secrets.h` with the following content:

```cpp
#define WIFI_SSID "NEIGHBORS_WIFI"
#define WIFI_PASSWORD "123456789"

#define INFLUXDB_URL "http://192.168.178.64:8086/"
#define INFLUXDB_TOKEN "token please" // Guide: https://docs.influxdata.com/influxdb3/cloud-serverless/admin/tokens/create-token/
#define INFLUXDB_ORG "bumble"
#define INFLUXDB_BUCKET "aereo" // Guide: https://docs.influxdata.com/influxdb/v2/admin/buckets/create-bucket/
```

3. Build & Upload to the ESP32c3: `CTRL+P` -> `PlatformIO: Upload`
4. The neopixel should blink blue while the ESP connects to wifi, when connected it should turn off - if it continues showing one static color, one of the sensors has not been detected -> Check serial output: `CTRL+P` -> `PlatformIO: Upload and Monitor`

## Hardware

Required components:

- `ENS160-AHT21`: Temp + TVOC sensor ~4€ on aliexpress
- `Senseair S8`: [NDIR](https://en.wikipedia.org/wiki/Nondispersive_infrared_sensor) CO2 Sensor ~20€ on aliexpress
- `PMS5003`:  Particulate matter sensor ~14€ on aliexpress
- `WeAct ESP32C3 CoreBoard`: ~4€ on aliexpress
- `SK6812 mini e`: 1x ARGB status LED ~3€ on aliexpress (for 100pcs)

PCB:

- Houses two independent projects to save costs, the right half can simply be ignored
- Exposed connectors:
  - 2x 3 Pin (5V, GND, Data)
  - 2x 4 Pin (5V, GND, Data, Data)
  - 3x Power (5V, GND)
- Size: 100x100mm
