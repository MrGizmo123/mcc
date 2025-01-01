assemble: if.s
	mas if.s if.hex

upload: assemble
	mup if.hex /dev/ttyUSB1
