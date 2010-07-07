/*
    Copyright (c) 2006 Michael P. Thompson <mpthompson@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

    $Id$
*/ 
//#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <inttypes.h>
#include <avr/io.h>
#include <math.h>
#include "openservo.h"
#include "config.h"
#include "pid.h"
#include "registers.h"
#include "banks.h"
#include "filter.h"
#include "timer.h"
#include "adc.h"

#ifdef REG_REVERSE_SEEK
#error "This version of pid.c no longer supports "reverse seek" (REG_REVERSE_SEEK), you'll need to put the code back in (assuming there is sufficient flash memory)"
#endif

// The minimum and maximum servo position as defined by 10-bit ADC values.
#define MIN_POSITION            (0)
#define MAX_POSITION            (1023)

// The minimum and maximum output.
#define MAX_OUTPUT              (255)
#define MIN_OUTPUT              (-MAX_OUTPUT)

// The minimum and maximum values for conditioning the integral term.
#define MAX_POSITION_INTEGRAL   (16<<(SEEKVRES))
#define MIN_POSITION_INTEGRAL   (-MAX_POSITION_INTEGRAL)

//
// NOTE: This version of pid.c uses a mixture of "fixed-point" (integers) and floats
//       to perform floating point maths.
//
// Advantages (floats over fixed-point in ints math)
//    1 Greater accuracy and range (for example: it is easier to use larger gain values
//      without causing overflows or clipping).
//    2 Already used (in motion.c and curve.c), so it does not introduce an additional
//      memory overhead for run-times (unlike the use of 64-bit integers for the fixed-
//      point float methods).
//    3 Simpler to work with (no constant need to deal with, or watch out for, problems
//      with variable sizes or granularity, overflows and so forth).
//
// Disadvantages (floats versus fixed-point in ints math)
//    1 Some operations (for example: multiple, and divide especially) are probably slower
//      than the 64-bit/32-bit integer math.
//
//       TODO: Rather than relying on vague internet info, find out how much
//             slower float use is compared to 64-bit and 32-bit integer use.
//

//
// SEEKVRES
//
// This parameter defines the resolution of the seek velocity (set in the seek
// velocity register) as a shift in terms of parts of a "ADC potentiometer (pot)
// unit" per call to the pid_position_to_pwm function (at time of writing, once
// every 9.987ms). It is defined as a shift-multiplier. For example a value of
// 4 means that the resolution of the seek velocity parameter (as set by the
// host) is 1<<4 (or 1/16th of a pot increment).
//
// NOTE: When using integers as fixed point floats, increasing this value can lead
//       to problems: usually caused by overflows in fixed point operations involving
//       integers.
//
//       As we're currently supporting the use of fixed point floats, things must be
//       handled correctly to match the super sampling (pseudo-oversampling) of the
//       position in adc.c, which in turn is limited by the sampling bandwidth that
//       can be supported, see adc.c/adc.h.
//
//       Increasing the values of SEEKVRES beyond 4 would require some variables to
//       be redeclared as (u)int32_t instead of (u)int16_t.
//
#define SEEKVRES 4

//
// NOTE: When declaring static variables, the default initialisation value at
//       application start-up is zero, and it does no harm (does not increase
//       flash usage, and also indicates intent) assigning zero. Nor is flash
//       memory usage increased by initialising static variables, at declaration,
//       with a value other than zero. And neither is flash memory saved if a
//       static variable is not assigned a value when it is declared. I.e. the
//       flash memory used by a program does not change if any of the following
//       types of static declaration is used (note however, if the variable is
//       never used in the code, then it may be optimised out of a build altogether):
//
//          static int16_t some_value;
//          static int16_t some_value=0;
//          static int16_t some_value=6;
//
//       If either of the first two declaration styles is used, the value of
//       some_value will be 0 when the program starts, if the third declaration
//       style is used, it will be 6 when the program starts. None of these
//       declaration methods affects the total ammount of flash memory used.
//
//       If a one-shot "init" function is used and called (for example: from main),
//       then it actually uses (of course) extra flash memory to initialise the
//       variables to values that can in fact be set at declaration time.
//
// Values preserved across multiple PID iterations.
static int16_t previous_seek=-1; // NOTE: 0 is a valid position on some of my servos (not a good thing), without
                                 //       interference with the physical mounting of the pot.
static int16_t previous_seek_velocity=-1;
static int16_t previous_filtered_position=0;
static float fderivative=0.0;
static float fposition_integral=0.0;
static float fprevious_position_error=0.0;
static int32_t previous_position_error=0;
static float fpwm_feedback=0.0;
static int16_t previous_position=0;
static int32_t previous_positionex=0;

static int32_t seek_delta=0;
static int32_t filter_reg_pos = 0;
#if BACKEMF_ENABLED
static int32_t filter_reg_emf = 0;
#endif
static int32_t filter_reg_velocity = 0;
static int32_t previous_pwm_output=0;
static uint8_t previous_pwm_is_enabled=0;

//
// static uint8_t reset_pid_algorithms
//         o Set non-zero if the "PID algorithms" needs to be reset, for example:
//
//              The first time the PID to PWM function is called (0xFF for initialisation)
//              When the seek position changes
//              When PWM is enabled/disabled
//              Velocity control is turned on/off
//
static uint8_t reset_pid_algorithms=0xFF;

