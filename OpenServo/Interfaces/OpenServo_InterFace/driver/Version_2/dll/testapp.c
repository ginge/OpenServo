/* Open Source InterFace (OSIF) Test application V1.1
  Test routines for the shared library libOSIFlib.so.1.x.x
    Copyright (C) 2007  Barry Carter <barry.carter@robotfuzz.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

http://www.gnu.org/licenses/gpl.txt

*/

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
void * libhandle; // handle to the shared lib when opened
#endif
#include <stdio.h>
#include <stdbool.h>

int main ( int argc, char *argv[] ) {

    if (argc < 2)
    {
        printf( "usage: %s [command] [servo] [address] [length]\n", argv[0] );
        printf( "  commands: flash   [servo] [filename]- flash the servo\n");
        printf( "            read    [servo] [address] [length]\n");
        printf( "            write   [servo] [address] [data] [data]... [length]\n");
        printf( "            probe   [servo] - probe to see if servo exists\n");
        printf( "            command [servo] [command]\n");
        printf( "            scan - list all devices on the bus\n");
        printf( "  Values can be specified as int (16) or hex (0x10).\n");
        printf( "(c) Barry Carter 2007 www.openservo.com www.headfuzz.co.uk\n");
        return 1;
    }

    /* Typedef the OSIF functions */
    typedef int  (*OSIF_initfunc   )();
    typedef int  (*OSIF_deinitfunc )();
    typedef int  (*OSIF_writefunc  )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
    typedef int  (*OSIF_readfunc   )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
    typedef int  (*OSIF_reflashfunc)(int adapter, int servo, int bootloader_addr, char *filename);
    typedef int  (*OSIF_commandfunc)(int adapter, int servo, unsigned char command);
    typedef int  (*OSIF_scanfunc   )(int adapter, int devices[], int *dev_count);
    typedef bool (*OSIF_probefunc  )(int adapter, int servo);

    /* Pointers to the functions */
    OSIF_deinitfunc OSIF_deinit;
    OSIF_initfunc OSIF_init;
    OSIF_writefunc OSIF_write;
    OSIF_readfunc OSIF_read;
    OSIF_reflashfunc OSIF_reflash;
    OSIF_scanfunc OSIF_scan;
    OSIF_probefunc OSIF_probe;
    OSIF_commandfunc OSIF_command;

#ifdef WIN32
    /*Windows handle*/
    HANDLE hdll;

    /*LoadLibrary*/
    hdll = LoadLibrary("OSIFdll.dll");

    if (!hdll) { printf("Error loading dll\n"); exit(0); }

    /*GetProcAddress*/
    OSIF_init    = (OSIF_initfunc)GetProcAddress(hdll, "OSIF_init");
    OSIF_deinit  = (OSIF_deinitfunc)GetProcAddress(hdll, "OSIF_deinit");
    OSIF_write   = (OSIF_writefunc)GetProcAddress(hdll, "OSIF_write");
    OSIF_read    = (OSIF_readfunc)GetProcAddress(hdll, "OSIF_read");
    OSIF_reflash = (OSIF_reflashfunc)GetProcAddress(hdll, "OSIF_reflash");
    OSIF_scan    = (OSIF_scanfunc)GetProcAddress(hdll, "OSIF_scan");
    OSIF_probe   = (OSIF_probefunc)GetProcAddress(hdll, "OSIF_probe");
    OSIF_command = (OSIF_commandfunc)GetProcAddress(hdll, "OSIF_command");

#else
    libhandle = dlopen ( "libOSIFlib.so.1", RTLD_LAZY ); // open the shared lib

    // if the open failed, NULL was returned.  Print the error code
    if ( libhandle == NULL ) 
    {
        fprintf ( stderr, "fail 1: %s\n", dlerror() );
        return;
    } 

    /*GetProcAddress*/
    OSIF_init    = (OSIF_initfunc)dlsym(libhandle, "OSIF_init");
    OSIF_deinit  = (OSIF_deinitfunc)dlsym(libhandle, "OSIF_deinit");
    OSIF_write   = (OSIF_writefunc)dlsym(libhandle, "OSIF_write");
    OSIF_read    = (OSIF_readfunc)dlsym(libhandle, "OSIF_read");
    OSIF_reflash = (OSIF_reflashfunc)dlsym(libhandle, "OSIF_reflash");
    OSIF_scan    = (OSIF_scanfunc)dlsym(libhandle, "OSIF_scan");
    OSIF_probe   = (OSIF_probefunc)dlsym(libhandle, "OSIF_probe");
    OSIF_command = (OSIF_commandfunc)dlsym(libhandle, "OSIF_command");
#endif
    /*Call the function*/
    if (OSIF_init() <0 )
    {
        printf("Failed to open OSIF device. Check cables\n");
#ifdef WIN32
        exit(1);
#endif
    }

    int devices[128];
    int dev_count;
    bool failcmd = false;
    unsigned char buf[255];

    switch ( argv[1][0] )
    {
        case 'f':       //flash
            if (argc < 4) { failcmd = true; break; }
            if (!OSIF_reflash(0, parse_option(argv[2]), 0x7F, argv[3]))
                printf( "Reflashing OpenServo failed\n" );
            else
                printf( "Reflashing OpenServo complete\n" );
            break;
        case 'r':       //read n bytes
            if (argc < 5) { failcmd = true; break; }
            if (OSIF_read(0,parse_option( argv[2]),parse_option( argv[3]),buf,parse_option( argv[4]))>0)
            {
                //print what we got
                int n;
                char tmpbuf[255];
                char newbuf[255];
                newbuf[0] ='\0';

                for (n=0;n<parse_option( argv[4]);n++)
                {
                    sprintf( tmpbuf, "0x%02x ", buf[n]);
                    strcat(newbuf, tmpbuf);
                }
                printf( newbuf);
                printf("\n");
            } 
            break;
        case 'l':       //read n bytes
            if (argc < 5) { failcmd = true; break; }
            for (;;)
            {
                if (OSIF_read(0,parse_option( argv[2]),parse_option( argv[3]),buf,parse_option( argv[4]))>0)
                {
                    //print what we got
                    int n;
                    char tmpbuf[255];
                    char newbuf[255];
                    newbuf[0] ='\0';

                    if (parse_option( argv[4])==2)
                    {
                        sprintf( newbuf, "%d ", buf[0]<<8|buf[1]);
                    }
                    else
                    {
                        for (n=0;n<parse_option( argv[4]);n++)
                        {
                            sprintf( tmpbuf, "0x%02x ", buf[n]);
                            strcat(newbuf, tmpbuf);
                        }
                    }
                    printf( newbuf);
                    printf("\n");
                } 
            }
            break;

        case 'w':       //write
            if (argc < 4) { failcmd = true; break; }
            char outdata[127];
            //get the command line data and compile into a character array 
            parse_data( argv, 4, argc - 5,&outdata );
            OSIF_write(0,parse_option( argv[2]),parse_option( argv[3]),outdata,parse_option( argv[argc-1]));
            printf ("wrote data\n");
            break;
        case 's':       // scan bus. Returns all bus devices in array
            OSIF_scan(0, &devices[0], &dev_count);
            printf("Device count: %d\n",dev_count);
            int n;
            for( n = 0; n< dev_count; n++)
                printf( "device at 0x%02x\n", devices[n]);
            break;
        case 'p':       //probe to see if the servo exists at address
            if (argc < 3) { failcmd = true; break; }
            if (OSIF_probe(0, parse_option( argv[2])))
                printf("probe found device\n");
            break;
        case 'c':       //send a command
            if (argc < 4) { failcmd = true; break; }
            OSIF_command(0, parse_option( argv[2]), parse_option( argv[3]) );
            break;

        default:
            break;
    }
    if (failcmd)
    {
        printf( "Command line error. Check and try again\n" );
    }
    //Free the USB
    OSIF_deinit();
    //Free the library:
#ifdef WIN32
    int freeResult = FreeLibrary(hdll); 
#else
    dlclose(libhandle);
#endif
    return 0;
}

