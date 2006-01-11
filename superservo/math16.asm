;
;*******************************************************************
;                 Double Precision Addition & Subtraction
;
;*******************************************************************;
;   Addition :  ACCb(16 bits) + ACCa(16 bits) -> ACCb(16 bits)
;      (a) Load the 1st operand in location ACCaLO & ACCaHI ( 16 bits )
;      (b) Load the 2nd operand in location ACCbLO & ACCbHI ( 16 bits )
;      (c) CALL x_add
;      (d) The result is in location ACCbLO & ACCbHI ( 16 bits )
;
;   Performance :
;               Program Memory  :       07
;               Clock Cycles    :       08
;*******************************************************************;
;   Subtraction : ACCb(16 bits) - ACCa(16 bits) -> ACCb(16 bits)
;      (a) Load the 1st operand in location ACCaLO & ACCaHI ( 16 bits )
;      (b) Load the 2nd operand in location ACCbLO & ACCbHI ( 16 bits )
;      (c) CALL x_sub
;      (d) The result is in location ACCbLO & ACCbHI ( 16 bits )
;
;   Performance :
;               Program Memory  :       14
;               Clock Cycles    :       17
;
;
;       Program:          DBL_ADD.ASM 
;       Revision Date:   
;                      1-13-97      Compatibility with MPASMWIN 1.40
;
;*******************************************************************;
;

; must define the following variables or accept these defaults
#ifndef ACCa
#define ACCa  78
#endif

#ifndef ACCb
#define ACCb  7a
#endif


comp16  macro	VAR
	comf    VAR+1, F
	incf    VAR+1, F
	btfsc   STATUS,Z
	decf    VAR, F
	comf    VAR, F
	endm

;*******************************************************************
;         Double Precision Subtraction ( ACCb - ACCa -> ACCb )
;
x_sub   comp16	ACCa           ; At first negate ACCa; Then add
;
;*******************************************************************
;       Double Precision  Addition ( ACCb + ACCa -> ACCb )
;
x_add   movf    ACCa+1,W
	addwf   ACCb+1, F       ;add lsb
	btfsc   STATUS,C        ;add in carry
	incf    ACCb, F
	movf    ACCa,W
	addwf   ACCb, F       ;add msb
	retlw   0


clr16	macro	VAR
; clears 16bit variable
	clrf	VAR
	clrf	VAR+1
	endm

WORD	macro VAR, X, Y
; placed the value X:Y into 16bit variable VAR
	movlw	X
	movwf	VAR
	movlw	Y
	movwf	VAR+1
	endm

move16	macro	S, D
; moves 16bit value from S to D
	movf	S,w
	movwf	D
	movf	S+1,w
	movwf	D+1
	endm

add16	macro	A, B
; adds two 16bit variables A and B
#if(A!=ACCa)
	move16	A, ACCa
#endif
#if(B!=ACCb)
	move16	B, ACCb
#endif
	call	x_add
	endm

sub16	macro	A, B
; subtracts two 16bit variables A and B
#if(A!=ACCa)
	move16	A, ACCa
#endif
#if(B!=ACCb)
	move16	B, ACCb
#endif
	call	x_sub
	endm

shr16	macro	A, B
; shifts 16bit variable 'A' B times to the right
	local	shr16loop, shr16e
	movf	B,w
	btfsc	STATUS,Z
	goto	shr16e
	movwf	ACCa	; use ACCa as counter
shr16loop
	bcf	STATUS,C
	rrf	A,f
	rrf	A+1,f
	decf	ACCa,f
	btfss	STATUS,Z
	goto	shr16loop
shr16e	
	endm


shl16	macro	A, B
; shifts 16bit variable 'A' B times to the left
	local	shl16loop, shl16e
	movf	B,w
	btfsc	STATUS,Z
	goto	shl16e
	movwf	ACCa	; use ACCa as counter
shl16loop
	bcf	STATUS,C
	rlf	A+1,f
	rlf	A,f
	decf	ACCa,f
	btfss	STATUS,Z
	goto	shl16loop
shl16e	
	endm

div16b2	macro A, B
; shifts 16bit variable 'A' B times to the left or right
; depending on the polarity of B. Impliments a cheap
; division algorithm using shifting.
	local div16b2shr, div16b2e
	; determine which direction to shift
	btfsc	B+1, 7
	goto	div16b2shr
	shl16	A, B+1
	goto div16b2e
div16b2shr
	decf	B+1,f
	comf	B+1,f
	shr16	A, B+1
	decf	B+1,f
	comf	B+1,f
div16b2e
	endm


abs16	macro A
; returns in A the absolute value of A.
	local abs16e
	btfss	A, 7	; is A negative
	goto abs16e
	comp16 A
abs16e
	endm




