#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := Homekit
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/src
include $(IDF_PATH)/make/project.mk


buildnumber:
	./buildnumber --header "src/HAP/HAPBuildnumber.hpp"

cppcheck:
	cppcheck --cppcheck-build-dir=build -i components -i src/ArduinoJson --enable=all --std=c++11 --force . 2>cppcheck.log

keystore:
	python '$IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate keystore_parttiion.csv keystore.bin 0x8000 --version 2'