#!/bin/sh

echo Compiling
make -j8 app

if [ $? -eq 0 ]
then
  echo "Successfully created file"
else
  echo "Could not create file"
  exit 2
fi


echo Increasing build number
./buildnumber --header "main/HAP/HAPBuildnumber.hpp"
make -j8 app

echo Flash
make flash 

#echo Uploading
#scp build/Homekit.bin pi@homebridge:/home/pi/docker/docker-update_server/firmwares/

#echo Copy to local update server
#cp build/Homekit.bin utils/update_server/

echo Start monitor
make monitor
