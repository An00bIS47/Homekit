
# Elgato Eve Energy Schedules

The schedule seem to be almost similar to the Eve Aqua Schedules.

## Required Characteristics:

* Config Write: E863F11D-079E-48FF-8F27-9C2605A29F52
* Config Read:  E863F131-079E-48FF-8F27-9C2605A29F52


## Data format
The schedule is handled as `TLV8` data although exposed as `data` in the characteristics!


## Example write config characteristic (=> from App):

```
450D0502 00000000 00020101 4B059646 5405161C 2C9F2449 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00441105 03000000 00000000 00000000 00000000
```

## Example read config characteristic (=> to App):
```
00022400 0302b804 040c4256 31324a31 41303732 31320602 6c010704 0c100000 
0b020000 05010002 04902700 005f0400 00000019 02960014 01030f04 00000000 
450d0502 00000000 00020101 3c059646 5405151c 2c9f2449 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
00000000 00441105 0c000503 3c000000 32c24242 a1933441 47110573 1b451cdf 
1cb81db4 0000003c 00000048 06050000 0000004a 06050000 0000001a 04000000 
00600164 d0045209 03009b04 d00f0300 d200
```

# Available TLV8 `types`:

Type | Description   		| Energy 			 | Energy Strip
-----|----------------------|--------------------|--------------
0x00 | ??            		| :white_check_mark: | :white_check_mark: 
0x02 | ??            		| :white_check_mark: |     
0x03 | ??            		| :white_check_mark: | :white_check_mark: 
0x04 | Serial Number 		| :white_check_mark: | :white_check_mark: 
0x05 | ??            		| :white_check_mark: |     
0x06 | Memory Used   		| :white_check_mark: |     
0x07 | Rolled Over Index  	| :white_check_mark: |     
0x09 | ??            		|     				 | :white_check_mark: 
0x0B | ??            		| :white_check_mark: | :white_check_mark: 
0x9B | ??            		|     				 | :white_check_mark: 
0x0F | ??            		| :white_check_mark: |     
0x19 | ??            		| :white_check_mark: |     
0x14 | ??            		| :white_check_mark: |     
0x1A | ??            		| :white_check_mark: |     
0x4A | ??            		| :white_check_mark: | :white_check_mark: 
0x5F | ??            		| :white_check_mark: |     
0x9C | ??            		|     				 | :white_check_mark: 
0x0C | Hostname      		|     				 | :white_check_mark: 
0x44 | Commands      		| :white_check_mark: | :white_check_mark: 
0x45 | Programs      		| :white_check_mark: | :white_check_mark: 
0x46 | Days          		| :white_check_mark: | :white_check_mark: 
0x47 | ??   	       		| :white_check_mark: | :white_check_mark: 
0x48 | ??            		| :white_check_mark: | :white_check_mark: 
0x60 | Status LED    		| :white_check_mark: |     
0xD0 | Last Switch Activity | :white_check_mark: |     
0xD2 | End mark      		| :white_check_mark: |     
0xD3 | ??            		|     				 | :white_check_mark: 
0x98 | Last Update   		| :white_check_mark: |     
0x32 | ??            		|     				 | :white_check_mark: 
0x33 | ??            		|     				 | :white_check_mark: 
0xA8 | ??            		|     				 | :white_check_mark: 


# Memory Used

`type` = `0x06`
`length` = `2`

Number of history entries



# Rolled Over Index

`type` = `0x07`
`length` = `4`

Index of oldest entry if rolled over, otherwise 0



# End Mark

`type` = `0xD2`

This seems to be a the end mark for a TLV.
The length is 0.


# Last Switch Activity

`type` = `0xD0`
`length` = `4`

Time in seconds from last switch activity 

```
timestamp last switching activity - reference time
```

# EVE Time

`type` = `0x98`
`length` = `4`

Actual time, in seconds from last time update

```
timestamp last update - reference time
```

# Toggle Schedules On/Off

`type` = `0x44`
`length` = `17`

Command to toggle schedules on/off.

## Example TLV8:
```
T: 0x44
L: 17 (HEX 11)
V: 0x0503000000000000000000000000000000
```

