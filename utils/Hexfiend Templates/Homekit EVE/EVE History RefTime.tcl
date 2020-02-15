

section "History RefTime" {

	uint8 "size"
	uint32 "entry Counter"
	uint32 "seconds since ref Time"

	set type [uint8]
	move -1
	hex 1 "type"

	uint32 "ref time"

	hex 7 "padding"

	#hex 2 "Used Memory"
	# uint16 "Size"
	# uint32 "Rollover"
	# hex 4 "??"
	# hex 2 "End"

}

