
proc bit_reverse {str} {
    binary scan $str B* bits
    binary format b* $bits
}


section "Log Info" {

	uint32 "Eve Time" 
	uint32 "Negativ Offset" 
	uint32 "Reference Time"
	
	set number_of_entries [uint8]
	move -1
	uint8 "Signature Length"

	set sig_len [expr {
    	$number_of_entries * 2
	}]	
	hex $sig_len "Signature"

	set usedMem [uint16 "Used Memory"]
	
	set v [bit_reverse $usedMem]
	

	#hex 2 "Used Memory"
	uint16 "Size"
	uint32 "Rollover"
	hex 4 "??"
	hex 2 "End"

}

