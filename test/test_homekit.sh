#!/bin/bash


PAIRINGDATAFILE="./homekitStorage.json"

#DEVICEID="24:6F:28:AF:5F:A4"
#ALIAS="heltec"

DEVICEID="BC:DD:C2:CA:FE:EC"
ALIAS="cafeec_admin"

ALIAS_ADD_PAIRING="cafeec_user"

SETUPCODE="031-45-712"
CHARACTERISTICS="2.10"

CHARACTERISTICS_FG="3.15"
CHARACTERISTICS_FG_REQ_ENTRY="3.17"
CHARACTERISTICS_FG_HISTORY="3.16"


ITERATIONS=16

# echo "Remove pairings through API" 
# echo "==========================================="
# curl --request DELETE \
#         --url https://esp32-cafeec/api/pairings \
#         --header 'Authorization: Basic YWRtaW46c2VjcmV0' \
#         --header 'Connection: keep-alive' \
#         --header 'Content-Type: application/json' \
#         -k \
# 		-s
# echo "==========================================="
# echo ""
# echo ""
# echo ""


echo "Init homekit storage file" 
echo "==========================================="
python3 -m homekit.init_controller_storage -f ${PAIRINGDATAFILE}
echo "==========================================="
echo ""
echo ""
echo ""



echo "Call unpaired /identify a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.identify -d ${DEVICEID}
	
done
echo "==========================================="
echo ""
echo ""
echo ""



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
	
done
echo "==========================================="
echo ""
echo ""
echo ""


echo "Get fakegato history info a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.get_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG} -m -p -e -t 
	
done
echo "==========================================="
echo ""
echo ""
echo ""


echo "Load fakegato history entries a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	# 0
	python3 -m homekit.put_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG_REQ_ENTRY} "MkIwMjEwMDAwMDAwMDA="
	python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
	python3 -m homekit.get_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG_HISTORY} -m -p -e -t 

	# 17
	python3 -m homekit.put_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG_REQ_ENTRY} "MkIwMjExMDAwMDAwMDEwMA=="
	python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
	python3 -m homekit.get_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG_HISTORY} -m -p -e -t 

	# 33
	python3 -m homekit.put_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG_REQ_ENTRY} "MkIwMjIxMDAwMDAwMDEwMA=="
	python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
	python3 -m homekit.get_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS} -c ${CHARACTERISTICS_FG_HISTORY} -m -p -e -t 


done
echo "==========================================="
echo ""
echo ""
echo ""



echo "List pairings a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.list_pairings -f ${PAIRINGDATAFILE} -a ${ALIAS}
done	
echo "==========================================="
echo ""
echo ""
echo ""


echo "Prepare additional pairing"
echo "==========================================="
RESPONSE_PREPARE=$(python3 -m homekit.prepare_add_remote_pairing -f ${PAIRINGDATAFILE} -a ${ALIAS_ADD_PAIRING})
echo "${RESPONSE_PREPARE}"
echo "==========================================="
echo ""
echo ""
echo ""

echo "Add additional pairing"
echo "==========================================="
RESPONSE_ADD=$(python3 -m homekit.add_additional_pairing -f ${PAIRINGDATAFILE} -a ${ALIAS} -p User ${RESPONSE_PREPARE:51}) 
echo "${RESPONSE_ADD}"
echo "==========================================="
echo ""
echo ""
echo ""

echo "Finish additional pairing"
echo "==========================================="
python3 -m homekit.finish_add_remote_pairing -f ${PAIRINGDATAFILE} -a ${ALIAS_ADD_PAIRING} ${RESPONSE_ADD:54}
echo "==========================================="
echo ""
echo ""
echo ""


echo "Call /accessory a $ITERATIONS times as additional user"
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
   	python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS_ADD_PAIRING} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
	
done	
echo "==========================================="
echo ""
echo ""
echo ""


echo "Get /characteristic a $ITERATIONS times as additional user"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.get_characteristic -f ${PAIRINGDATAFILE} -a ${ALIAS_ADD_PAIRING} -c ${CHARACTERISTICS} -m -p -e -t 	
done
echo "==========================================="
echo ""
echo ""
echo ""


echo "List pairings a $ITERATIONS times"
echo "==========================================="
for i in $(seq 1 $ITERATIONS);
do
   	echo "Loading $i times"
	python3 -m homekit.list_pairings -f ${PAIRINGDATAFILE} -a ${ALIAS}
done	
echo "==========================================="
echo ""
echo ""
echo ""



echo "Remove admin pairing"
echo "==========================================="
python3 -m homekit.remove_pairing -f ${PAIRINGDATAFILE} -a ${ALIAS}
echo "==========================================="
echo ""
echo ""
echo ""


echo "Remove storage file"
rm ${PAIRINGDATAFILE}

