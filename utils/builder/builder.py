#!/usr/bin/python
#-*- coding:utf-8 -*-
import re
import subprocess
import string
import random
import os
import shutil
import sys

def sh(cmd, input=""):
    rst = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, input=input.encode("utf-8"))
    assert rst.returncode == 0, rst.stderr.decode("utf-8")
    return rst.stdout.decode("utf-8")

def execute(command):
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    # Poll process for new output until finished
    output = ""
    while True:
        nextline = process.stdout.readline().decode("utf-8")
        if nextline == '' and process.poll() is not None:
            break

        output += nextline
        sys.stdout.write(nextline)
        sys.stdout.flush()

    #output = process.communicate()[0]
    exitCode = process.returncode
    return output, exitCode


def which(toolname):
    return sh("which " + toolname )[:-1]

def get_device_info(config):
    cmd = config["esptool"] + " flash_id"
    result = sh(cmd)
    #print(result)
    matchMAC = re.search(r'MAC:\s([A-Za-z0-9]{2}:[A-Za-z0-9]{2}:[A-Za-z0-9]{2}:[A-Za-z0-9]{2}:[A-Za-z0-9]{2}:[A-Za-z0-9]{2})', result)
    matchFlashSize = re.search(r'Detected flash size:\s(.*)([A-Z]{2})', result)
    mac = matchMAC.group(1).upper()
    flash_size = int(matchFlashSize.group(1))
    flash_size_unit = matchFlashSize.group(2)
    return mac, flash_size, flash_size_unit
    
def generate_random_id(size=4, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))

def generate_pincode(n=8):
    range_start = 10**(n-1)
    range_end = (10**n)-1
    pin = str(random.randint(range_start, range_end))
    return pin[:3] + '-' + pin[3:5] + "-" + pin[5:]

def generate_qr_code(config, mac, pincode, setup_id, category):
    cmd = config["python"] + " ../QRCode/homekit_qrcode.py -m " + mac + " -p " + pincode + " -s " + setup_id + " -c " + str(category) + " -o ./qrcode"
    sh(cmd)

def generate_prov_qr_code(config, name, pop, transport="ble"):
    cmd = config["python"] + " ../ESP32_Provisioning_QRCode/generateProvQR.py --transport " + transport + " --name " + name + " --pop " + pop + " --file ./" + name + ".png"
    sh(cmd)

def clone_git_repo(url, path, commit=None):
    print(sh("git clone " + url + " " + path) )
    cwd = os.getcwd()
    os.chdir(path)
    if commit != None:              
        print(sh("git checkout " + commit))
    print(sh("git submodule update --init --recursive"))
    os.chdir(cwd)

def get_env_variable(var):
    return os.getenv(var)   

def get_subfolders(dirname):
    return [f.path for f in os.scandir(dirname) if f.is_dir()]

def create_component_mk(dir):
    mode = 'a' if os.path.exists(dir + "/component.mk") else 'w'
    if os.path.isdir(dir + '/src'):        
        with open(dir + "/component.mk", mode) as f:
            f.write("COMPONENT_SRCDIRS:=./src\n")
            f.write("COMPONENT_ADD_INCLUDEDIRS:=./src\n")
    else:
        with open(dir + "/component.mk", mode) as f:
            f.write("COMPONENT_SRCDIRS:=.\n")
            f.write("COMPONENT_ADD_INCLUDEDIRS:=.\n")
    
def check_component_mk(project_dir):
    subfolders = get_subfolders(project_dir)

    for subf in subfolders:
        #print(subf)
        if not os.path.isfile(subf + '/component.mk'):                              
            print("Creating component.mk in " + subf)
            create_component_mk(subf)


def creating_partitions_csv(project_dir, size):
    mode = 'a' if os.path.exists(project_dir + "/component.mk") else 'w'
    with open(project_dir + "/partitions.csv", mode) as f:
        f.write("## Automatically created by Homekit builder.py\n")
        f.write("## Name,    Type,       SubType,    Offset,     Size,   Flags\n")        
        if size == 16:        
            f.write("otadata,    data,      ota,        0xD000,     8K,\n")
            f.write("phy_init,  data,       phy,        0xF000,     4K,\n")
            f.write("ota_0,      app,       ota_0,      0x10000,    6384K,\n")
            f.write("ota_1,      app,       ota_1,      ,           6384K,\n")
            f.write("nvs_key,    data,      nvs_keys,   ,           4K,\n")
            f.write("nvs,        data,      nvs,        ,           32K,\n")
            f.write("keystore_0, data,      nvs,        0xC95000,   32K,\n")
            f.write("keystore_1, data,      nvs,        0xC9D000,   32K,\n")
            f.write("spiffs,     data,      spiffs,     ,           2048K,\n")

            return "0xC95000", "0xC9D000"
        elif size == 8:        
            print("to do")
        elif size == 4:        
            print("to do")
        else:
            print("Not supported")

