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
#include "openservo_servo.h"
/**
 *
 *
 *
 **/
int openservo_get_ptr(servo_dev *sel_servo, int *reg, int *data)
{
    return io_data_rd(sel_servo, reg, data);
}

int openservo_set_ptr(servo_dev *sel_servo, int *reg, int *data)
{
    return io_data_wr(sel_servo, reg, data);
}

// int openservo_get_8_ptr(servo_dev *sel_servo, int *reg, int *data)
// {
//     return openservo_data_rd_8(sel_servo, reg, data);
// }
// 
// int openservo_set_8_ptr(servo_dev *sel_servo, int *reg, int *data)
// {
//     return openservo_data_wr_8(sel_servo, reg, data);
// }

int openservo_set_1_ptr(servo_dev *sel_servo, int *reg)
{
    return io_data_wr_1(sel_servo, reg);
}
