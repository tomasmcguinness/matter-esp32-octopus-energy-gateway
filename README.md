# Matter Octopus Energy Electricity Tariff Gateway

This project is my attempt at building a Matter powered Electricity Tariff Gateway for Octopus Energy. 

It provides Octopus Agile tariff information via the new Matter 1.5 Commodity Tariff Cluster.

This allows other Matter devices in your network to have access to electricity pricing without knowing *anything* about your energy provider.

> [!NOTE]
> This project is NOT affiliated with Octopus Energy in any way. It just uses their publically available Tariff information.

# Development

This code is developed using `esp-idf` and `esp-matter`.

# Setup

Flash the code onto an ESP32 board. I have tested this on a WaveShare ESP32-C6 Dev-Kit N8.

```
cd Firmware
idf.py build flash monitor
```

One flashed, commission the device using a Matter Controller. The default pairing code of 3497-011-2332 can be used. 

# Loading a tariff

To fetch the tariff information, you need to use a console command.

```
matter esp tariff fetch
```

Once this completes, which can take a few seconds, the Agile tariff information will be available via the Commodity Tariff Cluster on Endpoint 0x01

> [!NOTE]
> It only fetches data for the 8th of January, 2026.

# Testing

> [!NOTE]
> At this time, *nobody* supports the Commodity Tariff Cluster. You can use my Home Energy Manager project.

You have limited options for testing at this time. To get started the `chip-tool` is the best option. You can query all the different attributes.

I have added *limited* support to my Home Energy Manager, which you can find here - https://github.com/tomasmcguinness/matter-js-energy-manager

Check the `custom-commodity-tariff-support` branch

# TODO 

* [] Make date selection dyanmic.
* [] Populate more than one day.
* [] Fetch tariff data automatically.
* [] Allow choice of tariff.
