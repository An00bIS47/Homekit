//
// HAPPreferences.hpp
// Homekit
//
//  Created on: 15.05.2020
//      Author: michael
//
#ifndef HAPPREFERENCESEXT_HPP_
#define HAPPREFERENCESEXT_HPP_

#include <Arduino.h>
#include <Preferences.h>


class HAPPreferencesExt : public Preferences {

public:
    bool begin(const char* partitionName, const char * name, bool readOnly);

private:

    uint8_t _test;
};

#endif /* HAPPREFERENCESEXT_HPP_ */