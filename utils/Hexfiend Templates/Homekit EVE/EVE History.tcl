
while { ![end] } {
	section "Entry" {
		set size [uint8]
		move -1

		uint8 "size"
		uint32 "entry Counter"
		uint32 "seconds since ref Time"

		set type [uint8]
		move -1
		hex 1 "type"

		if {$type == 0x81} {
		    # Ref  Time 
		    uint32 "reference timestamp"
		    hex 7 "padding"
		} elseif {$type == 0x07} {
		    
		    uint16 "temperature"
		    uint16 "humidity"
		    uint16 "pressure"
		}  elseif {$type == 0x1F} {

			uint16 "??"
			uint16 "??"
			uint16 "power"
			uint16 "??"
			uint16 "??"
		} elseif {$type == 0x01} {
			uint8 "status"
		}
		# else {
		#     puts "vbl is not one or two"
		# }
	}
}

