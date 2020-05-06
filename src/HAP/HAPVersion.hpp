//
// HAPVersion.hpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//

#ifndef HAPVERSION_HPP_
#define HAPVERSION_HPP_

#include <Arduino.h>
#include "HAPBuildnumber.hpp"
#include "HAPGlobals.hpp"


/* Build automated generated version number */
#define HOMEKIT_VERSION \
	STR(HOMEKIT_VERSION_MAJOR) \
"." STR(HOMEKIT_VERSION_MINOR) \
"." STR(HOMEKIT_VERSION_REVISION) \
"+" STR(HOMEKIT_VERSION_BUILD) \


#define HOMEKIT_FEATURE_REV HAP_PLUGIN_FEATURE_NUMBER

struct HAPVersion {

	int major, minor, revision, build;

	HAPVersion(){
		major 		= 0;
		minor 		= 0;
		revision 	= 0;
		build 		= 0;
	}

	HAPVersion(const char* version)
	{
		sscanf(version, "%d.%d.%d+%d", &major, &minor, &revision, &build);
		if (major < 0) major = 0;
		if (minor < 0) minor = 0;
		if (revision < 0) revision = 0;
		if (build < 0) build = 0;
	}

	bool operator < (const HAPVersion& other)
	{
		if (major < other.major)
			return true;
		if (minor < other.minor)
			return true;
		if (revision < other.revision)
			return true;
		if (build < other.build)
			return true;
		return false;
	}

	bool operator == (const HAPVersion& other)
	{
		return major == other.major
			&& minor == other.minor
			&& revision == other.revision
			&& build == other.build;
	}

	void operator = (const HAPVersion& other)
	{
		major = other.major;
		minor = other.minor;
		revision = other.revision;
		build = other.build;
	}

	String toString(){
		char str[32];
		sprintf(str, "%d.%d.%d+%d", major, minor, revision, build);
		return String(str);
	}

	static String featureRev(){
		const char* binaryString = HAP_PLUGIN_FEATURE_NUMBER;
		
		// convert binary string to integer
		uint64_t value = (uint64_t)strtol(binaryString, NULL, 2);
		
		// convert integer to hex string
		char hexString[32]; // long enough for any 32-bit value, 4-byte aligned
		sprintf(hexString, "%llx", value);

		return String(hexString);
	}

	static uint64_t featureRevToInt(const char* rev){
		return (uint64_t)strtol(rev, NULL, 2);
	}

	static bool compareFeatureRev(uint64_t first, uint64_t second){		
		uint64_t result = second & first;
		return result == first;
	}
};

#endif /* HAPVERSION_HPP_ */
