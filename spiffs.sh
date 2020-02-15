

echo Creating SPIFFS image
mkspiffs -c spiffs -b 4096 -p 256 -s 0x00C000 build/spiffs.bin

echo Listing SPIFFS image
mkspiffs -l -b 4096 -p 256 -s 0x00C000 build/spiffs.bin

echo Uploading SPIFFS image
python2 /Users/michael/Development/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port  /dev/cu.SLAB_USBtoUART --baud 2000000 write_flash -z 0x3F2800 build/spiffs.bin
