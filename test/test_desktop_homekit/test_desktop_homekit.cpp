#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unity.h>
#include <stdio.h>
#include <sys/wait.h>

#define ITERATIONS                          1

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
#define TEST_KEEP_PAIRING                   0



#define TEST_REDIRECT_STDOUT                " > /dev/null 2>&1"

//#define TEST_DEBUG                          " --log DEBUG"




std::string cmd_pairing_prepare = "";
std::string cmd_pairing_add = "";
std::string additional_pairing_id = "";



void _setUp(void);
void _tearDown(void);

std::string exec(const char* cmd);

void test_homekit_identify_paired(void);
void test_homekit_pair(void);
void test_homekit_get_accessories(void);
void test_homekit_get_characteristics(void);

void test_homekit_get_characteristics_fg_entry_count(void);
void test_homekit_get_characteristics_fg_history(void);

void test_homekit_put_characteristics_fg_address_1(void);
void test_homekit_put_characteristics_fg_address_17(void);
void test_homekit_put_characteristics_fg_address_33(void);

void test_homekit_list_pairings(void);
void test_homekit_prepare_add_remote_pairing(void);
void test_homekit_add_additional_pairing(void);
void test_homekit_finish_add_remote_pairing(void);

void test_homekit_get_accessories_additional(void);

void test_homekit_remove_additional_pairing(void);
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

void test_homekit_identify_unpaired(void) {
    
    std::string cmd = "python3 -m homekit.identify -d ";
    cmd += DEVICE_ID;
    
    
    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif    
    std::string result = exec( cmd.c_str() );
    
    TEST_ASSERT_EQUAL_STRING("", result.c_str());
    

}


void test_homekit_identify_paired(void) {
    
    std::string cmd = "python3 -m homekit.identify";
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    
    
    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif    
    std::string result = exec( cmd.c_str() );
    
    TEST_ASSERT_EQUAL_STRING("", result.c_str());
    

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

void test_homekit_get_accessories_additional(void){
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_accessories ";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS_ADDITIONAL_CONTROLLER;
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

void test_homekit_get_characteristics(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.get_characteristic ";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS;
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

    TEST_ASSERT_EQUAL(0, exitCode);
    
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


void test_homekit_put_characteristics_fg_address_17(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.put_characteristic";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_REQ_ENTRY;
    cmd += " ";
    cmd += "MkIwMjExMDAwMDAwMDEwMA==";
    

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

void test_homekit_put_characteristics_fg_address_33(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.put_characteristic";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -c ";
    cmd += CHARACTERISTICS_FAKEGATO_REQ_ENTRY;
    cmd += " ";
    cmd += "MkIwMjIxMDAwMDAwMDEwMA==";
    

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

void test_homekit_list_pairings(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.list_pairings";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;    
    

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


void test_homekit_prepare_add_remote_pairing(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.prepare_add_remote_pairing";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS_ADDITIONAL_CONTROLLER;        
    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    std::string result = exec( cmd.c_str() );
    cmd_pairing_prepare = result.substr(51);
    additional_pairing_id = cmd_pairing_prepare.substr(6,36);

#if TEST_SHOW_CMD
    std::cout << "additional_pairing_id: " << additional_pairing_id << std::endl;    
#endif

    TEST_ASSERT_EQUAL_STRING_LEN("Please add this to homekit.add_additional_pairing:", result.c_str(), 50);
}


void test_homekit_add_additional_pairing(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.add_additional_pairing";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS;
    cmd += " -p User";
    cmd += " ";
    cmd += cmd_pairing_prepare;

    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    std::string result = exec( cmd.c_str() );
    cmd_pairing_add = result.substr(54);

    TEST_ASSERT_EQUAL_STRING_LEN("Please add this to homekit.finish_add_remote_pairing:", result.c_str(), 53);
}


void test_homekit_finish_add_remote_pairing(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.finish_add_remote_pairing";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS_ADDITIONAL_CONTROLLER;
    cmd += " ";
    cmd += cmd_pairing_add;

    
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

void test_homekit_remove_additional_pairing(void) {
    
    //python3 -m homekit.get_accessories -f ${PAIRINGDATAFILE} -a ${ALIAS} -o json | python3 ~/Development/Homekit/utils/accessory_validate/accval.py
    std::string cmd = "python3 -m homekit.remove_pairing";        
    cmd += " -f ";
    cmd += PAIRINGDATAFILE;
    cmd += " -a ";
    cmd += ALIAS_ADDITIONAL_CONTROLLER;    
    
#if TEST_SHOW_CMD
    std::cout << "cmd: " << cmd << std::endl;    
#endif

    std::string result = exec( cmd.c_str() );
    
    char cmpstr[256];
    sprintf(cmpstr, "Pairing for \"%s\" was removed.\n", ALIAS_ADDITIONAL_CONTROLLER);

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


#if TEST_KEEP_PAIRING
#else
    _setUp();
#endif
    UNITY_BEGIN();
    

    for (int i=0; i < ITERATIONS; i ++){

        std::cout << "Iteration " << (i + 1) << " of " << ITERATIONS << ":" << std::endl;    

        // RUN_TEST(test_homekit_identify_unpaired);
        

        // 
        // Pair
        // 
#if TEST_KEEP_PAIRING
#else        
        RUN_TEST(test_homekit_pair);
#endif

        RUN_TEST(test_homekit_identify_paired);
                
        RUN_TEST(test_homekit_get_accessories);
        
        RUN_TEST(test_homekit_get_characteristics);
        

        RUN_TEST(test_homekit_list_pairings);
        

        // 
        // FakeGato
        // 
        RUN_TEST(test_homekit_get_characteristics_fg_entry_count);        
        RUN_TEST(test_homekit_put_characteristics_fg_address_1);
        RUN_TEST(test_homekit_get_accessories);
        RUN_TEST(test_homekit_get_characteristics_fg_history);

        
        
        // RUN_TEST(test_homekit_put_characteristics_fg_address_17);
        // RUN_TEST(test_homekit_get_accessories);
        // RUN_TEST(test_homekit_get_characteristics_fg_history);



        // RUN_TEST(test_homekit_put_characteristics_fg_address_33);
        // RUN_TEST(test_homekit_get_accessories);
        // RUN_TEST(test_homekit_get_characteristics_fg_history);
        

        // 
        // Additional Pairings
        // 
        RUN_TEST(test_homekit_prepare_add_remote_pairing);
        RUN_TEST(test_homekit_add_additional_pairing);
        RUN_TEST(test_homekit_finish_add_remote_pairing);


        RUN_TEST(test_homekit_get_accessories_additional);
        RUN_TEST(test_homekit_list_pairings);
        

        // Remove additional pairing
        RUN_TEST(test_homekit_remove_additional_pairing);


        RUN_TEST(test_homekit_list_pairings);
        RUN_TEST(test_homekit_get_accessories);


        
#if TEST_KEEP_PAIRING
#else        
        // Remove pairing
        RUN_TEST(test_homekit_remove_pairing);
#endif

        std::cout << "\n\n" << std::endl;
    }

    UNITY_END();
#if TEST_KEEP_PAIRING
#else
    _tearDown();
#endif

    return 0;
}
