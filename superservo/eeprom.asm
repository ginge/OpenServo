; DIGITAL SERVO CONTROLLER										EEPROM
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
;
; MACROS:
;

#ifndef EEPROM_IDX
#define EEPROM_IDX		0x75
#endif

; this is the table of registers that will be saved to EEPROM. you may add registers here if
; you want them also saved to eeprom. This is a jump table so you can also jump to another
; routine that dynamically returns the register file addr in W. This is useful if you want to
; dynamically compute a value in a temp register then return that temp register address in W
; to have the contents of it stored.
eeprom_regtable
	addwf	PCL,f
	retlw	SERVO_IICADDR
	retlw	SERVO_DP
	retlw	SERVO_SPEED
	retlw	SERVO_OPTIONS
	retlw	SERVO_CLOCK
	retlw	PID_KP
	retlw	PID_KI
	retlw	PID_KD
	retlw	PID_KPID
	retlw	SERVO_IDH
	retlw	SERVO_IDL
	retlw	0



; sets the eeprom EEADR register with a new address value
eeprom_seek macro eeadr
	banksel	EEADR
	movlw	eeadr
	movwf	EEADR
	endm

; performs an asynchronous eeprom read
eeprom_read macro srcf 
	movlw	srcf
	movwf	FSR
	call	eeprom_sync_read_indf
	endm

; performs an asyncronous eeprom write
eeprom_write macro destf 
	movlw	destf
	movwf	FSR
	call	eeprom_sync_write_indf
	endm


eeprom_read_sync
	banksel	EEADR
	clrf	EEADR
eeprom_read_loop
	; get register from eeprom table
	movf	EEADR,w
	call	eeprom_regtable

	; set FSR to requested register	
	movwf	FSR
	
	; exit if all reads were made
	movf	FSR,w
	btfsc	STATUS,Z
	goto	eeprom_end_read_loop

	call	eeprom_sync_read_indf		; read byte from eeprom
	goto eeprom_read_loop	; next itr
eeprom_end_read_loop
	return

eeprom_sync_read_indf
; reads from eeprom into INDF register. 
; address is already in EEADR, writes data to INDF register, EEADR is incremented
	banksel	EECON1
	bcf 	EECON1,EEPGD 	; Point to Data memory
	bsf 	EECON1,RD 		; EE Read
	bcf 	STATUS,RP0 		; Bank 2
	
	movf 	EEDATA,W 		; W = EEDATA
	movwf	INDF			; write to INDF, FSR points to dest register
	incf	EEADR,f			; increment EE address for next read
	return


eeprom_configure
	banksel	PIE2
#ifdef EEPROM_ASYNC_SAVE
	bsf		PIE2, EEIE		; enable ee interrupt
#endif
	return



#ifdef EEPROM_ASYNC_SAVE

; this function initiates the writing of the eeprom table to the eeprom.
eeprom_write_async
	banksel	EEADR
	clrf	EEADR
	comf	EEADR,f			; -1, isr will inc
	banksel	PIR2
	bsf		PIR2, EEIF
	return

	clrw
	call	eeprom_regtable

	; eeprom_async_write_indf will reference the register pointed to by FSR
	movwf	FSR

	; initiate async writes, this routine calls return so it will return out of this routine
;	bcf 	INTCON,GIE 		; disable INTs.
	bcf		STATUS, RP0
	bcf		STATUS, RP1
 	goto	eeprom_async_write_indf
;	bsf 	INTCON,GIE 		; Enable INTs.
;	return

; the eeprom asynchronous read/write ISR handler
eeprom_isr
	bcf		PIR2, EEIF
;  goto exit_isr
	; get the next register to store in W
	banksel	EEADR
	incf	EEADR,f
	movf	EEADR,w
	call	eeprom_regtable
	
	; eeprom_async_write_indf will reference the register pointed to by FSR
	movwf	FSR
	
	; if FSR is 0, we are completed all writes
	movf	FSR,w
	btfsc	STATUS,Z
	goto	exit_isr		; done all writes

	; store next eeprom byte
	bcf		STATUS, RP0
	bcf		STATUS, RP1
	call	eeprom_async_write_indf

	goto	exit_isr

eeprom_async_write_indf
; initiates an eeprom write and returns immediately. FSR should point to register
; whose contents should be written. An eeprom interrupt is generated when the write
; is complete.
	MOVF 	INDF,W			; Data Memory Value
	banksel	EEDATA
	MOVWF 	EEDATA 			; to write
	BSF 	STATUS,RP0		; Bank 3
	BCF 	EECON1,EEPGD 	; Point to DATA memory
	BSF 	EECON1,WREN 	; Enable writes
	MOVLW 	0x55
	MOVWF 	EECON2 			; Write 55h
	MOVLW 	0xAA
	MOVWF 	EECON2 			; Write AAh
	BSF 	EECON1,WR 		; Set WR bit to begin write
	BCF 	EECON1,WREN 	; Disable writes
	BCF 	STATUS, RP0 	; Bank 0
	bcf		STATUS, RP1
	return

#endif


; this code can be removed to save program space if only async writes are performed.
#ifndef EEPROM_NO_SYNC_WRITE

; synchronously writes the eeprom reg table to the eeprom. this routine is slow
; and should be done asynchronously.
eeprom_write_sync
	banksel	EEADR
	clrf	EEADR
eeprom_syncwrite_loop
	; get register from eeprom table
	movf	EEADR,w
	call	eeprom_regtable

	; set FSR to requested register	
	movwf	FSR
	
	; exit if all writes were made
	movf	FSR,w
	btfsc	STATUS,Z
	goto	eeprom_end_syncwrite_loop

	call	eeprom_sync_write_indf		; read byte from eeprom
	goto eeprom_syncwrite_loop	; next itr
eeprom_end_syncwrite_loop
	return

; synchronous eeprom writes. asynchronous should be used so the servo continues
; operation while writing is occurring, there is no reason to use this method 
; anymore.
eeprom_sync_write_indf
	banksel	EEDATA			; bank 2
	MOVF 	INDF,w			; Data Memory Value
	MOVWF 	EEDATA 			; to write
	BSF 	STATUS,RP0		; Bank 3
	BCF 	EECON1,EEPGD 	; Point to DATA memory
	BSF 	EECON1,WREN 	; Enable writes
	BCF 	INTCON,GIE 		; Disable INTs.
	MOVLW 	0x55
	MOVWF 	EECON2 			; Write 55h
	MOVLW 	0xAA
	MOVWF 	EECON2 			; Write AAh
	BSF 	EECON1,WR 		; Set WR bit to begin write
	BSF 	INTCON,GIE 		; Enable INTs.
	BCF 	EECON1,WREN 	; Disable writes
	clrwdt
	BTFSC 	EECON1,WR 		; Wait for write
	GOTO 	$-1 			; to complete
	BCF 	STATUS, RP0 	; Bank 2
	incf	EEADR,f			; increment EE address for next write
	return

#endif
