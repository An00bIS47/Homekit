import tlv8
from enum import Enum
import argparse




class HAP_KEYSTORE_TYPE(Enum):
    HAP_KEYSTORE_TYPE_ROOT_CA                       = 0x00
    HAP_KEYSTORE_TYPE_ROOT_CA_PUBLIC_KEY_SIGNATURE  = 0x01 

    HAP_KEYSTORE_TYPE_DEVICE_CLIENT_CERT            = 0x10

    HAP_KEYSTORE_TYPE_DEVICE_PRIVATE_KEY            = 0x11      # Keystore only
    HAP_KEYSTORE_TYPE_DEVICE_PUBLIC_KEY             = 0x12      # Keystore only
    
    HAP_KEYSTORE_TYPE_DEVICE_WEBSERVER_CERT         = 0x13       

    HAP_KEYSTORE_TYPE_UPDATE_SERVER_CERT            = 0x20

    HAP_KEYSTORE_TYPE_PLUGIN_0                      = 0x30
    HAP_KEYSTORE_TYPE_PLUGIN_1                      = 0x31
    HAP_KEYSTORE_TYPE_PLUGIN_2                      = 0x32
    HAP_KEYSTORE_TYPE_PLUGIN_3                      = 0x33
    HAP_KEYSTORE_TYPE_PLUGIN_4                      = 0x34
    HAP_KEYSTORE_TYPE_PLUGIN_5                      = 0x35
    HAP_KEYSTORE_TYPE_PLUGIN_6                      = 0x36
    HAP_KEYSTORE_TYPE_PLUGIN_7                      = 0x37
    HAP_KEYSTORE_TYPE_PLUGIN_8                      = 0x38
    HAP_KEYSTORE_TYPE_PLUGIN_9                      = 0x39

    HAP_KEYSTORE_TYPE_CONTAINER_ID                  = 0xFD
    HAP_KEYSTORE_TYPE_SIGNATURE                     = 0xFE


def readBinaryFile(filename):
    with open(filename, mode='rb') as file: # b is important -> binary
        return file.read()


def writeBinaryFile(filename, content):
    with open(filename, mode='wb') as file: # b is important -> binary
        return file.write(content)


def signAndHashData(privateKey, bytes_data):
    with open(privateKey, mode='rb') as f:
        sk = SigningKey.from_pem(f.read(), hashlib.sha256)

    bytes_data = sha256(bytes_data)
    sig = sk.sign_deterministic(bytes_data, sigencode=sigencode_der)
    return sig

def verifySignature(publicKey, signature, bytes_data):
    with open(publicKey, mode='rb') as f:
        vk = VerifyingKey.from_pem(f.read())

    return vk.verify(signature, bytes_data, hashlib.sha256, sigdecode=sigdecode_der)


def buildTLV8(serverCert, verifyKey, rootCA):
    structure = [
        tlv8.Entry( HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_CONTAINER_ID.value,                 CONTAINER_VERSION ),
        tlv8.Entry( HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_ROOT_CA.value,                      readBinaryFile(rootCA) ),
        tlv8.Entry( HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_ROOT_CA_PUBLIC_KEY_SIGNATURE.value, readBinaryFile(verifyKey) ),
        
        tlv8.Entry( HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_DEVICE_WEBSERVER_CERT.value,        readBinaryFile(serverCert) )#,   
        #tlv8.Entry( HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_DEVICE_PUBLIC_KEY.value,           readBinaryFile("./certs/devices/esp32-cafeec/esp32-cafeec.publicKey.cer") )
    ]

    bytes_data = tlv8.encode(structure)
    #print(bytes_data)
    return bytes_data, structure




def makeKeystoreStructure(serverCert, verifyKey, rootCA):
    
    bytes_data, structure = buildTLV8(serverCert, verifyKey, rootCA)  
    #writeBinaryFile("data.tlv8", bytes_data)    
    return bytes_data, structure

def makeKeystorePre(bytes_data):
    writeBinaryFile("data.tlv8.pre", bytes_data)   
    return bytes_data


def makePartitionCsv(bytes_data, csvFile):  
    dict = {}
    containerId = ""
    for entry in tlv8.decode(bytes_data):
        if entry.type_id ==HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_CONTAINER_ID.value:
             containerId = entry.data.__str__()

        if entry.type_id in dict:
            dict[entry.type_id] = dict[entry.type_id] + entry.data
        else:
            dict[entry.type_id] = entry.data            

    lines = []
    lines.append("key,type,encoding,value")
    lines.append("keystore,namespace,,")
    lines.append("isValid,data,u8,1")

    for key, value in dict.items():
        if key is HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_SIGNATURE.value:
            pass
        elif key is HAP_KEYSTORE_TYPE.HAP_KEYSTORE_TYPE_CONTAINER_ID.value:
            lines.append("0x{:02X},data,u8,{}".format(key, int(value.hex(),16) ) )       
        else:   
            lines.append("0x{:02X},data,hex2bin,{}".format(key, value.hex() ) )       
            
    with open(csvFile, "w") as fp:
        for line in lines:
            fp.write("%s\n" % line)

    return containerId


parser = argparse.ArgumentParser(description='Make Key/Truststore ')
parser.add_argument('deviceId',                       
                       type=str,
                       help='The device id, e.g. esp32-CAFEEC')
parser.add_argument('-c', '--containerId',
                    type=int,
                    help='The container id')
parser.add_argument('-r', '--rootCA',
                    type=str,
                    help='The root CA certificate')
parser.add_argument('-s', '--serverCert',
                    type=str,
                    help='The path to the device server cert')
parser.add_argument('-v', '--verifyKey',
                    type=str,
                    help='The signing key')
args = parser.parse_args()


name = args.deviceId #"esp32-CAFEEC"
CONTAINER_VERSION = args.containerId


print("   verifyKey:     " + args.verifyKey)
print("   rootCA:        " + args.rootCA)
print("   serverCert:    " + args.serverCert)

print("Building TLV8 structure ...", end = "");
bytes_data, structure = makeKeystoreStructure(args.serverCert, args.verifyKey, args.rootCA)
print(" ✓ OK")

print("Building TLV8 pre ...", end = "");
bytes_data = makeKeystorePre(bytes_data)
print(" ✓ OK")

print("Creating truststore csv  ...", end = "");
containerId = makePartitionCsv(bytes_data, "./truststore.csv")
print(" ✓ OK - ID: " + containerId)



#python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate keystore_part.csv keystore.bin 0x8000 --version 2

#python2 /Users/michael/Development/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 2000000 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xd000 /Users/michael/Development/Homekit/build/ota_data_initial.bin 0x1000 /Users/michael/Development/Homekit/build/bootloader/bootloader.bin 0xf000 /Users/michael/Development/Homekit/build/phy_init_data.bin 0x10000 /Users/michael/Development/Homekit/build/Homekit.bin 0x8000 /Users/michael/Development/Homekit/build/partitions16.bin 0xC95000 /Users/michael/Development/TLV8Keystore/keystore.bin
