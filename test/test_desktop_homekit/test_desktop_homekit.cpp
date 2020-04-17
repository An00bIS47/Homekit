#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unity.h>
#include <stdio.h>
#include <sys/wait.h>

#define ITERATIONS      10
#define DEVICE_ID       "BC:DD:C2:CA:FE:EC"
#define SETUPCODE       "031-45-712"
#define ALIAS           "cafeec"
#define CHARACTERISTICS "2.10"

#define PAIRINGDATAFILE "./homekitStorage.json"



std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


void _setUp(void) {
    // set stuff up here
    std::string cmd = "python3 -m homekit.init_controller_storage -f ";
    cmd += PAIRINGDATAFILE;
    exec( cmd.c_str() );
}

void _tearDown(void) {
    // clean stuff up here
    std::string cmd = "rm ";
    cmd += PAIRINGDATAFILE;
    exec( cmd.c_str() );
}

void test_homekit_identify_unpaired(void) {
    
    std::string cmd = "python3 -m homekit.identify -d ";
    cmd += DEVICE_ID;
    
    //for (int i=0; i < ITERATIONS; i ++){
        std::string result = exec( cmd.c_str() );
        //std::cout << result << std::endl;          
        TEST_ASSERT_EQUAL_STRING("", result.c_str());
    //}

}

void test_homekit_pair(void) {
    



    //python3 -m homekit.pair -d ${DEVICEID} -p ${SETUPCODE} -f ${PAIRINGDATAFILE} -a ${ALIAS}
    std::string cmd = "python3 -m homekit.pair -d ";
    cmd += DEVICE_ID;
    cmd += " -p ";
    cmd += SETUPCODE;
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    
    //for (int i=0; i < ITERATIONS; i ++){
        std::string result = exec( cmd.c_str() );


        //std::cout << result << std::endl;          
        TEST_ASSERT_EQUAL_STRING_LEN("Pairing for \"cafeec\" was established.\n", result.c_str(), result.length());
    //}

}

void test_homekit_get_accessories(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_accessories ";        
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -o json";
    cmd += " | python3 ~/Development/Homekit/utils/accessory_validate/accval.py";
    
    //for (int i=0; i < ITERATIONS; i ++){
        std::string result = exec( cmd.c_str() );

        int ret = system( cmd.c_str() );
        int exitCode = 0;
        if (WEXITSTATUS(ret) == 0)
            exitCode = 0;
        else
            exitCode = 1;

        //std::cout << result << std::endl;          
        TEST_ASSERT_EQUAL(0, exitCode);
    //}

}


int main(int argc, char **argv) {

    _setUp();
    UNITY_BEGIN();

    for (int i=0; i < ITERATIONS; i ++){
        RUN_TEST(test_homekit_identify_unpaired);
    };
    
    RUN_TEST(test_homekit_pair);

    for (int i=0; i < ITERATIONS; i ++){
        RUN_TEST(test_homekit_get_accessories);
    }

    UNITY_END();

    _tearDown();
    return 0;
}
