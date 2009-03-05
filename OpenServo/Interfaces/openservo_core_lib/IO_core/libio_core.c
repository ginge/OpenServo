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
#include "IO_error.h"
#include "adapter.h"
#include "libio_core.h"
#include <stdio.h>
#include <stdlib.h>

void test(void)
{
    printf("hello IO test!\n");
}

// init
int iocore_init(void)
{
    adapter_dev_init();

    //initialise variables
    ADAPTER_LOCKED_MUTEX = 0;

    if (adapter_enumerate() < 0)
        return ERR_I2C_ADAPTER_ENUMERATE_FAIL;

    log_print("Init: Enumerate adapters", ERR_INFO);

    return ERR_SUCCESS;
}

int iocore_deinit(void)
{
    adapter_dev_deinit();
    return ERR_SUCCESS;
}

// read
int iocore_read(int adapter_num, unsigned char dev_addr, unsigned char reg_addr, unsigned char *buf, int read_len)
{ 
    int res;
    adapter_dev *adapter;

    adapter = *adapter_search(&adapter_device, adapter_num);

    /* Translate between our adapter numbering scheme, and the internal adapter num handled by a driver
       This is because a driver can handle more than one adapter
    */
    adapter_num = adapter_num - adapter->adapter_count_start;
    //locking
    adapter_mutex_lock();
    res = adapter->adapter_read(adapter_num, dev_addr, reg_addr, buf, read_len);
    adapter_mutex_unlock();

    return res;
}

// write
int iocore_write(int adapter_num, unsigned char dev_addr, unsigned char reg_addr, unsigned char *buf, int read_len)
{
    int res;
    adapter_dev *adapter;

    adapter = *adapter_search(&adapter_device, adapter_num);

    /* Translate between our adapter numbering scheme, and the internal adapter num handled by a driver
    This is because a driver can handle more than one adapter
    */
    adapter_num = adapter_num - adapter->adapter_count_start;
    //locking
    adapter_mutex_lock();
    res = adapter->adapter_write(adapter_num, dev_addr, reg_addr, buf, read_len);
    adapter_mutex_unlock();

    return res;
}

// getfunc
// scan
int iocore_scan(int adapter_num, int devices[], int *dev_count)
{
    int res;
    adapter_dev *adapter;

    adapter = *adapter_search(&adapter_device, adapter_num);

    /* Translate between our adapter numbering scheme, and the internal adapter num handled by a driver
    This is because a driver can handle more than one adapter
    */
    adapter_num = adapter_num - adapter->adapter_count_start;
    adapter_mutex_lock();
    res = adapter->adapter_scan(adapter_num, devices, dev_count);
    adapter_mutex_unlock();

    return res;
}

int iocore_get_adapter_count(int *adapters)
{
    *adapters = adapter_count;
}

int iocore_get_adapter_name(int adapter_num, unsigned char *name)
{
    adapter_dev *adapter;

    adapter = *adapter_search(&adapter_device, adapter_num);

    sprintf(name, "%s", adapter->adapter_internal_name);

    return ERR_SUCCESS;
}

void log_print(char *logstr, int loglevel)
{
    printf(logstr);
    printf("\n");
}


//lock instance
void adapter_mutex_lock(void)
{
    int lockchk=0;

    //If there is already a lock on a thread, wait
    // this allows a failsafe of 10000*10 microseconds before a bailout.
    //should give a fair idea of any deadlocks
    while (ADAPTER_LOCKED_MUTEX)
    {

#ifndef WIN32_DLL
//    usleep(100);
#else
        Sleep(10);
#endif

        if (lockchk >10000)
        {
            exit (-1);  //if we got here then the thread never unlocked. Some deadlock or race condition.
        }
        lockchk++;
    }
    // If we have exclusive use, lock
    ADAPTER_LOCKED_MUTEX = 1;
}

//unlock instance
void adapter_mutex_unlock(void)
{
    // reset the mutex
    ADAPTER_LOCKED_MUTEX=0;
}
