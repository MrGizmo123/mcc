assemble: binop.s
	mas binop.s binop.hex

upload: assemble
	mup binop.hex /dev/ttyUSB1
