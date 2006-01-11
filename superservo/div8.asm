; PIC Division Implimentation File
; ---------------------------------------------------------------------------------------
; http://www.colinmackenzie.net
;
;
; History:
;   This source code is based on the CORDIC algorithm. It was taken from Boulette's page
;   found here:
;      http://www.restena.lu/convict/Jeunes/Math/Fast_operations2.htm
;
;   Check out Boulette's robotics page, it has many useful robotic stuff:
;      http://www.restena.lu/convict/Jeunes/RoboticsIntro.htm
;
; KNOWN BUGS
;	None.
;
; WARNINGS
;	None.
;
; NOTES
;	__ cycles per division


#ifndef TEMPY8
#define TEMPY8	0x78	; divisor
#endif

#ifndef TEMPX8
#define TEMPX8	0x79	; numerator
#endif

#ifndef IDX16
#define IDX16	0x7a
#endif

#ifndef RESULT8
#define RESULT8	0x7b	; result
#endif



; ... Must make div using unsigned ints

div8w	macro N,D
; divides Numerator by Divisor, N/D
	movf  N,w
	movwf TEMPX8
	movf  D,w
	movwf TEMPY8
	call  DIVV8
	movf  RESULT8,w
	endm

div8f	macro N,D,R
; divides Numerator by Divisor, N/D
	div8w N,D
	movwf R
	endm

div8uf	macro N,D,R
	movf  N,w
	movwf TEMPX8
	movf  D,w
	movwf TEMPY8
	call  DIVV8_U
	movf  RESULT8,w
	movwf R
	endm

DIVV8_U
	btfsc TEMPX8,7 ; test neg bit
	goto div8uf_neg
	goto DIVV8
div8uf_neg
	comf  TEMPX8,f
	incf  TEMPX8,f
	call  DIVV8
	comf  RESULT8,f
	incf  RESULT8,f
	return


DIVV8
	MOVF  TEMPY8,F
	BTFSC STATUS,Z ;SKIP IF NON-ZERO
	RETURN
	CLRF  RESULT8
	MOVLW  1
	MOVWF  IDX16

SHIFT_IT8
	BCF   STATUS,C
	RLF   IDX16,F
	BCF   STATUS,C
	RLF   TEMPY8,F
	BTFSS  TEMPY8,7
	GOTO  SHIFT_IT8

DIVU8LOOP
	MOVF  TEMPY8,W
	SUBWF  TEMPX8,f
	BTFSC   STATUS,C
	GOTO   COUNT8
	ADDWF   TEMPX8,f
	GOTO  FINAL8

COUNT8
	MOVF  IDX16,W
	ADDWF  RESULT8,f

FINAL8
	BCF   STATUS,C
	RRF   TEMPY8,F
	BCF   STATUS,C
	RRF   IDX16,F
	BTFSS   STATUS,C
	GOTO   DIVU8LOOP
	RETURN
