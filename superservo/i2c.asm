; I2C Implimentation File
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
;	* I may use I2C and IIC interchangably, they mean the same thing!


#ifndef IIC_BASE
#define IIC_BASE			0x6b
#endif

#ifndef IIC_DOP_LENGTH
#define IIC_DOP_LENGTH		0x10
#endif

#define IIC_STATUS			IIC_BASE+0x00	; IIC status register
#define		IICS_SOP_FUNC_VALID 0	; function id in IIC_SOP_FUNCTION is valid
#define		IICS_DOP_FUNC_VALID 1	; function id in IIC_DOP_FUNCTION is valid
#define		IICS_DOP_WRITE		2	; DOP is performing a second write message
#define		IICS_SOP_WRITE		3	; SOP is performing a write message

#define	IIC_INDEX			IIC_BASE+0x01	; current offset into IIC_DOP_BASE
#define IIC_SOP_FUNCTION	IIC_BASE+0x02	; register or region the sop function is affecting
#define IIC_SOP_REGISTER	IIC_BASE+0x03	; register or region the sop function is affecting
#define IIC_TEMP			IIC_BASE+0x04	; temp variable
#define IIC_DOP_BASE		IIC_BASE+0x05	; base of dual operation register range



; This macro loads the correct value
; into the FSR given an initial data 
; memory address and offset value.
LFSR	macro	Address,Offset	
	movlw	Address		
	movwf	FSR
	movf	Offset,W
	addwf	FSR,F
	endm


; This macro loads a range of data memory
; with a specified value.  The starting
; address and number of bytes are also 
; specified.
memset	macro	Buf_addr,Value,Length
	local __setnext
	movlw	Length
	movwf	IIC_TEMP
	movlw	Buf_addr
	movwf	FSR
__setnext	movlw	Value
	movwf	INDF
	incf	FSR,F
	decfsz	IIC_TEMP,F
	goto	__setnext
	endm



onIICWrite
; called when IIC write operation occured, write data is
;   in the SSPBUF register.
	; are we in a SOP write message?
	banksel	IIC_STATUS
	btfsc	IIC_STATUS, IICS_SOP_WRITE
	goto	__call_sop_write_function	; we are in a second write message

	; are we in a DOP second write message?
	btfsc	IIC_STATUS, IICS_DOP_WRITE
	goto	__call_dop_write_function	; we are in a second write message

	; is IIC_INDEX==0?
	movf	IIC_INDEX,w
	btfss	STATUS, Z
	goto	__write_register_set	; not zero, proceed to write register index

	; IIC_INDEX is 0, this is the command byte, if bit 5 of SSPBUF 
	; is set we have a SOP command message
	banksel	SSPBUF
	btfsc	SSPBUF, 4
	goto	__setup_sop_function

	; IIC_INDEX is 0, if SSPBUF==0 we have a null command
	; and this write message belongs to the previous command message
	movf	SSPBUF,w
	btfss	STATUS, Z
	goto	__write_register_set	; new command

	; this is a dop write message
	banksel	IIC_STATUS
	bsf		IIC_STATUS, IICS_DOP_WRITE
	return

__setup_sop_function
	banksel	IIC_STATUS
	bsf		IIC_STATUS, IICS_SOP_WRITE

	; store sop function
	banksel SSPBUF
	movf	SSPBUF,w
	banksel	IIC_SOP_FUNCTION
	movwf	IIC_SOP_FUNCTION
	bcf		IIC_SOP_FUNCTION, 4	; remove the sop status bit

	; proceed into call_sop_write_function...
	
__call_sop_write_function
	call	SOPWriteFunctionVector	; we are in a sop write message

	; ensure SSPBUF is clear
	banksel	SSPBUF
	movf	SSPBUF,w

	; inc the index
	banksel IIC_INDEX
	incf	IIC_INDEX,f
	return

__call_dop_write_function
	call	DOPWriteFunctionVector	; we are in a second write message

	; inc the index
	banksel IIC_INDEX
	incf	IIC_INDEX,f
	return

