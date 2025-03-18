__input:

	;; accept input
	in intrinsic-input-userin

	;; move the input to r0
	ldam intrinsic-input-userin
	ldra %r0

	;; return
	ret
	ret2

intrinsic-input-userin:
	hlt