## Example Values:
```
var data = "0502000000000000000000000000000000"			// OFF
var data = "0503000000000000000000000000000000" 		// ON
```

## Script:
``` 
// 
// EVE Energy Decode Schedule Toggle
// 
// thanks to https://github.com/simont77/fakegato-history/issues/90
// 
// TLV8
// Type: 44
// 
var colors = require('colors');

var data = "0502000000000000000000000000000000"			// OFF
var data = "0503000000000000000000000000000000" 		// ON

curIndex = 2
len = 2

process.stdout.write(data.substr(0, curIndex))
process.stdout.write(data.substr(curIndex, len).green)
process.stdout.write(data.substr(curIndex + len))
console.log("")

// 03 for ON
// 02 for OFF
var schedule = parseInt(data.substr(curIndex, len).match(/[a-fA-F0-9]{2}/g).reverse().join(''), 16) & 0x1;
schedule ? console.log(colors.green("ON")) : console.log(colors.red("OFF"))
```

## Output:
```
0502000000000000000000000000000000
0
```

```
0503000000000000000000000000000000
1
```

# Set Status LED

`type` = `0x20`
`length` = `1`

Command to set the status LED. The actual value will be given back in `type` = `0x60`



## Example TLV8:
```
T: 0x20
L: 1 (HEX 01)
V: 0x64
```
## Available Values
```
00: IF OFF OFF
21: IF OFF LIGHT
42: IF OFF MEDIUM
64: IF OFF BRIGHT
a1: IF ON  LIGHT
c2: IF ON  MEDIUM
e1: IF ON  BRIGHT
```

## Script
```
// 
// EVE Energy Decode LED Status
// 
// thanks to https://github.com/simont77/fakegato-history/issues/90
// 
// TLV8
// Type: 20
// 
var colors = require('colors');

var array = ["00", "21", "42", "64", "a1", "c2", "e1"]

// OFF
// 00 == 00000000	

// IF ON 
// 21 == 00100001	// light
// 42 == 01000010	// medium
// 64 == 01100100	// bright

// IF OFF
// A1 == 10100001	// light
// C2 == 11000010	// medium
// E4 == 11100100	// bright


curIndex = 0
len = 2

for (var i = 0; i < array.length; i++) {
	data = array[i]

	process.stdout.write(data.substr(0, curIndex))
	process.stdout.write(data.substr(curIndex, len).green)	
	process.stdout.write((": ").green)

	var led = parseInt(data.substr(curIndex, len), 16);	

	var ifOn = ( led >>> 7 ) & 0x01
	var brightness = (led >>> 5) & 0x03

	var brightnessStr = ""


	switch (brightness){
		case 0:
			brightnessStr = "OFF"
			break;
		case 1:
			brightnessStr = "LIGHT"
			break;
		case 2:
			brightnessStr = "MEDIUM"
			break;
		case 3:
			brightnessStr = "BRIGHT"
			break;				
	}

	console.log(ifOn ? "IF ON " : "IF OFF", brightnessStr) 
}
```


# Programs

`type` = `0x45`
`length` = `variable`

There can be 7 programs (for each day 1) and up to 15 timers per program.

There are 4 different types of timers:
* timer & 0x1F == 1 => OFF by time
* timer & 0x1F == 5 => ON  by time
* timer & 0x1F == 3 => OFF by sunrise
* timer & 0x1F == 7 => ON  by sunrise

The offset for timed events is in seconds since 0:00, for sunset/sunrise +/- seconds.

## Example TLV8:
```
T: 0x45
L: 11 (HEX 08)
V: 0x0502000000000081000500
```


## Example values:
```
// Timed:
var data = "0502000000000081000500"					// 0:00 OFF
var data = "0502000000000081000500" 				// 0:00 ON
var data = "050200000000000201014B0596"				// 1O:00 OFF - 20:00 ON
var data = "050200000000008301C5764579458D" 		// 15:50 ON - 16:10 ON - 18:50 ON
var data = "050200000000000302010A013C858E0596" 	// 1:20 OFF - 8:00 OFF - 19:00 ON - 20:00 ON

// Sunrise / Sundown:
var data = "050200000000000201E707A307"				// sunrise - 15 min - ON - sunrise + 15 min - OFF
```