def get_last_mac_bytes(mac):
    mac = mac.replace(":", "")
    return mac[-6:]


def create_certificate(name, path, domain="local"):
    if not os.path.exists(path + "/" + name):
        os.mkdir(path + "/" + name)
        sh("openssl req -out " + path + "/" + name + "/" + name + ".csr -new -newkey rsa:2048 -nodes -keyout " + path + "/" + name + "/" + name + ".privatekey -subj /C=DE/ST=/L=Munich/O=ACME/OU=/CN=" + name + "." + domain)
        sh("openssl rsa -in " + path + "/" + name + "/" + name + ".privatekey -pubout -out " + path + "/" + name + "/" + name + ".publickey")
    return path + "/" + name + "/" + name + ".csr", path + "/" + name + "/" + name + ".privatekey", path + "/" + name + "/" + name + ".publickey"

def compile_pk_sign_and_verify(config):
    cwd = os.getcwd()
    os.chdir(config["project_dir"] + "/utils/TLV8Keystore")
    sh("make all")
    os.chdir(cwd)

def create_truststore_keys(name, path):
    if not os.path.exists(path):
        os.mkdir(path)

    if not os.path.exists(path + "/" + name + ".privatekey") and not os.path.exists(path + "/" + name + ".publickey"):        
        sh("openssl ecparam -name prime256v1 -genkey -noout -out " + path + "/" + name + ".privatekey")
        sh("openssl ec -in " + path + "/" + name + ".privatekey -pubout -out " + path + "/" + name + ".publickey")
    return path + "/" + name + ".privatekey", path + "/" + name + ".publickey"

def create_truststore(config, device_name, signingKey, verifyKey):
    cwd = os.getcwd()
    os.chdir(config["project_dir"] + "/utils/TLV8Keystore")
    print(sh("./makeKeystore.sh " + device_name + " " + str(config["truststore"]["containerId"]) + " " + config["domain"] + " " + config["certificate_dir"] + " " + signingKey + " " + verifyKey + " " + config["truststore"]["rootCA"]))
    os.chdir(cwd)
    return config["project_dir"] + "/utils/TLV8Keystore" + "/" + device_name + "/" + "truststore.bin"


def compile_homekit(config):
    cwd = os.getcwd()
    os.chdir(config["project_dir"])
    #print(sh("make -j8 app"))
    output, exitcode =execute("make -j8 app")
    matchObj = re.search(r'App built. Default flash app command is:\n(.*)', output)
    flash_command = matchObj.group(1)    
    os.chdir(cwd)

    return flash_command, exitcode


def flash_homekit(config, command):
    cwd = os.getcwd()
    os.chdir(config["project_dir"])
    output, exitcode =execute(command)
    os.chdir(cwd)
    return output, exitcode


