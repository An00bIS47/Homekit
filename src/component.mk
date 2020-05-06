#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the 
# src/ directory, compile them and link them into lib(subdirectory_name).a 
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

DEBUG  		= 1

BOARD		= SPARKFUN
# BOARD		= HELTEC
# BOARD		= HUZZAH

ENABLE_OTA			= 1
ENABLE_WEBSERVER 	= 1

WOLFSSL 	= 0

SRP_TEST 	= 0


#
# ESP32
# 
CXXFLAGS += -DESP32 -DARDUINO_ARCH_ESP32

# CXXFLAGS += -DMBEDTLS_HKDF_C=1
# CXXFLAGS += -DMBEDTLS_POLY1305_C=1
# CXXFLAGS += -DMBEDTLS_CHACHA20_C=1
# CXXFLAGS += -DMBEDTLS_CHACHAPOLY_C=1

#
# Boards
# 
ifeq ($(BOARD),SPARKFUN)
	#$(info ************ Feather variant ************)
	CXXFLAGS += -DARDUINO_FEATHER_ESP32
	CXXFLAGS += -I$(PROJECT_PATH)/components/arduino/variants/feather_esp32/
	#$(info Change the Serial Flasher baud rate to 2MB)
endif	

ifeq ($(BOARD),HELTEC)
	#$(info ************ Heltec variant ************)
	CXXFLAGS += -DARDUINO_HELTEC_WIFI_KIT_32
	CXXFLAGS += -I$(PROJECT_PATH)/components/arduino/variants/heltec_wifi_kit_32/

	CPPFLAGS += -DHAP_PLUGIN_USE_SSD1306=1

	
	#$(info Change the Serial Flasher baud rate to 921K)
endif	

ifeq ($(BOARD),HUZZAH)
	#$(info ************ Feather variant ************)
	CXXFLAGS += -DARDUINO_FEATHER_ESP32
	CXXFLAGS += -I$(PROJECT_PATH)/components/arduino/variants/feather_esp32/
	#$(info Change the Serial Flasher baud rate to 2MB)
endif

# 
# WolfSSL
# 
ifeq ($(WOLFSSL),1)
    WOLFSSL_SETTINGS =        \
	    -DSIZEOF_LONG_LONG=8  \
	    -DSMALL_SESSION_CACHE \
	    -DWOLFSSL_SMALL_STACK \
		-DWOLFCRYPT_HAVE_SRP  \
		-DWOLFSSL_SHA512      \
	    -DHAVE_CHACHA         \
		-DHAVE_HKDF			  \
	    -DHAVE_ONE_TIME_AUTH  \
	    -DHAVE_ED25519        \
		-DHAVE_ED25519_KEY_EXPORT\
		-DHAVE_ED25519_KEY_IMPORT\
	    -DHAVE_OCSP           \
	    -DHAVE_CURVE25519     \
		-DHAVE_POLY1305       \
	    -DHAVE_SNI            \
	    -DHAVE_TLS_EXTENSIONS \
	    -DTIME_OVERRIDES      \
	    -DNO_DES              \
	    -DNO_DES3             \
	    -DNO_DSA              \
	    -DNO_ERROR_STRINGS    \
	    -DNO_HC128            \
	    -DNO_MD4              \
	    -DNO_OLD_TLS          \
	    -DNO_PSK              \
	    -DNO_PWDBASED         \
	    -DNO_RC4              \
	    -DNO_RABBIT           \
	    -DNO_STDIO_FILESYSTEM \
	    -DNO_WOLFSSL_DIR      \
	    -DNO_DH               \
	    -DWOLFSSL_STATIC_RSA  \
	    -DWOLFSSL_IAR_ARM     \
	    -DNDEBUG              \
	    -DHAVE_CERTIFICATE_STATUS_REQUEST \
	    -DCUSTOM_RAND_GENERATE_SEED=os_get_random

	CFLAGS += -I$(PROJECT_PATH)/components/wolfssl/
	CFLAGS += -I$(PROJECT_PATH)/components/
	CFLAGS += $(WOLFSSL_SETTINGS) -DFREERTOS -DED25519_ENABLED -DMBEDTLS_ED25519_C
else
        
endif





#
# Add soruce files
#
#  HAP
COMPONENT_SRCDIRS += HAP 
COMPONENT_ADD_INCLUDEDIRS += HAP

#  Crypto
COMPONENT_SRCDIRS += Crypto
COMPONENT_ADD_INCLUDEDIRS += Crypto



#
# Plugins incl. all subdirectories
#
# ifeq ($(OS),Windows_NT)

