//
// HAPServices.hpp
// Homekit
//
//  Generated on: 23.09.2019
//

#ifndef HAPSERVICES_HPP_
#define HAPSERVICES_HPP_

#include <Arduino.h>

typedef enum {
    HAP_SERVICE_ACCESSORY_INFORMATION                           = 0x3E,     //    
    HAP_SERVICE_AIR_PURIFIER                                    = 0xBB,     //    
    HAP_SERVICE_AIR_QUALITY_SENSOR                              = 0x8D,     //    
    HAP_SERVICE_BATTERY_SERVICE                                 = 0x96,     //    
    HAP_SERVICE_CAMERA_RTP_STREAM_MANAGEMENT                    = 0x110,    //    
    HAP_SERVICE_CARBON_DIOXIDE_SENSOR                           = 0x97,     //    
    HAP_SERVICE_CARBON_MONOXIDE_SENSOR                          = 0x7F,     //    
    HAP_SERVICE_CONTACT_SENSOR                                  = 0x80,     //    
    HAP_SERVICE_DOOR                                            = 0x81,     //    
    HAP_SERVICE_DOORBELL                                        = 0x121,    //    
    HAP_SERVICE_FAN                                             = 0x40,     //    
    HAP_SERVICE_FANV2                                           = 0xB7,     //    
    HAP_SERVICE_FAUCET                                          = 0xD7,     //    
    HAP_SERVICE_FILTER_MAINTENANCE                              = 0xBA,     //    
    HAP_SERVICE_GARAGE_DOOR_OPENER                              = 0x41,     //    
    HAP_SERVICE_HEATER_COOLER                                   = 0xBC,     //    
    HAP_SERVICE_HUMIDIFIER_DEHUMIDIFIER                         = 0xBD,     //    
    HAP_SERVICE_HUMIDITY_SENSOR                                 = 0x82,     //    
    HAP_SERVICE_IRRIGATION_SYSTEM                               = 0xCF,     //    
    HAP_SERVICE_LEAK_SENSOR                                     = 0x83,     //    
    HAP_SERVICE_LIGHTBULB                                       = 0x43,     //    
    HAP_SERVICE_LIGHT_SENSOR                                    = 0x84,     //    
    HAP_SERVICE_LOCK_MANAGEMENT                                 = 0x44,     //    
    HAP_SERVICE_LOCK_MECHANISM                                  = 0x45,     //    
    HAP_SERVICE_MICROPHONE                                      = 0x112,    //    
    HAP_SERVICE_MOTION_SENSOR                                   = 0x85,     //    
    HAP_SERVICE_OCCUPANCY_SENSOR                                = 0x86,     //    
    HAP_SERVICE_OUTLET                                          = 0x47,     //    
    HAP_SERVICE_SECURITY_SYSTEM                                 = 0x7E,     //    
    HAP_SERVICE_SERVICE_LABEL                                   = 0xCC,     //    
    HAP_SERVICE_SLAT                                            = 0xB9,     //    
    HAP_SERVICE_SMOKE_SENSOR                                    = 0x87,     //    
    HAP_SERVICE_SPEAKER                                         = 0x113,    //    
    HAP_SERVICE_STATELESS_PROGRAMMABLE_SWITCH                   = 0x89,     //    
    HAP_SERVICE_SWITCH                                          = 0x49,     //    
    HAP_SERVICE_TEMPERATURE_SENSOR                              = 0x8A,     //    
    HAP_SERVICE_THERMOSTAT                                      = 0x4A,     //    
    HAP_SERVICE_VALVE                                           = 0xD0,     //    
    HAP_SERVICE_WINDOW                                          = 0x8B,     //    
    HAP_SERVICE_WINDOW_COVERING                                 = 0x8C,     //    
} HAP_SERVICE;


