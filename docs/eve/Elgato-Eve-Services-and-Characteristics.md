## Elgato Eve Energy (Firmware Revision 1.3.1;466)

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|------------------------------------
Accessory Information        | 0000003E-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
Name                         | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Manufacturer                 | 00000020-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Model                        | 00000021-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Serial Number                | 00000030-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Identify                     | 00000014-0000-1000-8000-0026BB765291 |   | X | Boolean    | HomeKit Standard
Firmware Revision            | 00000052-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Outlet                       | 00000047-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
Name                         | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Power State                  | 00000025-0000-1000-8000-0026BB765291 | X | X | Boolean    | HomeKit Standard
Outlet In Use                | 00000026-0000-1000-8000-0026BB765291 | X |   | Boolean    | HomeKit Standard
Volt                         | E863F10A-079E-48FF-8F27-9C2605A29F52 | X |   | float     | Volt = (value)
Ampere                       | E863F126-079E-48FF-8F27-9C2605A29F52 | X |   | float   | Ampere = (value)
Watt                         | E863F10D-079E-48FF-8F27-9C2605A29F52 | X |   | float    | Watt = (value). This is what is reported as "Consumption" in the Eve app.
Kilowatt-hour                | E863F10C-079E-48FF-8F27-9C2605A29F52 | X |   | float     | kWh = (value). This is what is reported as "Total Consumption" in the Eve app.
???                          | E863F007-079E-48FF-8F27-9C2605A29F52 | - | - |            | Custom service for meta and/or historical information. Characteristics for logging: E863F11C, E863F121, E863F116, E863F117
Volt-Ampere                  | E863F110-079E-48FF-8F27-9C2605A29F52 | X |   | UInt16     | Volt-Ampere = UInt16(value)
kVAh                         | E863F127-079E-48FF-8F27-9C2605A29F52 | X |   | UInt32     | kVAh = UInt32(value)
???                          | E863F11E-079E-48FF-8F27-9C2605A29F52 | X | X |            | ??? - Also available on Eve Room & Weather. Maybe model and version. If set to 01be00be 00f44fb8 0a000000 triggers an "Eve Weather firmware update advice" in Eve app.
Time from totalizer reset    | E863F112-079E-48FF-8F27-9C2605A29F52 | X | X | UInt32     | Set to seconds from 1.1.2001 upon Reset of total consumption in Eve.app. Also in Door
???                          | E863F10E-079E-48FF-8F27-9C2605A29F52 | X |   | UInt16?    | ??? - Seems to be 0 when no consumers are attached to the outlet
???                          | E863F11C-079E-48FF-8F27-9C2605A29F52 |   | X |            | ??? - Also available on Eve Room. Handshaking for log transfer. 
???                          | E863F121-079E-48FF-8F27-9C2605A29F52 |   | X |            | ??? - Also available on Eve Room - A time stamp is written here periodically by Eve.app. Format here https://gist.github.com/gomfunkel/b1a046d729757120907c#gistcomment-1841206
???                          | E863F116-079E-48FF-8F27-9C2605A29F52 | X |   |            | ??? - Also available on Eve Room & Weather. Info from the accessory needed to trigger the log transfer. Yet to be understood.
???                          | E863F117-079E-48FF-8F27-9C2605A29F52 | X |   |            | See below - Also available on Eve Room & Weather. Entries for the log from the accessory to Eve.app

## Elgato Eve Room (Firmware Revision 1.3.1;466)

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|-----------------------------------
Accessory Information        | 0000003E-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
  Name                      | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
  Manufacturer              | 00000020-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
 Model                     | 00000021-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
 Serial Number             | 00000030-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
 Identify                  | 00000014-0000-1000-8000-0026BB765291 |   | X | Boolean    | HomeKit Standard
  Firmware Revision         | 00000052-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
Battery                      | 00000096-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
 Name                      | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
  Status Low Battery        | 00000079-0000-1000-8000-0026BB765291 | ? | ? | ?          | HomeKit Standard
  Battery Level             | 00000068-0000-1000-8000-0026BB765291 | ? | ? | ?          | HomeKit Standard
  Charging State            | 0000008F-0000-1000-8000-0026BB765291 | ? | ? | ?          | HomeKit Standard