//
// static uint8_t velocity_control_on
//         o Controls, internally, whether or not velocity is being used to control the
//           positional PID algorithm which drives the servo. When this variable is 0,
//           velocity control is off (the servo will move as fast as it is able if it
//           is not at the set seek position and PWM is turned on). When it is non-zero
//           velocity control is enabled and will be used to move the servo towards the
//           set seek position. This makes it easier to turn velocity control off once
//           the set seek position has been reached (or passed).
//
//           NOTE: Velocity control is disabled if seek_velocity is 0x8000: the servo will
//                 be driven at whatever speed it can attain under the prevailing load.
//
static uint8_t velocity_control_on=0;
static int16_t working_seek_velocity=1; // Used to implement deacceleration when approaching the seek position
static int16_t working_seek_velocity_lag=1;

//
// The variable bAtOrPassed_last_set_seek_point is set non-zero if the servo is "at", or has
// gone past (overshot) the set seek position.
//
static uint8_t bAtOrPassed_last_set_seek_point=0;

//
// The following variables are used for velocity and position estimation
//
static int32_t volest_previous_positionex=0;
static int16_t volest_dt=0;
static int8_t volest_history=0;
//static int32_t volest_backemf_scaling_factor=0;
//static int32_t volest_backemf_sum=0;
static int32_t estimated_velocity=0;
static int32_t previous_estimated_velocity=0;

//
// Estimated current position produced from known pot positions and estimated
// velocities
//
static int32_t current_position_estimated=-1;
static int32_t previous_position_estimated=0;
static int16_t position_at_start_of_new_seek=-1;

//
// Test and debug aids
//
static int16_t max_position=-1;
static int16_t min_position=0;

////////////////////////////////////////////////////////////////////////////////
//
// Initialize the PID algorithm module.
//
// NOTE: Using auto-initialisation (assigning initial values where the variable
//       is declared) rather than initialising in a function, saves flash space.
//       pid_init is a one-shot call from main.c at start-up.
//       
void pid_init(void)
{
   reset_pid_algorithms=0xFF;
}

void pid_registers_defaults(void)
// Initialize the PID algorithm related register values.  This is done
// here to keep the PID related code in a single file.
{
    // Default deadband.
    banks_write_byte(POS_PID_BANK, REG_PID_DEADBAND, DEFAULT_PID_DEADBAND);

    // Default gain values.
    banks_write_word(POS_PID_BANK, REG_PID_PGAIN_HI, DEFAULT_PID_PGAIN);
    banks_write_word(POS_PID_BANK, REG_PID_DGAIN_HI, DEFAULT_PID_DGAIN);
    banks_write_word(POS_PID_BANK, REG_PID_IGAIN_HI, DEFAULT_PID_IGAIN);

    // Default position limits.
    banks_write_word(POS_PID_BANK, REG_MIN_SEEK_HI, DEFAULT_MIN_SEEK);
    banks_write_word(POS_PID_BANK, REG_MAX_SEEK_HI, DEFAULT_MAX_SEEK);
}

