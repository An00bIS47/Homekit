import hashlib
import base64
import struct
import base36
import pyqrcode

def setup_hash(setupid, deviceName):
	setup_hash_material = setupid + deviceName
	temp_hash = hashlib.sha512()
	temp_hash.update(setup_hash_material.encode())
	h = temp_hash.digest()[:4]
	"""
	new_str = ""
	for i in setup_hash_material:
		new_str += "0x%s " % (i.encode('hex'))
	print new_str

	

	new_str = ""
	for i in h:
		new_str += "0x%s " % (i.encode('hex'))
	print new_str
	"""

	return base64.b64encode(h)

def xhm_uri(category, pincode, setup_id):
	"""Generates the X-HM:// uri (Setup Code URI)
	:rtype: str
	"""
	buffer = bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')

	value_low = int(pincode.replace(b'-', b''), 10)
	#print value_low

	value_low |= 1 << 28
	#print value_low

	struct.pack_into('>L', buffer, 4, value_low)

	#print "buffer low:"
	#print ''.join(format(x, '02x') for x in buffer)


	if category == 0:
		buffer[4] = buffer[4] | 1 << 7

	value_high = category >> 1
	#print "value_high"
	#print value_high

	
	struct.pack_into('>L', buffer, 0, value_high)
	#print "buffer high:"
	#print ''.join(format(x, '02x') for x in buffer)

	#print "unpack 1"
	#print struct.unpack_from('>L', buffer, 4)[0]
	
	#print "unpack 2"
	#print struct.unpack_from('>L', buffer, 0)[0] * (1 << 32)

	#print "unpack3"
	#print struct.unpack_from('>L', buffer, 4)[0] + (struct.unpack_from('>L', buffer, 0)[0] * (1 << 32))

	encoded_payload = base36.dumps(struct.unpack_from('>L', buffer, 4)[0]
                                   + (struct.unpack_from('>L', buffer, 0)[0] * (1 << 32))).upper()
	
	#print "encoded_payload"
	#print encoded_payload

	encoded_payload = encoded_payload.rjust(9, '0')
	#print encoded_payload

	return 'X-HM://' + encoded_payload + setup_id


setupid = "UPFT"
print setup_hash(setupid, "24:0A:C4:0C:9D:6C")
print xhm_uri(2, "031-45-712", setupid)

qr = pyqrcode.create(xhm_uri(2, "031-45-712", setupid))
print(qr.terminal(module_color='white', background='black'))


