#!/bin/sh

################################################################################################
#
# Main
#
################################################################################################
git submodule add https://github.com/espressif/arduino-esp32.git components/arduino

# ArduinoJson
git submodule add https://github.com/bblanchon/ArduinoJson.git components/ArduinoJson

# ESP32 HTTPS Server
git submodule add https://github.com/fhessel/esp32_https_server.git components/esp32_https_server
# component.mk
if true ; then
	cat <<- EOF > components/esp32_https_server/component.mk
	COMPONENT_SRCDIRS:=./src
	COMPONENT_ADD_INCLUDEDIRS:=./src
	EOF
fi

# QRCode
git submodule add https://github.com/phildubach/QRCode.git components/QRCode
# component.mk
if true ; then
	cat <<- EOF > components/QRCode/component.mk
	COMPONENT_SRCDIRS:=./src
	COMPONENT_ADD_INCLUDEDIRS:=./src
	EOF
fi

################################################################################################
#
# Plugins
#
################################################################################################

# SSD_13XX
git submodule add https://github.com/sumotoy/SSD_13XX components/SSD_13XX
if true ; then
	cat <<- EOF > components/SSD_13XX/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi

# DHT
git submodule add https://github.com/adafruit/DHT-sensor-library.git components/DHT-sensor-library
if true ; then
	cat <<- EOF > components/DHT-sensor-library/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi

# Adafruit Unified Sensor
git submodule add https://github.com/adafruit/Adafruit_Sensor.git components/Adafruit_Sensor
# component.mk
if true ; then
	cat <<- EOF > components/Adafruit_Sensor/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi

# Neopixel
git submodule add https://github.com/Makuna/NeoPixelBus.git components/NeoPixelBus
# component.mk
if true ; then
	cat <<- EOF > components/NeoPixelBus/component.mk
	COMPONENT_SRCDIRS:=./src
	COMPONENT_ADD_INCLUDEDIRS:=./src
	EOF
fi

# Adafruit Neopixel
git submodule add https://github.com/adafruit/Adafruit_NeoPixel.git components/Adafruit_NeoPixel
# component.mk
if true ; then
	cat <<- EOF > components/Adafruit_NeoPixel/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi


# RC Switch
git submodule add https://github.com/sui77/rc-switch.git components/rc-switch
# component.mk
if true ; then
	cat <<- EOF > components/rc-switch/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi


# ESP8266 Infux
git submodule add https://github.com/An00bIS47/ESP8266_Influx_DB.git components/ESP8266_Influx_DB
# component.mk
if true ; then
	cat <<- EOF > components/ESP8266_Influx_DB/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi

# Adafruit BME280
git submodule add https://github.com/adafruit/Adafruit_BME280_Library.git components/Adafruit_BME280
if true ; then
	cat <<- EOF > components/Adafruit_BME280/component.mk
	COMPONENT_SRCDIRS:=.
	COMPONENT_ADD_INCLUDEDIRS:=.
	EOF
fi



################################################################################################
#
# Utils
#
################################################################################################

# homekit python
git submodule add https://github.com/jlusiardi/homekit_python.git utils/homekit_python

