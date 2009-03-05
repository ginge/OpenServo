/***************************************************************************
 *   Copyright (C) 2008 by Barry Carter,,,   *
 *   barry.carter@robotfuzz.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "../IO_core/libio_core.h"
#include "../IO_core/IO_error.h"
#include "openservo_servo.h"
#include "libopenservo_lib.h"

/**   Master servo initialisation. */
int openservo_init(void)
{
    int ret = ERR_SUCCESS;
    if ((ret = servo_init()) < 0) return ret;
    if ((ret = io_init()) < 0) return ret;
    if ((ret = util_init()) < 0) return ret;
    if ((ret = config_init()) < 0) return ret;

    printf("openservo init\n");
    return ret;
}

int openservo_deinit(void)
{
    int ret = ERR_SUCCESS;
    int n;

    if ((ret = servo_deinit()) < 0) return ret;
    if ((ret = io_deinit()) < 0) return ret;
    if ((ret = util_deinit()) < 0) return ret;
    if ((ret = config_deinit()) < 0) return ret;
    printf("openservo deinit\n");
    return ret;
}

int adapter_get_adapter_count(int *adapters)
{
    iocore_get_adapter_count(adapters);
    return ERR_SUCCESS;
}

int adapter_get_adapter_name(int adapter_no, unsigned char *name)
{
    iocore_get_adapter_name(adapter_no, name);
    return ERR_SUCCESS;
}

int openservo_scan(int adapter_no, int devices[], int *openservo_count)
{
    int res;
    int n;
    int dev_count = 0;
    int i = 0;

    *openservo_count = 0;

    if ((res = iocore_scan(adapter_no, &bus_devices, &dev_count)) < 0)
        return res;

    printf("devices found %d\n", dev_count);

    for(n = 0; n < dev_count; n++)
    {
        res = openservo_servo_add(adapter_no, bus_devices[n]);             //add a manual servo
        /* Is it an OpenServo? */
        if (res < 0)
        {
        }
        else if (res != ERR_NOT_OPENSERVO)
        {
            devices[i] = bus_devices[n];
            i++;
        }
    }
    *openservo_count = dev_count;
    return ERR_SUCCESS;
}

/**   read the current position from the servo */
int openservo_get_pos(int adapter_no, int servo_no, int *pos)
{
    return openservo_get(adapter_no, servo_no, POSITION_HI, pos);
}

/**   read the current power drain from the servo */
int openservo_get_current(int adapter_no, int servo_no, int *cur)
{
    return openservo_get(adapter_no, servo_no, POWER_HI, cur);
}

/**   read the current velocity from the servo */
int openservo_get_velocity(int adapter_no, int servo_no, int *vel)
{
    return openservo_get(adapter_no, servo_no, VELOCITY_HI, vel);
}

/**   Read the current pwm value */
int openservo_get_pwm_cw(int adapter_no, int servo_no, int *pwm)
{
    return openservo_get(adapter_no, servo_no, PWM_DIRA, pwm);
}

/**   counter clockwise */
int openservo_get_pwm_ccw(int adapter_no, int servo_no, int *pwm)
{
    return openservo_get(adapter_no, servo_no, PWM_DIRB, pwm);
}

/**   Read the current voltage */
int openservo_get_voltage(int adapter_no, int servo_no, int *volt)
{
    return openservo_get(adapter_no, servo_no, VOLTAGE_HI, volt);
}

/**   Read the current temperature */
int openservo_get_temperature(int adapter_no, int servo_no, int *temp)
{
    return openservo_get(adapter_no, servo_no, TEMPERATURE_HI, temp);
}

/**   Read the current EMF value */
int openservo_get_emf(int adapter_no, int servo_no, int *emf)
{
    return openservo_get(adapter_no, servo_no, BACKEMF_HI, emf);
}


int openservo_get_alert(int adapter_no, int servo_no, int *alert)
{
    int res = openservo_get(adapter_no, servo_no, ALERT_STATUS, alert);
    return res;
}

int openservo_get_flag(int adapter_no, int servo_no, int *flags)
{
    int res = openservo_get(adapter_no, servo_no, FLAGS_HI, flags);
    return res;
}

