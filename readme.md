# Aereo - Simple air quality logger

# SECRETS

Sensors

- Temp + TVOC: AHT20 + ENS160 Combo
- PM2.5: PMS5003
- CO2: Senseair S8

## Prototype

- Temp + TVOC sensor: <https://www.instructables.com/ENS160-AHT21-Sensor-for-Arduino/>
  - ENS160 (TVOC + eCO2): <https://github.com/adafruit/ENS160_driver>
  - AHT20: <https://github.com/adafruit/Adafruit_AHTX0>
- C02 Sensor: <https://github.com/jcomas/S8_UART/tree/main>

Pinout:

```txt
SenseAirS8:
                -
 -              -
 -              GPIO4
GND             GPIO5
5V              -
```
