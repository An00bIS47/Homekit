# Homekit

### This is work-in-progress!

This project aims to implemenent something like [homebridge](https://github.com/nfarina/homebridge), but for ESP32 devices. You can write your own plugins to expose the sensors to Homekit. Homebridge is no longer needed. Fakegato History support is currently available for temperature, humidity and pressure as well as for power consumption characteritics.


It uses [arduino-esp32](https://github.com/espressif/arduino-esp32) v1.0.4. 

[esp-idf](https://github.com/espressif/esp-idf) v3.3 (46b12a560) is also required.

### Features
* Expose accessories with sensors to Homekit, like [homebridge](https://github.com/nfarina/homebridge) does
* Easy to add your own plugins
* Fakegato history support builtin (temperature, humidity and pressure, and power consumption)
* HTTPS Webserver and a small rest api
* Configuration via a json file
* Updates via Arduino OTA and web download


## Build instructions

This is a application to be used with `Espressif IoT Development Framework (ESP-IDF)` and `arduino-esp32`. 

Please check ESP-IDF docs for getting started instructions and install instructions.

[Espressif IoT Development Framework](https://github.com/espressif/esp-idf)

Then install arduino-esp32 as esp-idf component as described [here](https://github.com/espressif/arduino-esp32/blob/master/docs/esp-idf_component.md)


Once the ESP-IDF and arduino-esp32 is installed:


```shell
$ git clone https://github.com/An00bIS47/Homekit
$ cd Homekit
$ git submodule update --init --recursive
$ make -j4
$ make flash monitor
```

If you encounter errors regarding compiling the components, add a file called `components.mk` to the affected directory and specify where to find the source files.
For example the `components.mk` for the `SSD13XX` component it would look like this:

```
COMPONENT_SRCDIRS:=.
COMPONENT_ADD_INCLUDEDIRS:=.
```

or 

```
COMPONENT_SRCDIRS:=src
COMPONENT_ADD_INCLUDEDIRS:=src
```
depending where the source files of the library are located.


## SDK configuration

There seems to be a bug with `Enable hardware MPI (bignum) acceleration` so you have to disable it when working with `mbedtls srp`

Add the following defines to `$IDF_PATH/components/mbedtls/port/include/mbedtls/esp_config.h` to fix linker errors regarding HKDF and POLY1305CHACHA20.

```c++
#define MBEDTLS_HKDF_C
#define MBEDTLS_POLY1305_C
#define MBEDTLS_CHACHA20_C
#define MBEDTLS_CHACHAPOLY_C
```

## WiFi Settings

Add a file called `WiFiCredentials.hpp` in the `main` folder and edit the settings:
```c++
//
// WiFiCredentials.hpp
// Homekit
//
//  Created on: 
//      Author: 
//

#ifndef WIFICREDENTIALS_HPP_
#define WIFICREDENTIALS_HPP_

#define WIFI_SSID       "SSID"
#define WIFI_PASSWORD   "PASSWORD"

#endif /* WIFICREDENTIALS_HPP_ */
```


## Configuration

Have a look at the file `main/HAP/HAPGlobals.hpp` in order to change some configs to your needs. There you can enable the plugins you want to use.


## Reset

To reset the configuration and remove all pairing data, you can use the following command
```
make erase_flash
```

## Webserver

The webserver uses minimal template processing, therefore a SPIFFS partition is needed. 
In order to create and upload this partition, please use the following commands

```
mkspiffs -c spiffs -b 4096 -p 256 -s 0x00C000 build/spiffs.bin
esptool.py --chip esp32 --port  /dev/cu.SLAB_USBtoUART --baud 2000000 write_flash -z 0x3F2800 build/spiffs.bin
```

The server certificate and keys are hardcoded. 

Valid username for access to the webinterface and to the api can be configured. 

You can create your own certificate and keys. Please have a look [here](https://github.com/fhessel/esp32_https_server/tree/master/extras)
The certificate and keys are specified in the `component.mk` file in the `main` folder. Change it to your needs.

```
COMPONENT_EMBED_FILES := $(PROJECT_PATH)/certs/server_cert.der
COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_privatekey.der
COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_publickey.der
```

## Rest API

### Open Endpoints

Open endpoints require no authentication. 

* none

### Endpoints that require api access rights

These endpoints require basic authentication. The default username and password for access to the api is `api` and `test`. Theses values can be configured.

#### System Information

* [Show uptime](docs/api/uptime.md) : `GET /api/uptime`
* [Show heap](docs/api/heap.md) : `GET /api/heap`

#### Configuration

* [Show config](docs/api/get_config.md) : `GET /api/config`
* [Update config](docs/api/post_config.md) : `POST /api/config`

#### Reference Time for Fakegato

* [Get reference time](docs/api/get_reftime.md) : `GET /api/reftime`
* [Set reference time](docs/api/post_reftime.md) : `POST /api/reftime`


### Endpoints that require admin access rights

* [Delete pairings](docs/api/pairings.md) : `DELETE /api/pairings`
* [Show setup code](docs/api/setup.md) : `GET /api/setup`



## Working example

Change the plugin section in the file `main/HAP/HAPGlobals.hpp` to this

```
/**
 * Plugins
 ********************************************************************/
#define HAP_PLUGIN_USE_DHT          0
#define HAP_PLUGIN_USE_LED          0
#define HAP_PLUGIN_USE_SWITCH       0
#define HAP_PLUGIN_USE_MIFLORA      0
#define HAP_PLUGIN_USE_BME280       1
#define HAP_PLUGIN_USE_INFLUXDB     0
#define HAP_PLUGIN_USE_SSD1331      0
#define HAP_PLUGIN_USE_PCA301       0
#define HAP_PLUGIN_USE_NEOPIXEL     0
```
Either connect an BME280 sensor to your board or you can use also a dummy with random values.
If you choose to use a dummy sensor, then change the define in the file `plugins/BME280/HAPPluginBME280.hpp` to this

```
#define HAP_PLUGIN_BME280_USE_DUMMY	0       // if 0 then use real sensor, 
                                            // if 1 then use random values without any real sensor connected
```

## Hostname

The hostname is generated using a prefix e.g. `esp32` + `-` and the last 3 bytes of the mac address.
The prefix can be configured in the file `main/HAP/HAPGlobals.hpp`. Have a look at the value of `HAP_HOSTNAME_PREFIX`.



## Pairing

A webserver is available for a convenient way of pairing your iOS device with a QR code. Just call `https://<hostname>` (see above) and scan the QR code with your device. 

If you connect an SSD1331 OLED Display and enable the SSD1331 plugin, a QR code will be shown on the display until the device is paired.

Otherwise the default pin code to pair is `031-45-712`. 

Pairings will be stored in the nvs.

## Fakegato

To have an history for specific values, [fakegato](https://github.com/simont77/fakegato-history) is implemented. You can use the [Elgato EVE app](https://www.evehome.com/en/eve-app) to view history graphs.


Currently supported are graphs for 
* Temperature
* Humidity
* Air pressure
* Power consumption
* Switches


## Partition Table

A `partitions.csv` is included in the project and should be compiled and flashed once. 
Use `make partition_table` to create the partition table bin.
Use `make partition_table-flash` to flash only the partition table.
`make flash` will flash everything including the partition table.

The partition table is defined as follows:

| Name     | Type  | Subtype  | Offset    | Size      |
|----------|-------|----------|-----------|-----------|
| otadata  | data  | ota      | 0xD000    | 8K        |
| phy_init | data  | phy      | 0xF000    | 4K        |
| ota_0    | app   | ota_0    | 0x10000   | 1958K     |
| ota_1    | app   | ota_1    |           | 1958K     |
| nvs_key  | data  | nvs_keys |           | 4K        |
| nvs      | data  | nvs      |           | 32K       |
| storage  | data  | spiffs   |           | 48K       |



## Plugins

Plugins are meant to provide services and characteristics for the Homekit Bridge. 

Have a look at the plugins available in this folder `main/HAP/plugins/`.

There are several plugins available like

#### BME280
Exposes a temperature, humidity and pressure sensor with fakegato history support.

#### PCA301
Exposes mutliple PCA301 outlets to Homekit with fakegato history for power consumption.

#### DHT22
Exposes a temperatue and a humidity sensor with fakegato history support.

#### InfluxDB
Uploads every x seconds the values of each characteristics to an influxdb server.

#### SSD1331
View the QR code for pairing and the sensor values on an SSD1331 OLED Display.

#### MiFlora
Connects MiFlora Flower bluetooth devices to Homekit. This plugin exposes a temperature, moisture, fertility and light itensity sensor with fakegato history support.

#### RCSwitch
Exposes multiple Intertechno outlets as a switch with fakegato history support. 

#### NeoPixel
Exposes one NeoPixel to Homekit as a LED bulb.


## Cryptography

This project uses by default mbedtls and libsodium for cryptography. 
WolfSSL is also support but commented out in the makefile.


## Used Libraries

| Name | Version | URL |
|-------------|-------------|----------------|
| esp32_https_server | v0.3.1 | https://github.com/fhessel/esp32_https_server.git | 
| QRCode | v0.0.1 | https://github.com/phildubach/QRCode.git | 
| ArduinoJson | v6.14.0 | https://github.com/bblanchon/ArduinoJson.git | 
| SSD_13XX | aed648a0430a1cdd9c4c2512f7971b0dddaeb26f | https://github.com/sumotoy/SSD_13XX | 
| DHT-sensor-library | 1.3.8 | https://github.com/adafruit/DHT-sensor-library.git | 
| Adafruit_Sensor | 1.1.1 | https://github.com/adafruit/Adafruit_Sensor.git | 
| Adafruit_BME280 | 2.0.1 | https://github.com/adafruit/Adafruit_BME280_Library.git | 
| NeoPixelBus | 2.5.1 | https://github.com/Makuna/NeoPixelBus.git | 
| Adafruit_NeoPixel | 1.3.4 | https://github.com/adafruit/Adafruit_NeoPixel.git | 
| rc-switch | 2.6.3 | https://github.com/sui77/rc-switch.git | 
| ESP8266_Influx_DB | 2.0.0 | https://github.com/An00bIS47/ESP8266_Influx_DB.git | 