__write_register_set
	; write data to DOP register sequence
	banksel	IIC_DOP_BASE
	LFSR	IIC_DOP_BASE, IIC_INDEX
	banksel SSPBUF
	movf	SSPBUF,w		; read ssp buffer (also clears BF flag)
	movwf	INDF			; write data to register

	; inc the index
	banksel IIC_INDEX
	incf	IIC_INDEX,f

	; check for out of range
	movf	IIC_INDEX,w		; Get the current buffer index.
	sublw	IIC_DOP_LENGTH		; Subtract the buffer length.
	btfsc	STATUS,Z			; Has the index exceeded the buffer length?
	clrf	IIC_INDEX		; Yes, clear the buffer index.

	return

onIICRead
; called when IIC read operation is requested, data should be
;   returned in the W register.

	; DOP Function should return byte in W
	call	DOPReadFunctionVector

	; inc the index
	banksel IIC_INDEX
	incf	IIC_INDEX,f

	return




iic_configure
; called to configure SSP peripheral for IIC mode communication
	; set SCL and SDA pins as inputs
	banksel TRISC
	bsf	TRISC, 4
	bsf	TRISC, 3

	; set device address
	call	iic_getAddress		; device address should be returned in W reg
	banksel SSPADD
	movwf	SSPADD

	; setup IIC mode
	banksel SSPCON
	bsf	SSPCON, CKP		; clock polarity high
	bcf	SSPCON, SSPM3		; set to IIC slave mode, 7bit addressing
	bsf	SSPCON, SSPM2
	bsf	SSPCON, SSPM1
	bcf	SSPCON, SSPM0

	; enable SSP
	bsf SSPCON, SSPEN

	; enable SSP interrupt
	banksel PIE1
	bsf	PIE1, SSPIE		; enable timer interrupt

	; clear variables
	banksel IIC_DOP_BASE
	clrf 	IIC_DOP_BASE
	clrf 	IIC_INDEX
	clrf 	IIC_STATUS
	memset	IIC_DOP_BASE, 0, IIC_DOP_LENGTH

	return

iic_isr
; called by the ISR when an IIC interrupt occurs
	; test for overflow
	banksel	SSPSTAT
	btfsc	SSPSTAT, SSPOV
	goto	__iic_overflow
	
	; was last byte received or transmitted addr or data
	btfss	SSPSTAT, D_A		; test I2C Data/Address bit
	goto	iic_address		; we just received the address byte

	; data byte, determine if read or write operation
	btfss	SSPSTAT, R_W		; test R/W bit
	goto	iic_write
	goto	iic_read

__iic_overflow
	; clear OV (overflow)
	banksel SSPSTAT
	bcf		SSPSTAT, SSPOV

	; ensure BF is clear
	banksel SSPBUF
	movf	SSPBUF,w
	goto	exit_isr

iic_address
; we just received the device address
	; reset the register set index and clear memory
	banksel	IIC_INDEX
	clrf	IIC_INDEX
	clrf 	IIC_STATUS

	; if we are reading, place next byte in xfer register
	banksel SSPSTAT
	btfsc	SSPSTAT, R_W	; test R/W bit
	goto	iic_read

	; write operation, clear SSPBUF
	banksel	SSPBUF
	movf	SSPBUF,w

	; clear SSP interrupt flag and return
	banksel	PIR1
	bcf		PIR1, SSPIF
	goto	exit_isr
			; else proceed to iicRead

iic_read
	; get register to read
	call	onIICRead

	; write byte in W to SSPBUF register, verifying
	; successful write.
	banksel	SSPCON
iic_write_sspbuf
	bcf		SSPCON,WCOL	; Clear the WCOL flag.
	movwf	SSPBUF		; Write the byte in WREG
	btfsc	SSPCON,WCOL	; Was there a write collision?
	goto	iic_write_sspbuf

	; clear SSP interrupt flag
	banksel	PIR1
	bcf		PIR1, SSPIF

	; enable CKP (clock polarity) on SSPCON (release the clock)
	banksel	SSPCON
	bsf		SSPCON, CKP

	goto 	exit_isr


iic_write
	; write byte to memory
	call	onIICWrite

	; clear SSP interrupt flag
	banksel	PIR1
	bcf		PIR1, SSPIF

	goto	exit_isr


