#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unity.h>
#include <stdio.h>
#include <sys/wait.h>

#define ITERATIONS      100

//#define DEVICE_ID       "24:6F:28:AF:5F:A4"
//#define ALIAS           "heltec"

#define DEVICE_ID       "BC:DD:C2:CA:FE:EC"
#define ALIAS           "cafeec"


#define SETUPCODE       "031-45-712"

#define CHARACTERISTICS "2.10"

#define CHARACTERISTICS_FAKEGATO_READ "3.15"

#define CHARACTERISTICS_FAKEGATO_REQ_ENTRY "3.17"
#define CHARACTERISTICS_FAKEGATO_HISTORY "3.16"

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

    // std::cout << "Creating storage file:" << std::endl;
    std::string cmd = "python3 -m homekit.init_controller_storage -f ";
    cmd += PAIRINGDATAFILE;

    // // std::cout << "cmd: " << cmd << std::endl;
    exec( cmd.c_str() );

    // cmd = "curl --request DELETE \
    //     --url https://esp32-cafeec/api/pairings \
    //     --header 'Authorization: Basic YWRtaW46c2VjcmV0' \
    //     --header 'Connection: keep-alive' \
    //     --header 'Content-Type: application/json' \
    //     -k \
    //     -s";
    // // std::cout << "cmd: " << cmd << std::endl;
    // exec( cmd.c_str() );
}

void _tearDown(void) {
    // clean stuff up here
    std::string cmd = "rm ";
    cmd += PAIRINGDATAFILE;

    // // std::cout << "cmd: " << cmd << std::endl;
    exec( cmd.c_str() );
}

void test_homekit_identify_unpaired(void) {
    
    std::string cmd = "python3 -m homekit.identify -d ";
    cmd += DEVICE_ID;
        
    // std::cout << "cmd: " << cmd << std::endl;
    std::string result = exec( cmd.c_str() );
    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL_STRING("", result.c_str());
    

}

void test_homekit_identify_paired(void) {
    
    std::string cmd = "python3 -m homekit.identify ";
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
        
    // std::cout << "cmd: " << cmd << std::endl;
    std::string result = exec( cmd.c_str() );
    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL_STRING("", result.c_str());
    

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
    
    // std::cout << "cmd: " << cmd << std::endl;
    std::string result = exec( cmd.c_str() );


    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL_STRING_LEN("Pairing for \"cafeec\" was established.\n", result.c_str(), result.length());


}

void test_homekit_remove_pair(void) {
    
   //python3 -m homekit.pair -d ${DEVICEID} -p ${SETUPCODE} -f ${PAIRINGDATAFILE} -a ${ALIAS}
    std::string cmd = "python3 -m homekit.remove_pairing ";
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    
    
    // std::cout << "cmd: " << cmd << std::endl;
    std::string result = exec( cmd.c_str() );


    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL_STRING_LEN("Pairing for \"cafeec\" was removed.\n", result.c_str(), result.length());


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
    
    

    // std::cout << "cmd: " << cmd << std::endl;
    
    //std::string result = exec( cmd.c_str() );

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL(0, exitCode);
    

}


void test_homekit_get_characteristics(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_characteristic ";        
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS;
    cmd += " > /dev/null 2>&1";
    
    
    // std::cout << "cmd: " << cmd << std::endl;
    
    std::string result = exec( cmd.c_str() );

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    //std::cout << result << std::endl;          
    TEST_ASSERT_EQUAL(0, exitCode);
    

}

void test_homekit_get_characteristics_fg_entry_count(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_characteristic ";        
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_READ;
    cmd += " > /dev/null 2>&1";
    
    // std::cout << "cmd: " << cmd << std::endl;
    
    // std::string result = exec( cmd.c_str() );

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
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_HISTORY;
    cmd += " > /dev/null 2>&1";    

    // std::cout << "cmd: " << cmd << std::endl;    
    //std::string result = exec( cmd.c_str() );

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    TEST_ASSERT_EQUAL(0, exitCode);
    
}

void test_homekit_put_characteristics_fg_address_1(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.put_characteristic ";        
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_REQ_ENTRY;
    cmd += " ";
    cmd += "MkIwMjEwMDAwMDAwMDA=";

    // std::cout << "cmd: " << cmd << std::endl;    
    

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    TEST_ASSERT_EQUAL(0, exitCode);
    
}


void test_homekit_put_characteristics_fg_address_17(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.put_characteristic ";        
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_REQ_ENTRY;
    cmd += " ";
    cmd += "MkIwMjExMDAwMDAwMDEwMA==";

    // std::cout << "cmd: " << cmd << std::endl;    
    
    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    TEST_ASSERT_EQUAL(0, exitCode);
    
}

void test_homekit_put_characteristics_fg_address_33(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.put_characteristic ";        
    cmd += " -f";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_REQ_ENTRY;
    cmd += " ";
    cmd += "MkIwMjIxMDAwMDAwMDEwMA==";

    // std::cout << "cmd: " << cmd << std::endl;    
    

    int ret = system( cmd.c_str() );
    int exitCode = 0;
    if (WEXITSTATUS(ret) == 0)
        exitCode = 0;
    else
        exitCode = 1;

    TEST_ASSERT_EQUAL(0, exitCode);
    
}


int main(int argc, char **argv) {

    _setUp();
    UNITY_BEGIN();



    // std::cout << "Identify unpaired device:" << std::endl;
    for (int i=0; i < ITERATIONS; i ++){

        // std::cout << i << ": times: " << "Identify unpaired device" << std::endl;
        RUN_TEST(test_homekit_identify_unpaired);
    };
    
    // std::cout << "Pair:" << std::endl;
    RUN_TEST(test_homekit_pair);


    for (int i=0; i < ITERATIONS; i ++){
        RUN_TEST(test_homekit_identify_paired);
    }

    // std::cout << "Get accessory:" << std::endl;
    for (int i=0; i < ITERATIONS; i ++){
        // std::cout << i << ": times: " << "Get ACCESSORY" << std::endl;
        RUN_TEST(test_homekit_get_accessories);
    }

    // std::cout << "Get characteristic:" << std::endl;
    for (int i=0; i < ITERATIONS; i ++){
        // std::cout << i << ": times: " << "Get CHARACTERISTICS" << std::endl;
        RUN_TEST(test_homekit_get_characteristics);
    }



    // std::cout << "Fakegato workflow:" << std::endl;
    for (int i=0; i < ITERATIONS; i ++){
        // std::cout << i << ": times: " << "Fakegato workflow" << std::endl;
        RUN_TEST(test_homekit_get_characteristics_fg_entry_count);        
        RUN_TEST(test_homekit_put_characteristics_fg_address_1);
        RUN_TEST(test_homekit_get_accessories);
        RUN_TEST(test_homekit_get_characteristics_fg_history);

        
        
        RUN_TEST(test_homekit_put_characteristics_fg_address_17);
        RUN_TEST(test_homekit_get_accessories);
        RUN_TEST(test_homekit_get_characteristics_fg_history);



        RUN_TEST(test_homekit_put_characteristics_fg_address_33);
        RUN_TEST(test_homekit_get_accessories);
        RUN_TEST(test_homekit_get_characteristics_fg_history);
    }


    // std::cout << "Remove pairing:" << std::endl;
    // RUN_TEST(test_homekit_remove_pair);

    UNITY_END();

    _tearDown();
    return 0;
}
