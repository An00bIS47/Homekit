//
// HAPClient.hpp
// Homekit
//
//  Created on: 12.08.2017
//      Author: michael
//

#ifndef HAPCLIENT_HPP_
#define HAPCLIENT_HPP_

#include <Arduino.h>
#include <WiFiClient.h>
#include <StreamString.h>
#include <set>

#include "HAPGlobals.hpp"
#include "HAPRequest.hpp"
#include "HAPVerifyContext.hpp"
#include "HAPTLV8Types.hpp"

#if HAP_API_ADMIN_MODE
#include <ArduinoJson.h>
#endif

#undef write

enum HAP_CLIENT_STATE {
	HAP_CLIENT_STATE_DISCONNECTED = 0,
	HAP_CLIENT_STATE_CONNECTED,
	HAP_CLIENT_STATE_AVAILABLE,
	HAP_CLIENT_STATE_SENT,
	HAP_CLIENT_STATE_RECEIVED,
	HAP_CLIENT_STATE_IDLE,	
	HAP_CLIENT_STATE_ALL_PAIRINGS_REMOVED
};



struct HAPSubscribtionItem {
	int aid;
	int iid;

	HAPSubscribtionItem(int aid_, int iid_) : aid(aid_), iid(iid_) {};
	bool operator<(const HAPSubscribtionItem& rhs) const {
		return rhs.aid < this->aid || (rhs.aid == this->aid && rhs.iid < this->iid);
  	};

	String describe() const {
		return String(aid) + "." + String(iid);
	}
};


class HAPClientHeader : public Printable {
public:
	String name;
	String value;

	HAPClientHeader(String name_, String value_) : name(name_), value(value_) {};

	inline size_t printTo(Print& p) const {
  		return p.print(name + ": " + value);
  	}

	inline String describe() const {
		return name + ": " + value;
	}

	// inline size_t length(){
	// 	return (name.length() + 2 + value.length() + 2);
	// }

	inline bool operator==(const HAPClientHeader &header) const {
		return header.name == name && header.value == value;
	}	  
};

class HAPClient : public Stream {
public:
	HAPClient();
	~HAPClient();

	//struct HAPKeys {
	//	byte accessorySRPProof[SHA512_DIGEST_LENGTH];
	//} keys;

	HAPRequest		request;
	WiFiClient 		client;
	HAP_CLIENT_STATE 	state;
	HAP_PAIR_STATE	pairState;
	HAP_VERIFY_STATE	verifyState;	



	struct HAPVerifyContext 		verifyContext;
	struct HAPEncryptionContext 	encryptionContext;	

	// From Stream:
	size_t write(Stream &stream);
  	size_t write(const uint8_t* buffer, size_t size);
  	size_t write(uint8_t b);

	int available();
	int read();
	int peek();
	void flush();

	inline void setEncryped(bool mode) {
		_isEncrypted = mode;
	}

	inline bool isEncrypted() {
		return _isEncrypted;
	}

	inline void setChunkedMode(bool mode) {
		_chunkedMode = mode;
	}

	inline bool chunkedMode() {
		return _chunkedMode;
	}

	//bool			shouldNotify;

	void setHeader(HAPClientHeader header);
	void setHeader(String name, String value);

	size_t sendHeaderAndStatus(int statusCode, size_t size);

	void clear();

	bool operator==(const HAPClient &hap) const;

	bool isAdmin(){
		return _isAdmin;
	}

	void setAdmin(bool mode){
		_isAdmin = mode;
	}

	void subscribe(int aid, int iid, bool value = true);
	bool isSubscribed(int aid, int iid) const;

	std::set<HAPSubscribtionItem> subscribtions;

#if HAP_API_ADMIN_MODE
	String describe() const;
#endif

	static String statusMessage(int statusCode);

	const uint8_t* getId(){
		return _idPtr;
	}

	void setId(const uint8_t *id){
		memcpy(_idPtr, id, HAP_PAIRINGS_ID_LENGTH);
	}

private:
	bool			_isEncrypted;
	bool			_headerSent;
	bool			_isAdmin;	
	bool			_chunkedMode;

	std::vector<HAPClientHeader> _headers;

	String buildHeaderAndStatus(int statusCode, size_t size = 0);

	StreamString 	_sstring;

	uint8_t	_idPtr[HAP_PAIRINGS_ID_LENGTH];
			
};

#endif /* HAPCLIENT_HPP_ */