////////////////////////////////////////////////////////////////////////////////
//
// int16_t pid_position_to_pwm(int16_t current_position, uint8_t *pbraking)
//
// This function computes a PWM value to reach a specified position (the set
// seek position) at a given velocity (the set seek velocity). At its core is
// a relatively straightforward implementation of the standard "PID controller
// algorithm".
//
// NOTE: The output value is conditioned to lie in the range [-MAX_OUTPUT, MAX_OUTPUT],
//       it is not scaled.
//
// Large integral gains (Ki) will probably lead to instability, small gains might
// help prevent oscillation when the set seek position has been reached. A 0
// integral gain will switch that part of the PID code off (creating a PD
// controller).
//
// So, what do we mean by velocity (speed) control? OpenServo is typically going
// to be used in robotics, animatronics and so forth, where it is desirable to
// be able to move the servo from its current position to a new position with a
// particular velocity (speed) - with as much accuracy as possible for both speed
// and velocity. For example, to move from position 100 to 600 in 10 seconds means
// move with a velocity of 50 "increments" per second. The units of velocity are
// abstract, (a scaling factor to convert to/from SI units can be determined
// later- it will depend on the rate at which the PID to PWM function is called
// in the final implementation). The latter depends on the physical setup of the
// servo, and will even differ for individual donor servos from the same make
// and model - this is no different from having to calibrate your unmodified
// R/C servos.
//
// NOTE: See "How to relate the set seek velocity to the physical world" below.
//
// A servo without velocity control will run at some arbitrary speed, often dependant
// on the battery type, its voltage and its charge level, and the servo in question:
// using over-spec batteries (such as fully charged "7.2V racepak" with a 6V servo)
// can lead to some very powerful and speedy moves. Whilst power is useful for
// robotics, lack of control in the servo can lead to damage to the robot and/or
// the servo, and so forth.
//
// NOTE: The 5v regulators used in the OpenServo typically means you'll need to
//       use a battery with a nominal voltage greater than 6V.
//
// With a standard PWM signal controlled RC servo, velocity control can be implemented
// by continually changing the seek position defined by the PWM signal, at an appropriate
// rate, until the desired position has been reached. However, there is normally no
// feedback available to the "controller", which is another reason why OpenServo might
// be used. This method of velocity control appears to work extremely well with RC servos,
// and therefore a similar approach is taken here for OpenServo in the code presented here:
//
//    The pid_position_to_pwm function presented uses a fully implemented PID controller
//    for position, but varies the seek target internally to control the velocity.
//
// A more complex implementation might be to cascade two or more PID controllers (for
// example: one for velocity and one for position), as described here:
//
//    http://www.openservo.com/Resources?action=AttachFile&do=get&target=automatic_control_theory_for_openservo_v0.3.pdf
//
// However:
//
//    Adding a second PID controller obviously increases the complexity of interactions
//    between velocity and position, as well as making the determination (tuning) of good
//    PID parameters that much more difficult. And one would need to determine what
//    advantages or disadvantages it may have, these are not explored here. It is
//    possible that anything that it can be provide can be added to what is provided
//    here (acceleration/deacceleration, speed ramping and so forth).
//
// NOTE: A velocity only PID controller and a combined controller was experimented with,
//       but whilst it worked well, it provided no useful torque for robotics applications,
//       even when the servo was only lightly loaded, and it was not immediately obvious
//       how to get the torque up to a useable level for robotics without making it
//       jittery/inaccurate whilst retaining adequate velocity control. After testing
//       it performed much worse than the experimental version of what is implemented
//       here, and this method still has desirable levels of torque when under velocity
//       control.
//
// Other options include "fuzzy PID" algorithms, or even something completely different.
//
// ****
// BLAH BLAH stuff to be edited/removed:
//
// One issue that needs to be thought about is what happens if the desired velocity
// is not being maintained. For example the servo is incapable of moving at the set
// speed, even unloaded, or the load is/has become so large that it cannot move it
// at the desired speed. In the "classic" RC speed control method described above,
// there is no feedback to the controlling entity, so the set seek position will
// continue to be a moving target and the servo will try harder and harder, possibly
// succeeding if the load is within the servo's capabilities or until something
// breaks.
//
// OpenServo could respond in the same way (and the initial implementation of this
// version of the PID function will), but as it has feedback and more control, power
// output could be limited/cut off to prevent servo damage and so forth.
//
// END OF BLAH BLAH stuff
// ***
//
// How to relate the set seek velocity to the physical world
//
// If the function is called once every 10ms, seek velocity is set to 96, the servo
// is at position 100 and is commanded to move to position 900, and SEEKVRES is 4,
// then it will take (with the code as it stands at the time of writing)
//
//    (900-100)*4/(96/(1<<SEEKVRES))*10/1000
//
// or ~5.3 seconds to do it (assuming it is capable of shifting any load attached
// to the output shaft at the set speed). If you grab and hold on to the servo horn
// it will take longer, but it will try harder and harder to move until either you
// cannot hold on any more or the servo reaches the limits of its power (or breaks)-
// which ever comes first!
//
// NOTE: Currently the OpenServo firmware is configured such that the function is
//       called (on average - its called from an "event loop") once every ~9.987ms.
//
//       Loading of the servo can obviously affect the time of travel and or the
//       ability of the combination of this code and the servo to maintain a set
//       velocity.
//
// [ ] TODO: The handling of the braking flags needs some tidying up.
//
int16_t pid_position_to_pwm(int16_t current_position, uint8_t *pbraking)
{
    // We declare these static to keep them off the stack.
    static int32_t current_positionex;
    static int16_t deadband;
    static int16_t seek_position;
    static int32_t seek_positionex;
    static int16_t seek_velocity;
    static int16_t tmpi16;
    static int16_t filtered_position;
    static int16_t current_velocity;
    static uint8_t pwm_is_enabled;
    static int32_t pwm_output;

//
// perror is the error in the current position, note that initially this is set as the difference
// between the set seek position and the "current position" as a 32:SEEKVRES fixed-point float and
// is later converted to a :8 fixed point float. perror represents our error function.
//
// Indeed, note that most position and velocity information is being handled as :SEEKVRES
// fixed-point floats and not in "pot" units as measured by the ADC.
//
    static int32_t perror; // TODO: Should be able to get away with a int16_t?
    static float fperror;

//
// Get the super-sampled position, it is ready as well
//
// NOTE: When handling an ADC measured position on the potentiometer it should be remembered
//       that it is an integer, and thus a position n _can_ be seen to representing a position,
//       in the physical world, in the range [n, n+1.0). As long as one keeps the same
//       perspective, 0.5 should/can be added to give the centre position when handling
//       the positions with greater accuracy than the measured integer position. This more
//       readily aligned with current or future handling of positions/velocities at higher
//       resolutions.
//
//       A simple average is used to help to filter out the lsbit error noise.
//
// TODO: Move this retrieval to adc.h/c. Eventually combine the variables (e.g. current_position
//       and current_positionex) to save space, remove duplication and so on - for current
//       development it is nice to keep them separate.
///
#if ADC_POSITION_SUPERSAMPLINGBITS>SEEKVRES
#error ADC_POSITION_SUPERSAMPLINGBITS must be <= SEEKVRES
#endif
#if 1
    current_positionex=((int32_t)adc_position_value_supersample<<(SEEKVRES-ADC_POSITION_SUPERSAMPLINGBITS))
       +((1<<(SEEKVRES-ADC_POSITION_SUPERSAMPLINGBITS))>>1);
    if(reset_pid_algorithms==0xFF)
    {
       previous_positionex=current_positionex;
    }
    current_positionex<<=1;
    current_positionex+=previous_positionex;
    current_positionex/=3;
#else
    current_positionex=current_position<<2;
    if(reset_pid_algorithms)
    {
       previous_position=current_position;
    }
    current_positionex+=previous_position;
    current_positionex<<=SEEKVRES;
    current_positionex/=3;
    current_positionex+=((1<<SEEKVRES)>>1);
#endif

//
// Get flag indicating if PWM is enabled or not. This saves fetching it each time
// we want to know.
//
    pwm_is_enabled=registers_read_byte(REG_FLAGS_LO)&(1<<FLAGS_LO_PWM_ENABLED);

//
// Whilst the servo is moving from one position to the other, we do not want
// hardware braking to be used, default it to off.
//
    *pbraking=0; // TODO: [ ] There may be a bug in handling the state of this if the user keeps turning on/off PWM?

//
// Get the currently estimated position and velocity of the servo
//
// NOTE: This is the old method, left intact with the original variables.
//
// TODO: explore the effects of calls to filter_update for filter_reg_pos combined with
//       filter_reg_emf. Currently there seems to be a lag issue with filter_update.
//
    // Filter the current position thru a digital low-pass filter.
    filtered_position = filter_update(current_position, &filter_reg_pos);


#if BACKEMF_ENABLED
    int16_t sign;
    // Use the filtered position to determine velocity.
    sign = filtered_position - previous_filtered_position;
    if (sign < 0)
        sign = -1;
    else
        sign = 1;

    current_velocity  = sign * filter_update(banks_read_word(INFORMATION_BANK, REG_BACKEMF_HI), &filter_reg_emf);

    previous_filtered_position = filtered_position;

#else
    // Use the filtered position to determine velocity.
    current_velocity = filtered_position - previous_filtered_position;
    previous_filtered_position = filtered_position;
#endif

//
// Get the desired position, velocity and deadband settings
//
    // Get the seek position and velocity.
    seek_position = (int16_t) registers_read_word(REG_SEEK_POSITION_HI);
    seek_velocity = (int16_t) registers_read_word(REG_SEEK_VELOCITY_HI);

    // Get the deadband.
    deadband = (int16_t) banks_read_byte(POS_PID_BANK, REG_PID_DEADBAND);

//
// TODO: review: In the original V3-dev pid.c it was using the "filtered position when the
// seek position is not changing". Why? The filter's "memory" is too long, for example even
// at a moderate constant speed the position returned in the filter lags 30+ pot steps off
// the true physical position! The filter is updated when this function is called, which is
// only once every ~10ms (at the time of writing). Noise and other issues might best be
// handled by super-sampling the ADC pot position - this has been implemented in a revised
// adc.h.
//
    // Use the filtered position when the seek position is not changing.
#if 0
    if (seek_position == previous_seek) current_position = filtered_position;
#endif

    // Keep the seek position bound within the minimum and maximum position.
    tmpi16 = (int16_t) banks_read_word(POS_PID_BANK, REG_MIN_SEEK_HI);
    if (seek_position < tmpi16) seek_position = tmpi16;
    tmpi16 = (int16_t) banks_read_word(POS_PID_BANK, REG_MAX_SEEK_HI);
    if (seek_position > tmpi16) seek_position = tmpi16;

//
// Using seek_positionex saves us having to do the same thing over and over: makes
// the code easier to read and saves flash space.
//
    seek_positionex=((int32_t)seek_position<<SEEKVRES)+((1<<SEEKVRES)>>1);

//
// Initialisation for first call...
//
    if(reset_pid_algorithms==0xFF) // Initialising... move to call to main from here...? (i.e. on retrieval of first position)
    { // Initialisation of velocity and position estimators for first call...
       current_position_estimated=current_positionex;
       previous_position_estimated=current_positionex;
       previous_position=current_position;
       previous_positionex=current_positionex;
    } // NOTE: The value of reset_pid_algorithms must be left as-is

//
// TODO: Rewrite and move these comments to a better place?
//
// We want more accuracy in the error term to accommodate the use of low velocities
// (for example: sub one-pot-step per call, probably affects one or two-pot-step per
// calls too), and to accommodate issues related to the "coarseness" of the pot measurements.
// Therefore the error values and so on are shifted to x:SEEKVRES fixed-point float values.
//
// Most of the maths below is performed using fixed-point float values, to save (flash)
// space the use of different sized variables and granularity is allowed (for example a
// 32-bit integer is only used if it is likely to be necessary to prevent overflow). All
// fixed-point float values are assumed to be x:8 unless otherwise noted.
//
// NOTE: When multiplying two x:8 fixed point float values, the result is an x:16 value.
//
//       Fixed point float example:
//
//
//          1.5625 (e.g. a gain) in "fixed point float <<8" is 400 (1.5625*256 is 400)
//          2.0000 (e.g. an error in position, we need fractional part to control low
//                  velocities) <<8 is 512
//
//          512*400 gives 204800, which is 3.1250<<16 (i.e. to get our "3" for pwm_output
//                  we now need to >>16, not >>8).
//
//       The code also assumes that the processor/compiler implements sign extension for
//       right shifts, for example "-1>>1" is still -1.
//
//       However, note that whilst bit shifting an integer right to divide it by a power of
//       two is faster than an actual division, doing so to a negative values gives an answer
//       that can be one out (1s vs 2s complement). For example:
//
//          -18945>>8= -75, but -18945/256 is -74
//
//       Sometimes this may not be a problem with "fixed-point floats", as some accuracy has
//       been thrown out anyway. But with some calculations it could cause a problem.
//
//       The example just given does not discuss the float to integer "truncation" issue,
//       for example a "fixed/floating point" PWM output of 0.99999 should be 1 on output
//       (not 0), -1.99999 should be -2, and so on.
//

//
// Some notes on velocity estimation
//
// Note that at the time of writing, the position_to_pwm function is called approximately
// 100 times per second and that it is called from an ordinary processing loop where things
// are triggered when conditions are set, so the true interval between calls is only
// approximate.
//
// Note that for most standard donor servo hardware the _ADC measurement_ of the position via
// the pot is (probably) around 0.2 to 0.25 degrees.
//
// Velocity is expressed in "fractions of an ADC pot unit" per call (every ~10ms). The resolution
// of this is determined by the SEEKVRES define. For example: if SEEKVRES is 4, then the difference
// between positions 100 and 101 on the pot is 16, and if the servo moves, or it desired that it
// moves, between the two positions in ~20ms, the velocity is 8.
//
// When certain conditions are met, a direct estimation of the physical velocity can be obtained
// when the ADC measured pot position changes between calls. If PWM (drive to the motors) is off,
// then any calculated velocities are likely to be suspect. Estimated velocities are more
// accurate when the rate of change in position exceeds the call rate. The estimated velocities
// are less accurate when the rate of change in position is close to, the same as or less than
// the call rate. If the velocity estimates themselves are changing between estimated when the
// rate of change is close to, the same as or less than the call rate, then the estimate is even
// less reliable and probably suspect.
//
// TODO: There may need to be a flag to indicate that a velocity cannot currently be estimated
//       and/or it is suspect.
//

//
// Calculate direct instantaneous velocity estimation from changes in position as measured
// on the position potentiometer (pot) by ADC. We use a call counter as our time base (rather
// a real clock).
//
// If PWM is off, the estimated velocity doesn't mean the same thing to the code (the
// servos position is no longer under processor control). However, it could be of use,
// so we still calculate an estimated velocity.
//
// Also calculated is an estimated position based on the "known" position from pot
// position, i.e. the estimated position has a greater "precision" than the current
// position as measured by the pot. Note the word "estimated" and the quotes around
// precision! An example should suffice to show some of the details: The position
// as provided by the pot measurement is an integer (no fractional position), there
// are reasons why we might want to recover a more precise position from available
// data, the use of low velocities is one example, improving the reliability of the
// math another. First, if we ignore the issue of "where" the pot measurement is
// in the range, we can see that there are positions between "100" and "101", such
// as "101.2" and "101.7". Now, think about what happens to the position measured
// by the pot. If, for example, the servo is moving from "100" to "200" and the
// measurement from the pot is "151" and the previous measurement (on the last call)
// was "150" and the servo is moving slowly, then the true position could be
// closer to "150" than "151" there is no reliable way of telling. For example:
// if the velocity appears to be "0.1" steps per call, the true position in the
// example is likely to be close to "150.1" (not "151" as measured by the pot).
// However, if the servo is moving from "200" to "100", measurement from the pot
// is "150" and the previous measurement (on the last call) was "151" and the servo
// is moving slowly as just described, then the true position is likely to be closer
// to "151" than "150", e.g. "151.9". If accounting for this, the code must the
// changes in pot position between calls and not start and set seek positions.
//
// TODO: Proofread and check the example just given is expressed correctly.
//
// NOTE: The "units" of these values matches the value set as the seek_velocity,
//       that was promoted by SEEKVRES.
//
// NOTE: Potential overkill, but worth it if we can get back EMF in on the act for
//       the lower velocities.
//
//       TODO: Back EMF appear to unreliable at present and has been set to one side,
//             it is probably not needed for the higher velocities - therefore when
//             back EMF is off the back-burner it potentially only need be applied
//             for low velocities.
//
//       TODO: At the moment the back EMF coding interrupts the drive to the motor
//             in a way which appears to disrupt it.
//

//
// Increment the call counter used to determine "change in time" (for example, between
// changes in position seen on the pot)...
//
// NOTE: If real floating point values are used we do not need to worry about "overflow",
//       because for a float incremented from 0, there comes a point where "x+1.0==x"!
//
//       If we do hit the top integer, then we might as well assume the velocity is zero...
//       which is what will happen if volest_dt is the maximum positive value.
//
//       Currently the code is assuming that this function is called at a consistent rate
//       in time, this is nearly true!
//
    volest_dt++;
    if(volest_dt<=0) // Incrementing a maximum integer wraps to -1
    {
       volest_dt=0x7FFF;
    }

//
// A large part of this is over-kill to better handle the low velocties, where a low
// velocity is definiately those that are sub one pot step on the ADC and possibily
// ~one pot step on the ADC. Current hardware on the OpenServo doesn't have the
// resolution to fully support this.
//
// TODO: Probably need to recheck this...?
//
// If the position as measured on the pot is unchanged, estimate (guess) where in the
// current pot position the servo might be using the previously estimated velocity and
// position.
//
// Otherwise guess where in the new position it is, based on the estimated velocity. Depends
// on "apparent" direction of travel.
//
// NOTE: This is most useful for helping to workout the issues with low velocity moves. The
//       faster the servo is moving (essentially changing the ADC measurements of pot position),
//       the less important it is (and the smaller any error in the estimated values).
//
//       The estimated position is conditioned by the known position.
//
// TODO: The estimate when the ADC measured current pot position hasn't changed might benefit
//       from including back EMF? Actually, it's a whole lot more complicated.
//
// TODO: So, I am sure it cannot be this simple? There is room for improvement here?
//
//       It may not even be worth it?
//
// Example: the servo is at position 18 on the pot, previously is was at 16, and
//          is supposed to be moving towards 200. Our estimated position based on
//          the (previous) velocity is 19.x. Therefore, maybe it is better to
//          assume the servo's current position is 18.9 (for example), rather
//          than 18.0 or 18.5: because that would fit better with the "evidence"
//          suggested by the measured velocity.
//
//          TODO: Does this work better with the pot positions directly, or the
//                super-sampled position?
//
//

//
// ...these may be conditioned, if necessary, to match the known, or expected, facts...
//
// TODO: Handle the fact that PWM may be "off"?
//
// TODO: Would this not be more accurate using floats? but does it matter?
    estimated_velocity=(current_positionex-volest_previous_positionex)/volest_dt;
// if(current_position!=previous_position) // There has been a change since the last call...
    if(current_positionex!=volest_previous_positionex)
    { // There has been a change since the last call or last "better estimate"...
       if(volest_dt>2 || (labs(current_positionex-volest_previous_positionex)>>SEEKVRES)>2) // NOTE: TODO: Potential for tuning
       { // The most accurate estimate, note under velocity control we hope the velocity is constant, although in practice this will not always be so, etc.
// NOTE: This is "Point A" referred to below.
          volest_dt=0;
          volest_previous_positionex=current_positionex;
          volest_history=1;
       } else
       { // Less accuracy possible, form an average from two instantaneous velocities if possible
          if(volest_history>0)
          { // x/3*2+y/3 = ((x*2)+y)/3
             estimated_velocity<<=1;
             estimated_velocity+=previous_estimated_velocity;
             estimated_velocity/=3;
          }
          if(volest_history<2)
          {
             volest_history++;
          }
       }
       previous_estimated_velocity=estimated_velocity;
    } else
    { // Pure guess work when there is insufficient information...
       if(seek_velocity<(1<<SEEKVRES))
       {
          estimated_velocity=seek_velocity;
       } else
       {
          if(volest_history>10)
          { // x/3*2+y/3 = ((x*2)+y)/3
             estimated_velocity<<=1;
             estimated_velocity+=previous_estimated_velocity;
             estimated_velocity/=3;
          }
//       volest_history=0;
       }
    }

//
// Position...
//
    current_position_estimated=current_positionex;

#if 0 // Temporarily out to test behaviour... or permanent if it is causing too much trouble
#if ((1<<(SEEKVRES-ADC_POSITION_SUPERSAMPLINGBITS))>>1)>0
    if(volest_dt>1 &&
       estimated_velocity>-((1<<(SEEKVRES-ADC_POSITION_SUPERSAMPLINGBITS))>>1) &&
       estimated_velocity<((1<<(SEEKVRES-ADC_POSITION_SUPERSAMPLINGBITS))>>1))
    { //NOTE: The inside of this condition statement could be relocated to "Point A" is only "accurate" velocities should be used
       current_position_estimated+=estimated_velocity;
    }
#endif
#endif

//          volest_backemf_scaling_factor=(estimated_velocity<<8)*volest_dt/volest_backemf_sum;

//
// Store the current position and velocity in the host accessible registers
//
// NOTE: I wonder which people prefer?
//
//       current_positionex is probably the most accurate pot based position available for
//          most purposes. >>SEEKVRES: we are preserving the resolution that current OpenServo
//          users are used to.
//
//          TODO: Add a option (or another standard register) to allow the host to
//                retrieve the current_position_estimated value.
//
//       estimated_velocity is the most accurate velocity available (and is also
//          directly related to the physical speed of the servo and SEEK_VELOCITY,
//          so is probably the most useful value to have).
//
     registers_write_word(REG_POSITION_HI, (uint16_t) (current_positionex>>SEEKVRES));
     registers_write_word(REG_VELOCITY_HI, (uint16_t) filter_update(estimated_velocity, &filter_reg_velocity));

//
// If the current position is not in the deadband range of the set seek position,
// then a PWM output is needed to drive the motor, otherwise the PWM output is set
// to 0. However we always run the PID controller math so that the state is up to
// date.
//
// Resources:
//
//    http://en.wikipedia.org/wiki/PID_controller
//    http://www.openservo.com/Resources?action=AttachFile&do=get&target=automatic_control_theory_for_openservo_v0.3.pdf
//    http://en.wikipedia.org/wiki/Derivative
//    http://www.controlviews.com/articles/QandA/standardPID.html
//    http://www.embedded.com/2000/0010/0010feat3.htm
//
// NOTE: Some of the maths is done using fixed-point floats (for example: x:8). Remember
//       to be wary of overflows and so on.
//
// TODO: How can we, or should we, integrate the current velocity computed above
//       with our velocity here... See back EMF note a little further down.
//
   
    {

//
// Local variables, static again so they're not on the stack
//
       static int8_t in_deadband;
       static float dt;
       static float f=(float)(1<<8);

//
// Control velocity, seek start-up and set position holding
//
// Reset: If a new seek position is set or PWM is turned back on when it has been off,
//        then the controller algorithms need to be reset.
//
// NOTE: Error in position information has to be retained (not reset), so that the
//       controller transitions cleanly from an "off" state when PWM is turned
//       back on after it has been off.
//
       if(previous_seek!=seek_position                // New seek position has been set...
          || previous_pwm_is_enabled!=pwm_is_enabled) // PWM enable state has changed...
       { 
          reset_pid_algorithms=1;
       }
       if(reset_pid_algorithms) // NOTE: reset_pid_algorithms is also set true at initialisation
       {
// TODO: May be able to improve this by remembering and using the previous seek_delta (as the
//       estimate of the current position) as appropriate, rather than just using current_position!
//       Ditto elsewhere.
          position_at_start_of_new_seek=current_position;
          seek_delta=current_position_estimated;
          velocity_control_on=1;
          working_seek_velocity=seek_velocity;
          working_seek_velocity_lag=1;
          bAtOrPassed_last_set_seek_point=0;
          fderivative=0.0;
          fposition_integral=0.0;
          fprevious_position_error=0.0;
          previous_position_error=0.0;

          reset_pid_algorithms=0;

// Debug and testing aids...
          max_position=current_position;
          min_position=current_position;
       } else
       {
          if(current_position>max_position)
          {
             max_position=current_position;
          } else
          {
             if(current_position<min_position)
             {
                min_position=current_position;
             }
          }
          if(seek_velocity!=previous_seek_velocity)
          {
             working_seek_velocity=seek_velocity;
             working_seek_velocity_lag=1;
          }
       }

// TEMPORARY: for testing and debuging- shows up in the servo test program under back EMF!
//      banks_write_word(INFORMATION_BANK, REG_BACKEMF_HI, 
//         (uint16_t)(position_at_start_of_new_seek<=seek_position ? max_position : min_position));

//
// Get initial error in position
//
// NOTE: The code assumes that current_position_estimated has been conditioned to be in the
//       window identified by current_positionex.
//
//       The specified deadband is in "SEEKVRES units"...
//
       perror=seek_positionex-current_position_estimated;
       in_deadband=(perror>=-deadband) && (perror<=deadband);
       
//
// Has the the servo now reached or passed (overshot) the last set seek position?
//
       if(!bAtOrPassed_last_set_seek_point)
       {
          bAtOrPassed_last_set_seek_point=
             (position_at_start_of_new_seek<=seek_position && current_positionex>=seek_positionex) ||
             (position_at_start_of_new_seek>=seek_position && current_positionex<=seek_positionex);
       }
       
//
// Turn off velocity control if any of the following criteria have been met:
//
//    The servo is at or has passed (overshot) the last set seek point position
//      or the current estimated velocity suggests that it will have done so by the time the function is next called
//      or the current estimated position is in the deadband position
// TODO: add?: or the "current estimated velocity suggests that it will be by the time the function is next called"?
       if(velocity_control_on && 
          (in_deadband || bAtOrPassed_last_set_seek_point || seek_velocity==0x8000 ||
             ((position_at_start_of_new_seek<seek_position && current_position_estimated+estimated_velocity>=seek_positionex) ||
              (position_at_start_of_new_seek>seek_position && current_position_estimated+estimated_velocity<=seek_positionex))
          ))
       {
          velocity_control_on=0;
       }

       if(!velocity_control_on)
       { // Use absolute position for perror if velocity control is off
       } else

//
// If velocity control is on, then it is controlled by making the position-to-seek-to move
// incrementally from the start position toward the set seek position, at the set seek
// velocity.
//
// NOTE: For low velocities, we assume that the servo is moving inbetween the positions that
//       can actually be read from the POT via the ADC.
//
//       If a 0 seek velocity is set, the servo will hold position.
//
// For very low velocities we cannot see the position changing via the pot (current_position)
// to sufficient resolution to directly prevent step-wise changes between positions, therefore
// we have to estimate the position at these velocities.
//
// It would be nice to use the back EMF functionality for that, however we need to calibrate
// the EMF to determine a velocity in relation to the position of the servo. This calibration
// is undoubtedly a per servo setting and will depend on factors such as speed and loading.
// Therefore this is left to another day, and we make an assumption about position between
// position steps. A practical way might be to write a program to run scenarios on a servo
// to extract calibration data for it.
//
#if BACKEMF_ENABLED
#endif
       {
          if(position_at_start_of_new_seek<seek_position)
          {
             seek_delta+=working_seek_velocity;
             if(seek_delta>=seek_positionex)
             {
                velocity_control_on=0; // Velocity controlled motion part of the seek is now complete
             }
          } else
          {
             if(position_at_start_of_new_seek>seek_position)
             {
                seek_delta-=working_seek_velocity;
                if(seek_delta<=seek_positionex)
                {
                   velocity_control_on=0; // Velocity controlled motion part of the seek is now complete
                }
             }
          }
          if(velocity_control_on)
          {

//
// Recalculate error based on seek delta when controlling velocity
//
             perror=seek_delta-current_position_estimated;

//
// Problem:  The servo may overshoot the target position due to inertia, the greater the
//           load and/or velocity and/or changes in load (such as moving a weight against
//           or with gravity) and so forth, the greater the problem. Of course, the effect
//           is highly dependant on the mix of the aforementioned contributing factors.
//
// Solution: The solution being explored here is to deaccelerate as the servo approaches
//           the set seek position. A very basic approach is provided to see how well this
//           idea performs in practice.
//
// TODO:     A better solution might use a more parameterised method, and possibly take
//           into account other factors, such as the amount of power being supplied to the
//           motor (i.e. the greater the power vs. velocity the greater the load is likely
//           to be). How about a PID to control the deacceleration?
//
#if 0
// [SNIP] deacceleration, currently in development
#endif
          }
       }

//
// NOTE: We might possibly help remove any instabilities caused by any inconsistency in the
//       rate at which this function is actually called by using a timer register to get a
//       clock time.
//
//          Don't forget clock wrap-around!
//          Be careful of introducing overflows in fixed-point maths
//
// TODO: Explain why 0.1 is probably good and 1.0 could be bad
       dt=0.1;

//
// Fixup perror to a x:8 fixed point float (relative to units used by user)
//
       if(perror<0)
       { // TODO: Test if -ve value handling is necessary
          perror=-perror;
          perror<<=(8-SEEKVRES);
          perror=-perror;
       } else
       {
          perror<<=(8-SEEKVRES);
       }

//
// A bit of a hack: when the servo is holding position, square the smaller errors so
// that more torque is applied to keep the servo position. This is done because our
// error function is probably not ideal.
//
#if 0
       if(bAtOrPassed_last_set_seek_point && perror>-512 && perror<512)
       {                             // TODO: This is the experimental mode,
//          if(perror<0)               //       maybe it should be applied to the
//          {                          //       p term only in the PID eqn?
//             perror*=perror;
//             perror=-perror;
//          } else
//          {
//             perror=perror*perror;
//          }
          perror+=perror;
       }
#endif
       fperror=(float)perror/f;

//
// Derivative term
//
       fderivative=(fperror-fprevious_position_error)/dt;

//
// Set the integral and condition it to prevent windup and overflow
//
// NOTE: The integral component is a useful term in the calculation, but can be somewhat
//       temperamental (in that it can lead to excessive movement and/or oscillation).
//
// TODO: Would it be better to limit its use based on position error?
//
// TODO: Updating the integral between "pot steps" is probably a source of error, because
//       we do not really have an accurate estimation of the position or velocity, therefore
//       we may only want to update it only when the position changes by a pot unit, or
//       correct it when that happens, or use two variables!
//
// TODO: Try: An incremental PID controller gives change required in u, rather than u itself.
//
// u(kT) = P(kT) + I(kT) + D(kT)
// P(kT) = Kp*e(kT)
// I(kT) = I(kT-T) (Kp*T/Ti))*e(kT)
// D(kT) = (Kp*Td/T)[y(kT)-y(kT-T)]


// Option 1: If |error| is not decreasing, don't wind up the integrator?
// Option 2: If the |error| is > x, don't wind up the integrator?
// Option 3: If the |output| is/was/will be > x, then don't wind up the integrator?

       if(((in_deadband || perror==0) && previous_position_error==0) ||
          (!pwm_is_enabled))
       {
          fposition_integral=0.0;
       } else
       {
          fposition_integral+=(fperror*dt);
          if(fposition_integral<MIN_POSITION_INTEGRAL) // The range is currently somewhat arbitrary
          {                                            // We should probably add it to the configuration registers?
             fposition_integral=MIN_POSITION_INTEGRAL;
          } else
          {
             if(fposition_integral>MAX_POSITION_INTEGRAL)
             {
                fposition_integral=MAX_POSITION_INTEGRAL;
             }
          }
       }
       {

//
// Calculate the pwm output to correct for the current apparent error
//
// NOTE: Standard form or? TODO: Could we have a register or flag that could be used to select between the two.
//
          static float pwm_f;
#if 0 // This is the standard form
          pwm_f=(float)banks_read_word(POS_PID_BANK, REG_PID_PGAIN_HI)/f*
                (fperror+
                 (float)banks_read_word(POS_PID_BANK, REG_PID_IGAIN_HI)/f*fposition_integral+
                 (float)banks_read_word(POS_PID_BANK, REG_PID_DGAIN_HI)/f*fderivative); //+
//                      fpwm_feedback;
#else
          // This is the "ideal" (not in the sense that it is the best to use) form, but seems to be the best
          pwm_f=(float)banks_read_word(POS_PID_BANK, REG_PID_PGAIN_HI)/f*fperror+
                (float)banks_read_word(POS_PID_BANK, REG_PID_IGAIN_HI)/f*fposition_integral+
                (float)banks_read_word(POS_PID_BANK, REG_PID_DGAIN_HI)/f*fderivative; // +
//                fpwm_feedback;
#endif
          if(pwm_f>0.0)
          {
             pwm_f+=0.5;
          } else
          {
             if(pwm_f<0.0)
             {
                pwm_f-=0.5;
             }
          }
          if(pwm_f>=MAX_OUTPUT)
          {
             pwm_output=MAX_OUTPUT; // Can't go higher than the maximum output value.
          } else
          {
             if(pwm_f<=MIN_OUTPUT)
             {
                pwm_output=MIN_OUTPUT; // Can't go lower than the minimum output value.
             } else
             {
                pwm_output=(int32_t)pwm_f;
                if(pwm_output==0)
                {
                   if(perror<0)
                   {
                      pwm_output=-1;
                   } else
                   {
                      if(perror>0)
                      {
                         pwm_output=+1;
                      }
                   }
                }
             }
          }
          fpwm_feedback=(pwm_f-(float)pwm_output);

//
// If we're in the deadband, we do not actually drive the motor (return 0 PWM), nor do we drive
// the motor if PWM is disabled. If we're not driving the motor, the integral term must be
// set to zero - if it isn't zeroed, the integral term can continue to increase in value which
// can result in a very bad "non-bumpless" transfer back to position control as well as the
// build-up of oscillations.
//
          if(in_deadband || !pwm_is_enabled)
          {
             fpwm_feedback=0.0;
             fposition_integral=0.0;
             pwm_output=0;
          }

//
// If holding position and the output from this call is a zero PWM, then enable braking.
// If we are not holding position, then the servo needs to be free to move, so braking
// remains disabled.
//
          if(bAtOrPassed_last_set_seek_point && pwm_output==0)
          {
             *pbraking=1;
          } 
       }
    }

//
// Record the state that was acted on, ready for the next call
//
    previous_position=current_position;
    previous_positionex=current_positionex;
    previous_seek=seek_position;
    previous_seek_velocity=seek_velocity;
    previous_pwm_output=pwm_output;
    previous_position_estimated=current_position_estimated;
    previous_pwm_is_enabled=pwm_is_enabled;
    fprevious_position_error=fperror;
    previous_position_error=perror;
    // Return the PID output.
    return (int16_t) pwm_output;
}
