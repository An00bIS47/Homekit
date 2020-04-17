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
#include <ArduinoFake.h>
#include "HAP/HAPTLV8.hpp"
#include <unity.h>

using namespace fakeit;

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_tlv_simple(void) {
    
    TLV8 tlv;
    uint8_t data[9] = {0x01, 0x07, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    tlv.encode(data, 9);

    uint8_t result[128];
    size_t s = 0;
    tlv.decode(result, &s);  
    
    tlv.clear();
  
    TEST_ASSERT_EQUAL_MEMORY(data, result, 9);
    TEST_ASSERT_EQUAL(s, sizeof(data));
    
}



int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_tlv_simple);
    UNITY_END();

    return 0;
}
