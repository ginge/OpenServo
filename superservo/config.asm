; DIGITAL SERVO CONTROLLER					Servo Config Parameters
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
;   This file contains all Build parameters/switches. Also, see registers.h for memory
;   layout configuration.
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

; intended for a PIC16F73
    title "Digital Servo Controller"
    list    p=16f873a, f=inhx32
    #include <p16f873a.inc>


; configuration registers to be embedded in the hex file
	__CONFIG   _CP_OFF & _WDT_ON & _BODEN_OFF & _PWRTE_ON & _HS_OSC & _LVP_OFF & _DEBUG_OFF & _WRT_OFF


; suppress bank selection warning messages
	ERRORLEVEL      -302

; the current version of the source. this value can be retrieved by the
; host using the iic command SERVO_VERSION.
#define SERVO_VERSION_MAJOR		0x03
#define SERVO_VERSION_MINOR		0x00

; define TESTBENCH for simulator testing of PID loop (see testbench.asm)
;#define TESTBENCH

; the default IIC address
#define IIC_ADDR_DEFAULT		0x10



				; *** COMPILE OPTIONS *** ;
; The remaining defines configure build options to include/disclude sections of
; code from being included in the binary. Each option when defined is defined as
; an int that becomes the bit set in the BUILD_OPIONS command (do not change this bit).

; define ADC_10BIT to use the ful 10bits of the ADC. causes little effect
; on cycle time, but should improve PID accuracy by 4. (Will require PID
; retuning!)
#define ADC_10BIT												0x01

; define PID_16BIT to use 16bit PID routines (slower)
;#define PID_16BIT												0x02

; if enabled, the servo output is always disabled on startup. The host controller
; can then have a chance to change the DP before enabling the servo output.
;#define DISABLE_OUTPUT_ON_STARTUP								0x04

; if enabled and eeprom contains valid servo settings, they will be loaded on reset.
;#define EEPROM_LOAD_ON_STARTUP

; if enabled, saving to EEPROM will be performed asynchronous so the servo will
; continue operating while the EEPROM is being written to. Saving to EEPROM can take
; a few seconds. Reading from eeprom is still synchronous since it doesnt take very
; long to complete.
#define EEPROM_ASYNC_SAVE

; define this to remove sync write routines to save program space. The servo doesnt
; require sync writes if async is used, so we can safely remove it.
#ifdef EEPROM_ASYNC_SAVE
#define EEPROM_NO_SYNC_WRITE
#endif

; if defined, the PID algorithm will not scale the final PID output value. This last 
; division is somewhat redundant considering you can calculate Kp,Ki,Kd coefficients 
; that would make Kpid=1. Removing this extra division allows you to increase the
; controller bandwidth.
;#define PID_NO_OUTPUT_COEFFICIENT								0x08


; these are the PID coefficient default values, Kp,Ki,Kd coefficients include the Kpid
; division when Kpid is disabled.
#ifdef PID_NO_OUTPUT_COEFFICIENT
#define PID_DEFAULT_KP		0x04
#define PID_DEFAULT_KI		0x20
#define PID_DEFAULT_KD		0x20
#define PID_DEFAULT_KPID	0x01
#else
#if ADC_10BIT
#define PID_DEFAULT_KP		0x03
#define PID_DEFAULT_KI		0x20
#define PID_DEFAULT_KD		0x20
#define PID_DEFAULT_KPID	0x0a
#else
#define PID_DEFAULT_KP		0x04
#define PID_DEFAULT_KI		0x20
#define PID_DEFAULT_KD		0x01
#define PID_DEFAULT_KPID	0x04
#endif
#endif

; the default sample timer reload value. Determines the frequency/bandwidth of the
; PID loop. Each time the timer wraps, it causes an ADC to occur and the PID loop to
; be executed. This is the default value, the real clock can be stored in EEPROM or
; can also be auto adjusted based on overruns occurring.
;  Note: this is the high-byte portion of the TMR1H:TMR1L, loading TMR1L is too fine
; a granularity to be useful.
#ifdef PID_NO_OUTPUT_COEFFICIENT
#define SAMPLETIMER_RELOAD_DEFAULT	0x01
#else
#define SAMPLETIMER_RELOAD_DEFAULT	0x02
#endif

; enable this to automatically lower the sample clock when servo overruns occur, this
; is a good way to determine max sample clock under in-system conditions. This can
; also be enabled temporarily using a servo option
#define AUTO_ADJUST_BANDWIDTH									0x10

; if enabled, servo code will include a speed limiter. The speed limit of the servo
; will be stored in the SERVO_SPEED register.
;#define SPEED_LIMITER											0x20

; if enabled, the PIC INT pin will act as an input for a bumper switch. The bumper
; switch detects when the servo hits an obsticle and it sets the desired position
; equal to the current position and the SSF_OBSTICLE flag in the status register.
; The SSF_OBSTACLE flag will clear when the status register is read from iic.
;#define OBSTACLE_DETECT											0x40

; A macro to build a literal byte containing the build options so the host can query 
; which build options were set via i2c.
GET_BUILD_OPTIONS macro
	movlw	0x00
#ifdef ADC_10BIT
	iorlw	ADC_10BIT
#endif
#ifdef PID_16BIT
	iorlw	PID_16BIT
#endif
#ifdef DISABLE_OUTPUT_ON_STARTUP
	iorlw	DISABLE_OUTPUT_ON_STARTUP
#endif
#ifdef PID_NO_OUTPUT_COEFFICIENT
	iorlw	PID_NO_OUTPUT_COEFFICIENT
#endif
#ifdef AUTO_ADJUST_BANDWIDTH
	iorlw	AUTO_ADJUST_BANDWIDTH
#endif
#ifdef SPEED_LIMITER
	iorlw	SPEED_LIMITER
#endif
#ifdef OBSTACLE_DETECT
	iorlw	OBSTACLE_DETECT
#endif
	endm
