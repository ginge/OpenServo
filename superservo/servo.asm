; DIGITAL SERVO CONTROLLER
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
;
; HISTORY:
;	Created Feb 25th, 2003	Colin MacKenzie
;   Dec 28th, 2005: Brought PID loop out of ISR into main loop, PID loop will now be
;					determinate based on sample clock.
;
; KNOWN BUGS:
;   * PID 16Bit is not finished.
;	* I think subtraction in PID is causing some carry's and making pos numbers <0, causing jitter
;
; WARNINGS:
;	None.
;
; TODO:
;   * is PID sign's correct?
;   * does enabling 10bit ADC work?
;   * complete PID 16bit version. (someday! 8bit works so good and fast!)
;	* if IIC overruns or stalls, does my code recover?

; config file has configurable compile options and the PIC target device
#include "config.asm"

; registers file configures the memory register layout of the servo code modules
#include "registers.asm"

#include "isr.asm"

; Set program memory base at reset vector 0x00
	org 0x0000

; STARTUP CODE
	goto restart	; normal reset vector
	goto restart	; firmware programming reset vector (if bootload exists)
	nop
	nop

isr_table
; Interrupt Service Routine for the Servo Controller
;     macros are defined in helpers.asm
	; save the user context
	ISR_CONTEXT_SAVE

	; timer interrupt jump table
	;			INT_REG, INT_FLAG, JUMP_VECTOR
	banksel PIR1
	isr_jumpif	PIR1, 	TMR1IF, sample_timer_isr
	isr_jumpif	PIR1, 	ADIF, 	adc_conversion_complete_isr
	isr_jumpif	PIR1, 	SSPIF, 	iic_isr
	isr_jumpif	INTCON, INTF, 	user_isr
#ifdef EEPROM_ASYNC_SAVE
	isr_jumpif	PIR2, 	EEIF,	eeprom_isr
#endif

exit_isr
	; all isr handlers should 'goto' here when they are done so the user
	; context can be restored!
	ISR_CONTEXT_RESTORE
	retfie					; return from interrupt


; Implement code here to be executed when the INT pin on the pic is toggled.
;   With OBSTACLE_DETECT enabled you can have it halt servo movement (DP <- CP) 
;   and set the servo status flag SSF_OBSTACLE. Wire a bumper switch to the 
;   PIC INT pin.
user_isr
	bcf	INTCON, INTF
#ifdef OBSTACLE_DETECT
	movf	SERVO_CP,w
	movwf	SERVO_DP
	bsf		SERVO_STATUS, SSF_OBSTACLE
#endif
	goto exit_isr


; include math subroutines
#include "math8.asm"

; some helper macros and routines
#include "helpers.asm"

; include Proportionate-Integral-Derivitive (PID) algorithm
#ifdef PID_16BIT
#include "math16.asm"
#include "pid16.asm"
#else
#include "pid8.asm"
#endif

; the eeprom source module
#include "eeprom.asm"


; BEGIN IIC SECTION

iic_getAddress
; called to get the device address during configuration
; you can provide your own method to set the device address
; if desired.
	; try loading iic address from eeprom location 0x02
	banksel SERVO_IICADDR
	bcf				STATUS, IRP
	eeprom_seek		0x00
	eeprom_read		PID_TEMP
	comf			PID_TEMP,w
	btfss			STATUS,Z
	goto			iic_eeprom_adr_valid
	rlf				SERVO_IICADDR,w					; return double addr for i2c register
	return	
iic_eeprom_adr_valid
	movff			PID_TEMP, SERVO_IICADDR
	rlf				SERVO_IICADDR,w					; return double addr for i2c register
	return

; the i2c implimentation
#include "i2c.asm"

; END IIC SECTION



sample_timer_isr
; Called when the sample timer has expired.
;   initiate a new ADC conversion.
	bcf	PIR1, TMR1IF		; clear timer1 interrupt flag

	; reload sample timer
	movf	SERVO_CLOCK,w
	sublw	0xff
	movwf	TMR1H			; MSByte

	; if servo is off, dont start a conversion, this will stop the PID loop too
	btfsc	SERVO_STATUS, SSF_OFF
	goto 	exit_isr

	bsf		ADCON0, GO		; start conversion
	goto 	exit_isr

