; DIGITAL SERVO CONTROLLER					Misc Helper Routines & Macros
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
;   This file contains useful helper routines and macros.
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
;    movlf macro l,d	-- move literal to F
;    mov8 macro s,d		-- move register S to D


; move a literal into a file register
movlf macro l,d
	movlw	l
	movwf	d
	endm

; move a register to another register
movff macro s,d
; move a file register contents from x to y
	movf	s,w
	movwf	d
	endm




; If FSR points to the SERVO_STATUS register, clear obsticle flag
CLEAR_STATUS_ON_READ macro
#ifdef OBSTACLE_DETECT
	; if status register was read, clear the obsticle flag
	movf	FSR,w
	sublw	SERVO_STATUS
	btfsc	STATUS,Z
	bcf		SERVO_STATUS, SSF_OBSTACLE
#endif
	endm
