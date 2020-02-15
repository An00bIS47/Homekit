//
// HAPBonjour.cpp
// Homekit
//
//  Created on: 24.06.2017
//      Author: michael
//

#include "HAPBonjour.hpp"

#ifndef LWIP_OPEN_SRC
#define LWIP_OPEN_SRC
#endif

#include "WiFi.h"
#include <functional>
#include "esp_wifi.h"

static void _on_sys_event(system_event_t *event){
    mdns_handle_system_event(NULL, event);
}

MDNSResponder::MDNSResponder() :results(NULL) {}
MDNSResponder::~MDNSResponder() {
    end();
}

bool MDNSResponder::begin(const char* hostName){
    if(mdns_init()){
        log_e("Failed starting MDNS");
        return false;
    }
    WiFi.onEvent(_on_sys_event);
    _hostname = hostName;
    _hostname.toLowerCase();
    if(mdns_hostname_set(hostName)) {
        log_e("Failed setting MDNS hostname");
        return false;
    }
    return true;
}

void MDNSResponder::end() {
    mdns_free();
}

void MDNSResponder::setInstanceName(String name) {
    if (name.length() > 63) return;
    if(mdns_instance_name_set(name.c_str())){
        log_e("Failed setting MDNS instance");
        return;
    }
}


void MDNSResponder::enableArduino(uint16_t port, bool auth){
    mdns_txt_item_t arduTxtData[4] = {
        {(char*)"board"         ,(char*)ARDUINO_VARIANT},
        {(char*)"tcp_check"     ,(char*)"no"},
        {(char*)"ssh_upload"    ,(char*)"no"},
        {(char*)"auth_upload"   ,(char*)"no"}
    };
    
    if(mdns_service_add(NULL, "_arduino", "_tcp", port, arduTxtData, 4)) {
        log_e("Failed adding Arduino service");
    }
    
    if(auth && mdns_service_txt_item_set("_arduino", "_tcp", "auth_upload", "yes")){
        log_e("Failed setting Arduino txt item");
    }
}

void MDNSResponder::disableArduino(){
    if(mdns_service_remove("_arduino", "_tcp")) {
        log_w("Failed removing Arduino service");
    }
}

