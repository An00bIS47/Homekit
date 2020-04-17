/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/

#ifdef ARDUINO

#include <Arduino.h>

#else
#include <ArduinoFake.h>

using namespace fakeit;

#endif

#include "HAP/HAPEncryption.hpp"
#include <unity.h>


// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }



void test_encrypt_simple(void) {
    


    // TEST_ASSERT_EQUAL_MEMORY(data, result, length);
    // TEST_ASSERT_EQUAL(s, sizeof(data));   
    // TEST_ASSERT_EQUAL(s, length);     
}


void process() {
    UNITY_BEGIN();

    RUN_TEST(test_encrypt_simple);    


#ifdef ARDUINO
    
#endif
    

    UNITY_END();
}

#ifdef ARDUINO


void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(5000);

    process();
}

void loop() {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(100);
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
}

#else


int main(int argc, char **argv) {
    process();
    return 0;
}

#endif