//gets all of the command line data and puts into a character array.
int parse_data( char *argv[], int count, int len, char *data )
{
    int n;
    for (n = 0; n< len; n++)
    {
        *data = parse_option(argv[count]);
        data++;
        count++;
    }

    printf("outdata %s\n", data);
    return 1;
}

int parse_option( char *arg )
{
    int byte;

    //check if hex detected 0x
    if (arg[0] == '0' && ( arg[1] == 'x' || arg[1] == 'X'))
    {
        arg+=2;

        byte = 0;

        if (( *arg >= 'A' ) && ( *arg <= 'F' ))
        {
            byte = *arg - 'A' + 10;
        }
        else if (( *arg>= 'a' ) && ( *arg <= 'f' ))
        {
            byte = *arg - 'a' + 10;
        }
        else if (( *arg >= '0' ) && ( *arg <= '9' ))
        {
            byte = *arg - '0';
        }
        else
        {
            printf( "Not a hex digit, found '%c'\n", *arg );
            return -1;
        }
        arg++;

        if ( *arg == '\0' )
        {
            return byte;
        }

        byte <<= 4;
        if (( *arg >= 'A' ) && ( *arg <= 'F' ))
        {
            byte |= *arg - 'A' + 10;
        }
        else if (( *arg >= 'a' ) && ( *arg <= 'f' ))
        {
            byte |= *arg - 'a' + 10;
        }
        else if (( *arg >= '0' ) && ( *arg <= '9' ))
        {
            byte |= *arg - '0';
        }
        else
        {
            printf(  "Not a hex digit, found '%c'\n", *arg );
            return -1;
        }
        arg++;
    }
    else 
    {
        return atoi(arg); 
    }
    return byte;
}
