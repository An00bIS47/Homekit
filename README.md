# Homekit

# This is work-in-progress!


## What is this

This application exposes sensors like temperature and humidity to Apple's Homekit on a ESP32. No other Bridge, like [homebridge](https://github.com/nfarina/homebridge) is required! It is build upon the [esp-idf](https://github.com/espressif/esp-idf) and uses [arduino-esp32](https://github.com/espressif/arduino-esp32) as a component.

To expose different sensors simultaneously, a plugin system is used. 
A plugin can 
* expose multiple sensors like a temperature probe as Homekit accessory 
* display information on e.g. a OLED display
* forward sensor values to e.g. a database or mqtt broker
* be configured via a webinterface or a REST API
* etc.

You can use multiple plugin at once.

There are several example plugins available like 


#### BME280 
Exposes a temperature, humidity and pressure sensor with EVE history support.

#### DHT22
Exposes a temperatue and a humidity sensor with EVE history support.


#### SSD1331
View the QR code for pairing and the sensor values on an SSD1331 OLED Display.

#### SSD1306
View the QR code for pairing and the sensor values on an SSD1306 OLED Display.


#### MiFlora
Connects MiFlora Flower bluetooth devices to Homekit. This plugin exposes a temperature, moisture, fertility and light itensity sensor with EVE history support.

#### RCSwitch
Exposes multiple Intertechno outlets as a switch with EVE history support. 


#### InfluxDB
Uploads every x seconds the values of each characteristics to an influxdb server.


#### LED
Exposes the blinking buildin LED as Light to Homekit.




### EVE History

