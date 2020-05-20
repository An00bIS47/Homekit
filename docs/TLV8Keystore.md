

# Key/Truststore TLV8 Structure

## General:

This describes the structure for exchanging a key or truststore. 
The data structure is based upon TLV8 which allows a predifened data structure which can easily be parsed. 

A signature is integrated to verify the integrity of the data. 

Optionally the data structure can be encrypted. This is currently not implemented as it is used as truststore only.


## Key/Truststore Data Types

### Root CA 									(0x00 - 0x09)
* 0x00 : Root CA 								
* 0x01 : Root CA Signature Public Key

### Device 										(0x10 - 0x19)
* 0x10 : Device Client Cert 

* 0x11 : Device Private Key 					<- Must be hardcoded into firmware or Keystore!
* 0x12 : Device Public Key 						<- Must be hardcoded into firmware or Keystore!

* 0x13 : Device Webserver Cert 

### Update Server 								(0x20 - 0x29)
* 0x20 : Update Server Cert

### Plugins 									(0x30 - 0x39)
* 0x30 : Plugin Cert / Public Key
...
* 0x39 : Plugin Cert / Public Key


### ContainerId 								(0xFD)
* 0xFD : Container Id 							<- uint8_t (0 ... 255)

### Signature 									(0xFE)
* 0xFE : Signature

## ENUM 
```python
### TLV8 Data Types
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
```


## Key/Truststore Structure
```
┌─────────────────────────────────────────┐
│ ┌────┬───┬────────────────────────────┐ │
│ │0x01│255│ABCD...                     │ │
│ ├────┼───┼────────────────────────────┤ │
│ │0x01│255│EF12...                     │ │ 
│ ├────┼───┼────────────────────────────┤ │
│ │0x01│255│3456...                     │ │
│ ├────┼───┼────────────────────────────┤ │
│ │0x02│255│ABCD...                     │ │
│ ├────┼───┼────────────────────────────┤ │
│ │0x02│255│EF12...                     │ │
│ ├────┼───┼────────────────────────────┤ │
│ │ .. │...│...                         │ │
│ └────┴───┴────────────────────────────┘ │ 
│        ┌────────┐     ┌────────┐        │
│   └─>  │ SHA256 │ ──> │  Sign  │  ──┐   │
│        └────────┘     └────────┘    v   │
│ ┌────┬───┬────────────────────────────┐ │
│ │254 │255│ABCD...                     │ │
│ ├────┼───┼────────────────────────────┤ │
│ │254 │ 1 │AB                          │ │
│ └────┴───┴────────────────────────────┘ │
└─────────────────────────────────────────┘
Optional:
	  ┌─────────┐
 └─>  │ Encrypt │
      └─────────┘
```

## Create partition csv

key,type,encoding,value     <-- column header
namespace_name,namespace,,  <-- First entry should be of type "namespace"
isValid,data,u8,1			<-- isValid true for initial truststore
...							<-- TLV Key/Truststore structure as hex2bin


## Create Key/Truststore binary
```
python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate truststore_part.csv truststore.bin 0x8000
``` 


## Flash Key/Truststore binary
```
esptool.py --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 2000000 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xC95000 ./truststore.bin
```