void MDNSResponder::enableWorkstation(wifi_interface_t interface){
    char winstance[21+_hostname.length()];
    uint8_t mac[6];
    esp_wifi_get_mac(interface, mac);
    sprintf(winstance, "%s [%02x:%02x:%02x:%02x:%02x:%02x]", _hostname.c_str(), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    if(mdns_service_add(NULL, "_workstation", "_tcp", 9, NULL, 0)) {
        log_e("Failed adding Workstation service");
    } else if(mdns_service_instance_name_set("_workstation", "_tcp", winstance)) {
        log_e("Failed setting Workstation service instance name");
    }
}

void MDNSResponder::disableWorkstation(){
    if(mdns_service_remove("_workstation", "_tcp")) {
        log_w("Failed removing Workstation service");
    }
}

void MDNSResponder::addService(char *name, char *proto, uint16_t port){
    char _name[strlen(name)+2];
    char _proto[strlen(proto)+2];
    if (name[0] == '_') {
        sprintf(_name, "%s", name);
    } else {
        sprintf(_name, "_%s", name);
    }
    if (proto[0] == '_') {
        sprintf(_proto, "%s", proto);
    } else {
        sprintf(_proto, "_%s", proto);
    }
    
    if(mdns_service_add(NULL, _name, _proto, port, NULL, 0)) {
        log_e("Failed adding service %s.%s.\n", name, proto);
    }
}

bool MDNSResponder::addServiceTxt(char *name, char *proto, char *key, char *value){
    char _name[strlen(name)+2];
    char _proto[strlen(proto)+2];
    if (name[0] == '_') {
        sprintf(_name, "%s", name);
    } else {
        sprintf(_name, "_%s", name);
    }
    if (proto[0] == '_') {
        sprintf(_proto, "%s", proto);
    } else {
        sprintf(_proto, "_%s", proto);
    }
    
    if(mdns_service_txt_item_set(_name, _proto, key, value)) {
        log_e("Failed setting service TXT");
        return false;
    }
    return true;
}

IPAddress MDNSResponder::queryHost(char *host, uint32_t timeout){
    struct ip4_addr addr;
    addr.addr = 0;
    
    esp_err_t err = mdns_query_a(host, timeout,  &addr);
    if(err){
        if(err == ESP_ERR_NOT_FOUND){
            log_w("Host was not found!");
            return IPAddress();
        }
        log_e("Query Failed");
        return IPAddress();
    }
    return IPAddress(addr.addr);
}


int MDNSResponder::queryService(char *service, char *proto) {
    if(!service || !service[0] || !proto || !proto[0]){
        log_e("Bad Parameters");
        return 0;
    }
    
    if(results){
        mdns_query_results_free(results);
        results = NULL;
    }
    
    char srv[strlen(service)+2];
    char prt[strlen(proto)+2];
    if (service[0] == '_') {
        sprintf(srv, "%s", service);
    } else {
        sprintf(srv, "_%s", service);
    }
    if (proto[0] == '_') {
        sprintf(prt, "%s", proto);
    } else {
        sprintf(prt, "_%s", proto);
    }
    
    esp_err_t err = mdns_query_ptr(srv, prt, 3000, 20,  &results);
    if(err){
        log_e("Query Failed");
        return 0;
    }
    if(!results){
        log_w("No results found!");
        return 0;
    }
    
    mdns_result_t * r = results;
    int i = 0;
    while(r){
        i++;
        r = r->next;
    }
    return i;
}

mdns_result_t * MDNSResponder::_getResult(int idx){
    mdns_result_t * result = results;
    int i = 0;
    while(result){
        if(i == idx){
            break;
        }
        i++;
        result = result->next;
    }
    return result;
}

String MDNSResponder::hostname(int idx) {
    mdns_result_t * result = _getResult(idx);
    if(!result){
        log_e("Result %d not found", idx);
        return String();
    }
    return String(result->hostname);
}

IPAddress MDNSResponder::IP(int idx) {
    mdns_result_t * result = _getResult(idx);
    if(!result){
        log_e("Result %d not found", idx);
        return IPAddress();
    }
    mdns_ip_addr_t * addr = result->addr;
    while(addr){
        if(addr->addr.type == MDNS_IP_PROTOCOL_V4){
            return IPAddress(addr->addr.u_addr.ip4.addr);
        }
        addr = addr->next;
    }
    return IPAddress();
}

IPv6Address MDNSResponder::IPv6(int idx) {
    mdns_result_t * result = _getResult(idx);
    if(!result){
        log_e("Result %d not found", idx);
        return IPv6Address();
    }
    mdns_ip_addr_t * addr = result->addr;
    while(addr){
        if(addr->addr.type == MDNS_IP_PROTOCOL_V6){
            return IPv6Address(addr->addr.u_addr.ip6.addr);
        }
        addr = addr->next;
    }
    return IPv6Address();
}

uint16_t MDNSResponder::port(int idx) {
    mdns_result_t * result = _getResult(idx);
    if(!result){
        log_e("Result %d not found", idx);
        return 0;
    }
    return result->port;
}

bool MDNSResponder::addServiceTxtSet(char *name, const char * proto, uint8_t num_items, mdns_txt_item_t *txt){

    /*
    mdns_txt_item_t arduTxtData[4] = {
        {(char*)"board"         ,(char*)ARDUINO_VARIANT},
        {(char*)"tcp_check"     ,(char*)"no"},
        {(char*)"ssh_upload"    ,(char*)"no"},
        {(char*)"auth_upload"   ,(char*)"no"}
    };

    if(mdns_service_add(NULL, "_arduino", "_tcp", port, arduTxtData, 4)) {
        log_e("Failed adding Arduino service");
    }

    if(auth && mdns_service_txt_item_set("_arduino", "_tcp", "auth_upload", "yes")){
        log_e("Failed setting Arduino txt item");
    }
    */
    
    if(mdns_service_txt_set(name, proto, txt, num_items)) {
        log_e("Failed setting service TXT set");
        return false;
    }
    return true;
}

bool MDNSResponder::removeServiceTxt(char *name, const char * proto){
    if(mdns_service_remove(name, proto)) {
        log_w("Failed removing TXT service");
        return false;
    }
    return true;
}



MDNSResponder MDNS;
