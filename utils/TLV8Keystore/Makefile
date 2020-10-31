

all: pk_sign pk_verify

pk_sign: pk_sign.c
	gcc pk_sign.c -lmbedtls -lmbedx509 -lmbedcrypto -o pk_sign

pk_verify: pk_verify.c
	gcc pk_verify.c -lmbedtls -lmbedx509 -lmbedcrypto -o pk_verify

clean:	
	rm pk_verify
	rm pk_sign