## Script:
```
// 
// EVE Energy Decode Timers
// 
// thanks to https://github.com/simont77/fakegato-history/issues/90
// 
// TLV8
// Type: 45
// 
var colors = require('colors');

var data = "0502000000000081000500"					// 0:00 OFF
var data = "0502000000000081000500" 				// 0:00 ON
var data = "050200000000000201014B0596"				// 1O:00 OFF - 20:00 ON
var data = "050200000000008301C5764579458D" 		// 15:50 ON - 16:10 ON - 18:50 ON
var data = "050200000000000302010A013C858E0596" 	// 1:20 OFF - 8:00 OFF - 19:00 ON - 20:00 ON

// var data = "0502000000000083024508013CC53C85690596"											// 5 timers
// var data = "050200000000000303013CA544255B256A056C0596" 										// 6 timers
// var data = "050200000000008303013CC5494560256FC58C0596C5A3"									// 7 timers
// var data = "050200000000000304013C8560856FA56F2586C58C0596A59C" 								// 8 timers
// var data = "0502000000000083070509851F013C6545655BA56A0581258705888588258F0590059685A6E5AD" 	// 15 timers

// var data = "050300000000000201013C05960201259E459E"										// 2 programs
// var data = "050400000000000201013C05960201A59EC59E8301A5532588E59D" 						// 3 programs
// var data = "050800000000000201013C0596810025A1810025A1810025A2810025A1810025A1810085A1" 	// 7 programs

// var data = "050200000000000201E707A307"				// sunrise - 15 min - ON - sunrise + 15 min - OFF

// Timer count
//   	  1 == 0081 == 000100000 01 =  129 ==  1 * 128 + 1
//   	  2 == 0102 == 001000000 10 =  258 ==  2 * 128 + 2
//   	  3 == 0183 == 001100000 11 =  387 ==  3 * 128 + 3
//   	  4 == 0203 == 010000000 11 =  515 ==  4 * 128 + 3
//   	  5 == 0283 == 010100000 11 =  643 ==  5 * 128 + 3
//   	  6 == 0303 == 011000000 11 =  771 ==  6 * 128 + 3
// 		  7 == 0383 == 011100000 11 =  899 ==  7 * 128 + 3
//        8 == 0403 == 100000000 11 = 1027 ==  8 * 128 + 3
//  	 ...
//  MAX: 15 == 0783 == 111100000 11 = 1923 == 15 * 128 + 3


function timerTypeToString(timer_type){
	if (timer_type == 0){
		return "TIME";
	} else if (timer_type == 1) {
		return "SUN"
	}
}

curIndex = 14

programs = []


process.stdout.write(data.substr(0, 2))
process.stdout.write(data.substr(2, 2).blue)

var programCount = parseInt(data.substr(2, 2).match(/[a-fA-F0-9]{2}/g).reverse().join(''), 16) - 1;


process.stdout.write(data.substr(4, curIndex - 4))

var pcount = 0;
for (var i = 0; i < programCount; i++) {
	
	process.stdout.write(data.substr(curIndex, 4).cyan);

	var timerCount = parseInt(data.substr(curIndex, 4).match(/[a-fA-F0-9]{2}/g).reverse().join(''), 16) >>> 7;
	curIndex = curIndex + 4;

	var timerEvents = [];	

	for (var tcount = 0; tcount < timerCount; tcount++) {				

		var timer = parseInt(data.substr(curIndex, 4).match(/[a-fA-F0-9]{2}/g).reverse().join(''), 16);

		
		if (tcount % 2 == 0) {
			process.stdout.write(data.substr(curIndex, 4).green)	
		} else {
			process.stdout.write(data.substr(curIndex, 4).magenta)		
		}
			

		var timer_state = (timer & 0x1f) >>> 2
		var timer_type  = ((timer & 0x1f) & 0x2 ) >>> 1
		
		if ((timer & 0x1f) == 1 || (timer & 0x1f) == 5) {				

			var timer_min = (timer >>> 5) % 60;					// Timer minute
			var timer_hr  = ((timer >>> 5) - timer_min) / 60; 	// Timer hour
			var timer_offset = ((timer >>> 5) * 60);    		// Seconds since 00:00

			var timerEvent = {"h": timer_hr, "min": timer_min, "offset": timer_offset, "type": timerTypeToString(timer_type), "state": timer_state ? "ON" : "OFF"}
			
		} else if ((timer & 0x1f) == 7 || (timer & 0x1f) == 3) {								

			var timer_sunrise = ((timer >>> 5) & 0x01);    // 1 = sunrise, 0 = sunset
			var timer_offset = ((timer >>> 6) & 0x01 ? ~((timer >>> 7) * 60) + 1 : (timer >>> 7) * 60);   // offset from sunrise/sunset (plus/minus value)
			var timerEvent = {"sunrise": timer_sunrise, "offset": timer_offset, "type": timerTypeToString(timer_type), "state": timer_state ? "ON" : "OFF"}
		}
		
		curIndex = curIndex + 4;
		timerEvents.push(timerEvent);
				
	} 	

	pcount = pcount + 1;

	var programEvent = {"program": pcount, "timers": tcount, "events": timerEvents};
	programs.push(programEvent);
}

console.log("")
// console.log(console_output)
console.log("programs: " + colors.blue(programCount))

for (var i = 0; i < programs.length; i++) {
	console.log(">>> program: " + programs[i].program)
	console.log("    timer:   " + programs[i].timers)
	console.log(programs[i].events)
	console.log("--------------------------------------------------------------------------");
}
```

