; PROPORTIONATE INTEGRAL DERIVITIVE	8BIT
; PID control routines
; -----------------------------------------------------------------------------------
; http://www.colinmackenzie.net
;
; History:
;	Created Mar 10th, 2003	Colin MacKenzie
;
; KNOWN BUGS/ISSUES
;	* Integral and Derivitive components not quite working properly.
;	* is ouput scaler required? prob not, eliminate in the future
;   * we are using a loop to move the history buffer, use a rolling buffer
; WARNINGS
;	init pid rolling buffer
;
; REQUISITES
;	must have the following memory locations defined:
;		ACCa, ACCb	math utility accumulators [math.asm]
;		
;	must have the following macros defined:
;		move16		copies 16bit values from src to dest [math.asm]
;		x_add, x_sub	16bit add/subtract subroutines [math.asm]
;		div16b2		16bit division by simply base 2 (shifting) [math.asm]
;
; SETUP
;   place input 8bit value into PID_P, then call CalculatePID, result in w


#ifndef PID_BASE
#define PID_BASE	0x20			; base of PID data block
#endif

#define PID_KP		PID_BASE+0x00	; proportionate gain scaler (2^Kp)
#define PID_KI		PID_BASE+0x01	; integral gain scaler (2^Ki)
#define PID_KD		PID_BASE+0x02	; derivitive gain scaler (2^Kd)
#define PID_KPID	PID_BASE+0x03	; output gain scaler (2^Kpid)
#define PID_P		PID_BASE+0x04	; PID proportionate component
#define PID_I		PID_BASE+0x05	; PID integral component
#define PID_D		PID_BASE+0x06	; PID derivitive component
#define PID_RESULT	PID_BASE+0x07	; PID output
#define PID_TEMP	PID_BASE+0x08	; temp var, will use result space

#define PID_HPTR	PID_BASE+0x09	; current history idx for rolling buffer
#define PID_HBASE	PID_BASE+0x0a	; history base register

#ifndef PID_HMASK
#define PID_HMASK	0x0f		; determines history size using masking
					; (memory reguired = 2^n where n is the number of bits set in mask)
#endif

#include "div8.asm"



pid_calculate
	; P expected in PID_P register
	banksel PID_BASE

	; set inderect ptr to HPTR - the last measured E value
	movf	PID_HPTR,w
	addlw	PID_HBASE
	movwf	FSR

; BEGIN DERIVATIVE
	; calculate derivitive component
	sub8f	PID_P, INDF, PID_D
; END DERIVATIVE

; BEGIN INTEGRAL
_PID_Integrate
; calculate the integral of the history buffer, move history buffer down one too

	; inc rolling buffer ptr, then apply mask
	incf	PID_HPTR,w
	andlw	PID_HMASK
	movwf	PID_HPTR

	; store current PE into next buffer pos
	incf	FSR,f
	movff	PID_P, INDF

	; loop init
	clrf	PID_I			; clear I register
	movlw	PID_HBASE		; move FSR to start of history buffer
	movwf	FSR

	; init loop counter
	movlw	PID_HMASK
	movwf	PID_TEMP
	incf	PID_TEMP,f

	; calc integral of history using a loop, use PID_TEMP as counter
_loop_I
	movf	INDF,w
	addwf	PID_I,f
	incf	FSR,f
	decfsz	PID_TEMP,f
	goto	_loop_I
; END INTEGRAL



#if 0
	; convert the components to absolute values
	abs8	PID_P
	abs8	PID_I
	abs8	PID_D
#endif

;	nop

	; scale compenents by gains
	div8uf	PID_P, PID_KP, PID_P
	div8uf	PID_I, PID_KI, PID_I
	div8uf	PID_D, PID_KD, PID_D

;	nop

	; combine components
	add8f	PID_P, PID_I, PID_RESULT
	add8f	PID_RESULT, PID_D, PID_RESULT

	; scale output
#ifndef PID_NO_OUTPUT_COEFFICIENT
	; not scaling output saves a costly division calculation (~150 cycles)
	div8uf	PID_RESULT, PID_KPID, PID_RESULT
#endif

	return


pid_init
	banksel PID_BASE

	clrf	PID_P			; clear I register
	clrf	PID_I			; clear I register
	clrf	PID_D			; clear I register
	clrf	PID_RESULT		; clear I register
	clrf	PID_HPTR		; clear rolling buffer index

	; loop init
	movlw	PID_HBASE		; move to start of history buffer
	movwf	FSR

	; init loop counter
	movlw	PID_HMASK
	movwf	PID_TEMP
	incf	PID_TEMP,f

	; calc integral of history using a loop, use PID_TEMP as counter
_loop_pidclear
	clrf	INDF
	incf	FSR,f
	decfsz	PID_TEMP,f
	goto	_loop_pidclear
	return


