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
#include "adapter_enumerate.h"
#include "adapter_parseconfig.h"


/**   Enumerate all adapters */
int adapter_enumerate(void)
{
    /* Load all adapters from the config file, and initialise */
    return adapter_load_plugins();
}

int adapter_load_plugins(void)
{
    int ret;
    char libnamestr[30];
    int done = 0;
    FILE *file_handle;
    char adapter_name[255];
    char adapter_internal_name[255];
    char adapter_lib[255];

    /* This is our config file name */
    sprintf(libnamestr, "adapters.ini");

    /* Open the config file and get the file handle */
    if ((file_handle = config_open(libnamestr)) == NULL)
        return ERR_FILE;

    /* Loop through the list of adapters looking for loadable adapters */
    while(done == 0)
    {
        /* Parse the config line by line. returns 0 when finished */
        ret = config_parse(file_handle, adapter_name, adapter_lib);
        if (ret == 0)
        {
            done = 1;
            break;
        }
        /* field error! continue on */
        else if (ret < 0)
        {
            continue;
        }

        /* Load adapters */
        adapter_dev *adapter;
        adapter = adapter_add(&adapter_device);
        /* Try and load the library parsed */
        if (adapter_library_open(adapter, adapter_lib) < 0)
        {
            printf("IO_core: failure finding adapters for adapter %s\n", libnamestr);
            /* We must continue on */
            adapter_remove(&adapter);
            continue;
        }
        adapter->adapter_count = adapter->adapter_get_adapter_count();
        adapter_count += adapter->adapter_count-1;
        adapter->adapter_get_adapter_name(adapter->adapter_count, adapter_internal_name);
        sprintf(adapter->adapter_internal_name, "%s", adapter_internal_name);
        printf("found %d adapter(s) on %s\n", adapter->adapter_count, adapter->adapter_internal_name);
    }
    config_close(file_handle);

    // TODO: Query adapters

    return ERR_SUCCESS;
}

int adapter_library_open(adapter_dev *adapter, char *libname, char *options)
{
#ifdef WIN32
    hdll = LoadLibraryW(libname);

    if (!hdll) { printf("Error loading dll\n"); return ERR_LIB_FAIL}

    /*GetProcAddress*/
    adapter->adapter_init              = (adapter_initfunc)GetProcAddress(hdll, "adapter_init");
    adapter->adapter_deinit            = (adapter_deinitfunc)GetProcAddress(hdll, "adapter_deinit");
    adapter->adapter_write             = (adapter_writefunc)GetProcAddress(hdll, "adapter_write");
    adapter->adapter_read              = (adapter_readfunc)GetProcAddress(hdll, "adapter_read");
    adapter->adapter_readonly          = (adapter_readonlyfunc)GetProcAddress(hdll, "adapter_readonly");
    adapter->adapter_reflash           = (adapter_reflashfunc)GetProcAddress(hdll, "adapter_reflash");
    adapter->adapter_scan              = (adapter_scanfunc)GetProcAddress(hdll, "adapter_scan");
    adapter->adapter_probe             = (adapter_probefunc)GetProcAddress(hdll, "adapter_probe");
    adapter->adapter_command           = (adapter_commandfunc)GetProcAddress(hdll, "adapter_command");
    adapter->adapter_get_adapter_name  = (adapter_get_adapter_namefunc)GetProcAddress(hdll, "adapter_get_name");
    adapter->adapter_get_adapter_count = (adapter_get_adapter_countfunc)GetProcAddress(hdll, "adapter_get_count");
    adapter->adapter_set_bitrate       = (adapter_set_bitratefunc)GetProcAddress(hdll, "adapter_set_bitrate");

#else

    libhandle = dlopen ( libname, RTLD_LAZY ); // open the shared lib

    // if the open failed, NULL was returned.  Print the error code
    if ( libhandle == NULL )
    {
        char logmsg[255];
        sprintf(logmsg, "fail 1: %s\n", dlerror());
        log_print(logmsg, ERR_LIB_FAIL);
        return ERR_LIB_FAIL;
    }

    /*GetProcAddress*/
    adapter->adapter_init              = (adapter_initfunc)dlsym(libhandle, "adapter_init");
    adapter->adapter_deinit            = (adapter_deinitfunc)dlsym(libhandle, "adapter_deinit");
    adapter->adapter_write             = (adapter_writefunc)dlsym(libhandle, "adapter_write");
    adapter->adapter_read              = (adapter_readfunc)dlsym(libhandle, "adapter_read");
    adapter->adapter_reflash           = (adapter_reflashfunc)dlsym(libhandle, "adapter_reflash");
    adapter->adapter_scan              = (adapter_scanfunc)dlsym(libhandle, "adapter_scan");
    adapter->adapter_probe             = (adapter_probefunc)dlsym(libhandle, "adapter_probe");
    adapter->adapter_command           = (adapter_commandfunc)dlsym(libhandle, "adapter_command");
    adapter->adapter_get_adapter_name  = (adapter_get_adapter_namefunc)dlsym(libhandle, "adapter_get_adapter_name");
    adapter->adapter_get_adapter_count = (adapter_get_adapter_countfunc)dlsym(libhandle, "adapter_get_adapter_count");
    adapter->adapter_set_bitrate       = (adapter_set_bitratefunc)dlsym(libhandle, "adapter_set_bitrate");

    // if adapter_init is NULL, init() wasn't found in the lib, print error message
    if ( adapter->adapter_init == NULL )
    {
        char buf[255];
        sprintf(buf,"fail 2: %s", dlerror() );
        log_print( buf, ERR_LIB_FAIL );
        return ERR_LIB_FAIL;
    }

#endif
    if (adapter->adapter_init("") < 0)
    {
        log_print("Error initialising adapter", ERR_LIB_FAIL);
        adapter->adapter_is_init = false;
        return ERR_LIB_FAIL; 
    }
    else
    {
        log_print("adapter initiased!", ERR_SUCCESS);
        adapter->adapter_is_init = true;
    }
    return ERR_SUCCESS;
}

