; PROPORTIONATE INTEGRAL DERIVITIVE
; PID control routines
; -----------------------------------------------------------------------------------
; http://www.colinmackenzie.net
;
; History:
;	Created Mar 10th, 2003	Colin MacKenzie
;
; KNOWN BUGS
;	* Integral and Derivitive components not quite working properly.
;
; WARNINGS
;	None.
;
; REQUISITES
;	must have the following memory locations defined:
;		ACCa, ACCb	math utility accumulators [math.asm]
;		
;	must have the following macros defined:
;		move16		copies 16bit values from src to dest [math.asm]
;		x_add, x_sub	16bit add/subtract subroutines [math.asm]
;		div16b2		16bit division by simply base 2 (shifting) [math.asm]



#define PID_DEPTH 16

#define PID_BASE	0x20			; base of PID data block
#define PID_KP		PID_BASE+0x00	; proportionate gain scaler (2^Kp)
#define PID_KI		PID_BASE+0x02	; integral gain scaler (2^Ki)
#define PID_KD		PID_BASE+0x04	; derivitive gain scaler (2^Kd)
#define PID_KPID	PID_BASE+0x06	; output gain scaler (2^Kpid)
#define PID_P		PID_BASE+0x08	; PID proportionate component
#define PID_I		PID_BASE+0x0a	; PID integral component
#define PID_D		PID_BASE+0x0c	; PID derivitive component
#define PID_RESERVED	PID_BASE+0x0e	; reserved for future use

#define PID_HBASE	PID_BASE+0x10	; history base register
#define PID_HSIZE	0x0e		; number of samples in history 
					; (memory reguired = PID_HSIZE*PID_DEPTH/8)

COPYBYTE	macro	offset
; copies the nth previous byte (specified by literal offset) to the 
; current byte pointed to by the FSR register. Used in 'PIDCalculate'/.
	fill (decf FSR,f), offset	; go back n elements
	movf	INDF,w			; read element to W
	fill (incf FSR,f), offset	; go forward n elements
	movwf	INDF			; write element with W
	endm


PIDCalculate
; given an input value in the ACCa register, calculates
; the new PID output value, returned via the ACCb register.

	; save ACCa in PID_P
	move16	ACCa, PID_P

	; clear math accumulators
	clrf	ACCbLO
	clrf	ACCb

	; move history down, use PID_D as a working register
	movlw	PID_HSIZE-1	; load sample count
	movwf	PID_D
	
	; move to end of queue - 2 bytes
	movlw	PID_HBASE+PID_HSIZE*(PID_DEPTH/8)-1; load indirect address pointer
	movwf	FSR

PIDIntegrateMoveLoop
	; lsb of element
	COPYBYTE 2		; copy lsb from previous element
	decf	FSR,f		; move to previous element

	movwf	ACCaLO		; copy element to ACC lsb

	; msb of element
	COPYBYTE 2		; copy msb from previous element
	decf	FSR,f		; move to previous element

	movwf	ACCa		; copy element to ACC msb
	call	x_add		; integrate history while we are at it
				; summed in ACCb:ACCbLO

	; dec loop counter and loop if not zero
	decf	PID_D,f
	btfss	STATUS,Z
	goto	PIDIntegrateMoveLoop

	; add current sample to history
	move16	PID_P, PID_HBASE

	; calculate integral component
	move16	PID_P, ACCa
	call	x_add
	move16	ACCb, PID_I

	; calculate derivitive component
	sub16	PID_HBASE+2, PID_HBASE
	move16	ACCb, PID_D

	; convert the components to absolute values
	abs16	PID_P
	abs16	PID_I
	abs16	PID_D

	; scale compenents by gains
	div16b2	PID_P, PID_KP
	div16b2	PID_I, PID_KI
	div16b2	PID_D, PID_KD

	; combine components
	move16	PID_P, ACCa
	move16	PID_I, ACCb
	call	x_add
	move16	PID_D, ACCa
	call	x_sub

	; scale output
	div16b2	ACCb, PID_KPID

	return



PIDL	macro X, Y
; sets the PID input to the literal value X:Y and calculates PID output. The
; output is returned in ACCb.
	movlw	X
	movwf	ACCa
	movlw	Y
	movwf	ACCa+1
	call	PIDCalculate
	endm

PID	macro VAR
; sets the PID input to the value in VAR and calculates PID output. The
; output is returned in ACCb.
#if(VAR!=ACCa)
	move16	VAR, ACCa
#endif
	call	PIDCalculate
	endm


