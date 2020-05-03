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

#include "HAP/HAPTLV8.hpp"
#include <unity.h>


// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_tlv_duo(void) {
    
    TLV8 tlv;
    const int length = 13;
    uint8_t data[length] = {0x01, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x02, 0x02, 0x01, 0x02};
    tlv.encode(data, length);

    uint8_t result[128];
    size_t s = 0;
    tlv.decode(result, &s); 

    tlv.clear();

    TEST_ASSERT_EQUAL_MEMORY(data, result, length);
    TEST_ASSERT_EQUAL(s, sizeof(data));   
    TEST_ASSERT_EQUAL(s, length);     
}

void test_tlv_duo_get_single(void) {
    
    uint8_t expected[2] = {0xA1, 0xA2};

    TLV8 tlv;
    const int length = 13;
    uint8_t data[length] = {0x01, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x02, 0x02, 0xA1, 0xA2};
    tlv.encode(data, length);

    uint8_t result[128];
    size_t s = 0;
    
    tlv.decode(0x02, result, &s);    

    tlv.clear();

    TEST_ASSERT_EQUAL_MEMORY(expected, result, 2);
    TEST_ASSERT_EQUAL(s, 2);           
}

void test_tlv_simple(void) {
    
    TLV8 tlv;
    const int length = 9;
    uint8_t data[length] = {0x01, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    tlv.encode(data, length);

    uint8_t result[128];
    size_t s = 0;
    tlv.decode(result, &s);  
    
    tlv.clear();

    TEST_ASSERT_EQUAL_MEMORY(data, result, length);
    TEST_ASSERT_EQUAL(s, sizeof(data)); 
    TEST_ASSERT_EQUAL(s, length);   
}

void test_tlv_stream(void) {
    
    TLV8 tlv;
    uint8_t data[9] = {0x01, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    tlv.encode(data, 9);
    
    size_t s = 0;
    s = tlv.decode(Serial);  
    
    tlv.clear();    
    TEST_ASSERT_EQUAL(s, sizeof(data));   
}

void test_tlv_separator(void) {
    
    TLV8 tlv;
    const int length = 6;
    uint8_t data[length] = {0xFF, 0x00, 0x01, 0x02, 0x03, 0x04};
    tlv.encode(data, length);
    

    uint8_t result[128];
    size_t s = 0;
    tlv.decode(result, &s);  
    
    tlv.clear();

    TEST_ASSERT_EQUAL_MEMORY(data, result, length);
    TEST_ASSERT_EQUAL(s, sizeof(data)); 
    TEST_ASSERT_EQUAL(s, length);   
}

void process() {
    UNITY_BEGIN();

    RUN_TEST(test_tlv_simple);    
    RUN_TEST(test_tlv_duo);  
    RUN_TEST(test_tlv_duo_get_single);  
    
    RUN_TEST(test_tlv_separator);

#ifdef ARDUINO
    RUN_TEST(test_tlv_stream);    
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
