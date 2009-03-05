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
#include "i2c_dev_driver.h"

int adapter_init(char *options);
int adapter_deinit(void);
int adapter_write(int adapter, int i2c_addr, unsigned char addr, unsigned char *data, size_t len);
int adapter_read(int adapter, int i2c_addr, unsigned char addr, unsigned char *data, size_t len);
int adapter_readonly(int adapter, int i2c_addr, unsigned char *data, size_t len);
int adapter_reflash(int adapter, int i2c_addr, int bootloader_addr, char *filename);
int adapter_command(int adapter, int i2c_addr, unsigned char command);
int adapter_scan(int adapter, int devices[], int *dev_count);
int adapter_probe(int adapter, int i2c_addr);
int adapter_get_adapter_name(int adapter, char *name);
int adapter_get_adapter_count(void);
int adapter_set_bitrate(int adapter, int bitrate);



int adapter_init(char *options)
{
    char tmpbuf[255];
    char filename[1024];
    int n;

    i2c_dev_count = 0;

    i2c_dev_enumerate(&i2c_dev_count);

    if (i2c_dev_count == 0)
        return ERR_SUCCESS;

    for (n = 0; n < i2c_dev_count; n++)
    {
        sprintf(filename, "/dev/i2c-%d", dev_adapters[n]);
        sprintf( tmpbuf, "----- i2c_open: i2c-device open. %s-----",filename);
        log_print(tmpbuf, ERR_INFO);

FILE *filea;
#if !defined DEBUG_NO_I2C
        if ((file[n] = open(filename,O_RDWR)) < 0)
        {
            printf("----- i2c_open: ERRNO: %s -----",strerror(errno));
            return -1;
        } else {
            sprintf(tmpbuf, "----- i2c_open: Opened %s -----",filename);
            log_print(tmpbuf, ERR_INFO);
        }
#endif
    }
    return 1;
}

int adapter_deinit(void)
{
    char tmpbuf[255];
    int n;

    for (n = 0; n < i2c_dev_count; n++)
    {
        sprintf( tmpbuf, "----- i2c_close: file: %d -----", file );
        log_print(tmpbuf, ERR_INFO);

        if(file[n] >= 0)
        {
    #if !defined WIN32_DLL    // stop MSVC whinging
            close(file[n]);
    #endif
            sprintf( tmpbuf, "----- i2c_close: Closed /dev/i2c-%d ------",0);
            log_print(tmpbuf, ERR_INFO);

            return ERR_SUCCESS;
        }

        sprintf( tmpbuf, "----- i2c_close: Closed file %d ------",file);
        log_print(tmpbuf, ERR_INFO);
    }

    return ERR_FAIL_DRIVER;
}

int adapter_write(int adapter, int i2c_addr, unsigned char addr, unsigned char *data, size_t len)
{
    unsigned char tmpdata[1024];

    if (i2c_dev_set_slave(file[adapter], i2c_addr) < 0)
        return ERR_WRITE;

    //write the command address
    if (i2c_dev_write(adapter, i2c_addr, addr, data, len + 1) < 0)
        return ERR_WRITE;

    return ERR_SUCCESS;
}

int adapter_read(int adapter, int i2c_addr, unsigned char addr, unsigned char *data, size_t len)
{
    if (i2c_dev_set_slave(file[adapter], i2c_addr) < 0)
        return ERR_WRITE;

    i2c_dev_write(adapter, i2c_addr, &addr, data, 1);
    i2c_dev_read(adapter, i2c_addr, addr, data, len);
}

int adapter_readonly(int adapter, int i2c_addr, unsigned char *data, size_t len)
{
    i2c_dev_read(adapter, i2c_addr, data, len);
}

int adapter_reflash(int adapter, int i2c_addr, int bootloader_addr, char *filename)
{
    printf("flashing\n");
    return 1;
}

int adapter_command(int adapter, int i2c_addr, unsigned char command)
{
    printf("linux-dev: command not implemented yet\n");
    return 1;
}

int adapter_scan(int adapter, int devices[], int *dev_count)
{
    i2c_dev_scanbus(adapter, devices, dev_count);
    return 1;
}

int adapter_probe(int adapter, int i2c_addr)
{
    return i2c_dev_probe(adapter, i2c_addr);
}

int adapter_get_adapter_name(int adapter, char *name)
{
    sprintf(name, "Linux /dev/i2c-x driver");
    return 1;
}

int adapter_get_adapter_count(void)
{
    return i2c_dev_count;
}

int adapter_set_bitrate(int adapter, int bitrate)
{
    printf("dummy: set bitrate %d\n", bitrate);
    return 1;
}

