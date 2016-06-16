# ESP8266 node

Firmware for mqopen [ESP8266](http://www.esp8266.com/) node. It is intended to allow
users to build stable and unified IoT devices without need of write single line of code.

mqopen ESP8266 firmware can be configured in various ways:

 - **Sensor** - Device reads data from connected hardware and sends them to the MQTT network.
 - **Reactor** - Reads data from MQTT network and is able to take some reactions.

Supported sensors:

 - [BMP180](https://www.bosch-sensortec.com/bst/products/all_products/bmp180) Barometric pressure sensor.
 - [DHT22](http://www.aosong.com/en/products/details.asp?id=117) Humidity and temperature sensor.
 - DHT11 Humidity and temperature sensor.
 - [BH1750FVI](http://www.rohm.com/web/global/products/-/product/BH1750FVI) Ambient light sensor.
 - [DS18B20](https://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html) Temperature sensor.
 - Button generic sensor.

Implemented reactors:

 - Pinstate

## Build firmware

### Dependencies

You must have [ESP8266 open SDK](https://github.com/pfalcon/esp-open-sdk) installed
on your computer.

### Configuration

Firmware is configured using kconfig language, originally developed by Linux kernel developers. To configure firmware run following command:

    $ make menuconfig

### Compile

Once you have firmware configured, compile it using following command:

    $ make

### Upload fimware to hardware

Finally, upload firmware to hardware using following command:

    $ make upload

## About

This is part of [mqopen](http://mqopen.org/) project. Firmware is intended to be
used mainly for **mqopen** hardware designs. More information can be found
at [mqopen wiki](http://wiki.mqopen.org/).
