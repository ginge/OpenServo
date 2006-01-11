; DIGITAL SERVO CONTROLLER					PIC Peripheral Initializers
; ---------------------------------------------------------------------------------------
; http://www.colinmackenzie.net
;
; CVS REPOSITORY: (get cvs at http://www.cvshome.org/)
;   The superservo source files are held in a cvs repository. CVS is the 
;   Concurrent Versions System, the dominant open-source network-transparent 
;   version control system.  
;
;   to login to cvs:
;   cvs -d :pserver:username@cvs.colinmackenzie.net:/var/lib/cvs login
;
;   to checkout servo controller:
;   cvs -d :pserver:username@cvs.colinmackenzie.net:/var/lib/cvs checkout superservo
;
; DESCRIPTION:
;   This file contains the routines that configure the PIC peripherals for the servo.
;
; HISTORY:
;	Created Dec 28th, 2005	Colin MacKenzie
;
; KNOWN BUGS:
;	None.
;
; WARNINGS:
;	None.
;
; ROUTINES:
;   ports_configure -- configure PIC ports
;   adc_configure -- configure ADC to capture server pot (current position)
;   pwm_configure -- configure PWM output to H-BRIDGE
;   timer_configure -- configure the sample timer (determines system bandwidth)


; Configure the two PWM channels, these PWM channels ouput to the mosfet H-BRIDGE.
pwm_configure
; FUTURE: Redo these two setup procedures into one, it would
; be faster and more efficient!
	; set PWM period
	movlw	0x3f	; our pwm period for PWM 1 & 2
	banksel PR2
	movwf	PR2

; CONFIGURE PWM #1
	; set PWM duty cycle
	banksel CCPR1L
	clrf	CCPR1L
	clrf	CCPR1H

	; clear the two LSBs of the duty cycle
	bcf	CCP1CON, 5
	bcf	CCP1CON, 4

	; make CCP1 bit (on PORTC) an output
	banksel TRISC
	bcf	TRISC, 2

	; Set TMR2 prescale
	banksel T2CON
	bcf	T2CON, T2CKPS1
	bcf	T2CON, T2CKPS0

	; enable TMR2
	bsf	T2CON, TMR2ON

	; set CCP1 to PWM mode
	bsf	CCP1CON, CCP1M3
	bsf	CCP1CON, CCP1M2
	bsf	CCP1CON, CCP1M1
	bsf	CCP1CON, CCP1M0
	
; CONFIGURE PWM #2
	; set PWM duty cycle
	banksel CCPR2L
	clrf	CCPR2L
	clrf	CCPR2H

	; clear the two LSBs of the duty cycle
	bcf	CCP2CON, 5
	bcf	CCP2CON, 4

	; make CCP1 bit (on PORTC) an output
	banksel TRISC
	bcf	TRISC, 1

	; set CCP1 to PWM mode
	banksel CCP2CON
	bsf	CCP2CON, CCP2M3
	bsf	CCP2CON, CCP2M2
	bsf	CCP2CON, CCP2M1
	bsf	CCP2CON, CCP2M0
	
	return


timer_configure
; configure the sample timer
	banksel TMR1L
	movf	SERVO_CLOCK,w
	movwf	TMR1H
	clrf	TMR1L			; clear timer

	; configure timer1
	clrf	T1CON			; clear timer1 control register
	bsf	T1CON, TMR1ON

	banksel PIE1
	bcf	PIE1, TMR1IF		; clear timer interrupt flag
	bsf	PIE1, TMR1IE		; enable timer interrupt
	return

; Configures the PIC ADC to capture the current servo position
adc_configure
	; configure ADC
	banksel ADCON1
	clrf	ADCON1
	; bcf	ADCON1, ADFM	; cleared for right justification of 10bit sample
							; in ADRESH:ADRESL (ADRESL[7:6] contains lsb of sample)

	banksel ADCON0
	clrf	ADCON0			; clear ADC control registers

	; set ADC to channel 2
	bsf		ADCON0, CHS2

	; set conversion clock
	bsf		ADCON0, ADCS1

	; enable the adc
	bsf	ADCON0, ADON

	; enable interrupts for timer1 and ADC
	banksel PIE1
	bcf	PIE1, ADIF			; clear adc flag
	bsf	PIE1, ADIE			; enable ADC interrupt

	return


; Configures the ports of the PIC
ports_configure
; clear ports
	banksel STATUS
	clrf	STATUS
	banksel PORTA
	clrf	PORTA
	clrf	PORTB
	clrf	PORTC

; configure directions for the port registers

; configure PORTA
;		A[0:7]	Configure as input
	movlw   0xff
	banksel TRISA
	movwf   TRISA

; configure PORTB
;		B[0]	int intput
;		B[1]	debug output
;		B[2]	debug output
;		B[2:7]	Unused
	movlw   0x79
	movwf   TRISB

; configure PORTC
;		C[0]	Unused
;		C[1]	CCP2 PWM output			(o)
;		C[2]	CCP1 PWM output			(o)
;		C[3]	I2C Serial Clock		(i on idle)
;		C[4]	I2C Serial Data			(i on idle)
;		C[5]	Unused					(i)
;		C[6]	RS232 Serial Transmit	(i)
;		C[7]	RS232 Serial Receive	(i)
	movlw   0xf9
	movwf   TRISC

	return
