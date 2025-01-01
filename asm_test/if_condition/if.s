	;; program
	lds 0xfffe           ; initialize stack pointer
main:
	;; Function prologue
	ldrs %r15            ; move rsp to rbp
	ldas                
	ldbi 2              
	sub                 
	ldsa                

	;; Load(Stack(0), Imm(2))

	ldai 3               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 0               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Cmp(Stack(0), Imm(2))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 0               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 2               ; immediate
	cmp                 


	;; JumpGreater(label1)

	jg label1            ; jump if lesser


	;; Load(Stack(1), Imm(0))

	ldai 0               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Jump(label2)

	jmp label2          


	;; Label(label1)

label1:


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

	;; Label(label2)

label2:


	;; Cmp(Stack(1), Imm(0))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldbi 0               ; immediate
	cmp                 


	;; JumpZero(label0)

	jz label0            ; jump if zero


	;; Load(Reg(0), Imm(3))

	ldai 3               ; immediate
	ldra %r0

	;; Return()

	hlt

	;; Label(label0)

label0:


	;; Load(Reg(0), Imm(5))

	ldai 5               ; immediate
	ldra %r0

	;; Return()

	hlt
	
	;; Load(Reg(0), Imm(0))

	ldai 0               ; immediate
	ldra %r0

	;; Return()


	hlt                  ; halt at the end of program
	