Temperature Sensor           | 0000008A-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
  Name                      | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
 Current Temperature       | 00000011-0000-1000-8000-0026BB765291 | ? | ? | float         | HomeKit Standard
Humidity Sensor              | 00000082-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
 Name                      | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
 Current Relative Humidity | 00000010-0000-1000-8000-0026BB765291 | ? | ? | float        | HomeKit Standard
 Air Quality Sensor          | 0000008D-0000-1000-8000-0026BB765291 | - | - |            | HomeKit Standard
  Name                      | 00000023-0000-1000-8000-0026BB765291 | X |   | String(64) | HomeKit Standard
  Air Quality               | 00000095-0000-1000-8000-0026BB765291 | ? | ? | ?          | HomeKit Standard (Level 0 - 5)
 Air Quality in ppm        | E863F10B-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | UInt16(value) in ppm. Rough classification: 0 - 700 Excellent, 700 - 1100 Good, 1100 - 1600 Acceptable, 1600 - 2000 Moderate, > 2000 Bad. Represented by simple levels with the characteristic above.
 ???                       | E863F132-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ???
???                          | E863F007-079E-48FF-8F27-9C2605A29F52 | - | - |            | Probably a service for meta and/or historical information
  ???                       | E863F11C-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ??? - Also available on Eve Energy
  ???                       | E863F121-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ??? - Also available on Eve Energy
  ???                       | E863F11D-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ???
  ???                       | E863F11E-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ??? - Also available on Eve Energy
  ???                       | E863F116-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ??? - Also available on Eve Energy
  ???                       | E863F117-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ??? - Also available on Eve Energy
 ???                       | E863F131-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | ???
  Status Fault              | 00000077-0000-1000-8000-0026BB765291 | ? | ? | ?          | HomeKit Standard

## Elgato Eve Door

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|-----------------------------------
Number of times opened       | E863F129-079E-48FF-8F27-9C2605A29F52 | ? | ? | UInt32     | UInt32(value) / 2
Battery Level                | E863F11B-079E-48FF-8F27-9C2605A29F52 | ? | ? | UInt16     | UInt16(value) in %
Last activation              |E863F11A-079E-48FF-8F27-9C2605A29F52  | x |   | UInt32?     | Time of last activation. ?Seconds from HW reset?
Total seconds open         | E863F118-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | From last HW reset?
Total seconds close        | E863F119-079E-48FF-8F27-9C2605A29F52 | ? | ? | ?          | From last HW reset?
Time from totalizer reset | E863F112-079E-48FF-8F27-9C2605A29F52 | X | X | UInt32    | Set to seconds from 1.1.2001 upon Reset of total openings in Eve.app. Also in Energy

## Elgato Eve Weather

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|-----------------------------------
Battery Level                | E863F11B-079E-48FF-8F27-9C2605A29F52 | ? | ? | UInt16     | UInt16(value) in %

## Elgato Eve Motion Sensor

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|-----------------------------------
Sensitivity                  |E863F120-079E-48FF-8F27-9C2605A29F52  | x | x | UInt8      |0 = high, 4 = medium, 7 = low. Set in Eve.app
Duration                     |E863F12D-079E-48FF-8F27-9C2605A29F52  | x |  x| UInt16     |Persistence of motion indication in seconds. Set in Eve.app
Last activation              |E863F11A-079E-48FF-8F27-9C2605A29F52  | x |   | UInt32?     | Time of last activation. ?Seconds from HW reset?

