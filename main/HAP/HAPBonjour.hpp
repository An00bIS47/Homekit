//
// HAPBonjour.hpp
// Homekit
//
//  Created on: 24.06.2017
//      Author: michael
//

#ifndef HAPBONJOUR_HPP_
#define HAPBONJOUR_HPP_

#include "Arduino.h"
#include "IPv6Address.h"
#include "mdns.h"

//this should be defined at build time
#ifndef ARDUINO_VARIANT
#define ARDUINO_VARIANT "esp32"
#endif

class MDNSResponder {
public:
    MDNSResponder();
    ~MDNSResponder();
    bool begin(const char* hostName);
    void end();
    
    void setInstanceName(String name);
    void setInstanceName(const char * name){
        setInstanceName(String(name));
    }
    void setInstanceName(char * name){
        setInstanceName(String(name));
    }
    
    void addService(char *service, char *proto, uint16_t port);
    void addService(const char *service, const char *proto, uint16_t port){
        addService((char *)service, (char *)proto, port);
    }
    void addService(String service, String proto, uint16_t port){
        addService(service.c_str(), proto.c_str(), port);
    }
    
    bool addServiceTxt(char *name, char *proto, char * key, char * value);
    void addServiceTxt(const char *name, const char *proto, const char *key,const char * value){
        addServiceTxt((char *)name, (char *)proto, (char *)key, (char *)value);
    }
    void addServiceTxt(String name, String proto, String key, String value){
        addServiceTxt(name.c_str(), proto.c_str(), key.c_str(), value.c_str());
    }
    
    void enableArduino(uint16_t port=3232, bool auth=false);
    void disableArduino();
    
    void enableWorkstation(wifi_interface_t interface=ESP_IF_WIFI_STA);
    void disableWorkstation();
    
    IPAddress queryHost(char *host, uint32_t timeout=2000);
    IPAddress queryHost(const char *host, uint32_t timeout=2000){
        return queryHost((char *)host, timeout);
    }
    IPAddress queryHost(String host, uint32_t timeout=2000){
        return queryHost(host.c_str(), timeout);
    }
    
    int queryService(char *service, char *proto);
    int queryService(const char *service, const char *proto){
        return queryService((char *)service, (char *)proto);
    }
    int queryService(String service, String proto){
        return queryService(service.c_str(), proto.c_str());
    }
    
    String hostname(int idx);
    IPAddress IP(int idx);
    IPv6Address IPv6(int idx);
    uint16_t port(int idx);
    
    // Added for Homekit
    bool removeServiceTxt(char *name, const char * proto);
    bool addServiceTxtSet(char *name, const char * proto, uint8_t num_items, mdns_txt_item_t *txt);
    
private:
    String _hostname;
    mdns_result_t * results;
    mdns_result_t * _getResult(int idx);
};

extern MDNSResponder MDNS;

#endif /* HAPBONJOUR_HPP_ */
