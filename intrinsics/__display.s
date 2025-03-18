	;; decimal print routine
__display:
	ldri %r4 10		;load a value greater than 9 to mark end condition
	pushr %r4		;push it
	pushr2 %r4

intrinsic-dpr_loop:
	ldri %r2 10		; load 10 into r2 as denominator
				; numerator is already been put in r1 by caller
	subr			; call fast division
	subr2 __f_div

	pushr %r8		; push r8 (remainder)
	pushr2 %r8		; these will be popped later

	ldar %r0		; load quotient into A reg
	ldbi 0			; load 0 for zero checking
	cmp

	je intrinsic-dpr_exit	;if quotient is 0, then all digits are calculated, begin printing

	ldar %r0		; move r0 into r1 for next iteration
	ldra %r1
	
	jmp intrinsic-dpr_loop

intrinsic-dpr_exit:
	
	subr
	subr2 __clear_display

intrinsic-dpr_loop2:
	popr %r1
	popr2 %r1
	
	ldar %r1
	ldbi 10			
	cmp
	je intrinsic-dpr_end	;if a digit is 10 then we have reached end condition, so end

	;; if not then 
	ldbi 0b100110000	;register select flag of LCD, plus conversion to ASCII by adding 48 (value of '0')
	add
	
	outa
	jmp intrinsic-dpr_loop2

intrinsic-dpr_end:
	ret
	ret2
