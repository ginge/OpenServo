; DIGITAL SERVO CONTROLLER					8bit Math Helper Routines & Macros
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

; MACROS:
;    sub8w macro x,y		-- subtract x from y into W
;    sub8f macro x,y,r		-- subtract x from y into R
;    add8w macro x,y		-- subtract x from y into W
;    add8f macro x,y,r		-- subtract x from y into R
;    com8fw macro x			-- 2's compliment x into W

sub8w macro x,y
; sub x from y, place in r
	movf	x,w
	subwf	y,w
	endm

sub8f macro x,y,r
	sub8w	x,y
	movwf	r
	endm

add8w macro x,y
; sub x from y, place in r
	movf	y,w
	addwf	x,w
	endm

add8f macro x,y,r
	add8w	x,y
	movwf	r
	endm

com8fw macro x
	comf	x,f
	incf	x,f
	endm

maxf macro reg, value_reg
	local	maxf_ok
	movf	value_reg, w
	subwf	reg, w
	btfss	STATUS,C	; did overflow occur?
	goto	maxf_ok		; reg < value_reg
	movf	value_reg,w
	movwf	reg
maxf_ok
	endm

maxl macro reg, literal
	local	maxf_ok
	movlw	literal
	subwf	reg, w
	btfss	STATUS,C	; did overflow occur?
	goto	maxf_ok		; reg < value_reg
	movlw	literal
	movwf	reg
maxf_ok
	endm
