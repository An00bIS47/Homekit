//
// HAPClient.cpp
// Homekit
//
//  Created on: 12.08.2017
//      Author: michael
//

#include "HAPClient.hpp"
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"
#include "HAPEncryption.hpp"

#if HAP_USE_MBEDTLS_POLY
#include "m_chacha20_poly1305.h"
#else
#include "chacha20_poly1305.h"
#endif

#include "m_chacha20_poly1305.h"
#include "mbedtls/chachapoly.h"


HAPClient::HAPClient()
: state(CLIENT_STATE_DISCONNECTED)
, pairState(PAIR_STATE_RESERVED)
, verifyState(VERIFY_STATE_RESERVED)
, _isEncrypted(false)
, _headerSent(false)
, _isAdmin(false)
, _chunkedMode(true)
// , shouldNotify(false)
{
#if HAP_DEBUG_HEAP        
    LogE("+++++++++++++++++++ " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " start", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif
}

HAPClient::~HAPClient() {
	

	subscribtions.clear();
	clear();

#if HAP_DEBUG_HEAP        
	LogE("=================== " + String(__CLASS_NAME__) + "->" + String(__FUNCTION__) + " end", true);
    HAPLogger::logFreeHeap(0,0, COLOR_MAGENTA);
#endif
}

bool HAPClient::operator==(const HAPClient &hap) const {
	return hap.client.fd() == client.fd();
}

void HAPClient::clear() {
	_headerSent = false;
	
	_headers.clear();	
	request.clear();		

}

void HAPClient::subscribe(int aid, int iid, bool value){
	struct HAPSubscribtionItem item = HAPSubscribtionItem(aid, iid);
	
	if (value){
		subscribtions.insert(item);
	} else {
		subscribtions.erase(item);
	}
	
}

bool HAPClient::isSubscribed(int aid, int iid) const {
	struct HAPSubscribtionItem item = HAPSubscribtionItem(aid, iid);
	return subscribtions.find(item) != subscribtions.end();
}

int HAPClient::available(){
	return client.available();
}

int HAPClient::peek(){
	return client.peek();
}

int HAPClient::read(){
	return client.read();
}

void HAPClient::flush(){
	client.flush();
}


String HAPClient::buildHeaderAndStatus(int statusCode, size_t size){
	
	String response = "";
	if (_headerSent == true) return response;

	// Status code and Status message
	response += "HTTP/1.1 ";
	response += String(statusCode) + " " + statusMessage(statusCode);
	response += "\r\n";		

	// Headers
	for (auto & elem : _headers){
		response += elem.describe();
		response += "\r\n";
	}	

	// Transfer-Encoding or Content length
	if (_chunkedMode) {
		response += "Transfer-Encoding: chunked\r\n";
	} else {
		response += "Content-Length: ";
		response += String(size);
		response += "\r\n";
	}
		
	// end of headers
	response += "\r\n";

	return response;
}


size_t HAPClient::sendHeaderAndStatus(int statusCode, size_t size){
	LogW("Send Headers", true);

	if (_headerSent == true) return 0;

	String response = buildHeaderAndStatus(statusCode, size);

	LogD("Response:", true);
	LogD(response, true);

	_headerSent = true;
	return _sstring.write((uint8_t*)response.c_str(), response.length());
}

size_t HAPClient::write(Stream &stream) {
	Serial.println("HAPClient::write stream ");

    uint8_t * buf = (uint8_t *)malloc(1360);
    if(!buf){
        return 0;
    }

    size_t toRead = 0, toWrite = 0, written = 0;
    size_t available = stream.available();
    while(available) {
        toRead = (available > 1360) ? 1360 : available;
        toWrite = stream.readBytes(buf, toRead);
        written += write(buf, toWrite);
        available = stream.available();
    }
    free(buf);
    return written;
}




size_t HAPClient::write(const uint8_t* buffer, size_t size) {	

	size_t bytesSend = 0;
	
	uint8_t writeBuffer[1360];
	size_t writeBufferUsed = 0;
	
	if (!_isEncrypted) {

		// size_t bytesHeader = 0;
		size_t bytesChunk = 0;

		// Send header 
		// bytesHeader += sendHeaderAndStatus(200, size);
		// bytesSend 	+= bytesHeader;

		String headerStr = buildHeaderAndStatus(200, size);

		if (headerStr != ""){
			memcpy(writeBuffer, headerStr.c_str(), headerStr.length());
			writeBufferUsed += headerStr.length();
			// bytesHeader = headerStr.length();
			_headerSent = true;
		}	
		
		int remainingSize = size;
		size_t written = 0;

		while (remainingSize > 0){
			size_t toWrite = (remainingSize > 1360 - writeBufferUsed) ? 1360 - writeBufferUsed : remainingSize;

			// chunk size for payload
			if (_chunkedMode) {
				char chunkSize[8];
				sprintf(chunkSize, "%x\r\n", toWrite);

				memcpy(writeBuffer + writeBufferUsed, chunkSize, strlen(chunkSize));
				writeBufferUsed	+= strlen(chunkSize);
				// bytesChunk += client.write((uint8_t*) chunkSize, strlen(chunkSize));			
			}

			
			// Serial.println("remainingSize: " + String(remainingSize));
			// Serial.println("toWrite: " + String(toWrite));

			memcpy(writeBuffer + writeBufferUsed, buffer + written, toWrite);
			writeBufferUsed += toWrite;

			// send payload
			// bytesSend  += _sstring.write(buffer + written, toWrite);
			// bytesChunk += _sstring.write((uint8_t*) "\r\n", 2);	

			// HAPHelper::array_print("writeBuffer", writeBuffer, writeBufferUsed);

			bytesSend += client.write(writeBuffer, writeBufferUsed);
			writeBufferUsed = 0;
			
			written += toWrite;
			remainingSize -= written;

			bytesChunk += client.write((uint8_t*) "\r\n", 2);			
			bytesSend += bytesChunk;
		}


		// End of request
		// send end chunk
		if (_chunkedMode) {
			char chunkSize[8];
			sprintf(chunkSize, "%x\r\n", 0);
			bytesSend += client.write((uint8_t*) chunkSize, strlen(chunkSize));				
		}

		// send end of request
		bytesSend += client.write((uint8_t*) "\r\n", 2);			

		
	} else {

		size_t bytesHeader = 0;
		size_t bytesChunk = 0;
		size_t encrypted_len = 0;

		int ret;

		mbedtls_chachapoly_context chachapoly_ctx;
		mbedtls_chachapoly_init(&chachapoly_ctx);
		mbedtls_chachapoly_setkey(&chachapoly_ctx, encryptionContext.encryptKey);

		// Send header 
		String headerStr = buildHeaderAndStatus(200, size);

		if (headerStr != ""){
			memcpy(writeBuffer, headerStr.c_str(), headerStr.length());
			writeBufferUsed += headerStr.length();
			bytesHeader = headerStr.length();
			
		}	

		// chunk size for payload
		if (_chunkedMode) {
			char chunkSize[8];
			sprintf(chunkSize, "%x\r\n", size);

			memcpy(writeBuffer + writeBufferUsed, chunkSize, strlen(chunkSize));
			writeBufferUsed	+= strlen(chunkSize);
			bytesChunk += strlen(chunkSize);

			// Serial.write(writeBuffer, writeBufferUsed);
			// Serial.println();

			// bytesChunk += client.write((uint8_t*) chunkSize, strlen(chunkSize));			
		}

		uint8_t nonce[12] = {0,};
		nonce[4] = encryptionContext.encryptCount % 256;
		nonce[5] = encryptionContext.encryptCount++ / 256;


		ret = mbedtls_chachapoly_starts( &chachapoly_ctx, nonce, MBEDTLS_CHACHAPOLY_ENCRYPT );
		if( ret != 0 ) {
			LogE("Error: mbedtls_chachapoly_starts failed", true);
		}

		// uint8_t* auth_tag = encrypted + plain_text_length;
		uint8_t aad[HAP_ENCRYPTION_AAD_SIZE];
		aad[0] = size % 256;
		aad[1] = size / 256;				

		// memcpy(encryptedBuffer, aad, HAP_ENCRYPTION_AAD_SIZE);

		// buffer += HAP_ENCRYPTION_AAD_SIZE;
		// encryptedBuffer += HAP_ENCRYPTION_AAD_SIZE;
		encrypted_len += HAP_ENCRYPTION_AAD_SIZE;


		ret = mbedtls_chachapoly_update_aad( &chachapoly_ctx, aad, HAP_ENCRYPTION_AAD_SIZE );
		if( ret != 0 ) {
			LogE("Error: mbedtls_chachapoly_update_aad failed", true);
		}

		// int err = mbedtls_chachapoly_encrypt_and_tag(&chachapoly_ctx,plain_text_length,nonce,aad,aad_len,plain_text,encrypted,auth_tag);

		int remainingSize = size;
		size_t written = 0;


		while (remainingSize > 0){
			int toWrite = (remainingSize > 1360 - writeBufferUsed) ? 1360 - writeBufferUsed : remainingSize;
			
			uint8_t encryptedBuffer[toWrite + bytesHeader + bytesChunk];

			if (!_headerSent){
				bytesHeader = 0;
				_headerSent = true;
				bytesChunk = 0;
			}

			// Serial.println("remainingSize: " + String(remainingSize));
			// Serial.println("toWrite: " + String(toWrite));

			memcpy(writeBuffer + writeBufferUsed, buffer + written, toWrite);

			ret = mbedtls_chachapoly_update( &chachapoly_ctx, toWrite, writeBuffer, encryptedBuffer);
			if( ret != 0 ) {
				LogE("Error: mbedtls_chachapoly_update failed", true);
			}				
			writeBufferUsed += toWrite + bytesHeader + bytesChunk;

			// send payload
			// bytesSend  += _sstring.write(buffer + written, toWrite);
			// bytesChunk += _sstring.write((uint8_t*) "\r\n", 2);	

			// HAPHelper::array_print("Header + data", writeBuffer, writeBufferUsed);

			// HAPHelper::array_print("Header + data [ENC]", encryptedBuffer, sizeof(encryptedBuffer));

			
			bytesSend += _sstring.write(encryptedBuffer, sizeof(encryptedBuffer));
			writeBufferUsed = 0;
			
			written += toWrite;
			remainingSize -= toWrite + bytesHeader + bytesChunk;
		}

		memset(writeBuffer, 0, 1360);

		{
			const char* chunkSize = "\r\n";
			memcpy(writeBuffer + writeBufferUsed, chunkSize, strlen(chunkSize));
			writeBufferUsed	+= strlen(chunkSize);
		}


		// End of request
		// send end chunk
		if (_chunkedMode) {
			char chunkSize[8];
			sprintf(chunkSize, "%x\r\n", 0);
			memcpy(writeBuffer + writeBufferUsed, chunkSize, strlen(chunkSize));
			writeBufferUsed	+= strlen(chunkSize);						
		}

		{
			const char* chunkSize = "\r\n";
			memcpy(writeBuffer + writeBufferUsed, chunkSize, strlen(chunkSize));
			writeBufferUsed	+= strlen(chunkSize);
		}

		
		uint8_t encryptedBuffer[writeBufferUsed];
		ret = mbedtls_chachapoly_update( &chachapoly_ctx, writeBufferUsed, writeBuffer, encryptedBuffer);
		if( ret != 0 ) {
			LogE("Error: mbedtls_chachapoly_update failed", true);
		}


		// HAPHelper::array_print("end data", writeBuffer, writeBufferUsed);

		// HAPHelper::array_print("end data [ENC]", encryptedBuffer, writeBufferUsed);

		bytesSend += _sstring.write(encryptedBuffer, writeBufferUsed);
		
		uint8_t tag[16];
		ret = mbedtls_chachapoly_finish( &chachapoly_ctx, tag );
		if( ret != 0 ) {
			LogE("Error: mbedtls_chachapoly_finish failed", true);
		}
		

		// HAPHelper::array_print("tag", tag, 16);

		bytesSend += _sstring.write(tag, 16);		
		client.write(_sstring);

		
		// size_t bytesHeader = 0;
		// size_t bytesChunk = 0;

		// // Send header + chunked
		// bytesHeader += sendHeaderAndStatus(200, size);
		// bytesSend 	+= bytesHeader;

		// if (_chunkedMode) {
		// 	char chunkSize[8];
		// 	sprintf(chunkSize, "%x\r\n", size);
		// 	bytesChunk += _sstring.write((uint8_t*) chunkSize, strlen(chunkSize));			
		// }

		// bytesSend 	+= _sstring.write(buffer, size);

		// // send end chunk
		// if (_chunkedMode) {
		// 	char chunkSize[8];
		// 	sprintf(chunkSize, "%x\r\n", 0);
		// 	bytesSend += _sstring.write((uint8_t*) chunkSize, strlen(chunkSize));				
		// }

		// // send end of request
		// bytesSend += _sstring.write((uint8_t*) "\r\n", 2);	

		// uint8_t encryptedBuffer[bytesHeader + size + CHACHA20_POLY1305_AUTH_TAG_LENGTH];

		// uint8_t b[3192];
		// _sstring.readBytes(b,_sstring.length());

		// size_t encryptedLen = HAPEncryption::encrypt(b, bytesHeader + size + CHACHA20_POLY1305_AUTH_TAG_LENGTH, encryptedBuffer, encryptionContext->encryptKey, encryptionContext->encryptCount++);

		// client.write(encryptedBuffer, encryptedLen);

		// mbedtls_chachapoly_context chachapoly_ctx;
		// mbedtls_chachapoly_init(&chachapoly_ctx);
		// mbedtls_chachapoly_setkey(&chachapoly_ctx, encryptionContext->encryptKey);


		// size_t toRead = 0, toWrite = 0, written = 0;
		// size_t available = stream.available();

		// while(available) {
		// 	toRead = (available > 1360) ? 1360 : available;
		// 	toWrite = stream.readBytes(buf, toRead);

		// 	uint8_t* auth_tag = encrypted + plain_text_length;


		// 	available = stream.available();
		// }
		// free(buf);
		// return written;




		// size_t encryptedLen = 0;

		// // verschlüssel header == response
		// String headerStr = buildHeaderAndStatus(200, size);		
		
		// size_t bufferSize = headerStr.length() + size > 1024 ? 1024 : headerStr.length() + size;

		// uint8_t encryptedBuffer[bufferSize];
		
		// if (headerStr != ""){
			
    	// 	encryptedLen = HAPEncryption::encrypt((uint8_t*)headerStr.c_str(), headerStr.length(), encryptedBuffer, encryptionContext->encryptKey, encryptionContext->encryptCount++);

		// 	HAPHelper::array_print("encryptedBuffer", encryptedBuffer, encryptedLen);

		// 	LogD("\n>>> Sending " + String(encryptedLen) + " bytes encrypted headers to client [" + client.remoteIP().toString() + "]", true);
		// 	bytesSend += _sstring.write(encryptedBuffer, encryptedLen);

		// 	memset(encryptedBuffer, 0, bufferSize);
		// }

		// // verschlüsselte chunksize


		// // verschlüssel payload
		// size_t bytesChunk = 0;
		// size_t remainingSize = size;
		// size_t offset = 0;
		// size_t written = 0;

		// // while (remainingSize > 0){						

		// 	encryptedLen = 0;
    	// 	encryptedLen = HAPEncryption::encrypt((uint8_t*)buffer + offset, remainingSize, encryptedBuffer + _sstring.length(), encryptionContext->encryptKey, encryptionContext->encryptCount++);
			
		// 	// send payload
		// 	bytesSend  += _sstring.write(encryptedBuffer, encryptedLen);
		// 	// bytesSend  += _sstring.write((uint8_t*) "\r\n", 2);

		// 	HAPHelper::array_print("encryptedBuffer", encryptedBuffer, encryptedLen);

		// 	LogD("\n>>> Sending " + String(encryptedLen) + " bytes encrypted payload to client [" + client.remoteIP().toString() + "]", true);
		// 	remainingSize = 0;						
		// // }

		// memset(encryptedBuffer, 0, bufferSize);
		
		// // verschlüssel end chunked
		// // verschlüssel end of request
		// char endRequest[] = {'\r','\n', '0','\r','\n','\r','\n'};
		// encryptedLen = 0;
		// encryptedLen = HAPEncryption::encrypt((uint8_t*)endRequest, strlen(endRequest), encryptedBuffer, encryptionContext->encryptKey, encryptionContext->encryptCount++);

		// _sstring.write(encryptedBuffer, encryptedLen);
	}	

	// client.write(_sstring);
	// _sstring.clear();	
	return bytesSend;
}


size_t HAPClient::write(uint8_t b){
	return client.write(b);
}

void HAPClient::setHeader(HAPClientHeader header){
	setHeader(header.name, header.value);
}

void HAPClient::setHeader(String name, String value) {
	bool found = false;
	for (auto & elem : _headers)
	{
		if (elem.name == name){
			elem.value = value;
			found = true;
			break;
		}
	}

	if (!found) {
		_headers.push_back(HAPClientHeader(name, value));
	}
}

// size_t HAPClient::headerLength(){
// 	size_t length = 0;
// 	for (auto & elem : _headers)
// 	{
// 		length += elem.length();
// 	}

// 	return length;
// }

#if HAP_API_ADMIN_MODE

String HAPClient::describe() const {
	
	String keys[4];
    String values[4];
    int i=0;
    {
        keys[i] = "isEncrypted";        
        values[i++] = String(_isEncrypted);
    }
	{
        keys[i] = "state";        
        values[i++] = String(state);
    }

	{
        keys[i] = "ip";        
        values[i++] = HAPHelper::wrap(client.remoteIP().toString());
    }

	{
        //Form subscribtions list
        int j=0;
		int noOfSubscribtions = subscribtions.size();
        String *subs = new String[noOfSubscribtions];
		for (auto &sub : subscribtions ) {
			subs[j++] = sub.describe();
		}
        keys[i] = "subscribtions";
        values[i++] = HAPHelper::arrayWrap(subs, noOfSubscribtions);
        delete [] subs;
    }
	
    return HAPHelper::dictionaryWrap(keys, values, i);
}


#endif

String HAPClient::statusMessage(int statusCode){

	switch(statusCode) {
        case 200:                        
            return "OK";
		case 201:                        
            return "Created";	
		case 202:                        
            return "Accepted";			
		case 204:                        
            return "No Content";					
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";	
		case 403:
			return "Forbidden";			
		case 404:
			return "Not Found";		
		case 405:
			return "Method not allowed";	
		case 409:
			return "Conflict";		
		case 413:
			return "Payload too large";
		case 420:
			return "Enhance your calm";	
		default:
			return "";											
	}
}


