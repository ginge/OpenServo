; Servo Functions Implimentation File
; ---------------------------------------------------------------------------------------
; http://www.colinmackenzie.net
;
;
; History:
;	Created Feb 25th, 2003	Colin MacKenzie
;	Created as part of the SuperServo servo controller
;
; KNOWN BUGS
;	None.
;
; WARNINGS
;	None.
;
; NOTES
;	* Must ensure jump table vectors do not cross memory page boundaries, (I.e. each
;	  jump vector table must fit within the 256 instruction word page and not cross
;	  on it.) The PCLATH must be properly set for the page the jump vectors are in.


	; place the jump vector tables at the following program space address, the
	; PCLATH should reflect this address for vector table jumps.
	org	0x300


DOPReadFunctionVector
; vector table provides jump vectors to function handlers.
	movlw	0x03				; jumping in 0x6xx page
	movwf	PCLATH

	; validate index bounds
	movlw	0x0f
	banksel	IIC_DOP_BASE
	andwf	IIC_DOP_BASE,w
	
	addwf	PCL,f
	
	; vector table begin
	retlw	0x00					; 0x00
	goto	readRegisters			; 0x01	SERVO_RW_REGISTERS
	goto	readRegion				; 0x02	SERVO_RW_REGION
	retlw	0x00					; 0x03

	retlw	0x00					; 0x04
	retlw	0x00					; 0x05
	retlw	0x00					; 0x06
	retlw	0x00					; 0x07

	retlw	0x00					; 0x08
	retlw	0x00					; 0x09
	goto	readDeviceInfo			; 0x0a	SERVO_DEVICE_INFO
	goto	readBuildOptions		; 0x0b	SERVO_BUILD_OPTIONS

	goto	readFirmwareVersion		; 0x0c  SERVO_VERSION
	goto	echoCommand				; 0x0d	SERVO_ECHO_COMMAND
	retlw	0x00					; 0x0e
	retlw	0x00					; 0x0f


DOPWriteFunctionVector
; vector table provides jump vectors to function handlers.
	movlw	0x03				; jumping in 0x6xx page
	movwf	PCLATH

	; validate index bounds
	movlw	0x0f
	banksel	IIC_DOP_BASE
	andwf	IIC_DOP_BASE,w

	addwf	PCL,f
	
	; vector table begin
	retlw	0x00					; 0x00
	goto	writeRegisters			; 0x01	SERVO_RW_REGISTERS
	goto	writeRegion				; 0x02	SERVO_RW_REGION
	retlw	0x00					; 0x03

	retlw	0x00					; 0x04
	retlw	0x00					; 0x05
	retlw	0x00					; 0x06
	retlw	0x00					; 0x07

	retlw	0x00					; 0x08
	retlw	0x00					; 0x09
	retlw	0x00					; 0x0a
	retlw	0x00					; 0x0b

	retlw	0x00					; 0x0c
	retlw	0x00					; 0x0d
	retlw	0x00					; 0x0e
	retlw	0x00					; 0x0f


SOPWriteFunctionVector
; vector table provides jump vectors to function handlers.
	movlw	0x03				; jumping in 0x6xx page
	movwf	PCLATH

	; validate index bounds
	movlw	0x0f
	banksel	IIC_SOP_FUNCTION
	andwf	IIC_SOP_FUNCTION,w

	addwf	PCL,f
	
	; vector table begin
	retlw	0x00					; 0x00
	goto	writeRegisterPair		; 0x01	SERVO_WRITE_REGISTERS
	goto	writeRegionB			; 0x02	SERVO_WRITE_REGION
	retlw	0x00					; 0x03

	goto	servoPosition			; 0x04	SERVO_POSITION
	goto	servoSpeed				; 0x05	SERVO_SPEED
	goto	servoOn					; 0x06	SERVO_ENABLE
	goto 	servoOff				; 0x07	SERVO_DISABLE

	goto	restoreSettings			; 0x08	SERVO_RESTORE_SETTINGS
	goto	eeprom_read_sync		; 0x09	SERVO_LOAD_SETTINGS
	goto	eeprom_write_async		; 0x0a	SERVO_SAVE_SETTINGS
	retlw	0x00					; 0x0b

	retlw	0x00					; 0x0c
	retlw	0x00					; 0x0d
	goto	restart					; 0x0e	SERVO_RESET
	goto	enterProgrammingMode	; 0x0f	SERVO_PROGRAMMING

;#ifdef EECON1		; some devices do not support EEPROM
; #ifdef EEPROM_ASYNC_SAVE


;############ SERVO SYSTEM COMMANDS ###############

servoSpeed
	; if index is greater than 1 we should
	; set the servo speed
	banksel IIC_INDEX
	movf	IIC_INDEX,f
	banksel STATUS
	btfsc	STATUS, Z
	retlw	0

	; load servo speed from SSPBUF
	banksel SSPBUF
	movf	SSPBUF,w
	banksel SERVO_SPEED
	movwf	SERVO_SPEED

	retlw	0


servoPosition
	; if index is greater than 1 we should
	; set the servo position
	banksel	IIC_INDEX
	movf	IIC_INDEX,f
	banksel STATUS
	btfsc	STATUS, Z
	retlw	0

	; load servo position from SSPBUF
	banksel SSPBUF
	movf	SSPBUF,w
	banksel	SERVO_DP
	movwf	SERVO_DP

	retlw	0


servoOn
	banksel	SERVO_STATUS
	bsf		SERVO_STATUS, SSF_OE
	retlw	0