int openservo_get_p(int adapter_no, int servo_no, int *p)
{
    int res = openservo_get(adapter_no, servo_no, PID_PGAIN_HI, p);
    return res;
}

int openservo_get_i(int adapter_no, int servo_no, int *i)
{
    int res = openservo_get(adapter_no, servo_no, PID_IGAIN_HI, i);
    return res;
}

int openservo_get_d(int adapter_no, int servo_no, int *d)
{
    int res = openservo_get(adapter_no, servo_no, PID_DGAIN_HI, d);
    return res;
}

int openservo_get_smin(int adapter_no, int servo_no, int *smin)
{
    int res = openservo_get(adapter_no, servo_no, MIN_SEEK_HI, smin);
    return res;
}

int openservo_get_smax(int adapter_no, int servo_no, int *smax)
{
    int res = openservo_get(adapter_no, servo_no, MAX_SEEK_HI, smax);
    return res;
}

int openservo_get_i2c_addr(int adapter_no, int servo_no, int *addr)
{
    int res = openservo_get(adapter_no, servo_no, TWI_ADDRESS, addr);
    return res;
}

int openservo_get_a_over_current(int adapter_no, int servo_no, int *a)
{
    int res = openservo_get(adapter_no, servo_no, ALERT_CURR_MAX_LIMIT_HI, a);
    return res;
}

int openservo_get_a_over_voltage(int adapter_no, int servo_no, int *a)
{
    int res = openservo_get(adapter_no, servo_no, ALERT_VOLT_MAX_LIMIT_HI, a);
    return res;
}

int openservo_get_a_under_voltage(int adapter_no, int servo_no, int *a)
{
    int res = openservo_get(adapter_no, servo_no, ALERT_VOLT_MIN_LIMIT_HI, a);
    return res;
}

int openservo_get_a_over_temp(int adapter_no, int servo_no, int *a)
{
    int res = openservo_get(adapter_no, servo_no, ALERT_TEMP_MAX_LIMIT_HI, a);
    return res;
}

int openservo_get_deadband(int adapter_no, int servo_no, int *deadband)
{
    int res = openservo_get(adapter_no, servo_no, PID_DEADBAND, deadband);
    return res;
}

int openservo_get_pwm_max(int adapter_no, int servo_no, int *pwm_max)
{
    int res = openservo_get(adapter_no, servo_no, PWM_MAX, pwm_max);
    return res;
}

int openservo_get_pwm_freq_div(int adapter_no, int servo_no, int *freq_div)
{
    int res = openservo_get(adapter_no, servo_no, PWM_FREQ_DIVIDER_HI, freq_div);
    return res;
}

int openservo_get_emf_collapse_delay(int adapter_no, int servo_no, int *pwm_del)
{
    int res = openservo_get(adapter_no, servo_no, EMF_COLLAPSE_DELAY, pwm_del);
    return res;
}

int openservo_get_emf_charge_delay(int adapter_no, int servo_no, int *freq_del)
{
    int res = openservo_get(adapter_no, servo_no, EMF_CHARGE_TIME, freq_del);
    return res;
}

int openservo_get_servo_version(int adapter_no, int servo_num, double *version)
{
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_num);
    if (sel_servo == NULL)
        return ERR_NOT_OPENSERVO;
    *version = sel_servo->version;
    return ERR_SUCCESS;
}

int openservo_is_a_over_current(int adapter_no, int servo_no)
{
    return openservo_alert_check(adapter_no, servo_no, V3_ALERT_OVERCURR);
}

int openservo_is_a_over_voltage(int adapter_no, int servo_no)
{
    return openservo_alert_check(adapter_no, servo_no, V3_ALERT_OVERVOLT);
}

int openservo_is_a_under_voltage(int adapter_no, int servo_no)
{
    return openservo_alert_check(adapter_no, servo_no, V3_ALERT_UNDERVOLT);
}

int openservo_is_a_over_temp(int adapter_no, int servo_no)
{
    return openservo_alert_check(adapter_no, servo_no, V3_ALERT_OVERTEMP);
}

