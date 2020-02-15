
headers = []
partitions = []


def readSizeValue(string):
    if string.startswith("0x"):
        return int(string, 16)
    elif string.endswith("K"):
        return int(string[:-1]) * 1024
    elif string.endswith("M"):
        return int(string[:-1]) * 1024 * 1024
    elif string == "":
        return 0

with open('partitions.csv') as fp:
    line = fp.readline()
    cnt = 1
    headerFound = False


    while line:
        
        line = line.strip()
        parts = line.split(",")
        # print(parts, len(parts))

        if len(parts) == 6:
            if not headerFound:
                if parts[0].startswith("# Name"):
                    parts[0] = "Name"
                    headers = parts
                    headerFound = True;
        
        
        if not line.startswith("#"):            
            #print("Line {}: {}".format(cnt, line.strip()))

             if len(parts) == 6:
                for i in range(0,5): 
                    parts[i] = parts[i].strip()

                partitions.append(list(parts))

        line = fp.readline()   
        cnt += 1

#print(headers)
#print(partitions)        

total_size = 0
for p in partitions:
    print(p)
    offset = readSizeValue(p[3])
    size = readSizeValue(p[4])
    total_size = total_size + size
    nextOffset = int(offset) + int(size)
    print(nextOffset, hex(nextOffset))

print(total_size)