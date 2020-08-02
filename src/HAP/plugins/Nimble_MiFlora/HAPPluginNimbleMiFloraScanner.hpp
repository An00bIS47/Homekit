//
// HAPPluginNimbleMiFloraScanner.hpp
// Homekit
//
//  Created on: 12.07.2019
//      Author: michael
//
// used code from https://github.com/sidddy/flora

#ifndef HAPPLUGINNIMBLEMIFLORADSCANNER_HPP_
#define HAPPLUGINNIMBLEMIFLORADSCANNER_HPP_

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "HAPGlobals.hpp"


// Root service for Flora Devices
static BLEUUID rootServiceDataUUID((uint16_t) 0xfe95);



class HAPPluginNimbleMiFloraScanner {

public:
    // Scan BLE and return true if flora devices are found
    bool scan();
    int scanDuration;

    int getDeviceCount() const {
      return _deviceCount;
    }

    std::string getDeviceAddress(int i) const {
      if (i < _deviceCount)
        return _devices[i];
      else
        return std::string();
    }

    void stop(){
        if (_scan)
            _scan->stop();
    }

    void clearDevices(){
        _deviceCount = 0;
    }

    bool isScanning(){
        return _isScanning;
    }

private:
    std::string _devices[HAP_PLUGIN_MIFLORA_MAX_DEVICES];
    int         _deviceCount = 0;
    BLEScan*    _scan = nullptr;
    bool        _isScanning = false;

    void registerDevice(BLEAdvertisedDevice& advertisedDevice) {


        std::string deviceAddress(advertisedDevice.getAddress().toString());
        Serial.print("Flora device found at address ");
        Serial.println(deviceAddress.c_str());
        
        if (_deviceCount < HAP_PLUGIN_MIFLORA_MAX_DEVICES) {
            _devices[_deviceCount++] = deviceAddress;
        } else {
            Serial.println("can't register device, no remaining slot");
            _scan->stop();
            _isScanning = false;
        }

    }

};

inline bool HAPPluginNimbleMiFloraScanner::scan() {
    Serial.println("Scan BLE, looking for Flora Devices");
    clearDevices();

    // detect and register Flora devices during BLE scan
    class FloraDevicesBLEDetector: public BLEAdvertisedDeviceCallbacks {
    public:
        FloraDevicesBLEDetector(HAPPluginNimbleMiFloraScanner &floraScanner) : _floraScanner(floraScanner) {}
      
        void onResult(BLEAdvertisedDevice* advertisedDevice)
        {
            if (advertisedDevice->haveServiceUUID()) {
                BLEUUID service = advertisedDevice->getServiceUUID();
                if (service.equals(rootServiceDataUUID)) {
                    _floraScanner.registerDevice(*advertisedDevice);
                }                    
            }
        }
      
    private:
        HAPPluginNimbleMiFloraScanner& _floraScanner;
    };

    _scan = BLEDevice::getScan();
    FloraDevicesBLEDetector floraDetector(*this);
    _scan->setAdvertisedDeviceCallbacks(&floraDetector);
    _isScanning = true;
    _scan->start(HAP_PLUGIN_MIFLORA_SCAN_DURATION);
    
    Serial.print("Number of Flora devices detected: ");
    Serial.println(_deviceCount);

    _isScanning = false;
    return (_deviceCount > 0);
}

#endif