## Elgato Eve Degree

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|-----------------------------------
Accessory Information |0000003E-0000-1000-8000-0026BB765291| | | |
Name | 00000023-0000-1000-8000-0026BB765291 | x | | | 
Manufacturer | 00000020-0000-1000-8000-0026BB765291 | x | | |
Model | 00000021-0000-1000-8000-0026BB765291 | x | | |
Serial Number | 00000030-0000-1000-8000-0026BB765291 | x | | |
Firmware Revision | 00000052-0000-1000-8000-0026BB765291 | x | | |
??? | 00000053-0000-1000-8000-0026BB765291 | x | | |
Identify | 00000014-0000-1000-8000-0026BB765291 | | x | |
Battery | 00000096-0000-1000-8000-0026BB765291
Name | 00000023-0000-1000-8000-0026BB765291 | x | | |
Battery Level | 00000068-0000-1000-8000-0026BB765291 | x | | |
Charging State | 0000008F-0000-1000-8000-0026BB765291 | x | | |
Status Low Battery | 00000079-0000-1000-8000-0026BB765291 | x | | |
Temperature Sensor | 0000008A-0000-1000-8000-0026BB765291
Name | 00000023-0000-1000-8000-0026BB765291 | x | | |
Current Temperature | 00000011-0000-1000-8000-0026BB765291 | x | | |
Temperature Units | 00000036-0000-1000-8000-0026BB765291 | x | | |
Humidity Sensor | 00000082-0000-1000-8000-0026BB765291
Name | 00000023-0000-1000-8000-0026BB765291 | x | | |
Current Relative Humidity | 00000010-0000-1000-8000-0026BB76529 | x | | |
Service for Pressure | E863F00A-079E-48FF-8F27-9C2605A29F52
Name | 00000023-0000-1000-8000-0026BB765291 | x | | |
Pressure | E863F10F-079E-48FF-8F27-9C2605A29F52 | x | | |
Elevation | E863F130-079E-48FF-8F27-9C2605A29F52 | x | | |
Logging | 863F007-079E-48FF-8F27-9C2605A29F52
Name | 00000023-0000-1000-8000-0026BB765291 | x | | |
Status Fault | 00000077-0000-1000-8000-0026BB765291 | x | | |
??? | E863F11E-079E-48FF-8F27-9C2605A29F52 | x | | |
??? | E863F112-079E-48FF-8F27-9C2605A29F52 | x | | |
??? | E863F116-079E-48FF-8F27-9C2605A29F52 | x | | |
??? | E863F117-079E-48FF-8F27-9C2605A29F52 | x | | |
??? | E863F131-079E-48FF-8F27-9C2605A29F52 | x | | |
??? | E863F11C-079E-48FF-8F27-9C2605A29F52 | | x | |
??? | E863F121-079E-48FF-8F27-9C2605A29F52 | | x | |
??? | E863F11D-079E-48FF-8F27-9C2605A29F52 | | x | |

## Elgato Eve Thermo

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|-----------------------------------
Valve position                  |E863F12E-079E-48FF-8F27-9C2605A29F52  | x |  | UInt8      |Valve position in %
Program command                     |E863F12C-079E-48FF-8F27-9C2605A29F52  |  |  x| Data     | Command from Eve to Thermo on Program (set, query, ...), format uknown
Program data             |E863F12F-079E-48FF-8F27-9C2605A29F52  | x |   | Data      | Data from Thermo to Eve, format unknown

## Elgato Eve Aqua

Service - Characteristic     | UUID                                 | R | W | Type       | Description
-----------------------------|--------------------------------------|---|---|------------|----------------------------------
Get status                  |E863F131-079E-48FF-8F27-9C2605A29F52  | x|  | Data      |"supercharacteristic" for sending status to Eve, e.g. schedules (not decoded), last event time, total water consumed and programmed water flux. For proper working this characteristic must be set to a proper value, such as "0002230003021b04040c4156323248314130303036330602080007042a3000000b0200000501000204f82c00001401030f0400000000450505000000004609050000000e000042064411051c0005033c0000003a814b42a34d8c4047110594186d19071ad91ab40000003c00000048060500000000004a06050000000000d004 XXXXXXXX 9b04 YYYYYYYY 2f0e WWWWWWWW 00000000 00000000 ZZZZ 2d06 0000000000001e02300c", where Z is 16bit value with water flux in ml/s, W is 32 bit value with total water usage in ml, last event is expressed in "seconds before now" encoded as the difference between Y and X 32 bit values
Send command                    |E863F11D-079E-48FF-8F27-9C2605A29F52  |  | x| Data     | "supercharacteristic" for setting parameter to accessory (e.g. schedules, water flux). Only command for water flux decoded so far, as "2e02 XXXX" with X reporting water flux in ml/s. This value should then set back by the accessory in E863F131 in order for the value to be accepted by Eve