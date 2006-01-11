; DIGITAL SERVO CONTROLLER					Test Bench
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
;   By defining TESTBENCH in register.h, the code here will be executed to test the
;   servo PID loop. Do this only for testing/probing code in the simulator.
;
; HISTORY:
;	Created Dec 28th, 2005	Colin MacKenzie
;
; KNOWN BUGS
;	None.
;
; WARNINGS
;	* enable the testbench only for simulation!
;

#ifdef TESTBENCH
	banksel	PID_BASE

#if 0
	movlw	0x13
	movwf	PID_P
	call pid_calculate

	movlw	0x08
	movwf	PID_P
	call pid_calculate

	movlw	0x03
	movwf	PID_P
	call pid_calculate

	movlw	0x00
	movwf	PID_P
	call pid_calculate
#endif

	banksel	ADRESL
	movlf	0xc0, ADRESL
	banksel	SERVO_E
	movlf	0x2c, SERVO_E
	bsf		SERVO_STATUS, SSF_OE
	goto convert_pe

#if 0
	; test our ADC isr
	banksel ADRESL
	movlf	0xc0, ADRESL
	banksel ADRESH
	movlf	0x5b, ADRESH
	call adc_conversion_complete_isr
#endif

	; now test our pid loop
	bsf		SERVO_STATUS, SSF_PIDRUN
	movlf	0x10, SERVO_DP
	movlf	0xf0, SERVO_CP
	goto pid_loop_idle


#endif