adc_conversion_complete_isr
; Called when an ADC conversion has completed.
;     store the new value into CP & the 2 lsb's, detect overrun conditions
	bcf		PIR1, ADIF		; clear ADC interrupt flag

	; save sample as current position
#ifdef ADRESH
	; device supports 10bit ADC
	movf	ADRESH,w
	movwf	SERVO_CP

 #ifdef ADC_10BIT
	; move the 2 lsb's of ADC value into status register
	movlw	~ ((1<<SSF_ADC0) | (1<<SSF_ADC1))
	andwf	SERVO_STATUS,f
	banksel	ADRESL
	movf	ADRESL,w
	banksel	SERVO_STATUS
	iorwf	SERVO_STATUS,f
 #endif
#else
	; device only supports 8bit ADC
	movf	ADRES,w
	movwf	SERVO_CP
#endif

	; detect overrun condition, is the previous PID loop iteration still calculating?
	btfsc	SERVO_STATUS, SSF_PIDRUN
	goto	servo_overrun				; overrun condition!

	; set PID loop to run
	bsf		SERVO_STATUS, SSF_PIDRUN
	goto	exit_isr


servo_overrun
; the PID loop took to long and is already running while we are ready for the next sample. Keep
; a total of the number of overruns. We can optionally automatically adjust the clock higher to 
; so this wont happen. In any case, this iteration will be skipped.
	incf	SERVO_OVERRUNS_L,f
	btfsc	STATUS,Z
	incf	SERVO_OVERRUNS_H,f

#ifdef AUTO_ADJUST_BANDWIDTH
	incf	SERVO_CLOCK,f
#else
	; only adjust bandwidth if servo option is set
	btfsc	SERVO_OPTIONS, SO_AUTO_ADJUST_BANDWIDTH
	incf	SERVO_CLOCK,f
#endif
	goto	exit_isr



restoreSettings
	; set servo default position
	banksel SERVO_DP
	movlw	0x7f
	movwf	SERVO_DP
	movlw	0xff
	movwf	SERVO_SPEED

	; reset servo status register
	clrf	SERVO_STATUS

#ifdef DISABLE_OUTPUT_ON_STARTUP
	bcf		SERVO_STATUS, SSF_OE	; disable servo output by default
#else
	bsf		SERVO_STATUS, SSF_OE	; enable servo output by default
#endif	

	; clear servo options register
	clrf	SERVO_OPTIONS

	; set default iic address
	banksel	SERVO_IICADDR
	movlw	IIC_ADDR_DEFAULT
	movwf	SERVO_IICADDR

	; reset sample clock freq
	movlw	SAMPLETIMER_RELOAD_DEFAULT
	movwf	SERVO_CLOCK

	; set PID coefficients
	movlf	PID_DEFAULT_KP, PID_KP
	movlf	PID_DEFAULT_KI, PID_KI
	movlf	PID_DEFAULT_KD, PID_KD
	movlf	PID_DEFAULT_KPID, PID_KPID

	; clear performance counters
	clrf	SERVO_OVERRUNS_L
	clrf	SERVO_OVERRUNS_H
	clrf	SERVO_ITER_L
	clrf	SERVO_ITER_H

	retlw 	0x00



; our initialization routines are defined here
#include "pic-init.asm"

restart
; Power-on startup procedure, can also be called to reset the
; device.
	; clear interrupt registers
	banksel	INTCON
	clrf	INTCON
	banksel PIE1
	clrf	PIE1
	clrf	PIE2
	clrf	SERVO_IDL
	clrf	SERVO_IDH

	; reset to servo defaults
	call	restoreSettings

	; configure PIC peripherals
	call 	pid_init
	call	ports_configure
	call	adc_configure
	call	pwm_configure
	call	timer_configure
	call	iic_configure
	call	eeprom_configure

	; call testbench if enabled
#ifdef TESTBENCH
#include "testbench.asm"
#endif

	; enable interrupts
	banksel OPTION_REG
	bsf	OPTION_REG, INTEDG	; set int to edge trigger
	banksel INTCON
	bsf	INTCON, INTE		; enable interrupt on RB0/INT
	bsf	INTCON, PEIE		; enable peripheral interrupts
	bsf	INTCON, GIE		; enable global interrupts

	;call eeprom_write_async
	; load settings from eeprom