# else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	COMPONENT_SRCDIRS += $(shell find $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
	COMPONENT_ADD_INCLUDEDIRS += $(shell find $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
endif
ifeq ($(UNAME_S),Darwin)
	COMPONENT_SRCDIRS += $(shell gfind $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
	COMPONENT_ADD_INCLUDEDIRS += $(shell gfind $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
endif
# endif	

# Print included components dir while compiling
#$(warning COMPONENT_SRCDIRS=$(COMPONENT_SRCDIRS))	# __deprecated__


# 
# Webserver and certs
# 
ifeq ($(ENABLE_WEBSERVER),0)
	CPPFLAGS += -DHAP_ENABLE_WEBSERVER=0
else
	#
	# Certificates and keys
	#
	COMPONENT_EMBED_FILES := $(PROJECT_PATH)/certs/server_cert.der
	COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_privatekey.der
	COMPONENT_EMBED_FILES += $(PROJECT_PATH)/certs/server_publickey.der

	#
	# Website incl. Font, css. images and javascripts
	#
	COMPONENT_EMBED_TXTFILES := $(PROJECT_PATH)/www/index.html
	COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/qrcode_font.css
	COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/qrcode_container.svg
endif



#
# InfluxDB server cert
#
#COMPONENT_EMBED_TXTFILES := $(PROJECT_PATH)/certs/localCA.pem	__not_yet_working__


#
# SRP TESTING
#
ifeq ($(SRP_TEST),1)
	CFLAGS += -DSRP_TEST -Wno-pointer-sign 
endif	


# Create a SPIFFS image from the contents of the 'spiffs' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'make flash'. 
#SPIFFS_IMAGE_FLASH_IN_PROJECT := 1
#$(eval $(call spiffs_create_partition_image,storage,spiffs))	# requires esp-idf > 4.0


ifeq ($(ENABLE_OTA),0)
	CPPFLAGS += -DHAP_UPDATE_ENABLE_OTA=0	
endif

ifeq ($(DEBUG),1)	
	#
	# DEBUG
	#
	#$(info ************ TEST VERSION ************)
	CPPFLAGS += -DHAP_DEBUG=1

	ifeq ($(BOARD),SPARKFUN)
		CPPFLAGS += -DHAP_FAKEGATO_INTERVAL=500
		CPPFLAGS += -DHAP_DEBUG_REQUESTS=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_SSD1306=0

		CPPFLAGS += -DHAP_PLUGIN_USE_DHT=1
		CPPFLAGS += -DHAP_PLUGIN_DHT_USE_DUMMY=1

		CPPFLAGS += -DHAP_PLUGIN_USE_BME280=1
		CPPFLAGS += -DHAP_PLUGIN_BME280_USE_DUMMY=1

		CPPFLAGS += -DHAP_PLUGIN_USE_HYGROMETER=0
		CPPFLAGS += -DHAP_PLUGIN_HYGROMETER_USE_DUMMY=0

		CPPFLAGS += -DHAP_PLUGIN_USE_INFLUXDB=0

		CPPFLAGS += -DHAP_PLUGIN_USE_RCSWITCH=0

		CPPFLAGS += -DHAP_PLUGIN_USE_LED=1
	endif

	ifeq ($(BOARD),HELTEC)
		CPPFLAGS += -DHAP_FAKEGATO_INTERVAL=3000
		
		CPPFLAGS += -DHAP_DEBUG_HOMEKIT=0
		CPPFLAGS += -DHAP_DEBUG_ENCRYPTION=0
		CPPFLAGS += -DHAP_DEBUG_TLV8=0
		CPPFLAGS += -DHAP_DEBUG_REQUESTS=0		

		CPPFLAGS += -DHAP_PLUGIN_USE_DHT=1
		CPPFLAGS += -DHAP_PLUGIN_DHT_USE_DUMMY=1

		CPPFLAGS += -DHAP_PLUGIN_USE_BME280=1
		CPPFLAGS += -DHAP_PLUGIN_BME280_USE_DUMMY=1

		CPPFLAGS += -DHAP_PLUGIN_USE_HYGROMETER=0
		CPPFLAGS += -DHAP_PLUGIN_HYGROMETER_USE_DUMMY=0

		CPPFLAGS += -DHAP_PLUGIN_USE_INFLUXDB=0

		CPPFLAGS += -DHAP_PLUGIN_USE_RCSWITCH=0

		CPPFLAGS += -DHAP_PLUGIN_USE_LED=1

	endif

	ifeq ($(BOARD),HUZZAH)
		CPPFLAGS += -DHAP_PLUGIN_USE_SSD1306=0

		CPPFLAGS += -DHAP_FAKEGATO_INTERVAL=3000

		CPPFLAGS += -DHAP_PLUGIN_USE_BME280=1
		CPPFLAGS += -DHAP_PLUGIN_BME280_USE_DUMMY=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_DHT=1
		CPPFLAGS += -DHAP_PLUGIN_DHT_USE_DUMMY=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_HYGROMETER=1
		CPPFLAGS += -DHAP_PLUGIN_HYGROMETER_USE_DUMMY=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_INFLUXDB=1
		
		CPPFLAGS += -DHAP_PLUGIN_USE_RCSWITCH=1

		CPPFLAGS += -DHAP_PLUGIN_USE_LED=1

		CPPFLAGS += -DENABLE_OTA=0
	endif


else
	#
	# RELEASE
	#
	#$(info ************ RELEASE VERSIOIN **********)
	CXXFLAGS += -DHAP_DEBUG=0
	CPPFLAGS += -DCONFIG_ESP32_PANIC_PRINT_REBOOT="y"

	ifeq ($(BOARD),HUZZAH)
		CPPFLAGS += -DHAP_PLUGIN_USE_SSD1306=0

		CPPFLAGS += -DHAP_FAKEGATO_INTERVAL=300000

		CPPFLAGS += -DHAP_PLUGIN_USE_BME280=1
		CPPFLAGS += -DHAP_PLUGIN_BME280_USE_DUMMY=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_DHT=1
		CPPFLAGS += -DHAP_PLUGIN_DHT_USE_DUMMY=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_HYGROMETER=1
		CPPFLAGS += -DHAP_PLUGIN_HYGROMETER_USE_DUMMY=0
		
		CPPFLAGS += -DHAP_PLUGIN_USE_INFLUXDB=1
		
		CPPFLAGS += -DHAP_PLUGIN_USE_RCSWITCH=1

		CPPFLAGS += -DHAP_PLUGIN_USE_LED=1
	endif
	
endif