int openservo_is_pwm_enabled(int adapter_no, int servo_no)
{
    return openservo_flag_check(adapter_no, servo_no, FLAGS_HI);
}

int openservo_alert_check(int adapter_no, int servo_no, int alert)
{
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);
    return is_bit_set(registers_read_byte(sel_servo, ALERT_STATUS), alert);
}

int openservo_flag_check(int adapter_no, int servo_no, int flag)
{
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);
    return is_bit_set(registers_read_byte(sel_servo, FLAGS_HI), flag);
}

/************************************
*
*     Writing data to the servo
*
*************************************/

/**   set the servo position */
int openservo_set_pos(int adapter_no, int servo_no, int pos)
{
    return openservo_set(adapter_no, servo_no, SEEK_POSITION_HI, pos);
}

/**
 *
 *   Do speed movement
 *   how should this be done. Set the servo to have a speed set by this, and always
 *   move by that speed, or pass the speed every time?
 *
 *   note 100=100 units/second
 *   Can speed register be used to confirm movement is over, as it is zero'd?
 */
int openservo_set_speed(int adapter_no, int servo_no, int pos, int speed, int method)
{
    return openservo_set(adapter_no, servo_no, SEEK_VELOCITY_HI, pos);
}


int openservo_set_p(int adapter_no, int servo_no, int p)
{
    return openservo_set(adapter_no, servo_no, PID_PGAIN_HI, p);
}

int openservo_set_i(int adapter_no, int servo_no, int i)
{
    return openservo_set(adapter_no, servo_no, PID_IGAIN_HI, i);
}


int openservo_set_d(int adapter_no, int servo_no, int d)
{
    return openservo_set(adapter_no, servo_no, PID_DGAIN_HI, d);
}

int openservo_set_smin(int adapter_no, int servo_no, int min)
{
    return openservo_set(adapter_no, servo_no, MIN_SEEK_HI, min);
}

int openservo_set_smax(int adapter_no, int servo_no, int max)
{
    return openservo_set(adapter_no, servo_no, MAX_SEEK_HI, max);
}

int openservo_set_i2c_addr(int adapter_no, int servo_no, int data)
{
     return openservo_set(adapter_no, servo_no, TWI_ADDRESS, data);
}

int openservo_set_a_over_current(int adapter_no, int servo_no, int max_current)
{
    return openservo_set(adapter_no, servo_no, ALERT_CURR_MAX_LIMIT_HI, max_current);
}

int openservo_set_a_over_voltage(int adapter_no, int servo_no, int max_voltage)
{
    return openservo_set(adapter_no, servo_no, ALERT_VOLT_MAX_LIMIT_HI, max_voltage);
}

int openservo_set_a_under_voltage(int adapter_no, int servo_no, int min_voltage)
{
    return openservo_set(adapter_no, servo_no, ALERT_VOLT_MIN_LIMIT_HI, min_voltage);
}

int openservo_set_a_over_temp(int adapter_no, int servo_no, int temp)
{
    return openservo_set(adapter_no, servo_no, ALERT_TEMP_MAX_LIMIT_HI, temp);
}

int openservo_set_deadband(int adapter_no, int servo_no, int deadband)
{
    return openservo_set(adapter_no, servo_no, PID_DEADBAND, deadband);
}

int openservo_set_pwm_max(int adapter_no, int servo_no, int pwm_max)
{
    return openservo_set(adapter_no, servo_no, PWM_MAX, pwm_max);
}

int openservo_set_pwm_freq_div(int adapter_no, int servo_no, int pwm_div)
{
    return openservo_set(adapter_no, servo_no, PWM_FREQ_DIVIDER_HI, pwm_div);
}

int openservo_set_emf_collapse_delay(int adapter_no, int servo_no, int emfcol)
{
    return openservo_set(adapter_no, servo_no, EMF_COLLAPSE_DELAY, emfcol);
}

int openservo_set_emf_charge_delay(int adapter_no, int servo_no, int emfdelay)
{
    return openservo_set(adapter_no, servo_no, EMF_CHARGE_TIME, emfdelay);
}









