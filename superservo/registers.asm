; DIGITAL SERVO CONTROLLER					Register Locations
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
;	Created Dec 28th, 2005	Colin MacKenzie
;
; KNOWN BUGS
;	None.
;
; WARNINGS
;	None.
;



; the desired (target) servo position
#define SERVO_DP			0x60

; servo's measured position
#define SERVO_CP			0x61

; registers value limits the servo's top speed
#define SERVO_SPEED			0x62

; servo's measured current, related to servo force output
#define SERVO_CURRENT		0x63

; servo status register
#define SERVO_STATUS		0x64		; servo status register
#define		SSF_PIDRUN					0	; set if PID loop is computing
#define		SSF_FREADY					1	; function complete, result ready
#define 	SSF_OE						2	; servo output enable
#define		SSF_OFF						3	; turns ADC conversion and output off
#define		SSF_OBSTACLE 				4	; if set, servo has hit an obstacle
#define		SSF_ENEG					5	; sign of the proportional error
#define		SSF_ADC0					6	; ADC least significant bit 0&1
#define		SSF_ADC1					7

; the computed Proportional Error, the sign if this value is in the option
; register
#define SERVO_E				0x65

; boolean run-time options for the servo
#define SERVO_OPTIONS		0x66		; servo options register
#define     SO_AUTO_ADJUST_BANDWIDTH	0	; adjust bandwidth when pid overruns occur

; free-running counter of servo overruns. The number of times the PID loop calculations
; extended beyond sample timer
#define SERVO_OVERRUNS_H	0x67		
#define SERVO_OVERRUNS_L	0x68

; free-running performance counter that is incremented each time the PID loop
; is executed. In theory this should be incremented n times/sec where:
;    n = Tosc / 4 / Tsc
;    Tosc -> typically 20Mhz, 
;    Tsc -> sample clock period determined by SERVO_CLOCK 
#define SERVO_ITER_H		0x69		; ticks for each PID iteration
#define SERVO_ITER_L		0x6a

; The sample clock reload value
;    to convert to Hz use Tosc / 1024 / SERVO_CLOCK
;    where Tosc is typically 20MHz
#define SERVO_CLOCK			0x6b		

; the servos i2c address
#define SERVO_IICADDR		0x6c

; a 16-bit location for storing an ID of the servo. This ID can be used to uniquely
; identify what this servo is for even when the iic address changes.
#define SERVO_IDH			0x6d
#define SERVO_IDL			0x6e


; Accumulators for 16bit math routines
#define ACCa	  			0x78
#define ACCaLO  			0x79
#define ACCb	  			0x7a
#define ACCbLO  			0x7b

; variables used to store state while in isr (DO NOT MOVE)
;   These are placed within special area 0x70-0x7f which is aliased accross all
; memory banks (i.e. these registers must be accessible no matter the bank switches)
ISR_W					equ	0x70	; must be in range of [0x70 - 0x7f]
ISR_STATUS				equ	0x71	; so they can be accessed through all
ISR_PCLATH				equ	0x72	; memory banks.

; Base memory address of PID variables and history buffer
; memory size is:
;     (8Bits)  10 bytes + PID_HMASK+1
;     (16Bits) 19 bytes + (PID_HMASK+1)*2
#define PID_BASE			0x20			; base of PID data block
											; memory size: (8bit) 8 bytes + bufferSize, (16bit) 16 bytes + bufferSize*2
#define PID_HMASK			0x0f			; determines history size using masking (efficiency)

; IIC Parameters
;   IIC requires 6 bytes plus the IIC_DOP_LENGTH which configures the IIC
;   command buffer size.
#define IIC_BASE			0x40
#define IIC_DOP_LENGTH		0x10
