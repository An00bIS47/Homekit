

# QR Code Scan

# Device information can be extracted from scanning valid QR code. API returns single ESPDevice instance on success. It supports both SoftAP and BLE. If your device does not have QR code, you can use any online QR code generator. QR code payload is a JSON string representing a dictionary with key value pairs listed in the table below. An example payload : {"ver":"v1","name":"PROV_CE03C0","pop":"abcd1234","transport":"softap"}

# Payload information :

# Key	Detail	Values	Required
# ver	Version of the QR code.	Currently, it must be v1.	Yes
# name	Name of the device.	PROV_XXXXXX	Yes
# pop	Proof of possession.	POP value of the device like abcd1234	Optional. Considered empty string if not available in QR code data.
# transport	Wi-Fi provisioning transport type.	It can be softap or ble.	Yes
# security	Security for device communication.	It can be 0 or 1 int value.	Optional. Considered Sec1 if not available in QR code data.
# password	Password of SoftAP device.	Password to connect with SoftAP device.	Optional


# Example Payload:
# {"ver":"v1","name":"PROV_CE03C0","pop":"abcd1234","transport":"softap"}

import qrcode
import qrcode.image.svg
import json
import argparse

def generatePayloadJson(name, pop, security=None, transport="ble", version="v1", password=None):
	payload = {
		"ver": version,
		"name": name,
		"pop": pop,
		"transport": transport	
	}

	if security != None:
		payload["security"] = security	
	if transport == "softap" and password != None:
		payload["password"] = password

	print(json.dumps(payload))
	return payload

def generateQRCode(payload, file, method="png"):
	# Create qr code instance
	qr = qrcode.QRCode(
	    version = 1,
	    error_correction = qrcode.constants.ERROR_CORRECT_M,
	    box_size = 1,
	    border = 2,
	)
	qr.add_data(json.dumps(payload))

	# png
	qr.make(fit=True)
	img = qr.make_image()
	

	# svg
	# img = qrcode.make(image_factory=qrcode.image.svg.SvgPathImage)
	# img = qrcode.make(json.dumps(payload), image_factory=qrcode.image.svg.SvgPathImage)
	img.save(file)


parser = argparse.ArgumentParser()
parser.add_argument('--name', help='Name of the device.', action='store', type=str, required=True)
parser.add_argument('--pop', help='Proof of possession', action='store', type=str, required=True)
parser.add_argument('--transport', help='Wi-Fi provisioning transport type.', action='store', choices=['ble', 'softap'], required=False, default='ble')
parser.add_argument('--security', help='Security for device communication.', action='store', choices=[0, 1], type=int, required=False,  default=None)
parser.add_argument('--password', help='Password of SoftAP device.', action='store', type=str, required=False, default=None)
parser.add_argument('--file', help='Filename of the QR code', action='store', type=str, required=True, default="./qr_code.png")
args = parser.parse_args()



payload = generatePayloadJson(name=args.name, pop=args.pop, transport=args.transport, security=args.security, password=args.password, version="v1");
generateQRCode(payload, args.file)