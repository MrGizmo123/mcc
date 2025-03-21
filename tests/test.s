	;; program
	lds 0xfffe           ; initialize stack pointer
	ldrs %r15            ; initialize rbp
	jmp main             ; jump to the main function label (not call)
main:
	;; Function prologue
	pushr %r15           ; save current rbp to stack
	pushr2 %r15          ; save current rbp to stack part 2
	ldrs %r15            ; move rsp to rbp
	;; AllocateStack(6)

	ldas                
	ldbi 6              
	sub                 
	ldsa                

	;; Call(__init_display)

	subr                 ; function call
	subr2 __init_display ; function call part 2

	;; Load(Stack(0), Reg(0))

	ldar %r0
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	;; As offset is 0, no further action is needed

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Load(Stack(1), Imm(1))

	ldai 1               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Label(start_for.0)

start_for.0:


	;; Cmp(Stack(1), Imm(5))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 5               ; immediate
	cmp                 


	;; JumpGreater(label0)

	jg label0            ; jump if lesser


	;; Load(Stack(2), Imm(1))

	ldai 1               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 2               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Jump(label1)

	jmp label1          


	;; Label(label0)

label0:


	;; Load(Stack(2), Imm(0))

	ldai 0               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 2               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Label(label1)

label1:


	;; Cmp(Stack(2), Imm(0))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 2               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 0               ; immediate
	cmp                 


	;; JumpZero(break_for.0)

	jz break_for.0       ; jump if zero


	;; Call(__clear_display)

	subr                 ; function call
	subr2 __clear_display ; function call part 2

	;; Load(Stack(3), Reg(0))

	ldar %r0
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 3               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Add(Stack(4), Stack(1), Imm(11))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 11              ; immediate
	add                 
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 4               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values


	;; Load(Reg(1), Stack(4))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 4               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldra %r1

	;; Call(__display)

	subr                 ; function call
	subr2 __display      ; function call part 2

	;; Load(Stack(5), Reg(0))

	ldar %r0
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 5               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Label(continue_for.0)

continue_for.0:


	;; Add(Stack(1), Stack(1), Imm(1))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 1               ; immediate
	add                 
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values


	;; Jump(start_for.0)

	jmp start_for.0     


	;; Label(break_for.0)

break_for.0:


	;; Load(Reg(0), Imm(0))

	ldai 0               ; immediate
	ldra %r0

	hlt                  ; halt as this is a return point of the main function
	;; Return()

	;; Function epilogue
	ldsr %r15            ; move rbp to rsp
	popr %r15            ; retreive old rbp from stack
	popr2 %r15           ; retreive old rbp from stack part2
	ret                 
	ret2                

	hlt                  ; halt at the end of program
;; Intrinsic functions are inserted after this point


;; --------- Intrinsic __clear_display---------

__clear_display:
	out 0b1

	ret
	ret2


;; --------- Intrinsic __display---------

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


;; --------- Intrinsic __f_div---------

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
	


;; --------- Intrinsic __init_display---------

__init_display:
	out 0b111000
	out 0b1110
	out 0b110

	ret
	ret2

