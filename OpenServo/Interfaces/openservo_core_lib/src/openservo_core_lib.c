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
#include "../OpenServo_lib/libopenservo_lib.h"
#include "../OpenServo_lib/register_feature_table.h"
#include "../OpenServo_lib/openservo_config.h"

int main(int argc, char *argv[])
{
    char buf[100];
    int devices[255];
    int dev_count;

    printf("Hello, world!\n");
    openservo_init();
    openservo_servo_add( 0, 0x10 );             //add a manual servo
    openservo_scan(1, devices, &dev_count);
    int adapter = 1;
    int servo = 0x10;
    test_accessors(adapter, servo);
    test_optimiser(adapter, servo);
    openservo_deinit(1);

    return EXIT_SUCCESS;
}

int test_accessors(int adapter, int servo)
{
    int var, var1;
    int ret;
    int ret1;

    var = 400;
    ret = openservo_set_pos(adapter, servo, var);
    ret1 = openservo_get_pos(adapter, servo, &var1);
    printf("pos: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return -1; }

    ret = openservo_get_current(adapter, servo, &var);
    printf("cur: %d return: %d\n ", var, ret);
    ret = openservo_get_velocity(adapter, servo, &var);
    printf("vel: %d return: %d\n ", var, ret);
    ret = openservo_get_pwm_cw(adapter, servo, &var);
    printf("pcw: %d return: %d\n ", var, ret);
    ret = openservo_get_pwm_ccw(adapter, servo, &var);
    printf("ccw: %d return: %d\n ", var, ret);
    ret = openservo_get_voltage(adapter, servo, &var);
    printf("vol: %d return: %d\n ", var, ret);


//     ret = openservo_set_speed(int adapter, int servo_no, int pos, int speed, int method);
    var = 0x800;
    ret = openservo_set_p(adapter, servo, var);
    ret1 = openservo_get_p(adapter, servo, &var1);
    printf("p: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    var = 0x200;
    ret = openservo_set_i(adapter, servo, var);
    ret1 = openservo_get_i(adapter, servo, &var1);
    printf("i: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    var = 0x400;
    ret = openservo_set_d(adapter, servo, var);
    ret1 = openservo_get_d(adapter, servo, &var1);
    printf("d: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_smin(adapter, servo, var);
    ret1 = openservo_get_smin(adapter, servo, &var1);
    printf("smin: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_smax(adapter, servo, var);
    ret1 = openservo_get_smax(adapter, servo, &var1);
    printf("smax: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    var = 0x10;
    ret = openservo_set_i2c_addr(adapter, servo, var);
    ret1 = openservo_get_i2c_addr(adapter, servo, &var1);
    printf("i2c: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_a_over_current(adapter, servo, var);
    ret1 = openservo_get_a_over_current(adapter, servo, &var1);
    printf("a_overc: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_a_over_voltage(adapter, servo, var);
    ret1 = openservo_get_a_over_voltage(adapter, servo, &var1);
    printf("a_overv: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_a_under_voltage(adapter, servo, var);
    ret1 = openservo_get_a_under_voltage(adapter, servo, &var1);
    printf("a_underv: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_a_over_temp(adapter, servo, var);
    ret1 = openservo_get_a_over_temp(adapter, servo, &var1);
    printf("a_overtemp: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_deadband(adapter, servo, var);
    ret1 = openservo_get_deadband(adapter, servo, &var1);
    printf("deadb: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_pwm_max(adapter, servo, var);
    ret1 = openservo_get_pwm_max(adapter, servo, &var1);
    printf("pwm_max: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_pwm_freq_div(adapter, servo, var);
    ret1 = openservo_get_pwm_freq_div(adapter, servo, &var1);
    printf("pwm_div: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_emf_collapse_delay(adapter, servo, var);
    ret1 = openservo_get_emf_collapse_delay(adapter, servo, &var1);
    printf("emf_col: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_set_emf_charge_delay(adapter, servo, var);
    ret1 = openservo_get_emf_charge_delay(adapter, servo, &var1);
    printf("emf_cha: %d return: %d return2: %d\n ", var1, ret, ret1);
    if (var != var1) { return ret; }

    ret = openservo_stop(adapter, servo);
    ret = openservo_start(adapter, servo);
    ret = openservo_reset(adapter, servo);
    sleep(3);
    ret = openservo_set_write_dis(adapter, servo);
    ret = openservo_set_write_en(adapter, servo);

    ret = openservo_config_default(adapter, servo);
    ret = openservo_config_restore(adapter, servo);
    ret = openservo_config_save(adapter, servo);
    ret = openservo_request_voltage(adapter, servo);

    return ret;
}

int test_optimiser(int adapter, int servo)
{
    int n;
    int var;
    int ret;

    // Set the mode to read the servos indirectly
    openservo_config_direct_read(adapter, servo, ADAPTER_BUFFERED);
    openservo_register_address(adapter, servo, POSITION_HI, 1);
    openservo_register_address(adapter, servo, VELOCITY_HI, 2);
    openservo_register_address(adapter, servo, POWER_HI, 3);
    openservo_register_address(adapter, servo, VOLTAGE_HI, 4);

    openservo_update_world(adapter, servo);

    // Do a few loops and see what happens
    for(n = 0;n < 10; n++)
    {
        openservo_update_world(adapter, servo);

        printf("updated world\n");
        ret = openservo_get_pos(adapter, servo, &var);
        printf("pos %d\n", var);
    }

    return 1;
}

int test_iocore(void)
{
/*    iocore_init();
    iocore_write(0, 0x10, 0x00, buf, 10);
    iocore_read(0, 0x10, 0x00, buf, 10);
    printf("read %s", buf);
    printf("\n");
    iocore_deinit();*/
}
