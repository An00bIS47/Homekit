#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unity.h>
#include <stdio.h>
#include <sys/wait.h>

#include <cctype>
#include <string>
#include <algorithm>

#include <mbedtls/base64.h>

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


int entryCounter    = 0;
int expectedEntries = 0;



void _setUp(void);
void _tearDown(void);

std::string exec(const char* cmd);


void test_homekit_pair(void);
void test_homekit_get_accessories(void);


void test_homekit_get_characteristics_fg_entry_count(void);
void test_homekit_get_characteristics_fg_history(void);

void test_homekit_put_characteristics_fg_address_1(void);


void test_homekit_remove_pairing(void);

std::string trim(const std::string &s);

std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

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

void test_homekit_get_accessories(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_accessories ";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -o json";
    cmd += " | python3 ~/Development/Homekit/utils/accessory_validate/accval.py";
    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;


    TEST_ASSERT_EQUAL(0, exitCode);
    

}



void test_homekit_get_characteristics_fg_entry_count(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_characteristic ";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_READ;
    cmd += TEST_REDIRECT_STDOUT;
        
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif
    

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL(0, exitCode);
    
}


void test_homekit_get_characteristics_fg_history(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_characteristic ";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_HISTORY;
    cmd += " | jq '.\"3.16\".value' -r";
    //cmd += TEST_REDIRECT_STDOUT;

#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    bool stop = false;

    while(!stop) {
        std::string result = exec( cmd.c_str() );

        result = trim(result);

        std::cout << "result: *" << result << "*" << std::endl;

        if (strncmp(result.c_str(), "null", 4) == 0) {

        } else if (strncmp(result.c_str(), "AA==", 4) == 0) {
            std::cout << "Stopping now: " << entryCounter << std::endl;

            stop = true;
        } else {            
        
        size_t len;
        uint8_t buffer[512];
        if( mbedtls_base64_decode( buffer, sizeof( buffer ), &len, (const uint8_t*)result.c_str(), result.length() ) != 0) {
            std::cout << "base64 decode failed" << std::endl;
            stop = true;
        }
            std::cout << "len: " << len << std::endl;
            std::cout << "entryCounter: " << entryCounter << std::endl;

            entryCounter += len / 16;
        }
    }
    
}

void test_homekit_put_characteristics_fg_address_1(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.put_characteristic ";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_REQ_ENTRY;
    cmd += " ";
    cmd += "MkIwMjEwMDAwMDAwMDA=";
    

#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    TEST_ASSERT_EQUAL(0, exitCode);
    
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
    
    // 
    // Pair
    //        
    RUN_TEST(test_homekit_pair);

    for (int i=0; i < ITERATIONS; i ++){

        std::cout << "Iteration " << (i + 1) << " of " << ITERATIONS << ":" << std::endl;    

        // RUN_TEST(test_homekit_identify_unpaired);
        

 
        RUN_TEST(test_homekit_get_accessories);
            

        // 
        // FakeGato
        // 
        RUN_TEST(test_homekit_get_characteristics_fg_entry_count);        
        RUN_TEST(test_homekit_put_characteristics_fg_address_1);
        RUN_TEST(test_homekit_get_accessories);

        
        RUN_TEST(test_homekit_get_characteristics_fg_history);

        

        
        std::cout << "\n\n" << std::endl;
    }

       
    // Remove pairing
    RUN_TEST(test_homekit_remove_pairing);


    UNITY_END();
#if TEST_KEEP_PAIRING
#else
    _tearDown();
#endif

    return 0;
}
