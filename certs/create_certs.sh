#!/bin/bash


name=esp32-cafeec
mkdir -o "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
#openssl req -noout -text -in "$name/$name.csr"


name=update_server
mkdir -o "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
#openssl req -noout -text -in "$name/$name.csr"
