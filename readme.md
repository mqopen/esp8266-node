# ESP8266 node

Firmware for mqopen [ESP8266](http://www.esp8266.com/) node. This is part of [mqopen](http://mqopen.org/) project.

mqopen ESP8266 firmware can be configured in various ways:

 - **Sensor** - Device reads data from connected hardware and sends them to the MQTT network.
 - **Reactor** - Reads data from MQTT network and is able to take some reactions.

Supported sensors:

 - [BMP180](https://www.bosch-sensortec.com/bst/products/all_products/bmp180) Barometric pressure sensor.
 - [DHT22](http://www.aosong.com/en/products/details.asp?id=117) Humidity and temperature sensor.
 - DHT11
 - [BH1750FVI](http://www.rohm.com/web/global/products/-/product/BH1750FVI) Ambient light sensor.
 - [DS18B20](https://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html) Temperature sensor.
 - Button

Implemented reactors:

 - Pinstate

## Build firmware

### Dependencies

You must have [ESP8266 open SDK](https://github.com/pfalcon/esp-open-sdk) installed
on your computer.

### Configure source tree

To configure firmware source tree, run following command:

    $ make menuconfig

### Compile

Once you have firmware configured, compile it using following command:

    $ make

### Upload fimware to hardware

Finally, upload firmware to hardware using following command:

    $ make upload
