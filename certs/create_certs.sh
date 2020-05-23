#!/bin/bash

#Sparkfun
name=esp32-CAFEEC
mkdir -p "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
#openssl req -noout -text -in "$name/$name.csr"
openssl rsa -in "$name/$name.privatekey" -pubout -out "$name/$name.publickey"

#Heltec
name=esp32-AF5FA4
mkdir -p "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
openssl rsa -in "$name/$name.privatekey" -pubout -out "$name/$name.publickey"

#Feather
name=esp32-0C9D6C
mkdir -p "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
openssl rsa -in "$name/$name.privatekey" -pubout -out "$name/$name.publickey"

# Sparkfun 2
name=esp32-CB3DC4
mkdir -p "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
openssl rsa -in "$name/$name.privatekey" -pubout -out "$name/$name.publickey"


name=update_server
mkdir -p "$name"
openssl req -out "$name/$name.csr" -new -newkey rsa:2048 -nodes -keyout "$name/$name.privatekey" -subj "/C=DE/ST=/L=Munich/O=ACME/OU=/CN=$name.local"
#openssl req -noout -text -in "$name/$name.csr"

