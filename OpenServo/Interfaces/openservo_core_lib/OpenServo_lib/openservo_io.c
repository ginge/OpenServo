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
#include "openservo_config.h"

// IO layer initialiser
int io_init(void)
{
    printf("io init\n");
    // Initialise the IO_core library
    return iocore_init();
}

int io_deinit(void)
{

    iocore_deinit();
}

int io_get_bank_register(servo_dev *sel_servo, unsigned char cmd, unsigned char *max_register, int *length)
{
    int bank;
    // Get the register and bank for the feature.
    cmd = openservo_feature_lookup(cmd, get_servo_version(sel_servo), &bank, length);

    if (cmd < 1)
    {
        return ERR_INVALID_REGISTER;
    }
    else if (cmd < 0)
    {
        return ERR_INPUT;
    }
    // If the firmware supports bank switching
    if (bank >= 0)
    {
        // Change to the bank in bank_table
        bank_select(sel_servo, bank);
        // calculate the start position of the bank as an added offset value
        // This is because bank registers start from 0
        *max_register = cmd + V3_MIN_BANK_REGISTER;
    }
    else
    {
        // not a bank so we dont need to offset.
        *max_register = cmd;
    }
    return ERR_SUCCESS;
}

/**
 *
 *   Write one command to the servo. sends only 1 byte
 *
 */
int io_data_wr_1(servo_dev *sel_servo, unsigned char cmd)
{
    unsigned char buf[3];
    buf[0] = 1;
    return iocore_write(sel_servo->adapter_num, (unsigned char) sel_servo->address, cmd, &buf, 1);
}

/**
 *
 *   Write one 8 bit register. sends only 1 byte of data
 *
 */
// int openservo_data_wr_8(servo_dev *sel_servo, unsigned char cmd, int value)
int io_data_wr(servo_dev *sel_servo, unsigned char cmd, int value)
{
    unsigned char value_arr[2];
    unsigned char max_register;
    int length;

    io_get_bank_register(sel_servo, cmd, &max_register, &length);

    if (length == 2)
    {
        int_to_hexarr(value, value_arr);
    }
    else if (length == 1)
    {
        //put the value integer into the string
        value_arr[0] = (unsigned char) value;
    }
    else return -1;

    return io_data_write(sel_servo, max_register, value_arr, length);
}

/**
 *
 *   Read one 8 bit register. reads only 1 byte of data
 *
 */
// int openservo_data_rd_8 (servo_dev *sel_servo, unsigned char cmd, int *value)
int io_data_rd(servo_dev *sel_servo, unsigned char cmd, int *value)
{
    unsigned char rd_value[3];
    int retval;
    int length;
    unsigned char max_register;

    io_get_bank_register(sel_servo, cmd, &max_register, &length);

    if ((retval = io_data_read(sel_servo, max_register, rd_value, length)) < 0)
        return retval;

    // Copy the return data into the integer value
    if (length == 2)
        *value = hexarr_to_int(rd_value);
    else if (length == 1)
        *value = (int)rd_value[0];
    else
        *value = -1;

    return retval;
}

/**
 *
 *   read length rd_len buf from 8 bit command cmd register
 *
 */
int io_data_read(servo_dev *sel_servo, unsigned char cmd, unsigned char *buf, int rd_len)
{
    int n;

    if (sel_servo == NULL)
        return ERR_INPUT;
    //lock the read/write to the servo structure
    servo_mutex_lock();
    //is this servo is configured to read directly, then return struct data
    if (config_check_direct(sel_servo) > 0)
    {
        if (iocore_read(sel_servo->adapter_num, (unsigned char)sel_servo->address, cmd, buf, rd_len) < 0)
        {
            log_print("Error reading from servo", ERR_COMMS);
            //unlock mutex
            servo_mutex_unlock();
            return -1;
        }

        // Check to see if it is in a bank so we can copy the data into our register array
        if ((cmd >= V3_MIN_BANK_REGISTER) && (sel_servo->current_bank >= 0))
        {
            memcpy(&sel_servo->banks[sel_servo->current_bank][cmd - V3_MIN_BANK_REGISTER], buf, rd_len);
            // timestamp the registers of the data we just read
            for(n = 0; n < rd_len; n++)
            {
                sel_servo->banks_timestamp[sel_servo->current_bank][cmd - V3_MIN_BANK_REGISTER] = openservo_config->tick;
            }
        }
        else
        {
            memcpy(&sel_servo->registers[cmd], buf, rd_len);
            // timestamp the registers of the data we just read
            for(n = 0; n < rd_len; n++)
            {
                sel_servo->registers_timestamp[cmd] = openservo_config->tick;
            }
        }
    }
    else
    {
        // Check to see if it is in a bank so we can copy the data into our register array
        if ((cmd >= V3_MIN_BANK_REGISTER) && (sel_servo->current_bank >= 0))
        {
            memcpy(buf, &sel_servo->banks[sel_servo->current_bank][cmd - V3_MIN_BANK_REGISTER], rd_len);
        }
        else
        {
            memcpy(buf, &sel_servo->registers[cmd], rd_len);
        }
    }
 /*   else
    {
        //map_cmd[] map_data[]
        int i;
        for ( i = 0; i< sel_servo->sv_map->map_len; i++ )
        {
            //if the command in the array is the same as the map
            if ( sel_servo->sv_map->map_cmd[i] == ( int ) cmd )
            {
                //check the data is 16 bit
                if ( rd_len > 1 )
                {
                    int_to_hexarr ( *sel_servo->sv_map->map_data[i], buf );
                }
                else
                {
                    buf[0]=*sel_servo->sv_map->map_data[i];
                }
                //unlock mutex
                servo_mutex_unlock();
                return 1;
            }
        }

    }
 */   //unlock mutex
    servo_mutex_unlock();
    return 1;
}

/**
 *
 *   write a block of data to the device at command cmd
 *
 */
int io_data_write(servo_dev *sel_servo, unsigned char cmd, unsigned char *buf,  int wr_len)
{
    if (sel_servo == NULL)
        return ERR_INPUT;
    //lock access to the servo
    servo_mutex_lock();
    if (config_check_direct(sel_servo) > 0)
    {
        if (iocore_write(sel_servo->adapter_num, (unsigned char)sel_servo->address, cmd, buf, wr_len) <0)
        {
            log_print("Error reading from servo", ERR_WRITE);
            //unlock mutex
            servo_mutex_unlock();
            return -1;
        }
    }
/*    else
    {
        //map_cmd[] map_data[]
        int i;
        for ( i = 0; i< sel_servo->sv_map->map_len; i++ )
        {
            //if the command in the array is the same as the map
            if ( sel_servo->sv_map->map_cmd[i] == ( int ) cmd )
            {

                //check to see if the data beng written has actually changed.
                if ( *sel_servo->sv_map->map_data[i] != hexarr_to_int ( buf ) )
                {
                    // flag that an update is needed
                    sel_servo->os_comm->write_change=1;
                }
                //set the data in structure to the written pos
                *sel_servo->sv_map->map_data[i]=hexarr_to_int ( buf );
                //unlock mutex
                servo_mutex_unlock();
                return 1;
            }
        }
    }*/
    servo_mutex_unlock();
    return 1;
}
