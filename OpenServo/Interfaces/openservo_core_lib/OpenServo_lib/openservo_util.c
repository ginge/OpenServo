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
#include "openservo_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int util_init(void)
{
    LOCKED_MUTEX = 0;
    printf("util init\n");

    return 1;
}

int util_deinit(void)
{
    return 1;
}

int is_bit_set(int set, int bit)
{
    if (set & (1 << bit))
        return 1;

    return 0;
}

/**
 *
 *   use bitwise operations to convert the integer into a hex array
 *
 */
void int_to_hex(unsigned char *hex, int n)
{
    int i;
    for (i = 2 * sizeof(int) - 1; i >= 0; i--)
    {
        hex[i]= "0123456789ABCDEF"[((n >> i*4) & 0xF)];
    }
    /*  hex[0]=n&0xF>>4;
    hex[1]=n &0xF>>8;
    printf("hex: 0x%02x  0x%02x\n", hex[0], hex[1]);*/
}

///small utility funcs
int hexarr_to_int(unsigned char *ncurrent)
{
    int a;
    a = (ncurrent[0] << 8) | ncurrent[1];
    return a;
}

void int_to_hexarr(int pos, unsigned char *npos)
{
    npos[0]= (pos >> 8) & 0x00FF;
    npos[1]= (pos) & 0x00FF;
}

void servo_mutex_lock(void)
{
    int lockchk = 0;

    //If there is already a lock on a thread, wait
    // this allows a failsafe of 10000*10 microseconds before a bailout.
    //should give a fair idea of any deadlocks
    while (LOCKED_MUTEX)
    {
#ifndef WIN32_DLL
//    usleep(100);
#else
        Sleep (10);
#endif
        if (lockchk >10000)
        {
            exit (-1);  //if we got here then the thread never unlocked. Some deadlock or race condition.
        }
        lockchk++;
    }
    // If we have exclusive use, lock
    LOCKED_MUTEX = 1;
}

void servo_mutex_unlock(void)
{
    // reset the mutex
    LOCKED_MUTEX = 0;
}

