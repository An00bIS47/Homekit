#!/bin/sh
git submodule add https://github.com/fhessel/esp32_https_server.git components/esp32_https_server
git submodule add https://github.com/phildubach/QRCode.git components/QRCode
git submodule add https://github.com/bblanchon/ArduinoJson.git main/ArduinoJson
git submodule add https://github.com/sumotoy/SSD_13XX components/SSD_13XX
git submodule add https://github.com/adafruit/DHT-sensor-library.git components/DHT-sensor-library
git submodule add https://github.com/adafruit/Adafruit_Sensor.git components/Adafruit_Sensor
git submodule add https://github.com/Makuna/NeoPixelBus.git components/NeoPixelBus
git submodule add https://github.com/adafruit/Adafruit_NeoPixel.git components/Adafruit_NeoPixel
git submodule add https://github.com/sui77/rc-switch.git components/rc-switch
git submodule add https://github.com/An00bIS47/ESP8266_Influx_DB.git components/ESP8266_Influx_DB
git submodule add https://github.com/adafruit/Adafruit_BME280_Library.git components/Adafruit_BME280
if true ; then
	cat <<- EOF > components/Adafruit_BME280/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi

#
# utils
# 
git submodule add https://github.com/jlusiardi/homekit_python.git utils/homekit_python
git submodule add https://github.com/forty2/hap-client.git utils/hap-client