## Output:
```
050200000000000304013C8560856FA56F2586C58C0596A59C
programs: 1
>>> program: 1
    timer:   8
[
  { h: 8, min: 0, offset: 28800, type: 'TIME', state: 'OFF' },
  { h: 12, min: 52, offset: 46320, type: 'TIME', state: 'ON' },
  { h: 14, min: 52, offset: 53520, type: 'TIME', state: 'ON' },
  { h: 14, min: 53, offset: 53580, type: 'TIME', state: 'ON' },
  { h: 17, min: 53, offset: 64380, type: 'TIME', state: 'ON' },
  { h: 18, min: 46, offset: 67560, type: 'TIME', state: 'ON' },
  { h: 20, min: 0, offset: 72000, type: 'TIME', state: 'ON' },
  { h: 20, min: 53, offset: 75180, type: 'TIME', state: 'ON' }
]
```

# Days

`type` = `0x46`
`length` = `84`

Active Days of each program

## Example TLV8:
```
T: 0x46
L: 84 (HEX 54)
V: 0x05161C2C9F24490000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
```


## Example values:
```
// every day
var data = "05181C2C9F24490000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

// every day - sun
var data = "05181C2C9F24090000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

```


## Script:
```
// 
// EVE Energy Decode Days
// 
// thanks to https://github.com/simont77/fakegato-history/issues/90
// 
// TLV8
// Type: 46
// 

var colors = require('colors');

// every day
var data = "05181C2C9F24490000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

// every day - sun
var data = "05181C2C9F24090000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

curIndex = 8
len = 6

process.stdout.write(data.substr(0, curIndex))
curIndex = 0 + curIndex

var console_output = ""

// active days across programs
var unknown = (data.substr(0, curIndex));   // Unknown data for first 6 bytes
// byte swapped 
// bits 6 - 14
// >>> 4
var daynumber = parseInt(data.substr(curIndex, 6).match(/[a-fA-F0-9]{2}/g).reverse().join(''), 16) >>> 4;

process.stdout.write(data.substr(curIndex, len).green)
process.stdout.write(data.substr(curIndex + len))
console.log("")
// console.log(daynumber)

// bit masks for active days
var mon = (daynumber & 0x7);
var tue = ((daynumber >>> 3) & 0x7)
var wed = ((daynumber >>> 6) & 0x7)
var thu = ((daynumber >>> 9) & 0x7)
var fri = ((daynumber >>> 12) & 0x7)
var sat = ((daynumber >>> 15) & 0x7)
var sun = ((daynumber >>> 18) & 0x7)

console.log("M T W T F S S")
console.log(mon, tue, wed, thu, fri, sat, sun)
```
## Output:
```
05181C2C9F24090000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
M T W T F S S
1 1 1 1 1 1 0
``
