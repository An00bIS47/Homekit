

baseUUID = "-6B66-4FFD-88CC-16A60B5C4E03"
baseName = "HAP_CUSTOM_"


customServices = {	
	"Fertility": 		1
}


customCharacteristics = {
	"FertilitySensor": 	1000,
	"Heartbeat": 		1001,
	"LastUpdate": 		1002
}

def generateUUID(dictionary, name):
	print("\n{}:".format(name))
	for k,v in dictionary.items():
		#print(k,v)
		print("{0:<20}: {1:0{2}X}{3}".format(k, v, 8, baseUUID))



def generateUUIDHeader(dictionary, typ):
	print("\n\n//\n// {}:\n//".format(typ))
	for k,v in dictionary.items():
		#print(k,v)
		define = "#define {}{}_{}".format(baseName, typ.upper(), k.upper())
		print("{:<55} {:0{}X}{}".format(define, v, 8, baseUUID))

generateUUID(customServices, "Services")
generateUUID(customCharacteristics, "Characteristics")

generateUUIDHeader(customServices, "Service")
generateUUIDHeader(customCharacteristics, "Characteristic")