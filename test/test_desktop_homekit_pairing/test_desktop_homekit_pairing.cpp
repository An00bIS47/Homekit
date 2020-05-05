#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unity.h>
#include <stdio.h>
#include <sys/wait.h>

#define ITERATIONS                          10

#define BOARD_HELTEC                        0
#define BOARD_CAFEEC                        1


#if BOARD_HELTEC == 1
#define DEVICE_ID                           "24:6F:28:AF:5F:A4"
#define ALIAS                               "heltec"
#define ALIAS_ADDITIONAL_CONTROLLER         "heltec_remote"
#elif BOARD_CAFEEC == 1
#define DEVICE_ID                           "BC:DD:C2:CA:FE:EC"
#define ALIAS                               "cafeec"
#define ALIAS_ADDITIONAL_CONTROLLER         "cafeec_remote"
#endif


#define SETUPCODE                           "031-45-712"

#define CHARACTERISTICS                     "2.10"

#define CHARACTERISTICS_FAKEGATO_READ       "3.15"
#define CHARACTERISTICS_FAKEGATO_HISTORY    "3.16"
#define CHARACTERISTICS_FAKEGATO_REQ_ENTRY  "3.17"

#define PAIRINGDATAFILE                     "./.pio/homekitStorage.json"

#define TEST_SHOW_CMD                       1
#define TEST_SHOW_RESULT                    0




#define TEST_REDIRECT_STDOUT                " > /dev/null 2>&1"

//#define TEST_DEBUG                          " --log DEBUG"



void _setUp(void);
void _tearDown(void);

std::string exec(const char* cmd);

void test_homekit_pair(void);
void test_homekit_remove_pairing(void);


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

#if TEST_SHOW_RESULT
    std::cout << "result: " << result << std::endl;    
#endif    
    return result;
}


void _setUp(void) {
    // set stuff up here

    // remove pairing file
    {
        std::string cmd = "rm ";
        cmd += PAIRINGDATAFILE;

#if TEST_SHOW_CMD
        std::cout << "cmd: " << cmd << std::endl;    
#endif
        exec( cmd.c_str() );
    }

    // init pairing file
    {
        std::string cmd = "python3 -m homekit.init_controller_storage -f ";
        cmd += PAIRINGDATAFILE;

#if TEST_SHOW_CMD
        std::cout << "cmd: " << cmd << std::endl;    
#endif

        exec( cmd.c_str() );
    }

//     cmd = "curl --request DELETE \
// --url https://esp32-cafeec/api/pairings \
// --header 'Authorization: Basic YWRtaW46c2VjcmV0' \
// --header 'Connection: keep-alive' \
// --header 'Content-Type: application/json' \
// -k \
// -s";

// #if TEST_SHOW_CMD
//     std::cout << "cmd: " << cmd << std::endl;    
// #endif

    //exec( cmd.c_str() );
}

void _tearDown(void) {
    // clean stuff up here

    // remove pairing file
    {
        std::string cmd = "rm ";
        cmd += PAIRINGDATAFILE;

#if TEST_SHOW_CMD
        std::cout << "cmd: " << cmd << std::endl;    
#endif
        exec( cmd.c_str() );
    }
}


void test_homekit_pair(void) {
    
    //python3 -m homekit.pair -d ${DEVICEID} -p ${SETUPCODE} -f ${PAIRINGDATAFILE} -a ${ALIAS}
    std::string cmd = "python3 -m homekit.pair";
    cmd += " -d ";
    cmd += DEVICE_ID;
    cmd += " -p ";
    cmd += SETUPCODE;
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;    
    

#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif
    std::string result = exec( cmd.c_str() );

    char cmpstr[256];
    sprintf(cmpstr, "Pairing for \"%s\" was established.\n", ALIAS);

    TEST_ASSERT_EQUAL_STRING_LEN(cmpstr, result.c_str(), result.length());

}

void test_homekit_remove_pairing(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.remove_pairing";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;    
    
    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    std::string result = exec( cmd.c_str() );

    char cmpstr[256];
    sprintf(cmpstr, "Pairing for \"%s\" was removed.\n", ALIAS);

    TEST_ASSERT_EQUAL_STRING_LEN(cmpstr, result.c_str(), result.length());
}


int main(int argc, char **argv) {



    _setUp();

    UNITY_BEGIN();
    

    for (int i=0; i < ITERATIONS; i ++){

        std::cout << "Iteration " << (i + 1) << " of " << ITERATIONS << ":" << std::endl;            

        // Pair
        RUN_TEST(test_homekit_pair);
      

        // Remove pairing
        RUN_TEST(test_homekit_remove_pairing);


        std::cout << "\n\n" << std::endl;
    }

    UNITY_END();

    _tearDown();


    return 0;
}
