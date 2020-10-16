//
// HAPButton.hpp
// Homekit
//
//  Created on: 09.10.2020
//      Author: michael
//

#ifndef HAPBUTTON_HPP_
#define HAPBUTTON_HPP_

#include <Arduino.h>
#include <functional>
#include "HAPGlobals.hpp"



enum HAPBUTTON_STATE {
	HAPBUTTON_STATE_UNKNOWN 	 = 0,
	HAPBUTTON_STATE_CLICK 		 = 1,
	HAPBUTTON_STATE_DOUBLE_CLICK = 2,
	HAPBUTTON_STATE_HOLD 		 = 3,
	HAPBUTTON_STATE_LONG_HOLD 	 = 4,
};

class HAPButton {
public:
	HAPButton();

	HAPBUTTON_STATE checkButton();
	void dispatchEvents();

	inline void setCallbackClick(std::function<void(void)> callback){
        _callbackClick = callback;
    }

	inline void setCallbackDoubleClick(std::function<void(void)> callback){
        _callbackDoubleClick = callback;
    }

	inline void setCallbackHold(std::function<void(void)> callback){
        _callbackHold = callback;
    }

	inline void setCallbackLongHold(std::function<void(void)> callback){
        _callbackLongHold = callback;
    }


protected:

	// Button variables
	bool _buttonVal;   		// value read from button
	bool _buttonLast;  		// buffered value of the button's previous state
	bool _DCwaiting;   		// whether we're waiting for a double click (down)
	bool _DConUp;     		// whether to register a double click on next release, or whether to wait and click
	bool _singleOK;     	// whether it's OK to do a single click
	int32_t _downTime;         // time the button was pressed down
	int32_t _upTime;           // time the button was released
	bool _ignoreUp;   		// whether to ignore the button release because the click+hold was triggered
	bool _waitForUp;        // when held, whether to wait for the up event
	bool _holdEventPast;    // whether or not the hold event happened already
	bool _longHoldEventPast;// whether or not the long hold event happened already
	uint8_t _debounce;         // ms debounce period to prevent flickering when pressing or releasing the button
	uint8_t _DCgap;            // max ms between clicks for a double click event
	uint16_t _holdTime;        // ms hold period: how long to wait for press+hold event
	uint16_t _longHoldTime;    // ms long hold period: how long to wait for press+hold event

	std::function<void(void)> _callbackClick;
    std::function<void(void)> _callbackDoubleClick;
	std::function<void(void)> _callbackHold;
	std::function<void(void)> _callbackLongHold;

private:

};

#endif