def make_component_mk(config, pincode, setup_id, pop):
    """
        #
        # General compiler flags
        #    
        CXXFLAGS += -DARDUINOJSON_USE_LONG_LONG=1   # allow 64bit integer in ArduinoJson    
        CXXFLAGS += -DESP32 -DARDUINO_ARCH_ESP32

        #
        # Adafruit feather specific
        #    
        CXXFLAGS += -DARDUINO_FEATHER_ESP32
        CXXFLAGS += -I$(PROJECT_PATH)/components/arduino/variants/feather_esp32/
        

        #
        # Source files
        #
        COMPONENT_SRCDIRS += HAP 
        COMPONENT_ADD_INCLUDEDIRS += HAP

        COMPONENT_SRCDIRS += Crypto
        COMPONENT_ADD_INCLUDEDIRS += Crypto
        

        #
        # Plugins
        #
        UNAME_S := $(shell uname -s)
        ifeq ($(UNAME_S),Linux)
            COMPONENT_SRCDIRS += $(shell find $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
            COMPONENT_ADD_INCLUDEDIRS += $(shell find $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
        endif
        ifeq ($(UNAME_S),Darwin)
            COMPONENT_SRCDIRS += $(shell gfind $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
            COMPONENT_ADD_INCLUDEDIRS += $(shell gfind $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ')
        endif

        
        #
        # Private Key
        #    
        COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/build/device.privatekey"

        #
        # Website incl. Font, css. images and javascripts
        #
        COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/index.html
        COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/qrcode_font.css
        COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/qrcode_container.svg


        #
        # Homekit Variables
        #
        CXXFLAGS += -DHAP_PIN_CODE=
        CXXFLAGS += -DHAP_SETUP_ID=
        CXXFLAGS += -DHAP_PROVISIONING_POP=
    """
    if not os.path.exists(config["project_dir"] + "/src/component.mk"):
        with open(config["project_dir"] + "/src/component.mk", "w") as f:
            f.write("# \n")
            f.write("# General\n")
            f.write("# \n")
            f.write("CXXFLAGS += -DARDUINOJSON_USE_LONG_LONG=1   # allow 64bit integer in ArduinoJson     \n")
            f.write("CXXFLAGS += -DESP32 -DARDUINO_ARCH_ESP32 \n")
            f.write(" \n")
            f.write("# \n")
            f.write("# Adafruit feather specific \n")
            f.write("# \n")
            f.write("CXXFLAGS += -DARDUINO_FEATHER_ESP32 \n")
            f.write("CXXFLAGS += -I$(PROJECT_PATH)/components/arduino/variants/feather_esp32/ \n")
            f.write(" \n")
            f.write("# \n")
            f.write("# Source files \n")
            f.write("# \n")
            f.write("COMPONENT_SRCDIRS += HAP  \n")
            f.write("COMPONENT_ADD_INCLUDEDIRS += HAP \n")
            f.write(" \n")
            f.write("COMPONENT_SRCDIRS += Crypto \n")
            f.write("COMPONENT_ADD_INCLUDEDIRS += Crypto \n")
            f.write(" \n")
            f.write(" \n")
            f.write("# \n")
            f.write("# Plugins \n")
            f.write("# \n")
            f.write("UNAME_S := $(shell uname -s) \n")
            f.write("ifeq ($(UNAME_S),Linux) \n")
            f.write("    COMPONENT_SRCDIRS += $(shell find $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ') \n")
            f.write("    COMPONENT_ADD_INCLUDEDIRS += $(shell find $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ') \n")
            f.write("endif \n")
            f.write("ifeq ($(UNAME_S),Darwin) \n")
            f.write("    COMPONENT_SRCDIRS += $(shell gfind $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ') \n")
            f.write("    COMPONENT_ADD_INCLUDEDIRS += $(shell gfind $(COMPONENT_PATH)/HAP/plugins -type d -printf 'HAP/plugins/%P ') \n")
            f.write("endif \n")
            f.write(" \n")
            f.write("# \n")
            f.write("# Private Key \n")
            f.write("# \n")
            f.write("COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/build/device.privatekey \n")
            f.write(" \n")            
            f.write("# \n")
            f.write("# Website\n")
            f.write("# \n")
            f.write("COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/index.html \n")
            f.write("COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/qrcode_font.css \n")
            f.write("COMPONENT_EMBED_TXTFILES += $(PROJECT_PATH)/www/qrcode_container.svg \n")  
            f.write(" \n")  
            f.write("# \n")  
            f.write("# Homekit specific \n")  
            f.write("# \n")
            f.write("CXXFLAGS += -DHAP_PIN_CODE='\"" + pincode + "\"'\n")
            f.write("CXXFLAGS += -DHAP_SETUP_ID='\"" + setup_id + "\"'\n")
            f.write("CXXFLAGS += -DHAP_PROVISIONING_POP='\"" + pop + "\"'\n")
            f.write(" \n")  



config = {
    "python": "python3",
    "esptool": "/usr/local/bin/esptool.py",    
    "device_prefix": "esp32-",    
    "domain": "local",
    "repos": {
        "esp-idf": {
            "url": "https://github.com/espressif/esp-idf.git",
            "commit": "48ea44f3d1e4b5763a2d0b65bc5c56a84690e910"
        },
        "arduino-esp32": {
            "url": "",
            "commit": None
        }
    },
    "truststore": {
        "containerId": 1,
        "rootCA": "../../certs/rootCA/ACME_CA.cer"
    }
}   

cwd = os.getcwd()
config["esptool"] = which("esptool.py")
config["idf_path"] = get_env_variable("IDF_PATH")

