#!/bin/bash


for i in {1..100}
do
   	echo "Loading $i times"
   	python3 -m homekit.get_accessories -f ~/hap.json -a cafeec -o json | python3 /Users/michael/Development/Homekit/utils/accessory_validate/accval.py
	sleep 1
done