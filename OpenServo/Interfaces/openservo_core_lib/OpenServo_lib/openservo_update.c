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
#include <stdlib.h>
#include "../IO_core/libio_core.h"
#include "../IO_core/IO_error.h"
#include "openservo_servo.h"
#include "openservo_io.h"

/**
 *
 *   Complete world update. Fills the strutures based on how many
 *   elements are in the map, and if the data has changed.
 *   This will read all data in the mapping, and will only write data is it
 *   is a RW register. (disallows write on read only).
 *   Basic optimisatisation to stop the lib resending the data if not changed
 *
 */
int update_world(void)
{
    int s;
    servo_dev *sel_servo;
    int adapter_count;

    sel_servo = servos;

    iocore_get_adapter_count(&adapter_count);
    //for each bus, and each servo, read and write
    while(sel_servo != NULL)
    {
        //point data to struct. It is important to do this after the write_diff
        // as any read operations will trounce the data set by user
        optimise_read(sel_servo);
        //increment pointer to next servo
        sel_servo=sel_servo->next;
    }
    servo_mutex_unlock();
    return 1;
}

// 
// /**
//  *
//  * Write only blocks that differ fromt he backup struct, and only
//  * that are writeable.
//  *
//  * Reworked due to i2c issues
//  *
//  **/
// int write_diff(servo *sel_servo) {
// 
//   //write to the buffer if the value has changed
//   //for each command in the map, compare to our backup struct
//     int i;
//     unsigned char ndata[2];
//     for (i = 0; i< sel_servo->sv_map->map_len; i++) {
//         char buf[255];
//         sprintf( buf, "data %d bckdata %d\n", *sel_servo->sv_map->map_data[i], *sel_servo->os_comm->servo_back->sv_map->map_data[i]);
//         log_message(buf,ERR_INFO);
//     //if the servo data has changed from the backup write it
// //    if ( (*sel_servo->sv_map->map_data[i] != *sel_servo->os_comm->servo_back->sv_map->map_data[i]) && sel_servo->sv_map->map_data_dir[i] != DIR_R ) { //check we can write that register
// 
//         if ( (*sel_servo->sv_map->map_data[i] != *sel_servo->os_comm->servo_back->sv_map->map_data[i]) && sel_servo->sv_map->map_data_dir[i] != DIR_R ) {
// 
//         //write the data from backup map
//             int_to_hexarr(*sel_servo->os_comm->servo_back->sv_map->map_data[i], ndata);
// 
//         //write the data from the backup command structure
//             i2c_block_write(sel_servo->adapter->file, (unsigned char)sel_servo->address, (unsigned char)sel_servo->os_comm->servo_back->sv_map->map_cmd[i], ndata, sel_servo->sv_map->map_cmd_len[i], sel_servo->adapter->adapter_type, sel_servo->adapter->dimax_handle);
// 
//         }
//     }
//   
//     return 1;
// }
// 
// int copy_to_backup(servo *sel_servo) {
//   // store the pointer to backup sv_map
//     nmapping *sv_mapbck;
// 
//     int maplen;
// 
//     sv_mapbck = sel_servo->os_comm->servo_back->sv_map;
// 
//     maplen = sel_servo->sv_map->map_len;
//   //copy the data struct over the backup
//     memcpy(sel_servo->os_comm->servo_back, sel_servo, sizeof( servo));
//   //reconfigure sv_map to point to the back buffer.
// 
// 
//     sel_servo->os_comm->servo_back->sv_map = sv_mapbck;
// 
//   //reconfigure the map pointers to point to the backup array
//   //sel_servo->sv_map->map_len = 5;
//     servo_config_map((sel_servo->os_comm->servo_back), sel_servo->sv_map->map_cmd, maplen);
// //   }
//     return 1;
// }
