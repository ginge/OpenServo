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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "v3_registers.h"

int adapter_init(char *options);
int adapter_deinit(void);
int adapter_write(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
int adapter_read(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
int adapter_readonly(int adapter, int servo, unsigned char *data, size_t len);
int adapter_reflash(int adapter, int servo, int bootloader_addr, char *filename);
int adapter_command(int adapter, int servo, unsigned char command);
int adapter_scan(int adapter, int devices[], int *dev_count);
int adapter_probe(int adapter, int servo);
int adapter_get_adapter_name(int adapter, char *name);
int adapter_get_adapter_count(void);
int adapter_set_bitrate(int adapter, int bitrate);

static unsigned char dummybuf[1024];
static int set_pos;
struct timespec start, end;
uint64_t elapsed;

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);

int adapter_init(char *options)
{
    memset(dummybuf, 0, 1024);
    //set the servo device type
    dummybuf[V3_REG_DEVICE_TYPE        ] = 0x01;         // ID
    dummybuf[V3_REG_DEVICE_SUBTYPE     ] = 0x00;         // ID
    dummybuf[V3_REG_VERSION_MAJOR      ] = 0x03;         // Version HI
    dummybuf[V3_REG_VERSION_MINOR      ] = 0x01;         // Version LO
    dummybuf[V3_REG_FLAGS_HI           ] = 0x00;
    dummybuf[V3_REG_FLAGS_LO           ] = 0x00;
    dummybuf[V3_REG_TIMER_HI           ] = 0x00;
    dummybuf[V3_REG_TIMER_LO           ] = 0x00;
    dummybuf[V3_REG_POSITION_HI        ] = 0x03;
    dummybuf[V3_REG_POSITION_LO        ] = 0x84;
    dummybuf[V3_REG_VELOCITY_HI        ] = 0x00;
    dummybuf[V3_REG_VELOCITY_LO        ] = 0x00;
    dummybuf[V3_REG_POWER_HI           ] = 0x00;
    dummybuf[V3_REG_POWER_LO           ] = 0x05;
    dummybuf[V3_REG_PWM_DIRA           ] = 0x00;
    dummybuf[V3_REG_PWM_DIRB           ] = 0x00;
    dummybuf[V3_REG_VOLTAGE_HI         ] = 0x04;
    dummybuf[V3_REG_VOLTAGE_LO         ] = 0x01;
    dummybuf[V3_REG_SEEK_POSITION_HI   ] = dummybuf[V3_REG_POSITION_HI];
    dummybuf[V3_REG_SEEK_POSITION_LO   ] = dummybuf[V3_REG_POSITION_LO];
    dummybuf[V3_REG_SEEK_VELOCITY_HI   ] = 0x00;
    dummybuf[V3_REG_SEEK_VELOCITY_LO   ] = 0x00;

    set_pos = 900;
    return 1;
}

int adapter_deinit(void)
{
    return 1;
}

int adapter_write(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len)
{
    if (addr == V3_REG_SEEK_POSITION_HI)
    {
        // store the set position
        set_pos = dummybuf[V3_REG_POSITION_HI] << 8 | dummybuf[V3_REG_POSITION_LO];
        clock_gettime(CLOCK_MONOTONIC, &start);
    }
    memcpy(&dummybuf[addr], data, len);
    printf("dummy: wrote %s\n", data);
}

int adapter_read(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len)
{
    if (addr == V3_REG_POSITION_HI)
    {
        int seek_pos =  dummybuf[V3_REG_SEEK_POSITION_HI] << 8 | dummybuf[V3_REG_SEEK_POSITION_LO];
        int cur_pos = dummybuf[V3_REG_POSITION_HI] << 8 | dummybuf[V3_REG_POSITION_LO];

        if (seek_pos != cur_pos)
        {
            //return the adjusted position
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed = timespecDiff(&end, &start);

            printf("elapsed: %d\n", (int)(elapsed/1000000));
            //2 seconds full sweep
            int cpos;
            int pos;
            if (seek_pos < cur_pos)
            {
                cpos = -cur_pos;
                pos = set_pos - (1024.0 / 2000.0) * (int)(elapsed/1000000);
            }
            else
            {
                cpos = cur_pos;
                pos = set_pos + (1024.0 / 2000.0) * (int)(elapsed/1000000);
            }
            
            printf("pos: %d\n", pos);
            if (pos > seek_pos && cpos >= 0)
            {
                pos = seek_pos;
            }
            else if (pos < seek_pos && cpos < 0)
                pos = seek_pos;
            dummybuf[V3_REG_POSITION_HI] = data[0] = pos >> 8;
            dummybuf[V3_REG_POSITION_LO] = data[1] = pos & 0xFF;
            return 1;
        }
    }
    printf("dummy: read %d len\n", (int)len);
    memcpy(data, &dummybuf[addr], len);
    return 1;
}

int adapter_readonly(int adapter, int servo, unsigned char *data, size_t len)
{
    printf("dummy: read %d len\n", (int)len);
    memset(data, 0, len);
    return 1;
}

int adapter_reflash(int adapter, int servo, int bootloader_addr, char *filename)
{
    printf("flashing\n");
    return 1;
}

int adapter_command(int adapter, int servo, unsigned char command)
{

    printf("dummy: command\n");
    return 1;
}

int adapter_scan(int adapter, int devices[], int *dev_count)
{
    printf("dummy: scan. returning 1 device\n");
    *dev_count = 1;
    devices[0] = 0x10;
    return 1;
}

int adapter_probe(int adapter, int servo)
{
    if(servo == 0x10)
        return 1;
    return 0;
}

int adapter_get_adapter_name(int adapter, char *name)
{
    sprintf(name, "Dummy Driver");
    return 1;
}

int adapter_get_adapter_count(void)
{
    return 1;
}

int adapter_set_bitrate(int adapter, int bitrate)
{
    printf("dummy: set bitrate %d\n", bitrate);
    return 1;
}

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
    return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
            ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}