servoOff
	banksel	SERVO_STATUS
	bcf		SERVO_STATUS, SSF_OE
		
	; no PWM output
	banksel CCPR1L
	clrf	CCPR1L
	clrf	CCPR1H
	clrf	CCPR2L
	clrf	CCPR2H

	retlw	0


readRegisters
; reads registers specified in the register set
	; fetch the address to our register pointer
	banksel	IIC_DOP_BASE
	movlw	IIC_DOP_BASE+1
	addwf	IIC_INDEX,w

	; dereference W to get the register host is requesting
	movwf	FSR
	bcf		STATUS, IRP
	movf	INDF,w

	; dereference W again, to get the data from requested register
	movwf	FSR
	movf	INDF,w

	; clear any status bits that should clear on a status read
	CLEAR_STATUS_ON_READ

	return


writeRegisters
; writes registers specified in the register set
	; fetch the address to our register pointer
	banksel	IIC_DOP_BASE
	movlw	IIC_DOP_BASE+1
	addwf	IIC_INDEX,w

	; dereference W register
	movwf	FSR
	bcf		STATUS, IRP
	movf	INDF,w

	; dereference W register again
	movwf	FSR

	; load SSPBUF into the register
	movf	SSPBUF,w
	movwf	INDF

	retlw	0x00


readRegion
; reads a region of memory 
	; fetch region and index from register set buffer
	movf	IIC_DOP_BASE+0x01,w	; region is at base+1
	movwf	FSR

	; offset FSR by DOP index
	movf	IIC_INDEX,w
	addwf	FSR,f

	; fetch byte at that region and return
	bcf		STATUS, IRP
	movf	INDF,w

	; clear any status bits that should clear on a status read
	CLEAR_STATUS_ON_READ

	return


writeRegion
; writes a region of memory 
	; fetch region and index from register set buffer
	movf	IIC_DOP_BASE+0x01,w	; region is at base+1
	movwf	FSR

	; offset FSR by DOP index
	movf	IIC_INDEX,w
	addwf	FSR,f

	; write byte at that region and return
	movf	SSPBUF,w
	bcf		STATUS, IRP
	movwf	INDF

	retlw	0x00


writeRegisterPair
	; if index is zero we return
	banksel	IIC_INDEX
	movf	IIC_INDEX,f
	btfsc	STATUS, Z
	retlw	0

	banksel IIC_INDEX
	btfss	IIC_INDEX, 0
	goto	__wrpwritepair

	; index bit 0 set; this is a register pointer, save it
	banksel SSPBUF
	movf	SSPBUF,w
	banksel IIC_SOP_REGISTER
	movwf	IIC_SOP_REGISTER
	retlw	0

	; index bit 0 clear; this is the register value
__wrpwritepair
	banksel	IIC_SOP_REGISTER
	movf	IIC_SOP_REGISTER,w
	movwf	FSR
	banksel	SSPBUF
	movf	SSPBUF,w
	bcf		STATUS, IRP
	movwf	INDF
	retlw	0



writeRegionB
; writes a region of memory using mode B
	; fetch region and index from register set buffer

	; if index is zero we return
	banksel	IIC_INDEX
	movf	IIC_INDEX,f
	btfsc	STATUS, Z
	retlw	0

	; if index is one, this is a region pointer, otherwise
	; we write byte to [region+offset].
	banksel IIC_INDEX
	decf	IIC_INDEX,w
	btfsc	STATUS, Z
	goto	__wrbsaveregion

	; move offset to FSR
	movwf	FSR

	; add region-1 to FSR, this achieves an register that is
	; region+index-2 (since index==2 is the first data byte).
	decf	IIC_SOP_REGISTER,w
	addwf	FSR,f

	; write byte to region
	banksel SSPBUF
	movf	SSPBUF,w
	bcf		STATUS, IRP
	movwf	INDF

	retlw	0x00

__wrbsaveregion
	; save SSPBUF into SOP register
	banksel SSPBUF
	movf	SSPBUF,w
	banksel IIC_SOP_REGISTER
	movwf	IIC_SOP_REGISTER
	retlw	0x00


echoCommand
; reads a region of memory 
	; fetch region and index from register set buffer
	banksel	IIC_DOP_BASE
	movlw	IIC_DOP_BASE

	; offset FSR by DOP index
	addwf	IIC_INDEX,w
	movwf	FSR

	; fetch byte at that region and return
	bcf		STATUS, IRP
	movf	INDF,w
	return


readBuildOptions
	GET_BUILD_OPTIONS
	return

readFirmwareVersion
	movf	IIC_INDEX,w
	btfss	STATUS,Z				; index == 0 or 1?
	retlw	SERVO_VERSION_MAJOR
	retlw	SERVO_VERSION_MINOR


; device info will be placed at 0x400
	org	0x600

readDeviceInfo
; reads the servo device information
	; validate index bounds
	; setup PCLATH bits
	movlw	0x06				; jumping in 0x6xx page
	movwf	PCLATH

	movlw	0x3f
	banksel	IIC_INDEX
	andwf	IIC_INDEX,w

	addwf	PCL,f

	retlw	0x38
	DT		"SuperServo\n(c)2003 colinmackenzie.net\n3.0\nA\nPIC16f73\nPID\000     "

	return



readIO
	retlw	0

writeIO
	retlw	0

configureIO
	retlw	0



enterProgrammingMode
	goto	0x0001
