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
#include "../IO_core/IO_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int servo_init(void)
{
    servo_count = 0;
    printf( "servo_init\n");
    servos = NULL;
    return ERR_SUCCESS;
}

int servo_deinit()
{
    int i;

    // deallocate all servos
    while(servos != NULL)
    {
        //close i2c handle
        servo_remove(&servos);
    }
    return ERR_SUCCESS;
}

/**
 *
 *  Add a servo manually. Useful if you already know the bus address
 *  and you want to save time.
 *
 */
int openservo_servo_add(int adapter_no, int servo_no)
{
    servo_dev *sel_servo;

    sel_servo = servo_add(&servos, servo_no);
    sel_servo->adapter_num = adapter_no;
    servo_count++;
    if (openservo_get_firmware_version(adapter_no, servo_no) < 0)
        return ERR_READ;
    return 1;
}

/**
 *
 *  Add a servo to the servo array
 *  allocates memory, and returns pointer to servo
 *
 */
servo_dev *servo_add ( servo_dev ** p, int i )
{
    servo_dev *n;

    n = ( servo_dev* ) malloc ( sizeof ( servo_dev ) );

    n->next = *p;
    *p = n;
    
    //set initial data
    memset ( n, 0, sizeof ( n ) );
    memset ( n->registers, 0, sizeof ( n->registers ) );
    memset ( n->registers_timestamp, 0, sizeof ( n->registers_timestamp ) );
    memset ( n->registers_flagged, 0, sizeof ( n->registers_flagged ) );
    memset ( n->banks, 0, sizeof ( n->banks ) );
    memset ( n->banks_timestamp, 0, sizeof ( n->banks_timestamp ) );
    memset ( n->banks_flagged, 0, sizeof ( n->banks_flagged ) );
    memset ( n->flags_registered, 0, sizeof ( n->flags_registered ) );

    n->address = i;
    n->adapter_num = -1;

    return n;
}

/**
 *
 *  Remove servo from head of linked list, dealloc mem
 *  TODO make sure this function is altered to accept position or pointer p, and remove from there
 *
 */
void servo_remove (servo_dev ** p)
{
    /* remove head */
    servo_dev *n;
    if (*p != NULL)
    {
        n = *p;
        // Check to see if we are removing the last servo device, if so set to NULL
        if ((*p)->next == NULL)
        {
            free (n);
            *p = NULL;
            return;
        }
        else
            *p = (*p)->next;
        // Free servo memory
        free (n);
    }
}

/**
 *
 *  returns a pointer to the servo with address i
 *
 */
servo_dev **servo_search (int adapter_no, servo_dev ** n, int i)
{
    if (!(*n)) return NULL;
    while (*n != NULL)
    {
        if (((*n)->address == i) && (adapter_no == (*n)->adapter_num))
        {
            return n;
        }
        n = & (*n)->next;
    }
    servo_dev *d = NULL;
    return &d;
}

/**
 *
 *   Get the servo instance
 *
 */
servo_dev *get_servo(int adapter_no, int servo_no)
{
    if (servos == NULL)
        return NULL;
    servo_dev *sel_servo = NULL;
    sel_servo = *servo_search(adapter_no, &servos, servo_no);
    return sel_servo;
}

int registers_read_word(servo_dev *sel_servo, int address_hi)
// Read a 16-bit word from the registers.
{
    int value;

    if (sel_servo == NULL)
        return -1;

    // Check if it is a register or a bank
    if ((sel_servo->current_bank >= 0) && (address_hi >= V3_MIN_BANK_REGISTER))
    {
        value = (sel_servo->banks[sel_servo->current_bank][address_hi - V3_MIN_BANK_REGISTER] << 8) | sel_servo->banks[sel_servo->current_bank][address_hi + 1 - V3_MIN_BANK_REGISTER];
    }
    else
    {
        // Read the registers.
        value = (sel_servo->registers[address_hi] << 8) | sel_servo->registers[address_hi+1];
    }

    return value;
}

int registers_read_byte(servo_dev *sel_servo, int address)
{
    int value;

    if (sel_servo == NULL)
        return -1;

    // Check if it is a register or a bank
    if ((sel_servo->current_bank >= 0) && (address >= V3_MIN_BANK_REGISTER))
    {
        value = sel_servo->banks[sel_servo->current_bank][address - V3_MIN_BANK_REGISTER];
    }
    else
    {
        // Read the registers.
        value = sel_servo->registers[address];
    }

    return value;
}

void registers_write_word(servo_dev *sel_servo, int address_hi, int value)
// Write a 16-bit word to the registers.
{
    int sreg;

    // Write the registers.
    sel_servo->registers[address_hi] = value >> 8;
    sel_servo->registers[address_hi + 1] = value;

}