#ifdef EEPROM_LOAD_ON_STARTUP
	call	loadSettings
#endif

	; enter pid loop!


pid_loop
	; reset run flag
	bcf		SERVO_STATUS, SSF_PIDRUN

pid_loop_idle
	; we must wait for SSF_PIDRUN to be set
	btfss	SERVO_STATUS, SSF_PIDRUN
	goto 	pid_loop_idle

	; increment servo iteration counter
	incf	SERVO_ITER_L,f
	btfsc	STATUS,Z
	incf	SERVO_ITER_H,f

	; hit the watchdog. We put this here to catch ISR or PIDLOOP freeze. If  interupts
	; freeze, PIDRUN would not get set to run PID loop. If a PIDLOOP freezes, PIDRUN
	; would not get cleared so the interrupt wound not start another.
	clrwdt

	; calculate Proportional Error (PE = desired_position - current_position)
	movf	SERVO_CP,w
	subwf	SERVO_DP,w
	movwf	SERVO_E

	; move the sign into the servo status register
	bcf		SERVO_STATUS, SSF_ENEG
	btfsc	STATUS, C
	goto	convert_pe				; PE is pos
	
	; PE is negative, compliment it
	bsf		SERVO_STATUS, SSF_ENEG	; save sign of PE
	comf	SERVO_E,f				; abs(PE) -- 2's compliment
	incf	SERVO_E,f

convert_pe
	; return to idle if output is disabled
	btfss	SERVO_STATUS, SSF_OE
	goto	stop_servo					; return to idle

	; the pid algorithm expects P to be a signed 8bit int. We will simply chop the 8th bit,
	; and then encode based on ESIGN flag. We want to maintain the resolution at small values
	; of E, not concerned with large values of E>127.
	; copy E to P
	movff	SERVO_E, PID_P

	; if enabled, shift the two lsb of ADC (ADRESL) into the Proportional Error
#ifdef ADRESL
#ifdef ADC_10BIT
	; make 0<=P<=127
	maxl	PID_P, 0x1f

	banksel	ADRESL
	movf	ADRESL,w
	banksel PID_TEMP
	movwf	PID_TEMP
	rlf		PID_TEMP,f
	rlf		PID_P,f
	rlf		PID_TEMP,f
	rlf		PID_P,f
#else
	; make 0<=P<=127
	movlw	0x7f					; if bit P[7] is set, we will max P at 127
	btfsc	PID_P,7
	movwf	PID_P					; max P at 127
#endif
#endif

	
	btfss	SERVO_STATUS, SSF_ENEG ; if ESIGN not set, we can now calc pid
	goto	calc_pid

	; compliment P back to neg for PID calculation
	comf	PID_P,f
	incf	PID_P,f	

calc_pid
	; calculate PID
	call	pid_calculate

	; determine PID polarity
	movf	PID_RESULT,f	; so we can test Z
	btfsc	STATUS, Z
	goto	stop_servo
	btfss	PID_RESULT, 7
	goto	ccw_servo

cw_servo
	; supress counter clockwise PWM
	clrf	CCPR2L

#ifdef SPEED_LIMITER
	mov8	PID_RESULT, PID_TEMP
	maxf	PID_TEMP, SERVO_SPEED
	movf	PID_TEMP,w
#endif

	; set clockwise PWM
	banksel	CCPR1L
	movwf	CCPR1L
	goto pid_loop

ccw_servo
	; supress clockwise PWM
	clrf	CCPR1L

	; get absolute value of PID
	comf	PID_RESULT,w
	movwf	PID_TEMP
	incf	PID_TEMP,f

#ifdef SPEED_LIMITER
	maxf	PID_TEMP, SERVO_SPEED
#endif

	movf	PID_TEMP,w
	banksel	CCPR2L
	movwf	CCPR2L		; set counter clockwise PWM
	goto pid_loop

stop_servo
	; no PWM output
	banksel CCPR1L
	clrf	CCPR1L
	clrf	CCPR2L
	goto pid_loop



; Include the vector tables and functions for i2c communication
#include "functions.asm"




	END



