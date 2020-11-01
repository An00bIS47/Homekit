import re

usb_port = "/dev/ttyUSB0"
command="python /home/pi/esp-idf/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/cu.SLAB_USBtoUART --baud 2000000 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0x10000 /home/pi/dev/Homekit/build/Homekit.bin"
command_port =  re.search(r'.*--port\s(\/dev\/.*)\s--baud\s.*', command)
flash_command = command.replace(command_port.group(1), usb_port)
print(command_port)
print(flash_command)