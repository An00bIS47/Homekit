## Characteristics for logging
These characteristics are under custom service E863F007-079E-48FF-8F27-9C2605A29F52 and are used by Elgato accessories to exchange logs with Eve.app. Note that the accessory may have to include some of the custom characteristics listed below in order for the history to be shown in Eve.app.

### E863F11C-079E-48FF-8F27-9C2605A29F52
This write-only characteristic seem to control data flux from accessory to Eve.app. A typical value when reading from a fake Eve Weather accessory is 01140100 000000. Tentative decoding:
- byte 1: ??
- byte 2: ??
- byte 3-6: Requested memory entry, based on the last entry that Eve.app downloaded. If set to 0000, asks the accessory the start restart from the beginning of the memory
- byte 7-8: ??

### E863F121-079E-48FF-8F27-9C2605A29F52
In this write-only characteristics a time stamp is written by Eve.app every second if the accessory is selected in the app. Format from https://gist.github.com/gomfunkel/b1a046d729757120907c#gistcomment-1841206: 

the current timestamp is in seconds since 1.1.2001
e.g.: written value: cf1b521d
-> reverse 1d521bcf
-> hex2dec 491920335
Epoch Timestamp 1.1.2001 = 978307200;
978307200 + 491920335 = 1470227535
= Wed, 03 Aug 2016 12:32:15 GMT
= 3.8.2016, 14:32:15 GMT+2:00 DST (MEST)

It is probably used to set time/date of the accessory.

### E863F117-079E-48FF-8F27-9C2605A29F52

This read-only characteristics is used to send the actual log entry to Eve.app
It is an array of logs with each entry having x bytes as determined by Byte 1. The first portion up to byte 10 included is common, the last portion contain the data specific for each accessory.

* Byte 1: Length (i.e. 14 for 20 Bytes)
* Bytes 2-5: entry counter
* Bytes 6-9: Seconds since reference time set with type 0x81 entry. In order to account for multiple iOS devices, the actual reference time and offset are apparently always reported also in E863F116
* Byte 10: Entry type
  * 0x81 Entry to set the reference timestamp, to be written on bytes 11-14. A negative offset can be written on bytes 6-9, and it's probably neeeded when the clock is updated (21 bytes in total).
  *  0x07 Entry for Eve Weather log.
  *  0x1f Entry for Eve Energy log (20 bytes in total)
  *  0x1e Entry for Eve Energy log (18 bytes in total - not working)
  *  0x0f Entry for Eve Room log.
  *  0x02 Entry for Eve Motion log.
  *  0x01 Entry for Eve Door log.
  *  0x1f Entry for Eve Thermo log.
  *  0x05 Entry for Eve Aqua, valve on, 13 bytes in total
  *  0x07 Entry for Eve Aqua, valve off + water usage, 21 bytes in total

Accessory specific portion:

#### Eve Energy (20 bytes in total)
* Bytes 11 & 12: ?? 
* Bytes 13 & 14: ?? 
* Bytes 15 & 16: Power multiplied by 10. Eve.app will assume that the same power is drawn for all the sampling time (10 minutes), so it will show a point with an energy consumption equal to this value divided by 60. Example. Your appliance is consuming 1000W, which means a total energy of 1kWh if run for 1 hour. The value reported is 1000 x 10. The value shown is 1000 x 10 / 60 = 166Wh, which is correct because this sample covers 10min, i.e. 1/6 of an hour. At the end of the hour, Eve.app will show 6 samples at 166Wh, totalizing 1kWh. 
* Bytes 17 & 18: ?? 
* Bytes 19 & 20: ??

#### Eve Energy (18 bytes in total) - At present not working!
* Bytes 11 & 12: Power multiplied by 10. Eve.app will assume that the same power is drawn for all the sampling time (10 minutes), so it will show a point with an energy consumption equal to this value divided by 60. Example. Your appliance is consuming 1000W, which means a total energy of 1kWh if run for 1 hour. The value reported is 1000 x 10. The value shown is 1000 x 10 / 60 = 166Wh, which is correct because this sample covers 10min, i.e. 1/6 of an hour. At the end of the hour, Eve.app will show 6 samples at 166Wh, totalizing 1kWh.  
* Bytes 13 & 14: V * 10 
* Bytes 15 & 16: A * 10 ??
* Bytes 17 & 18: ?? 

#### Eve Weather (16 bytes in total)
* Temperature * 100 on bytes 11-12
* humidity * 100 on bytes 13-14
* pressure * 10 on bytes 15-16

#### Eve Room (19 bytes in total)
* Temperature * 100  in bytes 11-12
* Humidity * 100 on bytes 13-14
* PPM on bytes 15-16
* ??? on bytes 17-19.

#### Eve Door and Eve motion (11 bytes in total)
* Status 0/1 on byte 11

#### Eve Thermo (17 bytes in total)
* Current Temperature x 100 on bytes 11-12
* Set Temperature x 100 on bytes 13-14
* Valve position % on byte 15
* ?? on bytes 16-17

#### Eve Aqua, valve on (13 bytes in total)
* Status 0/1 on byte 11
* 0x0300 on bytes 12-13

