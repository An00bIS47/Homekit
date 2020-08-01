//
// HAPCharacteristics.hpp
// Homekit
//
//  Generated on: 23.09.2019
//

#ifndef HAPCHARACTERISTICS_HPP_
#define HAPCHARACTERISTICS_HPP_

#include <Arduino.h>

typedef enum {
    HAP_CHARACTERISTIC_ACCESSORY_FLAGS                          = 0xA6,     //    uint32      pr|ev       
    HAP_CHARACTERISTIC_ACTIVE                                   = 0xB0,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_ADMINISTRATOR_ONLY_ACCESS                = 0x1,      //    bool        pr|pw|ev    
    HAP_CHARACTERISTIC_AIR_PARTICULATE_DENSITY                  = 0x64,     //    float       pr|ev       
    HAP_CHARACTERISTIC_AIR_PARTICULATE_SIZE                     = 0x65,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_AIR_QUALITY                              = 0x95,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_AUDIO_FEEDBACK                           = 0x5,      //    bool        pr|pw|ev    
    HAP_CHARACTERISTIC_BATTERY_LEVEL                            = 0x68,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_BRIGHTNESS                               = 0x8,      //    int         pr|pw|ev    
    HAP_CHARACTERISTIC_CARBON_DIOXIDE_DETECTED                  = 0x92,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CARBON_DIOXIDE_LEVEL                     = 0x93,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CARBON_DIOXIDE_PEAK_LEVEL                = 0x94,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CARBON_MONOXIDE_DETECTED                 = 0x69,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CARBON_MONOXIDE_LEVEL                    = 0x90,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CARBON_MONOXIDE_PEAK_LEVEL               = 0x91,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CHARGING_STATE                           = 0x8F,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_COLOR_TEMPERATURE                        = 0xCE,     //    uint32      pr|pw|ev    
    HAP_CHARACTERISTIC_CONTACT_SENSOR_STATE                     = 0x6A,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_COOLING_THRESHOLD_TEMPERATURE            = 0xD,      //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_CURRENT_AIR_PURIFIER_STATE               = 0xA9,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_AMBIENT_LIGHT_LEVEL              = 0x6B,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_DOOR_STATE                       = 0xE,      //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_FAN_STATE                        = 0xAF,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_HEATER_COOLER_STATE              = 0xB1,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_HEATING_COOLING_STATE            = 0xF,      //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_HORIZONTAL_TILT_ANGLE            = 0x6C,     //    int         pr|ev       
    HAP_CHARACTERISTIC_CURRENT_HUMIDIFIER_DEHUMIDIFIER_STATE    = 0xB3,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_POSITION                         = 0x6D,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY                = 0x10,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_SLAT_STATE                       = 0xAA,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_TEMPERATURE                      = 0x11,     //    float       pr|ev       
    HAP_CHARACTERISTIC_CURRENT_TILT_ANGLE                       = 0xC1,     //    int         pr|ev       
    HAP_CHARACTERISTIC_CURRENT_VERTICAL_TILT_ANGLE              = 0x6E,     //    int         pr|ev       
    HAP_CHARACTERISTIC_DIGITAL_ZOOM                             = 0x11D,    //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_FILTER_CHANGE_INDICATION                 = 0xAC,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_FILTER_LIFE_LEVEL                        = 0xAB,     //    float       pr|ev       
    HAP_CHARACTERISTIC_FIRMWARE_REVISION                        = 0x52,     //    string      pr          
    HAP_CHARACTERISTIC_HARDWARE_REVISION                        = 0x53,     //    string      pr          
    HAP_CHARACTERISTIC_HEATING_THRESHOLD_TEMPERATURE            = 0x12,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_HOLD_POSITION                            = 0x6F,     //    bool        pw          
    HAP_CHARACTERISTIC_HUE                                      = 0x13,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_IDENTIFY                                 = 0x14,     //    bool        pw          
    HAP_CHARACTERISTIC_IMAGE_MIRRORING                          = 0x11F,    //    bool        pr|pw|ev    
    HAP_CHARACTERISTIC_IMAGE_ROTATION                           = 0x11E,    //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_IN_USE                                   = 0xD2,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_IS_CONFIGURED                            = 0xD6,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_LEAK_DETECTED                            = 0x70,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_LOCK_CONTROL_POINT                       = 0x19,     //    tlv8        pw          
    HAP_CHARACTERISTIC_LOCK_CURRENT_STATE                       = 0x1D,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_LOCK_LAST_KNOWN_ACTION                   = 0x1C,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_LOCK_MANAGEMENT_AUTO_SECURITY_TIMEOUT    = 0x1A,     //    uint32      pr|pw|ev    
    HAP_CHARACTERISTIC_LOCK_PHYSICAL_CONTROLS                   = 0xA7,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_LOCK_TARGET_STATE                        = 0x1E,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_LOGS                                     = 0x1F,     //    tlv8        pr|ev       
    HAP_CHARACTERISTIC_MANUFACTURER                             = 0x20,     //    string      pr          
    HAP_CHARACTERISTIC_MODEL                                    = 0x21,     //    string      pr          
    HAP_CHARACTERISTIC_MOTION_DETECTED                          = 0x22,     //    bool        pr|ev       
    HAP_CHARACTERISTIC_MUTE                                     = 0x11A,    //    bool        pr|pw|ev    
    HAP_CHARACTERISTIC_NAME                                     = 0x23,     //    string      pr          
    HAP_CHARACTERISTIC_NIGHT_VISION                             = 0x11B,    //    bool        pr|pw|ev    
    HAP_CHARACTERISTIC_NITROGEN_DIOXIDE_DENSITY                 = 0xC4,     //    float       pr|ev       
    HAP_CHARACTERISTIC_OBSTRUCTION_DETECTED                     = 0x24,     //    bool        pr|ev       
    HAP_CHARACTERISTIC_OCCUPANCY_DETECTED                       = 0x71,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_ON                                       = 0x25,     //    bool        pr|pw|ev    
    HAP_CHARACTERISTIC_OPTICAL_ZOOM                             = 0x11C,    //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_OUTLET_IN_USE                            = 0x26,     //    bool        pr|ev       
    HAP_CHARACTERISTIC_OZONE_DENSITY                            = 0xC3,     //    float       pr|ev       
    HAP_CHARACTERISTIC_PAIRING_FEATURES                         = 0x4F,     //    uint8       pr          
    HAP_CHARACTERISTIC_PAIRING_PAIRINGS                         = 0x50,     //    tlv8        pr|pw       
    HAP_CHARACTERISTIC_PAIR_SETUP                               = 0x4C,     //    tlv8        pr|pw       
    HAP_CHARACTERISTIC_PAIR_VERIFY                              = 0x4E,     //    tlv8        pr|pw       
    HAP_CHARACTERISTIC_PM10_DENSITY                             = 0xC7,     //    float       pr|ev       
    HAP_CHARACTERISTIC_PM2_5_DENSITY                            = 0xC6,     //    float       pr|ev       
    HAP_CHARACTERISTIC_POSITION_STATE                           = 0x72,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_PROGRAMMABLE_SWITCH_EVENT                = 0x73,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_PROGRAM_MODE                             = 0xD1,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_RELATIVE_HUMIDITY_DEHUMIDIFIER_THRESHOLD = 0xC9,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_RELATIVE_HUMIDITY_HUMIDIFIER_THRESHOLD   = 0xCA,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_REMAINING_DURATION                       = 0xD4,     //    uint32      pr|ev       
    HAP_CHARACTERISTIC_RESET_FILTER_INDICATION                  = 0xAD,     //    uint8       pw          
    HAP_CHARACTERISTIC_ROTATION_DIRECTION                       = 0x28,     //    int         pr|pw|ev    
    HAP_CHARACTERISTIC_ROTATION_SPEED                           = 0x29,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_SATURATION                               = 0x2F,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_SECURITY_SYSTEM_ALARM_TYPE               = 0x8E,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_SECURITY_SYSTEM_CURRENT_STATE            = 0x66,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_SECURITY_SYSTEM_TARGET_STATE             = 0x67,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_SELECTED_RTP_STREAM_CONFIGURATION        = 0x117,    //    tlv8        pr|pw       
    HAP_CHARACTERISTIC_SERIAL_NUMBER                            = 0x30,     //    string      pr          
    HAP_CHARACTERISTIC_SERVICE_LABEL_INDEX                      = 0xCB,     //    uint8       pr          
    HAP_CHARACTERISTIC_SERVICE_LABEL_NAMESPACE                  = 0xCD,     //    uint8       pr          
    HAP_CHARACTERISTIC_SETUP_ENDPOINTS                          = 0x118,    //    tlv8        pr|pw       
    HAP_CHARACTERISTIC_SET_DURATION                             = 0xD3,     //    uint32      pr|pw|ev    
    HAP_CHARACTERISTIC_SLAT_TYPE                                = 0xC0,     //    uint8       pr          
    HAP_CHARACTERISTIC_SMOKE_DETECTED                           = 0x76,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_STATUS_ACTIVE                            = 0x75,     //    bool        pr|ev       
    HAP_CHARACTERISTIC_STATUS_FAULT                             = 0x77,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_STATUS_JAMMED                            = 0x78,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_STATUS_LOW_BATTERY                       = 0x79,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_STATUS_TAMPERED                          = 0x7A,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_STREAMING_STATUS                         = 0x120,    //    tlv8        pr|ev       
    HAP_CHARACTERISTIC_SULPHUR_DIOXIDE_DENSITY                  = 0xC5,     //    float       pr|ev       
    HAP_CHARACTERISTIC_SUPPORTED_AUDIO_STREAM_CONFIGURATION     = 0x115,    //    tlv8        pr          
    HAP_CHARACTERISTIC_SUPPORTED_RTP_CONFIGURATION              = 0x116,    //    tlv8        pr          
    HAP_CHARACTERISTIC_SUPPORTED_VIDEO_STREAM_CONFIGURATION     = 0x114,    //    tlv8        pr          
    HAP_CHARACTERISTIC_SWING_MODE                               = 0xB6,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_AIR_PURIFIER_STATE                = 0xA8,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_AIR_QUALITY                       = 0xAE,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_DOOR_STATE                        = 0x32,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_FAN_STATE                         = 0xBF,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_HEATER_COOLER_STATE               = 0xB2,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_HEATING_COOLING_STATE             = 0x33,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_HORIZONTAL_TILT_ANGLE             = 0x7B,     //    int         pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_HUMIDIFIER_DEHUMIDIFIER_STATE     = 0xB4,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_POSITION                          = 0x7C,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_RELATIVE_HUMIDITY                 = 0x34,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_SLAT_STATE                        = 0xBE,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_TEMPERATURE                       = 0x35,     //    float       pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_TILT_ANGLE                        = 0xC2,     //    int         pr|pw|ev    
    HAP_CHARACTERISTIC_TARGET_VERTICAL_TILT_ANGLE               = 0x7D,     //    int         pr|pw|ev    
    HAP_CHARACTERISTIC_TEMPERATURE_DISPLAY_UNITS                = 0x36,     //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_VALVE_TYPE                               = 0xD5,     //    uint8       pr|ev       
    HAP_CHARACTERISTIC_VERSION                                  = 0x37,     //    string      pr|ev       
    HAP_CHARACTERISTIC_VOC_DENSITY                              = 0xC8,     //    float       pr|ev       
    HAP_CHARACTERISTIC_VOLUME                                   = 0x119,    //    uint8       pr|pw|ev    
    HAP_CHARACTERISTIC_WATER_LEVEL                              = 0xB5,     //    float       pr|ev       
} HAP_CHARACTERISTIC;


