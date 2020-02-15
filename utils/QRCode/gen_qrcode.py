#!/usr/bin/env python
from argparse import ArgumentParser, SUPPRESS
import os.path
import PIL.Image
import PIL.ImageDraw
import PIL.ImageFont
import qrcode
import re
import sys

import hashlib
import base64


from subprocess import Popen, PIPE

def addToPhotos(filePath):
    scpt = '''
on run argv
    set filePath to POSIX file (item 1 of argv)
    
    set imageList to {}
    copy filePath to the end of imageList
    repeat with i from 1 to number of items in imageList
        set this_item to item i of imageList as alias
    end repeat
    
    
    tell application "Photos"
        import imageList into container named "Homekit Devices"
    end tell
end run
'''
    args = [filePath]

    p = Popen(['osascript', '-'] + args, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate(scpt)
    print (p.returncode, stdout, stderr)

script_dir = os.path.dirname(os.path.realpath(__file__))

BASE36 = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

PAYLOAD_VERSION = 0
PAYLOAD_FLAGS = 2  # 2=IP, 4=BLE, 8=IP_WAC

def gen_homekit_setup_uri(category, password, setup_id, version=PAYLOAD_VERSION, reserved=0, flags=PAYLOAD_FLAGS):
    payload = 0
    payload |= (version & 0x7)

    payload <<= 4
    payload |=(reserved & 0xf)  # reserved bits

    payload <<= 8
    payload |= (category & 0xff)

    payload <<= 4
    payload |= (flags & 0xf)

    payload <<= 27
    payload |= (int(password.replace('-', '')) & 0x7fffffff)

    print("payload: {}".format(payload))

    encodedPayload = ''
    for _ in range(9):
        encodedPayload += BASE36[payload % 36]
        #print(encodedPayload)
        payload //= 36

    return 'X-HM://%s%s' % (''.join(reversed(encodedPayload)), setup_id)


def gen_homekit_qrcode(setup_uri, password):
    code = qrcode.QRCode(version=2, border=0, box_size=12,
                         error_correction=qrcode.constants.ERROR_CORRECT_Q)
    code.add_data(setup_uri)

    # open template
    img = PIL.Image.open(os.path.join(script_dir, 'qrcode.png'))
    # add QR code to it
    img.paste(code.make_image().get_image(), (50, 180))

    # add password digits
    setup_code = password.replace('-', '')

    font = PIL.ImageFont.truetype(os.path.join(script_dir, 'Scancardium_2.0.ttf'), 56)
    draw = PIL.ImageDraw.Draw(img)

    for i in range(4):
        draw.text((170 + i*50, 40), setup_code[i], font=font, fill=(0, 0, 0))
        draw.text((170 + i*50, 100), setup_code[i+4], font=font, fill=(0, 0, 0))

    return img

def calculate_sh(setup_id, mac):
    setup_hash_material = setup_id + mac
    #print(":".join("{:02x}".format(ord(c)) for c in setup_hash_material))
    
    temp_hash = hashlib.sha512()
    temp_hash.update(setup_hash_material.encode())

    h = temp_hash.digest()[:4]
    #print(":".join("{:02x}".format(ord(c)) for c in temp_hash.digest()))

    #print(":".join("{:02x}".format(ord(c)) for c in h))
    return base64.b64encode(h)
    

def main():    
    parser = ArgumentParser(description='Homekit QR Code Generator', add_help=False)
    
    required = parser.add_argument_group('required arguments')
    optional = parser.add_argument_group('optional arguments')
    optional.add_argument('-m', '--mac', help="MAC address of the device")
    optional.add_argument('-o', '--output', default="esp32.png", help="name of the output image")
    optional.add_argument('-x', '--xhm', help="X:HM path")
    
    optional.add_argument('-a', '--add', help="Add to Photos.app", default=False, action='store_true')

    # Add back help 
    optional.add_argument('-h', '--help', action='help', default=SUPPRESS, help='show this help message and exit') 

    optional.add_argument('-c', '--category', type=int, help="Accessory category, e.g. 2 for Bridge")
    optional.add_argument('-p', '--pin', required=True, help="Pincode seperated by -")
    optional.add_argument('-s', '--setup_id', help="The setup_id")


    args = parser.parse_args()


    if args.xhm is not None:
        setupURI = args.xhm
    else:
        if not re.match('[0-9A-Z]{4}', args.setup_id):
            raise ValueError('Invalid setup ID')

        if args.mac is not None:
            if not re.match('[a-fA-F0-9:]{17}$', args.mac):
                raise ValueError('Invalid mac address')

        if args.mac is not None and args.setup_id is not None:
            print(calculate_sh(args.setup_id, args.mac.upper()))

   
        setupURI = gen_homekit_setup_uri(args.category, args.pin, args.setup_id)
        print(setupURI)
    
    if not re.match('\d{3}-\d{2}-\d{3}', args.pin):
        raise ValueError('Invalid pin code! Use "-" between the digits')

    qrcodeImage = gen_homekit_qrcode(setupURI, args.pin)

    output = args.output
    
    if args.mac is not None:
        output = os.path.splitext(output)
        output = output[0] + "-"
        mac = args.mac.upper();
        mac = mac.replace(":", "")
        print(mac[6:])
        output = output + mac[6:] + ".png"
    
    currentDirectory = os.getcwd()
    output_fullpath = currentDirectory + "/" + output

    qrcodeImage.save(output_fullpath)

    if args.add is True:
        addToPhotos(output_fullpath)

if __name__ == '__main__':
    main()
