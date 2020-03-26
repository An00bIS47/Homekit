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

#if HAP_API_ADMIN_MODE
#include <ArduinoJson.h>
#endif

#undef write

enum HAPClientState {
	CLIENT_STATE_DISCONNECTED = 0,
	CLIENT_STATE_CONNECTED,
	CLIENT_STATE_AVAILABLE,
	CLIENT_STATE_SENT,
	CLIENT_STATE_RECEIVED,
	CLIENT_STATE_IDLE,
	CLIENT_STATE_ADMIN_REMOVED,
};

enum HAPPairState {
	PAIR_STATE_RESERVED = 0,
	PAIR_STATE_M1,
	PAIR_STATE_M2,
	PAIR_STATE_M3,
	PAIR_STATE_M4,
	PAIR_STATE_M5,
	PAIR_STATE_M6,
};

enum HAPVerifyState {
	VERIFY_STATE_RESERVED = 0,
	VERIFY_STATE_M1,
	VERIFY_STATE_M2,
	VERIFY_STATE_M3,
	VERIFY_STATE_M4,
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
	HAPClientState 	state;
	HAPPairState	pairState;
	HAPVerifyState	verifyState;	

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

	String getPairState() const;
	String getVerifyState() const;
	String getClientState() const;

	void subscribe(int aid, int iid, bool value = true);
	bool isSubscribed(int aid, int iid) const;

	std::set<HAPSubscribtionItem> subscribtions;

#if HAP_API_ADMIN_MODE
	String describe() const;
#endif

	static String statusMessage(int statusCode);

private:
	bool			_isEncrypted;
	bool			_isAdmin;

	bool			_headerSent;
	bool			_chunkedMode;

	std::vector<HAPClientHeader> _headers;

	String buildHeaderAndStatus(int statusCode, size_t size = 0);

	StreamString 	_sstring;
			
};

#endif /* HAPCLIENT_HPP_ */
