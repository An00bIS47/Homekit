
while { ![end] } {
	section "Entry" {
		
		set type [hex 1 "Type"]

		set length [uint8 "Length"]

		if {$length > 0} {
			if {$type == 0x45} {
				set schedule_type [hex 1 "Schedule Type"]
				if {$schedule_type == 0x05} {					
					set program_count [uint8 "Programs Count"]
					hex 7 "unknown"

					# for {set i 1} {$i < $program_count} {incr i} {
						set start_time [hex 2 "Start Time"]		
						set end_time [hex 2 "End Time"]		
					# }
				} else {
					hex 2 "unknown"
				}
			} elseif {$type == 0xD0} {
				uint32 "Last Switch Act."
			} elseif {$type == 0x9B} {
				uint32 "EVE Time"
			} elseif {$type == 0x06} {				
				uint16 "Memory Used"
			} elseif {$type == 0x07} {
				uint32 "Rolled Over Index"
			} elseif {$type == 0x60} {
				hex 1 "Status LED"				
			} else {
				hex $length "Value"								
			}
		}		
	}
}