config["project_dir"] = os.path.abspath(os.path.join("..", os.pardir));
config["build_dir"] = config["project_dir"] + "/build"
config["certificate_dir"] = config["project_dir"] + "/certs"

print("\nConfiguration used: ")
print("   current path:   " + cwd)
print("   idf_path:       " + config["idf_path"])
print("   project dir:    " + config["project_dir"])
print("   build dir:      " + config["build_dir"])
print()
print("   esptool:        " + config["esptool"])
print("   python:         " + config["python"])
print()
print("   device prefix:  " + config["device_prefix"])
print("   esp-idf url:    " + config["repos"]["esp-idf"]["url"])
print("   esp-idf commit: " + config["repos"]["esp-idf"]["commit"])
print()

print("Cloning esp-idf ...")
clone_git_repo(config["repos"]["esp-idf"]["url"], config["idf_path"], config["repos"]["esp-idf"]["commit"])
print(" ✓ OK")

print("Checking component.mk files ...", end="")
check_component_mk(config["project_dir"] + "/components")
print(" ✓ OK")


print("Creating KEYSTORE private and public keys ...", end="")   
sign_priv_key, sign_pub_key = create_truststore_keys("keystore", config["certificate_dir"] + "/TLV8Keystore")
print(" ✓ OK")


print("Compiling pk_sign and pk_verify", end="")
compile_pk_sign_and_verify(config)
print(" ✓ OK")

print("Reading connected ESP32 device ...")
mac, flash_size, flash_size_unit = get_device_info(config)
#print(mac, flash_size, flash_size_unit)

category = 2 # 2 = bridge
setup_id = generate_random_id(4)
pincode = generate_pincode()
pop = generate_random_id(8)
# print(setup_id, pincode)
device_name = config["device_prefix"] + get_last_mac_bytes(mac)

print("\nBuilding Homekit for ")
print("   MAC:                {}".format(mac))
print("   device name         " + device_name)
print("   Flash size:         {} {}".format(flash_size,flash_size_unit))
print("   Setup ID:           {}".format(setup_id))
print("   Pin Code:           {}".format(pincode))
print("   Proof of Posession: {}".format(pop))
print("   Category:           {}".format(category))
print()


print("Creating paritions.csv ...", end="")
truststore_1_addr, truststore_2_addr = creating_partitions_csv(config["project_dir"], flash_size)
print(" ✓ OK")


print("Creating DEVICE private key and csr ...", end="")
cer_name = config["certificate_dir"] + "/" + device_name + "/" + device_name + "." + config["domain"] + ".cer"
priv_key = config["certificate_dir"] + "/" + device_name + "/" + device_name + ".privatekey"
if not os.path.exists(cer_name):    
    csr, priv_key, pub_key = create_certificate(device_name, config["certificate_dir"], config["domain"])
    print(" ✓ OK")   

    print("Please sign your csr {} and store the certifcate as {}!".format(csr, cer_name))
    print("Press ENTER when you are done!")
    input()
else:
    print("SKIPPED")

print("Copying " + priv_key + " to build dir ...", end="")
shutil.copy(priv_key, config["build_dir"] + "/device.privatekey")
print(" ✓ OK")


print("Creating Truststore ...", end="")
truststore = create_truststore(config, device_name, sign_priv_key, sign_pub_key)
print(" ✓ OK")

print("Building Homekit component.mk ...", end="")
make_component_mk(config, pincode, setup_id, pop)
print(" ✓ OK")


print("Compiling Homekit")
flash_command, exitcode = compile_homekit(config)
print(exitcode)

if exitcode == 0:
    print("Building flash command ...", end="")
    flash_command = flash_command + " " + truststore_1_addr + " " + truststore + " " + truststore_2_addr + " " + config["project_dir"] + "/truststore_empty.bin"    
    print(" ✓ OK")
    print(flash_command)

    print("Flashing Homekit ...")
    flash_homekit(config, flash_command)


    print("Creating Homekit QR code ...", end="")
    generate_qr_code(config, mac, pincode, setup_id, category)
    print(" ✓ OK")

    print("Creating provisioning QR code ...", end="")
    generate_prov_qr_code(config, "PROV_" + get_last_mac_bytes(mac), pop, "ble")
    print(" ✓ OK")


print("Removing private key from build dir ...", end="")
if os.path.exists(config["build_dir"] + "/device.privatekey"):    
    os.remove(config["build_dir"] + "/device.privatekey")
    print(" ✓ OK")
else:
    print("SKIPPED")