#### Eve Aqua, valve off + water usage (21 bytes in total)
* Status 0/1 on byte 11
* Water usage for just ended irrigation cycle in ml on bytes 12-15
* 0x00000000 on bytes 16-19
* 0x0300 on bytes 20-21

### E863F116-079E-48FF-8F27-9C2605A29F52 (tentative)
This read-only characteristic is used by the accessory to signal how many entries are in the log (and other infos). Comparing this characteristics over different type of accessory, it was possible to obtain the following partial decoding. Data is composed by a fixed size portion (12 bytes) with info about time, 1 byte indicating the length of the following variable length portion with accessory "signature" and finally a fixed length portion with info about memory status.

* 4 bytes: Actual time, in seconds from last time update
* 4 bytes: negative offset of reference time
* 4 bytes: reference time/last Accessory time update (taken from E863F117-079E-48FF-8F27-9C2605A29F52)
* 1 byte: number of 16 bits word of the following "signature" portion
* 2-12 bytes: variable length "signature"
* 2 bytes: last physical memory position occupied (used by Eve.app to understand how many transfers are needed). If set to an address lower than the last successfully uploaded entry, forces Eve.app to start from the beginning of the memory, asking address 00 in E863F11C. Accessory answers with entry 01. Once the memory is fully written and memory overwriting is necessary this field remains equal to history size.
* 2 bytes: history size
* 4 bytes: once memory rolling occurred it indicates the address of the oldest entry present in memory (if memory rolling did not occur yet, these bytes are at 0)
* 4 bytes:?? 
* 2 bytes:?? always 01ff or 0101

The "signature" seems to be a fixed portion for each accessory type (i.e. it does not change in time). If a wrong signature is given data are not recognized, however it seems that not all bytes are really needed to identify the accessory (in fact, it is possible to use the Room signature to upload data from Weather, Thermo and Energy by changing only one or two bytes). Signatures as follows:

* Door: 1 16bits word, "0601", example of full data "44340000 6e270000 b102f51f 01 0601 b600 0010 00000000 01000000 0100" 
* Motion: 2 16bits word, "1301 1c01", example of full data "a6000000 00000000 00000000 02 1301 1c01 0100 ed0f 00000000 00000000 01ff"
* Weather (to be checked): 3 16bits word, "0102 0202 0302", example of full data "01010000 FF000000 3C0F0000 03 0102 0202 0302 1D00 F50F 00000000 00000000 01FF"
* Room: 4 16bits word, "0102 0202 0402 0f03", example of full data "5f837400 d8bd7300 de12a91f 04 0102 0202 0402 0f03 ed0f ed0f 10220000 02654f00 01ff"
* Thermo: 5 16bits word, "0102 1102 1001 1201 1d01", example of full data "e1f50500 41dd0500 09c5ef1f 05 0102 1102 1001 1201 1d01 9202 de0f 00000000 00000000 01ff"
* Energy (from dump but currently not working): 5 16bits word, "0502 0b02 0c02 0d02 0702", example of full data "a209d301 4b09d301 71f0381f 05 0502 0b02 0c02 0d02 0702 ed0f ed0f 0fbb0000 e53aae01 01ff"
* Energy (working, tweeking from Room): 4 16bits word, "0102 0202 0702 0f03", example of full data "58020000 00000000 cd8f0220 04 0102 0202 0702 0f03 0300 c00f 00000000 00000000 0101"
* Aqua: 3 16bits words, “03 1f01 2a08 2302”, example of full data “1a030000 14030000 83f7df20 03 1f01 2a08 2302 1100 0010 05100000 01000000 0100


### TENTATIVE: protocol for fake accessory sending history to Eve.app
1 - Accessory should advertise the address of the last available log entry on E863F116, along with the current time. If no memory rolling yet occurred this address is reported in bytes 22-23. If memory rolling occurred, bytes 22-23 are fixed to history size, and bytes 26-29 reports the oldest entry in the history. The address of the most recent entry is thus given adding up bytes 22-23 with bytes 26-29. The address starts from zero when the accessory is reset and it is then incremented for each new entry (since it seems that 4 bytes are used, there are plenty of binary codes for the entire life of the accessory, so no address rolling is required)

2 - Accessory monitors E863F11C for data request

3 - When a data request occurs, accessory starts sending out log entries on E863F117, beginning with the address requested and stopping when the last log is reached. Multiple log entries can be sent with a single update of the characteristics (up to 11 entries seen on Eve Room). The first entry should be of type 81 (reference time). Type 81 entries are sometimes sent along with data entries (maybe when the internal clock is adjusted), but it seems that the actual reference time (given by the time stamp in bytes 9-12 of E863F116, minus the negative offset in 5-8 of E863F116) set upon accessory reset is never changed, probably to ensure data consinstency (in fact, if from time to time reference time is changed, should an iOS device miss the corresponding type 81 entry, all following data will be misplaced in time). What is seen on real Elgato accessories it that when a new type 81 entry is sent, the time stamp stamp is incremented (probably to the actual time), but the negative offset is incremented as well by more or less the same quantiy (a part small differences probably due to clock adjustment). Reason unknown.

4 - This should be useless since homebridge has its own clock: monitor E863F121 for clock updates. Eve.app will ignore any entry "in the future".