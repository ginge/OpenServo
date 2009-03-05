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

int openservo_init(void);
int openservo_scan(int adapter_no, int devices[], int *openservo_count);
int openservo_get_pos(int adapter_no, int servo_no, int *pos);

/**   read the current power drain from the servo */
int openservo_get_current(int adapter_no, int servo_no, int *cur);

/**   read the current velocity from the servo */
int openservo_get_velocity(int adapter_no, int servo_no, int *vel);

/**   Read the current pwm value */
int openservo_get_pwm_cw(int adapter_no, int servo_no, int *pwm);

/**   counter clockwise */
int openservo_get_pwm_ccw(int adapter_no, int servo_no, int *pwm);

/**   Read the current voltage */
int openservo_get_voltage(int adapter_no, int servo_no, int *volt);

/**   set the servo position */
int openservo_set_pos(int adapter_no, int servo_no, int pos);

/**   Do speed movement */
int openservo_set_speed(int adapter_no, int servo_no, int pos, int speed, int method);

int openservo_set_p(int adapter_no, int servo_no, int p);

int openservo_set_i(int adapter_no, int servo_no, int i);

int openservo_set_d(int adapter_no, int servo_no, int d);

int openservo_set_smin(int adapter_no, int servo_no, int min);

int openservo_set_smax(int adapter_no, int servo_no, int max);

int openservo_set_i2c_addr(int adapter_no, int servo_no, int data);

int openservo_set_a_over_current(int adapter_no, int servo_no, int max_current);

int openservo_set_a_over_voltage(int adapter_no, int servo_no, int max_voltage);

int openservo_set_a_under_voltage(int adapter_no, int servo_no, int min_voltage);

int openservo_set_a_over_temp(int adapter_no, int servo_no, int temp);

int openservo_set_deadband(int adapter_no, int servo_no, int deadband);

int openservo_set_pwm_max(int adapter_no, int servo_no, int pwm_max);

int openservo_set_pwm_freq_div(int adapter_no, int servo_no, int pwm_div);

int openservo_set_emf_collapse_delay(int adapter_no, int servo_no, int emfcol);

int openservo_set_emf_charge_delay(int adapter_no, int servo_no, int emfdelay);

/**  turn off the pwm. Stop the servo dead in its tracks */
int openservo_stop(int adapter_no, int servo_no);

/**   (re)enable pwm to the motors */
int openservo_start(int adapter_no, int servo_no);

/**   Reboot the servo */
int openservo_reset(int adapter_no, int servo_no);

int openservo_set_write_en(int adapter_no, int servo_no);

int openservo_set_write_dis(int adapter_no, int servo_no);

int openservo_config_default(int adapter_no, int servo_no);

int openservo_config_restore(int adapter_no, int servo_no);

int openservo_config_save(int adapter_no, int servo_no);

int openservo_request_voltage(int adapter_no, int servo_no);

int openservo_update_world(int adapter_no, int servo_no);