#!/bin/sh

#deviceIds=("esp32-CAFEEC" "esp32-AF5FA4" "esp32-0C9D6C" "esp32-CB3DC4" "esp32-134248" "esp32-13994C")
deviceIds=("$1")		# ("esp32-13994C")
containerId="$2"		# 1

domain=$3			# "local"
certDir=$4			# "../../certs"
signingKey=$5		#"../../certs/rootCA/Sign/SigningPrivateKey.pem"
verificationKey=$6	#"../../certs/rootCA/Sign/SigningPublicKey.pem"
rootCA_cert=$7		#"../../certs/rootCA/ACME_CA.cer"

for i in "${deviceIds[@]}"
do
	serverCert="${certDir}/${i}/${i}.${domain}.cer"	
	#echo "${serverCert}"
	mkdir -p "$i"

	python3 makeKeystore.py "$i" -c "${containerId}" -v "${verificationKey}" -r "${rootCA_cert}" -s "${serverCert}"

	python3 $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate truststore.csv "$i"/truststore.bin 0x8000 --version 2

	echo "Signing truststore ..."
	pk_sign "${signingKey}" data.tlv8.pre
	echo "OK"

	echo "Verifying signature ..."
	pk_verify "${verificationKey}" data.tlv8.pre
	echo "OK"

	echo "Creating truststore.tlv8 ..."
	cat data.tlv8.pre > "$i"/truststore.tlv8

	size="$(wc -c data.tlv8.pre.sig | awk '{print $1}')"
	hex=$( printf "%x" $size ) 

	#echo -n -e '\xFE\x46' | hexf 

	output="\xFE\x$hex"
	printf "%b" $output >> "$i"/truststore.tlv8

	cat data.tlv8.pre.sig >> "$i"/truststore.tlv8

	echo "OK"

	echo "Cleaning ..."
	rm data.tlv8.pre.sig
	rm data.tlv8.pre
	rm truststore.csv
	echo "OK"

done