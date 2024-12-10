	;; program
	lds 0xfffe           ; initialize stack pointer
main:
	;; Function prologue
	ldrs %r15            ; move rsp to rbp
	ldas                
	ldbi 2              
	sub                 
	ldsa                

	;; JumpZero(Imm(21), label0)

	ldai 21              ; immediate
	ldbi 0               ; for comparison
	cmp                  ; do the comparison
	jz label0            ; jump if zero


	;; JumpZero(Imm(0), label0)

	ldai 0               ; immediate
	ldbi 0               ; for comparison
	cmp                  ; do the comparison
	jz label0            ; jump if zero


	;; Load(Stack(0), Imm(1))

	ldai 1               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 0               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Jump(label1)

	jmp label1          


	;; Label(label0)

label0:


	;; Load(Stack(0), Imm(0))

	ldai 0               ; immediate
	ldra %r12            ; save the A register
	ldrb %r13            ; save the B register
	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 0               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmra %r12           ; load the value to the stack
	ldar %r12            ; restore old values
	ldbr %r13            ; restore old values

	;; Label(label1)

label1:


	;; JumpZero(Psuedo(tmp0), label2)

	ldbi 0               ; for comparison
	cmp                  ; do the comparison
	jz label2            ; jump if zero


	;; JumpZero(Imm(3), label2)

	ldai 3               ; immediate
	ldbi 0               ; for comparison
	cmp                  ; do the comparison
	jz label2            ; jump if zero


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

	;; Jump(label3)

	jmp label3          


	;; Label(label2)

label2:


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

	;; Label(label3)

label3:


	;; Load(Reg(0), Stack(1))

	;; Stack
	ldar %r15            ; load rsp into A register
	ldbi 1               ; load offset into B reg
	sub                  ;  now address is stored in A reg, can be acessed using ldmaa

	ldmaa                ; load the value from calculated address in stack frame
	ldra %r0

	;; Return()


	hlt                  ; halt at the end of program
