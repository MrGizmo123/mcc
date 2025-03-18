__f_div:
	
	ldri %r0 0		;push 0 onto the stack
	pushr %r0		;this denotes stop condition for subtraction
	pushr2 %r0
	
	ldar %r2		;load denominator into A reg

intrinsic-div_loop:
	ldbr %r1		;load numerator for checking magnitude
	cmp

	
	jg intrinsic-div_exit			;if shifted denominator greater than numerator, exit the loop

	pushr %r2		;else push it
	pushr2 %r2
	
	ldar %r2		;load a reg again as pushr clobbers it
	shl			;shift it left

	ldra %r2		;put result back in r2

	
	jmp intrinsic-div_loop

intrinsic-div_exit:
	ldri %r0 0		; initialize r0 as quotient
	ldri %r8 0		; initialize r8 as remainder 

intrinsic-div_loop2:
	popr %r2		;pop the last 
	popr2 %r2

	ldbr %r2		;load subtrahend
	ldai 0			;check if its 0, if so then exit loop
	cmp
	
	je intrinsic-div_end

	ldar %r0		;left shift quotient
	shl
	ldra %r0
	
	ldar %r1		;load numerator, as denom is already loaded
	ldbr %r2		;load subtrahend
	cmp
	
	jl intrinsic-div_loop2		;if numerator is less than subtrahend, then go to next iteration
	sub			;else subtract it

	ldra %r1		;put result back into numerator slot

	ldar %r0		;add 1 to the quotient
	ldbi 1			;before it is left shifted
	add
	ldra %r0
	
	jmp intrinsic-div_loop2

intrinsic-div_end:
	ldar %r1
	ldra %r8		;put the remaining numerator into the remainder slot
	;; quotient is already filled into r0
	ret
	ret2
	