Some of the plugins also support Elgato EVE histories. You can use the [Elgato EVE app](https://www.evehome.com/en/eve-app) to view history graphs for different sensors.


Currently the following services supported EVE history
* Temperature
* Humidity
* Air pressure
* Power consumption
* Switches



## Required Versions
* esp-idf v3.3.x
* arduino-esp32 v1.0.4


## Build instructions

This is a application to be used with `Espressif IoT Development Framework (ESP-IDF)` and `arduino-esp32`. 

Please check ESP-IDF docs for getting started instructions and install instructions.

[Espressif IoT Development Framework](https://github.com/espressif/esp-idf)


Once the ESP-IDF is installed:

```shell
$ git clone https://github.com/An00bIS47/Homekit
$ cd Homekit
$ git submodule update --init --recursive
$ make menuconfig
$ make app flash monitor
```

## macOS
The Makefile uses `gfind` to look for source code file for compilation. You need to install it via:
```shell
brew install findutils
```

## Linker Errors
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
depending where the source files of the libraries are located.


## SDK configuration

There seems to be a bug with `Enable hardware MPI (bignum) acceleration` so you have to disable it when working with `mbedtls srp` (default)

Add the following defines to `$IDF_PATH/components/mbedtls/port/include/mbedtls/esp_config.h` to fix linker errors regarding HKDF and POLY1305CHACHA20.

```c++
#define MBEDTLS_HKDF_C
#define MBEDTLS_POLY1305_C
#define MBEDTLS_CHACHA20_C
#define MBEDTLS_CHACHAPOLY_C
```
This seems to change with v4.0 of the esp-idf but it is still required for v3.3.x!


## WiFi Provisioning

There are several ways to provide WiFi credentials. It uses `WiFiMulti`, so you can provide multiple WiFi credentials. 

Currently supported methods are:
    * WPS (Push Button)
    * via `WiFiCredentials.hpp` or compile time CFLAG
    * BLE Provisioning (under construction)
    * Access Point Provisioning (under construction)
    * Captive Portal (under construction)


### WPS
Add the following to the end of the file `src/component.mk`
```
CPPFLAGS += -DHAP_WIFI_DEFAULT_MODE=2
```

### Compile-time CFLAG
Add the following to the end of the file `src/component.mk`
```
CPPFLAGS += -DHAP_WIFI_DEFAULT_MODE=1
CPPFLAGS += -DWIFI_SSID="YOUR_SSID_HERE"
CPPFLAGS += -DWIFI_PASSWORD="YOUR_PASSWORD_HERE"
```

### `WiFiCredentials.hpp`
Add the following to the end of the file `src/component.mk`
```
CPPFLAGS += -DHAP_WIFI_DEFAULT_MODE=1
```

And add a file called `WiFiCredentials.hpp` in the `src` folder and edit the settings:
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

Once configured and online, you can add additional networks via the REST API.

### BLE

under construction

### Access Point Provisioning

under construction

### Captive Portal

under construction



## Working example

Add the following at end of the file `src/component.mk`

```
CPPFLAGS += -DHAP_PLUGIN_USE_LED=1
CPPFLAGS += -DHAP_PLUGIN_USE_BME280=1
CPPFLAGS += -DHAP_PLUGIN_BME280_USE_DUMMY=1
```

This will expose one Light Bulb Accessory for the buildin LED and an accessory with temperature, relative humidity and air pressure sensor, complete with EVE history support. 


## Configuration

Have a look at the file `src/HAP/HAPGlobals.hpp` in order to change the compile time configuration to your needs. There you can also enable the plugins you want to include in the build. 

Once compiled and running, you can also configure the device via the webinterface or REST API as described below.


## Reset

To reset the configuration and remove all pairing data, you can use the following command
```
make erase_flash
```

or use the REST API.


## Webinterface and API

### Access to the Webinterface and API

Two possible groups of users are available:
* `admin`: Users of this group have access to the webinterface with all admin options
* `api`: Users of this group have access to the api only without admin options

The default username for admin access is `admin` with the password `secret`.
The default username for (only!) access to the api is `api` with the password `test`.

You edit one user of each group in the `HAPGlobals.hpp` file:
```
#define HAP_WEBSERVER_ADMIN_USERNAME	"admin"
#define HAP_WEBSERVER_ADMIN_PASSWORD	"secret"

#define HAP_WEBSERVER_API_USERNAME		"api"
#define HAP_WEBSERVER_API_PASSWORD		"test"
```
You can add/change/remove multiple users via the REST interface.



### HTTPS

The Webserver uses by default HTTPS. 

You can disable HTTPS and use HTTP if you edit the `HAPGlobals.hpp` file:
```
#define HAP_WEBSERVER_USE_SSL		0		// use SSL for WebServer 
											// Default: 1
```

### Server Certificates

HTTPS requires to create your own certificates. You can create your own certificate and keys. Please have a look [here](https://github.com/fhessel/esp32_https_server/tree/master/extras) how to do this.

The certificate and keys are specified in the `component.mk` file in the `src` folder. Change it to your needs.

```
COMPONENT_EMBED_FILES := $(PROJECT_PATH)/certs/server_cert.der
COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_privatekey.der
COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_publickey.der
```

### Template

The webserver uses a template for the webpages stored here `www/index.html`.


### SPIFFS

If you want to use `SPIFFS` instead of embedded webpages, you can enable `HAP_WEBSERVER_USE_SPIFFS` in the `HAPGlobals.hpp` file.

Then place your webpages into the `www` folder and comment the follwing lines out in `src/component.mk`.

```
#COMPONENT_EMBED_FILES := $(PROJECT_PATH)/certs/server_cert.der
#COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_privatekey.der
#COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_publickey.der
```


To create the required partition for `SPIFFS`, you can use the following command:
```
mkspiffs -c www -b 4096 -p 256 -s 0x00C000 build/spiffs.bin
esptool.py --chip esp32 --port  /dev/cu.SLAB_USBtoUART --baud 2000000 write_flash -z 0x3F2800 build/spiffs.bin
```


## Rest API

### Open Endpoints

Open endpoints require no authentication. 

* none

### Endpoints that require api access rights

These endpoints require basic authentication with username and password.

#### System Information

* [Show uptime](docs/api/uptime.md) : `GET /api/uptime`
* [Show heap](docs/api/heap.md) : `GET /api/heap`

#### Configuration

* [Show config](docs/api/get_config.md) : `GET /api/config`
* [Update config](docs/api/post_config.md) : `POST /api/config`

#### Reference time for Elgato EVE

* [Get reference time](docs/api/get_reftime.md) : `GET /api/reftime`
* [Set reference time](docs/api/post_reftime.md) : `POST /api/reftime`


### Endpoints that require admin access rights

#### Pairing Information

* [Delete pairings](docs/api/pairings.md) : `DELETE /api/pairings`
* [Show setup code](docs/api/setup.md) : `GET /api/setup`

#### Reset / Restart

* [Show setup code](docs/api/reset.md) : `POST /api/reset`
* [Show setup code](docs/api/restart.md) : `POST /api/restart`


## SNTP Client

To get the current time, this implementation uses SNTP.
You can change the SNTP host and time zone and DST in the `HAPGlobals.hpp` file.

This example uses Berlin as time zone and Apple's SNTP server.
```
#define HAP_NTP_SERVER_URL			"time.euro.apple.com"				
#define HAP_NTP_TZ_INFO     		"WET-1WEST,M3.5.0/01:00,M10.5.0/01:00"		
```

## Update

### Arduino OTA

Arduino OTA is enabled by default, but can be disabled via the file `src/HAP/HAPGlobals.hpp`. Have a look at the value of 

```
#define HAP_UPDATE_ENABLE_OTA 1
```

The default password is `secret` and can be changed or disabled in the `src/HAP/HAPGlobals.hpp`. 

Have a look at the value of 
```
#define HAP_UPDATE_OTA_PASSWORD "secret"
```

To disable a password, set the value to `""`. You can also change the default port `3232`.


### Update from the web

under construction


### Update via Webinterface

under construction


## Hostname

The hostname of a device is generated using a prefix e.g. `esp32` + `-` and the last 3 bytes of the mac address.
The prefix can be configured in the file `src/HAP/HAPGlobals.hpp`. Have a look at the value of 
```
#define HAP_HOSTNAME_PREFIX "esp32"
```


## Pairing

A webserver is available for a convenient way of pairing your iOS device with a QR code. Just call `https://<hostname>` (see above) and scan the QR code with your device. 

If you connect an SSD1331 OLED Display and enable the SSD1331 plugin, a QR code will be shown on the display until the device is paired.

Otherwise the default pin code to pair is `031-45-712`. 

Pairings will be stored in the nvs and can be deleted via Homekit, REST API (admin only) and `make erase_flash`.



## Partition Table

A `partitions.csv` is included in the project and should be compiled and flashed once. 
Use `make partition_table` to create the partition table bin.
Use `make partition_table-flash` to flash only the partition table.
`make flash` will flash everything including the partition table.

The filesize of the compiled binary with debug options and without BLE is around 2MB.

There are 3 different partition tables provided:
*  4MB
*  8MB
* 16MB


### 4MB Flash

Filename: paritions4.csv

Only one factory partition is available, therefore you have to disable all update components, such as OTA, WEB Update.


The partition table is defined as follows:

| Name     | Type  | Subtype  | Offset    | Size      |
|----------|-------|----------|-----------|-----------|
| phy_init | data  | phy      | 0xF000    | 4K        |
| factory  | app   |          | 0x10000   | 3192K     |
| nvs_key  | data  | nvs_keys |           | 4K        |
| nvs      | data  | nvs      |           | 32K       |



### 8MB Flash

Filename: paritions.csv

The partition table is defined as follows:

| Name     | Type  | Subtype  | Offset    | Size      |
|----------|-------|----------|-----------|-----------|
| otadata  | data  | ota      | 0xD000    | 8K        |
| phy_init | data  | phy      | 0xF000    | 4K        |
| ota_0    | app   | ota_0    | 0x10000   | 3192K     |
| ota_1    | app   | ota_1    |           | 3192K     |
| nvs_key  | data  | nvs_keys |           | 4K        |
| nvs      | data  | nvs      |           | 32K       |


### 16MB Flash

Filename: paritions16.csv

The partition table is defined as follows:

| Name     | Type  | Subtype  | Offset    | Size      |
|----------|-------|----------|-----------|-----------|
| otadata  | data  | ota      | 0xD000    | 8K        |
| phy_init | data  | phy      | 0xF000    | 4K        |
| ota_0    | app   | ota_0    | 0x10000   | 6384K     |
| ota_1    | app   | ota_1    |           | 6384K     |
| nvs_key  | data  | nvs_keys |           | 4K        |
| nvs      | data  | nvs      |           | 32K       |
| spiffs   | data  | spiffs   |           | 2048K     |






## Plugins

Plugins are meant to provide services and characteristics to Homekit like a temperature sensor or process the values of the accessory.

Have a look at the plugins available in this folder `src/HAP/plugins/`. 

By default, no plugin is enabled. You have to enable them yourself. You can add multiple plugin at once.

To enable a plugin, change the defines in `src/HAP/HAPGlobals.hpp`

```
#ifndef HAP_PLUGIN_USE_LED
#define HAP_PLUGIN_USE_LED			0
#endif

#ifndef HAP_PLUGIN_USE_SWITCH
#define HAP_PLUGIN_USE_SWITCH		0
#endif

#ifndef HAP_PLUGIN_USE_MIFLORA2
#define HAP_PLUGIN_USE_MIFLORA2		0
#endif

#ifndef HAP_PLUGIN_USE_SSD1331
#define HAP_PLUGIN_USE_SSD1331		0
#endif

#ifndef HAP_PLUGIN_USE_PCA301
#define HAP_PLUGIN_USE_PCA301		0
#endif

#ifndef HAP_PLUGIN_USE_NEOPIXEL
#define HAP_PLUGIN_USE_NEOPIXEL		0
#endif

#ifndef HAP_PLUGIN_USE_INFLUXDB
#define HAP_PLUGIN_USE_INFLUXDB		0
#endif

#ifndef HAP_PLUGIN_USE_HYGROMETER
#define HAP_PLUGIN_USE_HYGROMETER	0
#endif

#ifndef HAP_PLUGIN_USE_RCSWITCH
#define HAP_PLUGIN_USE_RCSWITCH		0
#endif

#ifndef HAP_PLUGIN_USE_DHT
#define HAP_PLUGIN_USE_DHT			0
#endif

#ifndef HAP_PLUGIN_USE_BME280
#define HAP_PLUGIN_USE_BME280		0	// < last digit of feature number
#endif
```


or add the according define at end of the file `src/component.mk`, for example

```
CPPFLAGS += -DHAP_PLUGIN_USE_BME280=1
CPPFLAGS += -DHAP_PLUGIN_USE_LED=1
```


## Cryptography

This project uses by default mbedtls and libsodium for cryptography. 
WolfSSL is also support but commented out in the makefile. (will be removed completely)




## Used Libraries

| Name | Version | URL |
|-------------|-------------|----------------|
| esp32_https_server | v1.0.0 | https://github.com/fhessel/esp32_https_server.git | 
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
