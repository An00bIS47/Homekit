
import re
import sys
import os
import io
import hashlib
import argparse
import coloredlogs, logging

class FirmwareParser():
    def __init__(self, firmware_file=None):
        self.firmware_file = firmware_file
        self.firmware_binary = None
        self.firmware_size = None
        
        self.version = None
        self.brand = None
        self.feature_rev = None
        self.feature_rev_binary = None
        self.md5 = None


    def readMetadata(self, filename=None):
        regex_homekit = re.compile(b"\x25\x48\x4f\x4d\x45\x4b\x49\x54\x5f\x45\x53\x50\x33\x32\x5f\x46\x57\x25")
        regex_name = re.compile(b"\xbf\x84\xe4\x13\x54(.+)\x93\x44\x6b\xa7\x75")
        regex_version = re.compile(b"\x6a\x3f\x3e\x0e\xe1(.+)\xb0\x30\x48\xd4\x1a")
        regex_feature_rev = re.compile(b"\x6a\x3f\x3e\x0e\xe2(.+)\xb0\x30\x48\xd4\x1b")
        regex_brand = re.compile(b"\xfb\x2a\xf5\x68\xc0(.+)\x6e\x2f\x0f\xeb\x2d")

        try:    
                firmware_file = open(self.firmware_file, "rb")
        except Exception as err:
                #logging.error(" * {0}".format(err.strerror))
                #sys.exit(2)
                return False

        self.firmware_binary = firmware_file.read()
        firmware_file.close()

        regex_name_result = regex_name.search(self.firmware_binary)
        regex_version_result = regex_version.search(self.firmware_binary)
        regex_revFeature_result = regex_feature_rev.search(self.firmware_binary)

        if not regex_homekit.search(self.firmware_binary) or not regex_name_result or not regex_version_result:
                logging.warning(" * Not a valid Homekit firmware detected")
                #sys.exit(3)
                return False


        regex_brand_result = regex_brand.search(self.firmware_binary)

        name = regex_name_result.group(1).decode()
        version = regex_version_result.group(1).decode()
        brand = regex_brand_result.group(1).decode() if regex_brand_result else "unset (default is DEFAULT)"

        feature_rev_binary = regex_revFeature_result.group(1).decode()
        feature_revDecimal = int(feature_rev_binary, 2)
        feature_revHex = format(feature_revDecimal, 'x')

        self.md5 = self._getMD5(self.firmware_binary)
        self.firmware_size = self._getSize(self.firmware_file);

        self.name = name
        self.version = version
        self.brand = brand
        self.feature_rev_binary = feature_rev_binary
        self.feature_rev = feature_revHex;

        logging.debug(" * {:20} {}".format("name:", self.name))
        logging.debug(" * {:20} {}".format("brand:", self.brand))
        logging.debug(" * {:20} {}".format("version:", self.version))
        logging.debug(" * {:20} {}".format("feature_rev binary:", self.feature_rev_binary))
        logging.debug(" * {:20} {}".format("feature_rev:", self.feature_rev))
        #logging.debug(" * {}".format(self))
        return True


    def toJson(self):
        data = {}
        data["name"] = self.name
        data["version"] = self.version
#        data["feature_rev_binary"] = self.feature_rev_binary
        data["feature_rev"] = self.feature_rev        
        data["brand"] = self.brand
        data["md5"] = self.md5
        #logging.debug(data)
        return data

    def renameFile(self, storage=None):
        str_version = self.version.replace(".", "_")
        new_filename = str(self.name) + "_" + str_version + "_" + str(self.feature_rev) + ".bin"

        new_path = ""
        if storage is not None:            
            new_path = storage + "/" + new_filename
        else:
            new_path = new_filename      

        logging.debug(" * Renaming {} to {}".format(self.firmware_file, new_path))                
        os.rename(self.firmware_file, new_path)
        self.firmware_file = new_path


    def __str__(self):
        res = ""
        res += "{:20} {}\n".format("name:", self.name)
        res += "{:20} {}\n".format("brand:", self.brand)
        res += "{:20} {}\n".format("version:", self.version)
        res += "{:20} {}\n".format("feature_rev binary:", self.feature_rev_binary)
        res += "{:20} {}\n".format("feature_rev:", self.feature_rev)
        res += "{:20} {}\n".format("size:", self.firmware_size)
        res += "{:20} {}\n".format("file:", self.firmware_file)
        res += "{:20} {}".format("md5:", self.md5)
        return res

    def _getSize(self, filename=None):
        if filename is None:
            filename = self.firmware_file
        st = os.stat(filename)
        
        self.firmware_size = st.st_size
        logging.debug(" * {:20} {}".format("size:", self.firmware_size))
        return self.firmware_size

    def _getMD5(self, binary=None):
        if binary is None:
            binary = self.firmware_binary

        self.md5 = hashlib.md5(binary).hexdigest()
        logging.debug(" * {:20} {}".format("md5:", self.md5))
        return self.md5

if __name__ == '__main__':



    parser = argparse.ArgumentParser(description='Homekit Firmware Parser')    #parser.add_argument("echo", help="echo the string you use here")
    parser.add_argument('firmware', help='Location of the firmware file', default="Homekit.bin")
    parser.add_argument('-r', '--rename', help='Auto rename the firmware file', default=False, action='store_true')
    parser.add_argument('-s', '--storage', help='Path to firmware storage', default=None, required=False)    
    parser.add_argument('-d', '--debug', help='Debug mode', default=False, action='store_true')
    args = parser.parse_args()

    if args.debug:        
        coloredlogs.install(fmt='%(asctime)s [ %(levelname)-7s ] %(message)s', level='DEBUG')

    fparser = FirmwareParser(args.firmware)
    
    if fparser.readMetadata() == True:
        print(fparser)
        if args.rename:
            fparser.renameFile(storage=args.storage)
