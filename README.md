# Matter Octopus Energy Gateway

This project is my attempt at building Matter Energy Gateway for Octopus Energy. 

It will provide tariff information via the new Matter 1.5 clusters.

> [!NOTE]
> This project is NOT affiliated with Octpus Energy in any way.

> [!CAUTION]
> This project is on-ice. The ESP-Matter SDK doesn't support the Commodity Tariff cluster at present (13 Dec 2025).

## Setup

Flash the code onto an ESP32 board. I have tested this using an ESP32-C6 DevKit.

```
cd Firmware
idf.py build flash monitor
```
