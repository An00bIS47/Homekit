import json



def sort_list(sub_li): 
  
    # reverse = None (Sorts in Ascending order) 
    # key is set to sort using second element of  
    # sublist lambda has been used 
    sub_li.sort(key = lambda x: int(x[1], 16)) 
    return sub_li 


def loadCharacteristics(filename):
    chars = []
    with open(filename) as json_file:  
        output_json = json.load(json_file)
        for majorkey, subdict in output_json.iteritems():
            
            charType = majorkey.encode("UTF-8")
            uuid = subdict["UUID"].encode("UTF-8")

            name = "charType_" + charType[:1].lower() + charType[1:]

            uid = "0x" + uuid.split("-")[0].lstrip("0")
            
            #print(name, uid)
            chars.append( (name, uid) )
           
        chars = sort_list(chars)
        return chars

def loadServices(filename):
    chars = []
    with open(filename) as json_file:  
        output_json = json.load(json_file)
        for majorkey, subdict in output_json.iteritems():
            
            charType = majorkey.encode("UTF-8")
            uuid = subdict["UUID"].encode("UTF-8")

            name = "serviceType_" + charType[:1].lower() + charType[1:]

            uid = "0x" + uuid.split("-")[0].lstrip("0")
            
            #print(name, uid)
            chars.append( (name, uid) )
           
        chars = sort_list(chars)
        return chars        


def printCharacteristics(filename_header, chars, printOld = False):
    chars_header = [] 
    with open(filename_header, "r") as header_file:       
        printLine = False;
        for line in header_file:
            line = line.rstrip('\n')
            if line.startswith("// #pragma - Characteristics start"):
                printLine = True
            else:    
                if printLine:
                    if "=" in line:
                        ctype = line.split("=")[0].strip()
                        uuid = line.split("=")[1].replace(",","").strip()

                        chars_header.append( ( ctype, uuid ) )
                        if printOld:
                            print("    {:<48}= {},").format(ctype, uuid)

            if line.startswith("// #pragma - Characteristics end"):
                break

    for c in chars:    
        if not any(e[1] == c[1] for e in chars_header):
            print("    {:<48}= {},").format(c[0], c[1])



def printServices(filename_header, chars, printOld = False):
    chars_header = [] 
    with open(filename_header, "r") as header_file:       
        printLine = False;
        for line in header_file:
            line = line.rstrip('\n')
            if line.startswith("// #pragma - Services start"):
                printLine = True
            else:    
                if printLine:
                    if "=" in line:
                        ctype = line.split("=")[0].strip()
                        uuid = line.split("=")[1].replace(",","").strip()

                        chars_header.append( ( ctype, uuid ) )
                        if printOld:
                            print("    {:<48}= {},").format(ctype, uuid)

            if line.startswith("// #pragma - Services end"):
                break

    for c in chars:    
        if not any(e[1] == c[1] for e in chars_header):
            print("    {:<48}= {},").format(c[0], c[1])


chars = loadCharacteristics("characteristics.json")  
printCharacteristics("HAPCharacteristics.hpp", chars, True)

print("\n\n\n\n\n\n\n")

services = loadServices("services.json")
printServices("HAPCharacteristics.hpp", services, False)
