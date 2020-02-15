
import json
import re
from time import gmtime, strftime


first_cap_re = re.compile('(.)([A-Z][a-z]+)')
all_cap_re = re.compile('([a-z0-9])([A-Z])')
def convert(name):
    s1 = first_cap_re.sub(r'\1_\2', name)
    return all_cap_re.sub(r'\1_\2', s1).upper()

def sort_list(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using second element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: x[0]) 
    return sub_li 


def sort_list_number(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using second element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: int(x[1], 16)) 
    return sub_li 

def loadServices(filename):
    chars = []
    with open(filename) as json_file:  
        output_json = json.load(json_file)
        for majorkey, subdict in output_json.iteritems():
            
            name_org = majorkey.encode("UTF-8")
            uuid = subdict["UUID"].encode("UTF-8")

            name = convert(name_org)
            name.replace(".", "_")

            name = "HAP_SERVICE_" + name

            uid = "0x" + uuid.split("-")[0].lstrip("0")
            
            #print(name, uid)
            chars.append( (name, uid, name_org) )
           
        chars = sort_list(chars)
        return chars   

chars = loadServices("services.json")   

print("//")
print("// HAPServices.hpp")
print("// Homekit")
print("//")
print("//  Generated on: " + strftime("%d.%m.%Y", gmtime()));
print("//")
print("")
print("#ifndef HAPSERVICES_HPP_")
print("#define HAPSERVICES_HPP_")
print("")
print("#include <Arduino.h>")
print("")
print("typedef enum {")
for c in chars:        
    print("    {:<60}= {:<8}  //    ").format(c[0], c[1] + "," )
print("} HAP_SERVICE;")    
print("")
print("")

print("inline String serviceName(int type){")
print("    switch(type) {")
for c in chars: 
    print("        case {:<60} // {:<8} == {:>5}".format(c[0] + ":", c[1], int(c[1], 16)));
    print("            return \"" + c[2] + "\";")
print("        default:")    
print("            return \"\";");   
print("    }")
print("}")
print("")
print("#endif /* HAPSERVICES_HPP_ */")
