

section "History Weather" {

	uint8 "size"
	uint32 "entry Counter"
	uint32 "seconds since ref Time"

	set type [uint8]
	move -1
	hex 1 "type"

	uint32 "ref time"
	uint16 "temperature"
	# uint16 "humidity"
	# uint16 "pressure"
}

