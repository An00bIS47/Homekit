
set(HAP_SRCS
	src/HAP/HAPWebServer.cpp
	src/HAP/HAPHelper.cpp
	src/HAP/HAPFakeGatoEnergy.cpp
	src/HAP/HAPKeystore.cpp
	src/HAP/HAPFakeGato.cpp
	src/HAP/HAPDailyTimer.cpp
	src/HAP/HAPCharacteristic.cpp
	src/HAP/HAPClient.cpp
	src/HAP/HAPButton.cpp
	src/HAP/HAPRequest.cpp
	src/HAP/EventManager.cpp
	src/HAP/HAPWebServerJWT.cpp
	src/HAP/HAPPreferencesExt.cpp
	src/HAP/HAPTLV8.cpp
	src/HAP/HAPWebServerTemplateProcessor.cpp
	src/HAP/HAPPlugins.cpp
	src/HAP/HAPWebServerBodyParserURLEncoded.cpp
	src/HAP/HAPEncryption.cpp
	src/HAP/HAPServer.cpp
	src/HAP/HAPWiFiHelper.cpp
	src/HAP/HAPDeviceID.cpp
	src/HAP/HAPLogger.cpp
	src/HAP/HAPUpdate.cpp
	src/HAP/HAPFakeGatoWeather.cpp
	src/HAP/base64url.cpp
	src/HAP/HAPService.cpp
	src/HAP/HAPIndicatorPixel.cpp
	src/HAP/HAPBonjour.cpp
	src/HAP/HAPSVG.cpp
	src/HAP/HAPConfig.cpp
	src/HAP/HAPAccessory.cpp
	src/HAP/HAPFakeGatoFactory.cpp
	src/HAP/HAPWebServerBodyParserMultipart.cpp
	src/HAP/HAPPairings.cpp
	src/HAP/HAPFakeGatoScheduleEnergy.cpp
	src/HAP/HAPDailyTimerFactory.cpp
	src/HAP/HAPAccessorySet.cpp
	src/HAP/HAPFakeGatoSwitch.cpp
	src/HAP/base36.c
	src/HAP/concat.c
)

set(CRYPTO_SRCS
	src/Crypto/m_srp.c
	src/Crypto/m_curve25519.c
	src/Crypto/tutils.c
	src/Crypto/m_hkdf.c
	src/Crypto/m_chacha20_poly1305.c
)

set(PLUGIN__SRCS
)

set(PLUGIN_NIMBLE_MIFLORA_SRCS
	src/HAP/plugins/Nimble_MiFlora/HAPPluginNimbleMiFlora.cpp
	src/HAP/plugins/Nimble_MiFlora/HAPPluginNimbleMiFloraDevice.cpp
)

set(PLUGIN_HYGROMETER_SRCS
	src/HAP/plugins/Hygrometer/HAPPluginHygrometer.cpp
)

set(PLUGIN_RCSWITCH_SRCS
	src/HAP/plugins/RCSwitch/HAPPluginRCSwitch.cpp
	src/HAP/plugins/RCSwitch/HAPPluginRCSwitchDevice.cpp
)

set(PLUGIN_LED_SRCS
	src/HAP/plugins/LED/HAPPluginLED.cpp
)

set(PLUGIN_IR_SRCS
	src/HAP/plugins/IR/HAPPluginIRDevice.cpp
	src/HAP/plugins/IR/HAPPluginIR.cpp
)

set(PLUGIN_DHT_SRCS
	src/HAP/plugins/DHT/HAPPluginDHT.cpp
)

set(PLUGIN_NEOPIXEL_SRCS
	src/HAP/plugins/NeoPixel/HAPPluginNeoPixel.cpp
)

set(PLUGIN_BME280_SRCS
	src/HAP/plugins/BME280/HAPPluginBME280.cpp
)

set(PLUGIN_INFLUXDB_SRCS
	src/HAP/plugins/InfluxDB/HAPPluginInfluxDB.cpp
)

set(PLUGIN_FANHONEYWELL_SRCS
	src/HAP/plugins/FanHoneywell/HAPPluginFanHoneywell.cpp
)

set(PLUGIN_SSD1306_SRCS
	src/HAP/plugins/SSD1306/HAPPluginSSD1306.cpp
)

set(PLUGIN_PCA301_SRCS
	src/HAP/plugins/PCA301/HAPPluginPCA301.cpp
	src/HAP/plugins/PCA301/HAPPluginPCA301Device.cpp
	src/HAP/plugins/PCA301/funky_rfm69.cpp
)

set(PLUGIN_RF24_SRCS
	src/HAP/plugins/RF24/HAPPluginRF24DeviceDHT.cpp
	src/HAP/plugins/RF24/HAPPluginRF24DeviceWeather.cpp
	src/HAP/plugins/RF24/HAPPluginRF24Device.cpp
	src/HAP/plugins/RF24/HAPPluginRF24.cpp
)

set(PLUGIN_SSD1331_SRCS
	src/HAP/plugins/SSD1331/HAPPluginSSD1331.cpp
)

set(COMPONENT_SRCS
	${HAP_SRCS}
	${CRYPTO_SRCS}
	${PLUGIN__SRCS}
	${PLUGIN_NIMBLE_MIFLORA_SRCS}
	${PLUGIN_HYGROMETER_SRCS}
	${PLUGIN_RCSWITCH_SRCS}
	${PLUGIN_LED_SRCS}
	${PLUGIN_IR_SRCS}
	${PLUGIN_DHT_SRCS}
	${PLUGIN_NEOPIXEL_SRCS}
	${PLUGIN_BME280_SRCS}
	${PLUGIN_INFLUXDB_SRCS}
	${PLUGIN_FANHONEYWELL_SRCS}
	${PLUGIN_SSD1306_SRCS}
	${PLUGIN_PCA301_SRCS}
	${PLUGIN_RF24_SRCS}
	${PLUGIN_SSD1331_SRCS}
)

set(COMPONENT_ADD_INCLUDEDIRS
	/src/HAP
	/src/Crypto
	/src/HAP/plugins/
	/src/HAP/plugins/Nimble_MiFlora
	/src/HAP/plugins/Hygrometer
	/src/HAP/plugins/RCSwitch
	/src/HAP/plugins/LED
	/src/HAP/plugins/IR
	/src/HAP/plugins/DHT
	/src/HAP/plugins/NeoPixel
	/src/HAP/plugins/BME280
	/src/HAP/plugins/InfluxDB
	/src/HAP/plugins/FanHoneywell
	/src/HAP/plugins/SSD1306
	/src/HAP/plugins/PCA301
	/src/HAP/plugins/RF24
	/src/HAP/plugins/SSD1331
)

register_component()
