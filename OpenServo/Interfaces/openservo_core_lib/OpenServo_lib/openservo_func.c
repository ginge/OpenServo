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
#include <stdio.h>
#include "openservo_servo.h"
#include "openservo_io.h"
#include "register_feature_table.h"
#include "openservo_global.h"



int bank_select(servo_dev *sel_servo, int bank)
{
    int addr = BANK_SELECT;
    int ret;
    unsigned char bank_arr[3];
    if (sel_servo->current_bank == bank)
        return 1;

    bank_arr[0] = bank;

    ret = io_data_write(sel_servo, addr, bank_arr, 1);

    sel_servo->current_bank = bank;

    printf ("Selected bank %d\n", bank);

    return ret;
}

double get_servo_version(servo_dev *sel_servo)
{
    return sel_servo->version;
}


int openservo_feature_lookup(int reg, double ver, int *bank, int *length)
{
    int done = 0;
    int n = 0;
    int register_loc = -1;

    while (!done)
    {
        if (register_features[n].feature_name == END_TABLE)
        {
            return ERR_INPUT;
        }

        if (register_features[n].feature_name == reg)
        {
            if (ver >= register_features[n].feature_version_min && ver <= register_features[n].feature_version_max)
            {
                *bank = register_features[n].bank_location;
                register_loc = register_features[n].register_location;
                *length = register_features[n].width;
                done = 1;
            }
        }

        //Feature not supported reached end of table.
        if (register_features[n].feature_name == UNSUPPORTED_FEATURE)
        {
            return -2;
        }

        n++;
    }

    if (register_loc >= 0)
        return register_loc;
    return ERR_INPUT;
}

int openservo_get_firmware_version(int adapter_num, int device_addr)
{

    char buf[255];
    char type[255];
    if (openservo_get_version_type(adapter_num, device_addr, type)<0)
        return ERR_READ;

    if ( type[0] == I2C_DEVICE_OPENSERVO )
    {
        sprintf ( buf, "new servo %x addr 0x%02x\n", type[0], device_addr );
        log_print( buf, ERR_INFO );

        //allocate a new servo structure
        servo_dev *sel_servo;
        int i = 0;

        sel_servo = get_servo(adapter_num, device_addr);

        sel_servo->version = (double)(((int)type[2]) + (((int)type[3]) / 10.0));

        printf ( "OpenServo V%x.%x at 0x%02x\n", type[2], type[3], device_addr );
    }
    else return ERR_NOT_OPENSERVO;
    return ERR_SUCCESS;
}

int openservo_get_version_type(int adapter_num, int device_addr, unsigned char *type)
{
    int res;
    //get the handle for the current adapter
    printf ("get dev type\n");
    //write the command string 0x00

    res = iocore_read(adapter_num, (unsigned char)device_addr, 0x00, type, 4);
    if ( res < 0 )
    {
#ifdef DEBUG
        if ( DEBUG > 0 )
        {
            printf ( "----- i2c_get_device_addr: write error getting device string -----\n" );
        }
#endif
    }

    return res;
}
