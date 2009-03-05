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
#include "adapter_enumerate.h"
typedef struct adapter_dev_t
{
    int adapter_type;       // type of adapter
    char adapter_internal_name[255];
    int adapter_nr;         // integer number for adapter.
    int adapter_is_init;
    int adapter_count_start; //where in the list this adapter is
    int adapter_count;      // Number of adapters supported by this driver

    /*A pointer to a function*/
    adapter_deinitfunc adapter_deinit;
    adapter_initfunc adapter_init;
    adapter_writefunc adapter_write;
    adapter_readfunc adapter_read;
    adapter_readonlyfunc adapter_readonly;
    adapter_reflashfunc adapter_reflash;
    adapter_scanfunc adapter_scan;
    adapter_probefunc adapter_probe;
    adapter_commandfunc adapter_command;
    adapter_get_adapter_namefunc adapter_get_adapter_name;
    adapter_get_adapter_countfunc adapter_get_adapter_count;
    adapter_set_bitratefunc adapter_set_bitrate;

    struct adapter_dev_t *next;
} adapter_dev;

// Global pointer to the head of the linked list
adapter_dev *adapter_device;

void adapter_deinit(int adapter_no);
int adapter_count;

int adapter_count_adapters(adapter_dev ** n);
adapter_dev *adapter_add(adapter_dev ** p);
void adapter_remove ( adapter_dev ** p );
adapter_dev **adapter_search (adapter_dev ** n, int i);

