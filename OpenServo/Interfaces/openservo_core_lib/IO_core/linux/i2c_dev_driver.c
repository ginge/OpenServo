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


int i2c_dev_enumerate(int *dev_count)
{
    DIR *dp;
    struct dirent *ep;

    *dev_count = 0;

    dp = opendir ("/dev/");
    if (dp != NULL)
    {
        while (ep = readdir (dp))
        {
            if ((ep->d_name[0] == 'i') && (ep->d_name[1] == '2') && (ep->d_name[2] == 'c') && (ep->d_name[3] == '-'))
            {
                printf("i2c-%d count %d\n", (int)ep->d_name[4]-48, *dev_count);
                dev_adapters[*dev_count] = (int)ep->d_name[4]-48;

                *dev_count+=1;
            }
        }
        (void) closedir (dp);
    }
    else
        perror ("Couldn't open the directory");

    return ERR_SUCCESS;
}


int i2c_dev_read(int adapter, int i2c_addr, unsigned char addr, unsigned char *data, size_t len)
{
    int i;

    char tmpbuf[255];
    sprintf( tmpbuf, "----- i2c_read i2c_dev_addr:0x%02x -----\n", i2c_addr );
    log_print(tmpbuf, ERR_INFO);


#if !defined DEBUG_NO_I2C
    if (read(file[adapter], data, len) != len)
    {
        printf("----- i2c_read %x Failed -----\n",i2c_addr);
        log_print(tmpbuf, ERR_INFO);

        return ERR_READ;
    }
#endif
#if defined DEBUG_NO_I2C
    static unsigned char nbuf[2];
    //return some fake data.
    nbuf[0]=1+(unsigned char) (255.0*rand()/(255+1.0));
    nbuf[1]=1+(unsigned char) (255.0*rand()/(255+1.0));
    data[0] = nbuf[0]; data[1]=nbuf[1];
#endif

    sprintf(tmpbuf,"----- i2c_read: read Addr: %x Len %d Data:\n ",i2c_addr, len);
    log_print(tmpbuf, ERR_INFO);

    for(i = 0; i < len; i++)
    {
        sprintf(tmpbuf, "0x%02x ", data[i]);
    }
    sprintf(tmpbuf,"%s -----\n", tmpbuf);
    log_print(tmpbuf, ERR_INFO);
    return ERR_SUCCESS;
}

int i2c_dev_write(int adapter, int i2c_addr, unsigned char addr, unsigned char *data, size_t len)
{
    char tmpbuf[255];
    char tmpdata[255];

    sprintf(tmpbuf,"----- i2c_write i2c_dev_addr:0x%02x -----\n", i2c_addr);
    log_print(tmpbuf, ERR_INFO);

    tmpdata[0]=addr;
    memcpy ( &tmpdata[1], data, len + 1);

#if !defined DEBUG_NO_I2C
    if ( write(file[adapter], tmpdata, len) != len)
    {
        sprintf(tmpbuf, "----- i2c_write: I2C Send %x Failed -----\n", i2c_addr);
        log_print(tmpbuf, ERR_INFO);
        return ERR_WRITE;
    }
#endif
    int x;
    sprintf(tmpbuf, "----- i2c_write i2c_data %d:\n", i2c_addr);
    for (x = 0; x < len; x++)
    {
        sprintf(tmpbuf, "%s0x%02x ", tmpbuf, data[x]);
    }
    sprintf(tmpbuf, "%s----- \n",tmpbuf);
    log_print(tmpbuf, ERR_INFO);
    return ERR_SUCCESS;
}

int i2c_dev_set_slave(int file, unsigned char i2c_dev_addr)
{
    char tmpbuf[255];
    sprintf( tmpbuf, "----- i2c_dev_set_slave i2c_dev_addr:0x%02x -----\n", i2c_dev_addr );
    log_print(tmpbuf, ERR_INFO);

    // Indicate which slave we wish to speak to
#if !defined WIN32_DLL    // stop MSVC whinging
#if !defined DEBUG_NO_I2C
    if ( ioctl( file, I2C_SLAVE, i2c_dev_addr ) < 0 )
    {
        printf( "----- i2c_dev_set_slave: Error trying to set slave address to 0x%02x (%d %s) -----\n",
                i2c_dev_addr, errno, strerror( errno ));
        log_print(tmpbuf, ERR_INFO);
        return -1;
    }
#endif
#endif

    return 1;
}

/**
 *
 *   Scan the i2c bus for all i2c devices. Query the devices, and then put the devies
 *   into the array bus_devices along with the adapter.
 *
 */
int i2c_dev_scanbus(int adapter, int devices[], int *dev_count)
{
    int i,s;

    *dev_count = 0;

    //search all addresses for a response
    for (i=0x01; i<=MAX_I2C_DEVICES; i++)
    {
        int res;

        char read[255];
        read[0]=0x00;

        //sequentially set the slave address to read/write
        res = i2c_dev_set_slave(file[adapter], (unsigned char)i);

        printf ( "setslave %d\n", res );
        //write one byte
        if ( res >0 )
        {
            res = i2c_dev_write(adapter, (unsigned char)i, 0x00, (unsigned char*)read, 1);
        }
        if ( res > 0 )
        {
            printf ( "----- i2c_scanbus: found i2c-chip at: 0x%02x (%d) (real-address: 0x%02x) -----\n",
                            i, i, i*2 );
            //add instance to bus array
            devices[*dev_count]=i;
            *dev_count += 1;
        }
    }
    return ERR_SUCCESS;
}

int i2c_dev_probe(int adapter, int device)
{
    int res;
    char read[255];
    read[0]=0x00;

    //sequentially set the slave address to read/write
    res = i2c_dev_set_slave(file[adapter], (unsigned char)device);

    //write one byte
    if ( res >0 )
    {
        res = i2c_dev_write(adapter, (unsigned char)device, 0x00, (unsigned char*)read, 1);
    }
    if ( res > 0 )
    {
        printf ( "----- i2c_scanbus: found i2c-chip at: 0x%02x (%d) (real-address: 0x%02x) -----\n",
                 device, device, device*2 );
    }
}

log_print(char *log, int err)
{
    printf("%s\n", log);
}
