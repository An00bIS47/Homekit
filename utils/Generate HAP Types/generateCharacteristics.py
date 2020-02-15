import json
import re
from time import gmtime, strftime


first_cap_re = re.compile('(.)([A-Z][a-z]+)')
all_cap_re = re.compile('([a-z0-9])([A-Z])')
def convert(name):
    s1 = first_cap_re.sub(r'\1_\2', name)
    return all_cap_re.sub(r'\1_\2', s1).upper()


def sort_list_number(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using second element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: int(x[1], 16)) 
    return sub_li 


def sort_list(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using second element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: x[0]) 
    return sub_li 


def loadCharacteristics(filename):
    chars = []

    with open(filename) as json_file:  
        output_json = json.load(json_file)

        for majorkey, subdict in output_json.iteritems():
            
            name_org = majorkey.encode("UTF-8")
            uuid = subdict["UUID"].encode("UTF-8")

            name = convert(name_org)
            name = name.replace(".", "_")

            uid = "0x" + uuid.split("-")[0].lstrip("0")
            
            name = "HAP_CHARACTERISTIC_" + name
            #print(name, uid)

            chars.append( (name, uid, name_org, subdict["Permissions"], subdict["Format"]) )
           
        #chars = sort_list_number(chars)
        chars = sort_list(chars)
        return chars


chars = loadCharacteristics("characteristics.json")

print("//")
print("// HAPCharacteristics.hpp")
print("// Homekit")
print("//")
print("//  Generated on: " + strftime("%d.%m.%Y", gmtime()));
print("//")
print("")
print("#ifndef HAPCHARACTERISTICS_HPP_")
print("#define HAPCHARACTERISTICS_HPP_")
print("")
print("#include <Arduino.h>")
print("")
print("typedef enum {")
for c in chars:        
    print("    {:<60}= {:<8}  //    {:<10}  {:<12}").format(c[0], c[1] + ",", c[4], '|'.join(c[3]))
print("} HAP_CHARACTERISTIC;")    
print("")
print("")

print("inline String characteristicsName(int type){")
print("    switch(type) {")
for c in chars: 
    print("        case {:<60} // {:<8} == {:>5}".format(c[0] + ":", c[1], int(c[1], 16)));
    print("            return \"" + c[2] + "\";")
print("        default:")    
print("            return \"\";");
print("    }")
print("}")
print("")
print("#endif /* HAPCHARACTERISTICS_HPP_ */")