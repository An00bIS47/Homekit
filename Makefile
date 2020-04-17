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