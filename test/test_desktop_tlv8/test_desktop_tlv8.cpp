//
// test_desktop_tlv8.cpp
// Homekit Test
//
//  Created on: 06.05.2020
//      Author: michael
//

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
