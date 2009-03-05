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

#include "adapter.h"
#include "IO_error.h"
#include "stdio.h"
#include "stdlib.h"

void adapter_dev_init(void)
{
    adapter_count = 0;
    adapter_device = NULL;
}

void adapter_dev_deinit(void)
{
    int n;

    // deallocate all adapters
    for(n = 0;n < adapter_count; n++);
    {
        adapter_deinit(n);
    }
}

void adapter_deinit(int adapter_no)
{
    adapter_dev *adapter;

    adapter = *adapter_search(&adapter_device, adapter_no);

    //close i2c handle
    if (adapter->adapter_deinit() < 0)
    {
        char buf[1024];
        sprintf(buf, "Error De-initialising I2C bus %d", adapter_no);
        log_print(buf, ERR_ADAPTER_DEALLOC_FAIL);
    }
    adapter_remove(&adapter);

}

/**
 *
 *  Add a new i2c adapter. Allows multiple i2c ports to be used
 *
 */
adapter_dev *adapter_add(adapter_dev ** p)
{
    char logbuf[255];

    adapter_dev *n;
    n = (adapter_dev*) malloc (sizeof(adapter_dev));

    n->next = *p;
    *p = n;
    n->adapter_count_start = adapter_count;
    adapter_count++;

    sprintf (logbuf, "added adapter %d\n", adapter_count);
    log_print(logbuf, ERR_INFO);
    return n;
}

/**
 *
 *  Delete the adapter
 *
 */
void adapter_remove ( adapter_dev ** p )
{
    /* remove head */

    adapter_dev *n;

    n = *p;
    *p = (*p)->next;
    free (n);

    adapter_count-=n->adapter_count;
}

/**
 *
 *  Find adapter with integer value i
 *  adapters are incremented starting from adapter 0
 *
 */
adapter_dev **adapter_search (adapter_dev ** n, int i)
{
    if (!(*n)) return NULL;

    while (*n != NULL)
    {
        /* Stop searching when we find the adapter number, and how many device supports*/
        if (((*n)->adapter_count >= i) && (i >= (*n)->adapter_count_start))
        {
            return n;
        }
        n = &(*n)->next;
    }
    return NULL;
}

int adapter_count_adapters(adapter_dev ** n)
{
    int count=0;
    if (!(*n)) return 0;
    while (*n != NULL)
    {
        count++;
        n = & (*n)->next;
    }
    return count;
}

