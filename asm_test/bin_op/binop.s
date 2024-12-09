	;; program
	lds 0xfffe           ; initialize stack pointer
main:
	;; Function prologue
	ldrs %r15            ; move rsp to rbp
	ldas                
	ldbi 3              
	sub                 
	ldsa                

	;; Sub(Stack(0), Imm(6), Imm(7))

	ldai 6               ; immediate
	ldbi 7               ; immediate
	sub                 
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 0               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values


	;; Add(Stack(1), Imm(21), Stack(0))

	ldai 21              ; immediate
	ldra %r12            ; save the A register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 0               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldra %r13            ; auxiliary
	ldbr %r13            ; transfer to B register
	ldar %r12            ; put original value back in A register
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


	;; BitAnd(Stack(2), Stack(1), Imm(6))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 6               ; immediate
	and                 
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 2               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values


	;; Load(Reg(0), Stack(2))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 2               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldra %r0

	;; Return()


	hlt                  ; halt at the end of program
