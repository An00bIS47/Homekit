//
// HAPCustomCharacteristics+Services.hpp
// Homekit
//
//  Generated on: 23.09.2019
//
#ifndef HAPCUSTOMCHARACTERISTICSSERVICES_HPP_
#define HAPCUSTOMCHARACTERISTICSSERVICES_HPP_

// ============================================================================================================
// Services
// ============================================================================================================

// Fertility Service 
// used for:
//	- MiFlora
#define HAP_CUSTOM_SERVICE_FERTILITY 			        "00000001-6B66-4FFD-88CC-16A60B5C4E03"


// ============================================================================================================
// Characteristics
// ============================================================================================================

// Fertility Characteristic 
// used for:
//	- MiFlora
#define HAP_CUSTOM_CHARACTERISTICS_FERTITLITY	        "000001EA-6B66-4FFD-88CC-16A60B5C4E03"


// Measure Mode Characteristic 
// used for:
//	- RF24 Remote Weather Device
#define HAP_CUSTOM_CHARACTERISTICS_MEASURE_MODE	        "000003EA-6B66-4FFD-88CC-16A60B5C4E03"

// Heartbeat Characteristic 
// used for:
//	- RF24 Remote Weather Device
//  - ToDo: MiFlora
#define HAP_CUSTOM_CHARACTERISTICS_HEARTBEAT	        "000004EA-6B66-4FFD-88CC-16A60B5C4E03"

#endif /* HAPCUSTOMCHARACTERISTICSSERVICES_HPP_ */
