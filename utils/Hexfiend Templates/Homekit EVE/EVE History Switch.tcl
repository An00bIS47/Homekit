

section "History Switch" {

	uint8 "size"
	uint32 "entry Counter"
	uint32 "seconds since ref Time"

	set type [uint8]
	move -1
	hex 1 "type"

	uint32 "ref time"
	uint8 "status"
}

