// @dir pca301.h (2013-08-10)
// RFM69 communication library for PCA 301
//
// authors: ohweh + trilu
// see http://forum.fhem.de/index.php?t=msg&th=11648
//
// This code is derived from RF12.cpp + Ports.cpp being part of JeeLib
// https://github.com/jcw/jeelib
// 2009-05-06 - <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// 2014-05-05 - set command for center frequenz implemented, 
//              moved declaration "static uint16_t rf12_center_freq = 0xA70D;" from main to pca301.h
// 2017-01-27 - <dev@mcbachmann.de> adapted pca301serial code to RFM69
//

#ifndef _PCA301_h
#define _PCA301_h
#include "Arduino.h"

//- Shorthand for first RFM69 data byte in rfm69_buf. ----------------------------------------------
#define rfm69_data       (rfm69_buf)

#define RFM69_MAXDATA   66              // maximum message size in bytes

//- PCA301 device settings -------------------------------------------------------------------------
#define PCA_MAXDEV      20              // max PCA301 devices
#define PCA_MAXRETRIES  5               // how often a device get's polled before considered "dead"

//- struct for EEPROM config read/write ------------------------------------------------------------
// struct struct_pcaDev {
//   uint8_t   channel;                    // associated device channel
//   uint32_t  devId;                      // device ID
//   bool      pState;                     // device powered on/off
//   uint16_t  pNow;                       // actual power consumption (W)
//   uint16_t  pTtl;                       // total power consumption (KWh)
//   uint32_t  nextTX;                     // last packet submitted  
//   uint16_t  retries;                    // outstanding answers


//   uint8_t   iidState;                   // iid of the state (isOn)
//   uint8_t   iidInUse;                   // iid of the inUse state
//   uint8_t   iidPowerCurrent;            // iid of the pNow state
//   uint8_t   iidPowerTotal;              // iid of the pTtl state
//   String    name;
// };

// struct struct_pcaConf {
//   uint8_t  numDev;                      // devices in use
//   uint16_t pollIntv;                    // polling intervall in 1/10th of seconds for regular devices
//   uint16_t deadIntv;                    // retry intervall in 1/10th of seconds for dead devices
//   uint8_t  quiet;                       // quiet mode on/off  
//   struct struct_pcaDev pcaDev[PCA_MAXDEV];
//   uint16_t crc;
// };

#endif