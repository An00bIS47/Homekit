#!/bin/bash


PAIRINGDATAFILE="../build/homekitStorage.json"
ALIAS="cafeec"
DEVICEID="BC:DD:C2:CA:FE:EC"
SETUPCODE="031-45-712"
CHARACTERISTICS="2.10"

ITERATIONS=10

echo "Init homekit storage file" 
python3 -m homekit.init_controller_storage -f ${PAIRINGDATAFILE}

echo "Call unpaired /identify a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.identify -d ${DEVICEID}
	sleep 1
done
echo "==========================================="

echo ""
echo ""
echo ""


# echo "Call pair a device $ITERATIONS times"
# echo "==========================================="
# for i in $(seq 1 $ITERATIONS);
# do
#    	echo "Loading $i times"
# 	python3 -m homekit.pair -d ${DEVICEID} -p ${SETUPCODE} -f ${PAIRINGDATAFILE} -a ${ALIAS}
# 	sleep 1
# done
# echo "==========================================="

# echo ""
# echo ""
# echo ""


echo "Pair a device"
echo "==========================================="
python3 -m homekit.pair -d ${DEVICEID} -p ${SETUPCODE} -f ${PAIRINGDATAFILE} -a ${ALIAS}
echo "==========================================="

echo ""
echo ""
echo ""


echo "Call paired /identify a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.identify -f ${PAIRINGDATAFILE} -a ${ALIAS}
	sleep 1
done
echo "==========================================="

echo ""
echo ""
echo ""


echo "Call /accessory a $ITERATIONS times"
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
   	python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
	sleep 1
done	
echo "==========================================="

echo ""
echo ""
echo ""


echo "Get /characteristic a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.get_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS} -m -p -e -t 
	sleep 1
done
echo "==========================================="

echo ""
echo ""
echo ""

echo "Remove storage file"
rm ${PAIRINGDATAFILE}

