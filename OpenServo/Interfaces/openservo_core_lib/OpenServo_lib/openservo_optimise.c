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

int optimise_init(void)
{

}

int optimise_deinit(void)
{

}

int optimise_read(servo_dev *sel_servo)
{
    int read_count = 0;
    int min_flag[255];
    int max_flag[255];
    int got_beginning = 0;
    int n, i;
    int total_bytes = 0;
    
    memset(min_flag, 0, 255);
    memset(max_flag, 0, 255);
    

    printf("Optimised Read\n");
    // for the selected servo, calculate the shortest path to getting the servo data
    for (n = 0; n < 255; n++)
    {
        // Check to see if there are registed flags and determine the minimum register
        if ((sel_servo->registers_flagged[n] > 0) && (got_beginning == 0))
        {
            min_flag[read_count] = n;
            got_beginning = 1;
        }
        // Have We found the end of a block?
        else if ((sel_servo->registers_flagged[n] == 0) && (got_beginning == 1))
        {
            max_flag[read_count] = n - 1;
            got_beginning = 0;
            read_count++;
            total_bytes += (max_flag[read_count] - min_flag[read_count]);
        }
    }
    printf("Read count %d, total bytes %d\n", read_count, total_bytes);
    // Now do the same for the banks
    int bank_min_flag[MAX_BANKS][255];
    int bank_max_flag[MAX_BANKS][255];
    int bank_read_count[MAX_BANKS];
    bank_read_count[0] = 0;
    bank_read_count[1] = 0;
    bank_read_count[2] = 0;
    memset(bank_min_flag, 0, 255*MAX_BANKS);
    memset(bank_max_flag, 0, 255*MAX_BANKS);
    for (i = 0; i < MAX_BANKS; i++)
    {
        for (n = 0; n < 255; n++)
        {
            // Check to see if there are registed flags and determine the minimum register
            if ((sel_servo->banks_flagged[i][n - V3_MIN_BANK_REGISTER] > 0) && (got_beginning == 0))
            {
                bank_min_flag[i][bank_read_count[i]] = n;
                got_beginning = 1;
            }
            // Have We found the end of a block?
            else if ((sel_servo->banks_flagged[i][n  - V3_MIN_BANK_REGISTER] == 0) && (got_beginning == 1))
            {
                bank_max_flag[i][bank_read_count[i]] = n - 1;
                got_beginning = 0;
                bank_read_count[i]++;
                total_bytes += (bank_max_flag[i][bank_read_count[i]] - bank_min_flag[i][bank_read_count[i]]);
            }
        }
    }

    printf("bank Read count %d. total bytes %d\n", bank_read_count, total_bytes);
    // Calculate the difference in space between the registers and find optimum
    // The amount bytes of overhead that a new transaction generates
    // This is assuumed to be this value by experimentation. This value is used
    // to determine if it is worth starting a new transaction to read a block, or
    // to merge the transactions together.
    int transaction_overhead = 4;

    int read_min[255];
    int read_max[255];
    int data_read_count = 0;
    int block_bytes;
    memset(read_min, 0, 255);
    memset(read_max, 0, 255);
    // If there is more than one block
    if (read_count > 1)
    {
        // Cycle through the blocks and work out the difference from min to max-1
        for (n = 0; n < read_count-1; n++)
        {
            if ((min_flag[n + 1] - max_flag[n]) < transaction_overhead)
            {
                read_max[data_read_count] = max_flag[n + 1];
                read_min[data_read_count] = min_flag[0];
                data_read_count++;
            }
        }
    }

    int bank_read_min[MAX_BANKS][255];
    int bank_read_max[MAX_BANKS][255];
    int bank_data_read_count[MAX_BANKS];
    // If there is more than one block

    // Cycle through the blocks and work out the difference from min to max-1
    for (i = 0; i < MAX_BANKS; i++)
    {
        // Set the first element to -1 for later detection
        bank_read_min[i][0] = -1;
        for (n = 0; n < bank_read_count[i] - 1; n++)
        {
            if ((bank_min_flag[n + 1] - bank_max_flag[n]) < transaction_overhead)
            {
                bank_read_max[i][bank_data_read_count[i]] = bank_max_flag[i][n + 1];
                bank_read_min[i][bank_data_read_count[i]] = bank_min_flag[i][0];
                bank_data_read_count[i]++;
            }
        }
    }

/*    else
    {
        bank_read_max[0][data_read_count] = bank_max_flag[i][n + 1];
        bank_read_min[i][data_read_count] = bank_min_flag[i][0];
        bank_data_read_count[i]++;
    }*/
    // Do our reads

    // Start by reading the register ranges
    for (n = 0; n < data_read_count; n++)
    { 
        unsigned char buf[1024];
        iocore_read(sel_servo->adapter_num, sel_servo->address, read_min[n], buf, read_max[n] - read_min[n]);
        memcpy(&sel_servo->registers[read_min[n]], buf, read_max[n] - read_min[n]);
    }

    // Now update our banks
    for (i = 0; i < MAX_BANKS; i++)
    {
        // This bank has data
        if (bank_read_min[i][0] != -1)
        {
            for (n = 0; n < bank_data_read_count[n]; n++)
            {
//                    read(...);
  //                  memcpy(...);
            }
        }
    }
    printf("Merged %d blocks, from %d blocks, or %d bytes from individual reads\n", (data_read_count + bank_data_read_count) - (read_count + bank_read_count) , read_count + bank_read_count);
    return ERR_SUCCESS;
}