inline String serviceName(int type){
    switch(type) {
        case HAP_SERVICE_ACCESSORY_INFORMATION:                           // 0x3E     ==    62
            return "AccessoryInformation";
        case HAP_SERVICE_AIR_PURIFIER:                                    // 0xBB     ==   187
            return "AirPurifier";
        case HAP_SERVICE_AIR_QUALITY_SENSOR:                              // 0x8D     ==   141
            return "AirQualitySensor";
        case HAP_SERVICE_BATTERY_SERVICE:                                 // 0x96     ==   150
            return "BatteryService";
        case HAP_SERVICE_CAMERA_RTP_STREAM_MANAGEMENT:                    // 0x110    ==   272
            return "CameraRTPStreamManagement";
        case HAP_SERVICE_CARBON_DIOXIDE_SENSOR:                           // 0x97     ==   151
            return "CarbonDioxideSensor";
        case HAP_SERVICE_CARBON_MONOXIDE_SENSOR:                          // 0x7F     ==   127
            return "CarbonMonoxideSensor";
        case HAP_SERVICE_CONTACT_SENSOR:                                  // 0x80     ==   128
            return "ContactSensor";
        case HAP_SERVICE_DOOR:                                            // 0x81     ==   129
            return "Door";
        case HAP_SERVICE_DOORBELL:                                        // 0x121    ==   289
            return "Doorbell";
        case HAP_SERVICE_FAN:                                             // 0x40     ==    64
            return "Fan";
        case HAP_SERVICE_FANV2:                                           // 0xB7     ==   183
            return "Fanv2";
        case HAP_SERVICE_FAUCET:                                          // 0xD7     ==   215
            return "Faucet";
        case HAP_SERVICE_FILTER_MAINTENANCE:                              // 0xBA     ==   186
            return "FilterMaintenance";
        case HAP_SERVICE_GARAGE_DOOR_OPENER:                              // 0x41     ==    65
            return "GarageDoorOpener";
        case HAP_SERVICE_HEATER_COOLER:                                   // 0xBC     ==   188
            return "HeaterCooler";
        case HAP_SERVICE_HUMIDIFIER_DEHUMIDIFIER:                         // 0xBD     ==   189
            return "HumidifierDehumidifier";
        case HAP_SERVICE_HUMIDITY_SENSOR:                                 // 0x82     ==   130
            return "HumiditySensor";
        case HAP_SERVICE_IRRIGATION_SYSTEM:                               // 0xCF     ==   207
            return "IrrigationSystem";
        case HAP_SERVICE_LEAK_SENSOR:                                     // 0x83     ==   131
            return "LeakSensor";
        case HAP_SERVICE_LIGHTBULB:                                       // 0x43     ==    67
            return "Lightbulb";
        case HAP_SERVICE_LIGHT_SENSOR:                                    // 0x84     ==   132
            return "LightSensor";
        case HAP_SERVICE_LOCK_MANAGEMENT:                                 // 0x44     ==    68
            return "LockManagement";
        case HAP_SERVICE_LOCK_MECHANISM:                                  // 0x45     ==    69
            return "LockMechanism";
        case HAP_SERVICE_MICROPHONE:                                      // 0x112    ==   274
            return "Microphone";
        case HAP_SERVICE_MOTION_SENSOR:                                   // 0x85     ==   133
            return "MotionSensor";
        case HAP_SERVICE_OCCUPANCY_SENSOR:                                // 0x86     ==   134
            return "OccupancySensor";
        case HAP_SERVICE_OUTLET:                                          // 0x47     ==    71
            return "Outlet";
        case HAP_SERVICE_SECURITY_SYSTEM:                                 // 0x7E     ==   126
            return "SecuritySystem";
        case HAP_SERVICE_SERVICE_LABEL:                                   // 0xCC     ==   204
            return "ServiceLabel";
        case HAP_SERVICE_SLAT:                                            // 0xB9     ==   185
            return "Slat";
        case HAP_SERVICE_SMOKE_SENSOR:                                    // 0x87     ==   135
            return "SmokeSensor";
        case HAP_SERVICE_SPEAKER:                                         // 0x113    ==   275
            return "Speaker";
        case HAP_SERVICE_STATELESS_PROGRAMMABLE_SWITCH:                   // 0x89     ==   137
            return "StatelessProgrammableSwitch";
        case HAP_SERVICE_SWITCH:                                          // 0x49     ==    73
            return "Switch";
        case HAP_SERVICE_TEMPERATURE_SENSOR:                              // 0x8A     ==   138
            return "TemperatureSensor";
        case HAP_SERVICE_THERMOSTAT:                                      // 0x4A     ==    74
            return "Thermostat";
        case HAP_SERVICE_VALVE:                                           // 0xD0     ==   208
            return "Valve";
        case HAP_SERVICE_WINDOW:                                          // 0x8B     ==   139
            return "Window";
        case HAP_SERVICE_WINDOW_COVERING:                                 // 0x8C     ==   140
            return "WindowCovering";
        default:
            return "";
    }
}

#endif /* HAPSERVICES_HPP_ */
