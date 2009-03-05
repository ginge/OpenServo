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
#include "registers_v2.h"
#include "registers_v3.h"
#include "registers_features.h"

//openservo servo's
typedef struct servo_dev_t
{
    int adapter_num;

    // Configuration Variables
     int address;
    int current_bank;
    int read_direct;

    int registers[255];
    int registers_timestamp[255];
    int registers_flagged[255];
    int banks[8][255];
    int banks_timestamp[8][255];
    int banks_flagged[8][255];
    double version;            // The hardwares firmware version

    // this lib's config data
    int flags_registered[255];
    int flags_registered_len;

    struct servo_dev_t *next;
} servo_dev;

servo_dev *servos;

int servo_count;
int bus_devices[255];

servo_dev *servo_add ( servo_dev ** p, int i );
servo_dev **servo_search (int adapter, servo_dev ** n, int i);
void servo_remove ( servo_dev ** p );
servo_dev *get_servo(int adapter, int servo_no);
