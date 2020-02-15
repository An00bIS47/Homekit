# Install TIAO USB Multi-Protocol-Adapter (TUMPA) Lite on macos High Sierra


## Get idvender, bcddevice and ipproduct
```
ioreg -r -n "USB Multi-Protocol Adapter Lite@14200000" -c AppleUSBDevice | grep -i "bcddevice\|idproduct\|idvendor"
```

## Edit kext
`sudo sublime /Library/Extensions/FTDIUSBSerialDriver.kext/Contents/Info.plist` 

#### Add the following entries to the plist
```
<key>TIAO USB Multi-protocol Adapter Lite</key>
  <dict>
        <key>CFBundleIdentifier</key>
        <string>com.FTDI.driver.FTDIUSBSerialDriver</string>
        <key>IOClass</key>
        <string>FTDIUSBSerialDriver</string>
        <key>IOProviderClass</key>
        <string>IOUSBInterface</string>
        <key>bConfigurationValue</key>
        <integer>1</integer>
        <key>bInterfaceNumber</key>
        <integer>0</integer>
        <key>bcdDevice</key>
        <integer>2304</integer>
        <key>idProduct</key>
        <integer>35481</integer>
        <key>idVendor</key>
        <integer>1027</integer>
    </dict>
<key>TIAO USB Multi-protocol Adapter Lite</key>
 <dict>
        <key>CFBundleIdentifier</key>
        <string>com.FTDI.driver.FTDIUSBSerialDriver</string>
        <key>IOClass</key>
        <string>FTDIUSBSerialDriver</string>
        <key>IOProviderClass</key>
        <string>IOUSBInterface</string>
        <key>bConfigurationValue</key>
        <integer>1</integer>
        <key>bInterfaceNumber</key>
        <integer>1</integer>
        <key>bcdDevice</key>
        <integer>2304</integer>
        <key>idProduct</key>
        <integer>35481</integer>
        <key>idVendor</key>
        <integer>1027</integer>
    </dict>
```

## Disable SIP

* Reboot into recovery mode
	`APFEL + R while starting`
	
* Open Terminal and enter
`csrutil disable` to disable SIP and `csrutil enable` to enable SIP

* Reboot into normal mode

## Load KEXT
`sudo kextload /Library/Extensions/FTDIUSBSerialDriver.kext`

## Unload KEXT
`sudo kextunload /Library/Extensions/FTDIUSBSerialDriver.kext`


## Download OpenOCD
Download repo from github
```
cd ~/esp
git clone --recursive https://github.com/espressif/openocd-esp32.git
cd openocd-esp32
```


## Compile OpenOCD
* `./bootstrap`
* `./configure`
* `make`
* `make install` is **optional**

## Edit esp32.cfg
```

```

