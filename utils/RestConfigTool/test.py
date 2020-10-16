import requests
import json

print("Please enter the house address in binary form:")
houseAddress = input()

print("Please enter the device address in binary form:")
deviceAddress = input()

print("Please enter the name of the device:")
deviceName = input()

print("name:          " + deviceName)
print("houseAddress:  " + str(int(houseAddress, 2)))
print("deviceAddress: " + str(int(deviceAddress, 2)))


newDeviceJson = {
	"name": deviceName,
	"houseAddress": houseAddress,
	"deviceAddress": deviceAddress
}

print(newDeviceJson)



# url = "https://esp32-CB3DC4/api/config"
# payload = {}
# headers = {
#     'Connection': 'keep-alive',
#     'Authorization': 'Basic QWRtaW46c2VjcmV0'
# }

# response = requests.request("GET", url, headers=headers, data=payload, verify=False)

# currentConfig = response.json()
# print(currentConfig)


