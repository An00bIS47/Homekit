//
// HAPButton.cpp
// Homekit
//
//  Created on: 09.10.2020
//      Author: michael
//

#include "HAPButton.hpp"
#include "HAPLogger.hpp"
#include "HAPServer.hpp"

HAPButton::HAPButton(){
	_debounce = 20;
	_DCgap = 250;
	_holdTime = 1000;
	_longHoldTime = 3000;
	_buttonVal = HIGH;   // value read from button
	_buttonLast = HIGH;  // buffered value of the button's previous state
	_DCwaiting = false;  // whether we're waiting for a double click (down)
	_DConUp = false;     // whether to register a double click on next release, or whether to wait and click
	_singleOK = true;    // whether it's OK to do a single click
	_downTime = -1;         // time the button was pressed down
	_upTime = -1;           // time the button was released
	_ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
	_waitForUp = false;        // when held, whether to wait for the up event
	_holdEventPast = false;    // whether or not the hold event happened already
	_longHoldEventPast = false;// whether or not the long hold event happened already

    _callbackClick          = nullptr;
    _callbackDoubleClick    = nullptr;
    _callbackHold           = nullptr;
    _callbackLongHold       = nullptr;

    pinMode(HAP_BUTTON_PIN, INPUT);
}


HAPBUTTON_STATE HAPButton::checkButton() {    
    HAPBUTTON_STATE event = HAPBUTTON_STATE_UNKNOWN;

    _buttonVal = digitalRead(HAP_BUTTON_PIN);
   
    // Button pressed down
    if (_buttonVal == LOW && _buttonLast == HIGH && (millis() - _upTime) > _debounce) {
        _downTime = millis();
        _ignoreUp = false;
        _waitForUp = false;
        _singleOK = true;
        _holdEventPast = false;
        _longHoldEventPast = false;
        if ((millis()-_upTime) < _DCgap && _DConUp == false && _DCwaiting == true) {
            _DConUp = true;
        } else {
            _DConUp = false;
            _DCwaiting = false;
        }
       	
    } else if (_buttonVal == HIGH && _buttonLast == LOW && (millis() - _downTime) > _debounce)    {
            // Button released        
        if (!_ignoreUp) {
            _upTime = millis();
            if (_DConUp == false) _DCwaiting = true;
            else {
                event = HAPBUTTON_STATE_DOUBLE_CLICK;
                _DConUp = false;
                _DCwaiting = false;
                _singleOK = false;
            }
        }
    }

    // Test for normal click event: _DCgap expired
    if ( _buttonVal == HIGH && (millis()-_upTime) >= _DCgap && _DCwaiting == true && _DConUp == false && _singleOK == true && event != 2) {
        event = HAPBUTTON_STATE_CLICK;
        _DCwaiting = false;
    }

    // Test for hold
    if (_buttonVal == LOW && (millis() - _downTime) >= _holdTime) {
        // Trigger "normal" hold
        if (!_holdEventPast){
            event = HAPBUTTON_STATE_HOLD;
            _waitForUp = true;
            _ignoreUp = true;
            _DConUp = false;
            _DCwaiting = false;
            //_downTime = millis();
            _holdEventPast = true;
        }
        // Trigger "long" hold
        if ((millis() - _downTime) >= _longHoldTime){
            if (!_longHoldEventPast){
                event = HAPBUTTON_STATE_LONG_HOLD;
                _longHoldEventPast = true;
            }
        }
    }

    _buttonLast = _buttonVal;
    return event;
}

void HAPButton::dispatchEvents() {
    // Get button event and act accordingly
    HAPBUTTON_STATE b = checkButton();
    if (b == HAPBUTTON_STATE_CLICK && _callbackClick) _callbackClick();
    if (b == HAPBUTTON_STATE_DOUBLE_CLICK && _callbackDoubleClick) _callbackDoubleClick();
    if (b == HAPBUTTON_STATE_HOLD && _callbackHold) _callbackHold();
    if (b == HAPBUTTON_STATE_LONG_HOLD && _callbackLongHold) _callbackLongHold();
}

