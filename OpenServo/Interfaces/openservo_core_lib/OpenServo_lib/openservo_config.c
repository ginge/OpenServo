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
#include "openservo_config.h"
#include "register_feature_table.h"
#include "openservo_global.h"
#include "../IO_core/IO_error.h"

 
int config_init(void)
{
    //
    //alloc mem for config
    openservo_config = (config_dev*) malloc(sizeof(config_dev));
    openservo_config->global_read_direct = ADAPTER_DIRECT;

    openservo_config->tick = 0;

    return ERR_SUCCESS;
}

int config_deinit(void)
{
    //
    free(openservo_config);
    return ERR_SUCCESS;
}

/**
 *
 *   Configuration options
 *   Set the read type. This detmines whether the data from the servo is read directly,#
 *   or returned from the struture values. See Documemtation regarding this, as you need to
 *   a periodical servo read to keep the servo values fresh. Valid values are
 *     SERVO_BUFFERED (0) disable direct comms with servo (must specify valid servo number)
 *     SERVO_DIRECT (1) enable direct comms to the servo
 *     ADAPTER_BUFFERED (2) disable all direct comms with all servos
 *     ADAPTER_DIRECT (3) enable direct comms to all servos   (default)
 *
 */
int openservo_config_direct_read(int adapter_no, int servo_no, int mode)
{
    servo_dev *sel_servo;

    sel_servo = get_servo(adapter_no, servo_no);
    switch(mode)
    {
        case SERVO_BUFFERED:
        case SERVO_DIRECT:
            sel_servo->read_direct = mode;
            break;
        case ADAPTER_BUFFERED:
        case ADAPTER_DIRECT:
            openservo_config->global_read_direct=mode;
            break;
        default:
            sel_servo->read_direct = ADAPTER_DIRECT;
            openservo_config->global_read_direct = ADAPTER_DIRECT;
    }

    return ERR_SUCCESS;
}


int config_check_direct(servo_dev *sel_servo)
{
    // if the servo is configured to read directly
    //see if the global direct flag overrides
    if (openservo_config->global_read_direct == ADAPTER_BUFFERED)
    {
        //not configured global, check servo level
        if (sel_servo->read_direct == SERVO_DIRECT)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else if (openservo_config->global_read_direct == ADAPTER_DIRECT)
    {
        //return structure values
        return 1;
    }
    return 1;
}