/**  turn off the pwm. Stop the servo dead in its tracks */
int openservo_stop(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_PWM_DISABLE);
}

/**   (re)enable pwm to the motors */
int openservo_start(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_PWM_ENABLE);
}

/**   Reboot the servo */
int openservo_reset(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_RESET);
}

int openservo_set_write_en(int adapter_no, int servo_no)
{

//     enableWrite(adapter_no, servo_no, 1);
    return 1;
}

int openservo_set_write_dis(int adapter_no, int servo_no)
{
//     disableWrite(adapterNum, servo, 1);
    return 1;
}

int openservo_config_default(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_REGISTERS_DEFAULT);
}

int openservo_config_restore(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_REGISTERS_RESTORE);
}

int openservo_config_save(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_REGISTERS_SAVE);
}

int openservo_request_voltage(int adapter_no, int servo_no)
{
    return openservo_set_1(adapter_no, servo_no, TWI_CMD_VOLTAGE_READ);
}

/* Handler for registration of registers to watch
 *
 */
int openservo_register_address(int adapter_no, int servo_no, int register_name, int priority)
{
    int loc;
    int bank;
    int length;
    int n;
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);

    // Check to see if it is in a bank so we can copy the data into our register array
    loc = openservo_feature_lookup(register_name, sel_servo->version, &bank, &length);

    if (bank >=0)
    {
        for (n = 0; n < length; n++)
            sel_servo->banks_flagged[bank][loc + n - V3_MIN_BANK_REGISTER] = priority;
    }
    else
    {
        for (n = 0; n < length; n++)
            sel_servo->registers_flagged[loc + n] = priority;
    }

    return ERR_SUCCESS;
}

int openservo_deregister_address(int adapter_no, int servo_no, int register_name)
{
    int loc;
    int bank;
    int length;
    int n;
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);

    // Check to see if it is in a bank so we can copy the data into our register array
    loc = openservo_feature_lookup(register_name, sel_servo->version, &bank, &length);

    if (bank >=0)
    {
        for (n = 0; n < length; n++)
            sel_servo->banks_flagged[bank][loc + n - V3_MIN_BANK_REGISTER] = 0;
    }
    else
    {
        for (n = 0; n < length; n++)
            sel_servo->registers_flagged[loc + n] = 0;
    }

    return ERR_SUCCESS;
}

int openservo_set_priority(int adapter_np, int servo_no)
{
    return ERR_SUCCESS;
}

int openservo_update_world(int adapter_no, int servo_no)
{
    return update_world(adapter_no, servo_no);
}

/* These are provided to make each of the smaller accessor functions as small as possible.
   The functions look up a given servo number, and fetches the pointer instance to the servo
   for comms with the pointer based functions.
*/

/**   Reading data from the servo */
int openservo_get(int adapter_no, int servo_no, int reg_addr, int *data)
{
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);
    if (sel_servo == NULL)
        return -1;
    openservo_get_ptr(sel_servo, reg_addr, data);
    return 1;
}

int openservo_set(int adapter_no, int servo_no, unsigned char reg_addr, int data)
{
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);
    if (sel_servo == NULL)
        return -1;
    openservo_set_ptr(sel_servo, reg_addr, data);
    return 1;
}


/**   Reading data from the servo */
// int openservo_get(int adapter_no, int servo_no, int reg_addr, int *data)
// {
//     servo_dev *sel_servo;
//     sel_servo = get_servo(adapter_no, servo_no);
//     openservo_get_8_ptr(sel_servo, reg_addr, data);
//     return 1;
// }
// 
// int openservo_set(int adapter_no, int servo_no, unsigned char reg_addr, int data)
// {
//     servo_dev *sel_servo;
//     sel_servo = get_servo(adapter_no, servo_no);
//     openservo_set_ptr(sel_servo, reg_addr, data);
//     return 1;
// }

/**   Reading data from the servo */
int openservo_set_1(int adapter_no, int servo_no, int reg_addr)
{
    servo_dev *sel_servo;
    sel_servo = get_servo(adapter_no, servo_no);
    openservo_set_1_ptr(sel_servo, reg_addr);
    return 1;
}