inline const char* characteristicsName(int type){
    switch(type) {
        case HAP_CHARACTERISTIC_ACCESSORY_FLAGS:                          // 0xA6     ==   166
            return "AccessoryFlags";
        case HAP_CHARACTERISTIC_ACTIVE:                                   // 0xB0     ==   176
            return "Active";
        case HAP_CHARACTERISTIC_ADMINISTRATOR_ONLY_ACCESS:                // 0x1      ==     1
            return "AdministratorOnlyAccess";
        case HAP_CHARACTERISTIC_AIR_PARTICULATE_DENSITY:                  // 0x64     ==   100
            return "AirParticulateDensity";
        case HAP_CHARACTERISTIC_AIR_PARTICULATE_SIZE:                     // 0x65     ==   101
            return "AirParticulateSize";
        case HAP_CHARACTERISTIC_AIR_QUALITY:                              // 0x95     ==   149
            return "AirQuality";
        case HAP_CHARACTERISTIC_AUDIO_FEEDBACK:                           // 0x5      ==     5
            return "AudioFeedback";
        case HAP_CHARACTERISTIC_BATTERY_LEVEL:                            // 0x68     ==   104
            return "BatteryLevel";
        case HAP_CHARACTERISTIC_BRIGHTNESS:                               // 0x8      ==     8
            return "Brightness";
        case HAP_CHARACTERISTIC_CARBON_DIOXIDE_DETECTED:                  // 0x92     ==   146
            return "CarbonDioxideDetected";
        case HAP_CHARACTERISTIC_CARBON_DIOXIDE_LEVEL:                     // 0x93     ==   147
            return "CarbonDioxideLevel";
        case HAP_CHARACTERISTIC_CARBON_DIOXIDE_PEAK_LEVEL:                // 0x94     ==   148
            return "CarbonDioxidePeakLevel";
        case HAP_CHARACTERISTIC_CARBON_MONOXIDE_DETECTED:                 // 0x69     ==   105
            return "CarbonMonoxideDetected";
        case HAP_CHARACTERISTIC_CARBON_MONOXIDE_LEVEL:                    // 0x90     ==   144
            return "CarbonMonoxideLevel";
        case HAP_CHARACTERISTIC_CARBON_MONOXIDE_PEAK_LEVEL:               // 0x91     ==   145
            return "CarbonMonoxidePeakLevel";
        case HAP_CHARACTERISTIC_CHARGING_STATE:                           // 0x8F     ==   143
            return "ChargingState";
        case HAP_CHARACTERISTIC_COLOR_TEMPERATURE:                        // 0xCE     ==   206
            return "ColorTemperature";
        case HAP_CHARACTERISTIC_CONTACT_SENSOR_STATE:                     // 0x6A     ==   106
            return "ContactSensorState";
        case HAP_CHARACTERISTIC_COOLING_THRESHOLD_TEMPERATURE:            // 0xD      ==    13
            return "CoolingThresholdTemperature";
        case HAP_CHARACTERISTIC_CURRENT_AIR_PURIFIER_STATE:               // 0xA9     ==   169
            return "CurrentAirPurifierState";
        case HAP_CHARACTERISTIC_CURRENT_AMBIENT_LIGHT_LEVEL:              // 0x6B     ==   107
            return "CurrentAmbientLightLevel";
        case HAP_CHARACTERISTIC_CURRENT_DOOR_STATE:                       // 0xE      ==    14
            return "CurrentDoorState";
        case HAP_CHARACTERISTIC_CURRENT_FAN_STATE:                        // 0xAF     ==   175
            return "CurrentFanState";
        case HAP_CHARACTERISTIC_CURRENT_HEATER_COOLER_STATE:              // 0xB1     ==   177
            return "CurrentHeaterCoolerState";
        case HAP_CHARACTERISTIC_CURRENT_HEATING_COOLING_STATE:            // 0xF      ==    15
            return "CurrentHeatingCoolingState";
        case HAP_CHARACTERISTIC_CURRENT_HORIZONTAL_TILT_ANGLE:            // 0x6C     ==   108
            return "CurrentHorizontalTiltAngle";
        case HAP_CHARACTERISTIC_CURRENT_HUMIDIFIER_DEHUMIDIFIER_STATE:    // 0xB3     ==   179
            return "CurrentHumidifierDehumidifierState";
        case HAP_CHARACTERISTIC_CURRENT_POSITION:                         // 0x6D     ==   109
            return "CurrentPosition";
        case HAP_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY:                // 0x10     ==    16
            return "CurrentRelativeHumidity";
        case HAP_CHARACTERISTIC_CURRENT_SLAT_STATE:                       // 0xAA     ==   170
            return "CurrentSlatState";
        case HAP_CHARACTERISTIC_CURRENT_TEMPERATURE:                      // 0x11     ==    17
            return "CurrentTemperature";
        case HAP_CHARACTERISTIC_CURRENT_TILT_ANGLE:                       // 0xC1     ==   193
            return "CurrentTiltAngle";
        case HAP_CHARACTERISTIC_CURRENT_VERTICAL_TILT_ANGLE:              // 0x6E     ==   110
            return "CurrentVerticalTiltAngle";
        case HAP_CHARACTERISTIC_DIGITAL_ZOOM:                             // 0x11D    ==   285
            return "DigitalZoom";
        case HAP_CHARACTERISTIC_FILTER_CHANGE_INDICATION:                 // 0xAC     ==   172
            return "FilterChangeIndication";
        case HAP_CHARACTERISTIC_FILTER_LIFE_LEVEL:                        // 0xAB     ==   171
            return "FilterLifeLevel";
        case HAP_CHARACTERISTIC_FIRMWARE_REVISION:                        // 0x52     ==    82
            return "FirmwareRevision";
        case HAP_CHARACTERISTIC_HARDWARE_REVISION:                        // 0x53     ==    83
            return "HardwareRevision";
        case HAP_CHARACTERISTIC_HEATING_THRESHOLD_TEMPERATURE:            // 0x12     ==    18
            return "HeatingThresholdTemperature";
        case HAP_CHARACTERISTIC_HOLD_POSITION:                            // 0x6F     ==   111
            return "HoldPosition";
        case HAP_CHARACTERISTIC_HUE:                                      // 0x13     ==    19
            return "Hue";
        case HAP_CHARACTERISTIC_IDENTIFY:                                 // 0x14     ==    20
            return "Identify";
        case HAP_CHARACTERISTIC_IMAGE_MIRRORING:                          // 0x11F    ==   287
            return "ImageMirroring";
        case HAP_CHARACTERISTIC_IMAGE_ROTATION:                           // 0x11E    ==   286
            return "ImageRotation";
        case HAP_CHARACTERISTIC_IN_USE:                                   // 0xD2     ==   210
            return "InUse";
        case HAP_CHARACTERISTIC_IS_CONFIGURED:                            // 0xD6     ==   214
            return "IsConfigured";
        case HAP_CHARACTERISTIC_LEAK_DETECTED:                            // 0x70     ==   112
            return "LeakDetected";
        case HAP_CHARACTERISTIC_LOCK_CONTROL_POINT:                       // 0x19     ==    25
            return "LockControlPoint";
        case HAP_CHARACTERISTIC_LOCK_CURRENT_STATE:                       // 0x1D     ==    29
            return "LockCurrentState";
        case HAP_CHARACTERISTIC_LOCK_LAST_KNOWN_ACTION:                   // 0x1C     ==    28
            return "LockLastKnownAction";
        case HAP_CHARACTERISTIC_LOCK_MANAGEMENT_AUTO_SECURITY_TIMEOUT:    // 0x1A     ==    26
            return "LockManagementAutoSecurityTimeout";
        case HAP_CHARACTERISTIC_LOCK_PHYSICAL_CONTROLS:                   // 0xA7     ==   167
            return "LockPhysicalControls";
        case HAP_CHARACTERISTIC_LOCK_TARGET_STATE:                        // 0x1E     ==    30
            return "LockTargetState";
        case HAP_CHARACTERISTIC_LOGS:                                     // 0x1F     ==    31
            return "Logs";
        case HAP_CHARACTERISTIC_MANUFACTURER:                             // 0x20     ==    32
            return "Manufacturer";
        case HAP_CHARACTERISTIC_MODEL:                                    // 0x21     ==    33
            return "Model";
        case HAP_CHARACTERISTIC_MOTION_DETECTED:                          // 0x22     ==    34
            return "MotionDetected";
        case HAP_CHARACTERISTIC_MUTE:                                     // 0x11A    ==   282
            return "Mute";
        case HAP_CHARACTERISTIC_NAME:                                     // 0x23     ==    35
            return "Name";
        case HAP_CHARACTERISTIC_NIGHT_VISION:                             // 0x11B    ==   283
            return "NightVision";
        case HAP_CHARACTERISTIC_NITROGEN_DIOXIDE_DENSITY:                 // 0xC4     ==   196
            return "NitrogenDioxideDensity";
        case HAP_CHARACTERISTIC_OBSTRUCTION_DETECTED:                     // 0x24     ==    36
            return "ObstructionDetected";
        case HAP_CHARACTERISTIC_OCCUPANCY_DETECTED:                       // 0x71     ==   113
            return "OccupancyDetected";
        case HAP_CHARACTERISTIC_ON:                                       // 0x25     ==    37
            return "On";
        case HAP_CHARACTERISTIC_OPTICAL_ZOOM:                             // 0x11C    ==   284
            return "OpticalZoom";
        case HAP_CHARACTERISTIC_OUTLET_IN_USE:                            // 0x26     ==    38
            return "OutletInUse";
        case HAP_CHARACTERISTIC_OZONE_DENSITY:                            // 0xC3     ==   195
            return "OzoneDensity";
        case HAP_CHARACTERISTIC_PAIRING_FEATURES:                         // 0x4F     ==    79
            return "PairingFeatures";
        case HAP_CHARACTERISTIC_PAIRING_PAIRINGS:                         // 0x50     ==    80
            return "PairingPairings";
        case HAP_CHARACTERISTIC_PAIR_SETUP:                               // 0x4C     ==    76
            return "PairSetup";
        case HAP_CHARACTERISTIC_PAIR_VERIFY:                              // 0x4E     ==    78
            return "PairVerify";
        case HAP_CHARACTERISTIC_PM10_DENSITY:                             // 0xC7     ==   199
            return "PM10Density";
        case HAP_CHARACTERISTIC_PM2_5_DENSITY:                            // 0xC6     ==   198
            return "PM2.5Density";
        case HAP_CHARACTERISTIC_POSITION_STATE:                           // 0x72     ==   114
            return "PositionState";
        case HAP_CHARACTERISTIC_PROGRAMMABLE_SWITCH_EVENT:                // 0x73     ==   115
            return "ProgrammableSwitchEvent";
        case HAP_CHARACTERISTIC_PROGRAM_MODE:                             // 0xD1     ==   209
            return "ProgramMode";
        case HAP_CHARACTERISTIC_RELATIVE_HUMIDITY_DEHUMIDIFIER_THRESHOLD: // 0xC9     ==   201
            return "RelativeHumidityDehumidifierThreshold";
        case HAP_CHARACTERISTIC_RELATIVE_HUMIDITY_HUMIDIFIER_THRESHOLD:   // 0xCA     ==   202
            return "RelativeHumidityHumidifierThreshold";
        case HAP_CHARACTERISTIC_REMAINING_DURATION:                       // 0xD4     ==   212
            return "RemainingDuration";
        case HAP_CHARACTERISTIC_RESET_FILTER_INDICATION:                  // 0xAD     ==   173
            return "ResetFilterIndication";
        case HAP_CHARACTERISTIC_ROTATION_DIRECTION:                       // 0x28     ==    40
            return "RotationDirection";
        case HAP_CHARACTERISTIC_ROTATION_SPEED:                           // 0x29     ==    41
            return "RotationSpeed";
        case HAP_CHARACTERISTIC_SATURATION:                               // 0x2F     ==    47
            return "Saturation";
        case HAP_CHARACTERISTIC_SECURITY_SYSTEM_ALARM_TYPE:               // 0x8E     ==   142
            return "SecuritySystemAlarmType";
        case HAP_CHARACTERISTIC_SECURITY_SYSTEM_CURRENT_STATE:            // 0x66     ==   102
            return "SecuritySystemCurrentState";
        case HAP_CHARACTERISTIC_SECURITY_SYSTEM_TARGET_STATE:             // 0x67     ==   103
            return "SecuritySystemTargetState";
        case HAP_CHARACTERISTIC_SELECTED_RTP_STREAM_CONFIGURATION:        // 0x117    ==   279
            return "SelectedRTPStreamConfiguration";
        case HAP_CHARACTERISTIC_SERIAL_NUMBER:                            // 0x30     ==    48
            return "SerialNumber";
        case HAP_CHARACTERISTIC_SERVICE_LABEL_INDEX:                      // 0xCB     ==   203
            return "ServiceLabelIndex";
        case HAP_CHARACTERISTIC_SERVICE_LABEL_NAMESPACE:                  // 0xCD     ==   205
            return "ServiceLabelNamespace";
        case HAP_CHARACTERISTIC_SETUP_ENDPOINTS:                          // 0x118    ==   280
            return "SetupEndpoints";
        case HAP_CHARACTERISTIC_SET_DURATION:                             // 0xD3     ==   211
            return "SetDuration";
        case HAP_CHARACTERISTIC_SLAT_TYPE:                                // 0xC0     ==   192
            return "SlatType";
        case HAP_CHARACTERISTIC_SMOKE_DETECTED:                           // 0x76     ==   118
            return "SmokeDetected";
        case HAP_CHARACTERISTIC_STATUS_ACTIVE:                            // 0x75     ==   117
            return "StatusActive";
        case HAP_CHARACTERISTIC_STATUS_FAULT:                             // 0x77     ==   119
            return "StatusFault";
        case HAP_CHARACTERISTIC_STATUS_JAMMED:                            // 0x78     ==   120
            return "StatusJammed";
        case HAP_CHARACTERISTIC_STATUS_LOW_BATTERY:                       // 0x79     ==   121
            return "StatusLowBattery";
        case HAP_CHARACTERISTIC_STATUS_TAMPERED:                          // 0x7A     ==   122
            return "StatusTampered";
        case HAP_CHARACTERISTIC_STREAMING_STATUS:                         // 0x120    ==   288
            return "StreamingStatus";
        case HAP_CHARACTERISTIC_SULPHUR_DIOXIDE_DENSITY:                  // 0xC5     ==   197
            return "SulphurDioxideDensity";
        case HAP_CHARACTERISTIC_SUPPORTED_AUDIO_STREAM_CONFIGURATION:     // 0x115    ==   277
            return "SupportedAudioStreamConfiguration";
        case HAP_CHARACTERISTIC_SUPPORTED_RTP_CONFIGURATION:              // 0x116    ==   278
            return "SupportedRTPConfiguration";
        case HAP_CHARACTERISTIC_SUPPORTED_VIDEO_STREAM_CONFIGURATION:     // 0x114    ==   276
            return "SupportedVideoStreamConfiguration";
        case HAP_CHARACTERISTIC_SWING_MODE:                               // 0xB6     ==   182
            return "SwingMode";
        case HAP_CHARACTERISTIC_TARGET_AIR_PURIFIER_STATE:                // 0xA8     ==   168
            return "TargetAirPurifierState";
        case HAP_CHARACTERISTIC_TARGET_AIR_QUALITY:                       // 0xAE     ==   174
            return "TargetAirQuality";
        case HAP_CHARACTERISTIC_TARGET_DOOR_STATE:                        // 0x32     ==    50
            return "TargetDoorState";
        case HAP_CHARACTERISTIC_TARGET_FAN_STATE:                         // 0xBF     ==   191
            return "TargetFanState";
        case HAP_CHARACTERISTIC_TARGET_HEATER_COOLER_STATE:               // 0xB2     ==   178
            return "TargetHeaterCoolerState";
        case HAP_CHARACTERISTIC_TARGET_HEATING_COOLING_STATE:             // 0x33     ==    51
            return "TargetHeatingCoolingState";
        case HAP_CHARACTERISTIC_TARGET_HORIZONTAL_TILT_ANGLE:             // 0x7B     ==   123
            return "TargetHorizontalTiltAngle";
        case HAP_CHARACTERISTIC_TARGET_HUMIDIFIER_DEHUMIDIFIER_STATE:     // 0xB4     ==   180
            return "TargetHumidifierDehumidifierState";
        case HAP_CHARACTERISTIC_TARGET_POSITION:                          // 0x7C     ==   124
            return "TargetPosition";
        case HAP_CHARACTERISTIC_TARGET_RELATIVE_HUMIDITY:                 // 0x34     ==    52
            return "TargetRelativeHumidity";
        case HAP_CHARACTERISTIC_TARGET_SLAT_STATE:                        // 0xBE     ==   190
            return "TargetSlatState";
        case HAP_CHARACTERISTIC_TARGET_TEMPERATURE:                       // 0x35     ==    53
            return "TargetTemperature";
        case HAP_CHARACTERISTIC_TARGET_TILT_ANGLE:                        // 0xC2     ==   194
            return "TargetTiltAngle";
        case HAP_CHARACTERISTIC_TARGET_VERTICAL_TILT_ANGLE:               // 0x7D     ==   125
            return "TargetVerticalTiltAngle";
        case HAP_CHARACTERISTIC_TEMPERATURE_DISPLAY_UNITS:                // 0x36     ==    54
            return "TemperatureDisplayUnits";
        case HAP_CHARACTERISTIC_VALVE_TYPE:                               // 0xD5     ==   213
            return "ValveType";
        case HAP_CHARACTERISTIC_VERSION:                                  // 0x37     ==    55
            return "Version";
        case HAP_CHARACTERISTIC_VOC_DENSITY:                              // 0xC8     ==   200
            return "VOCDensity";
        case HAP_CHARACTERISTIC_VOLUME:                                   // 0x119    ==   281
            return "Volume";
        case HAP_CHARACTERISTIC_WATER_LEVEL:                              // 0xB5     ==   181
            return "WaterLevel";
        default:
            return "";
    }
}

#endif /* HAPCHARACTERISTICS_HPP_ */
