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
        printf( "usage: %s [command] [i2c_addr] [address] [length]\n", argv[0] );
        printf( "  commands: flash   [i2c_addr] [filename]- flash the i2c_addr\n");
        printf( "            read    [i2c_addr] [address] [length]\n");
        printf( "            write   [i2c_addr] [address] [data] [data]... [length]\n");
        printf( "            probe   [i2c_addr] - probe to see if i2c_addr exists\n");
        printf( "            command [i2c_addr] [command]\n");
        printf( "            bitrate [speed (khz)] - Set the bitrate\n");
        printf( "            scan - list all devices on the bus\n");
        printf( "            gpio ddr - ORed bits of direction control\n");
        printf( "            gpio r - read all gpio pins as ORed bits\n");
        printf( "            gpio w [gpios] - write all gpios from a bitwise OR int\n");
        printf( "            gpio s [gpio num] [1|0] - set a single gpio high or low\n");
        printf( "            more - advanced functions\n");
        printf( "\n");
        printf( "  example:  write 0x02 0x41 1 2 3 4 5 6 6\n");
        printf( "\n");
        printf( "  Values can be specified as int (16) or hex (0x10).\n");
        printf( "(c) Barry Carter 2007-2010 www.openservo.com www.robotfuzz.co.uk\n");
        return 1;
    }

    /*Call the function*/
    if (OSIF_init() <0 )
    {
        printf("Failed to open OSIF device. Check cables\n");
#ifdef WIN32
        exit(1);
#endif
    }

    int devices[128];
    int dev_count=0;
    bool failcmd = false;
    unsigned char buf[255];
    int ret = 0;
    int len = 1;

    switch ( argv[1][0] )
    {
        case 'b':       //bitrate
            OSIF_set_bitrate(0, parse_option(argv[2]));
            break;
        case 'f':       //flash
            if (argc < 4) { failcmd = true; break; }
            if (!OSIF_reflash(0, parse_option(argv[2]), 0x7F, argv[3]))
                printf( "Reflashing OpenServo failed\n" );
            else
                printf( "Reflashing OpenServo complete\n" );
            break;
        case 'r':       //read n bytes
            if (argc < 5) { failcmd = true; break; }
            switch (argv[1][1])
            {
                case '1':
                    len = parse_option( argv[4]);
                    ret = OSIF_read_data(0,parse_option( argv[2]),parse_option( argv[3]),buf,len, parse_option( argv[5]));
                    
                    break;
                case '2':
                    len = parse_option( argv[3]);
                    ret = OSIF_readonly(0,parse_option( argv[2]),buf,len, parse_option( argv[4]));
                    
                    break;
                default:
                    len = parse_option( argv[4]);
                    if (len > 64)
                        len=64;
                    ret = OSIF_read(0,parse_option( argv[2]),parse_option( argv[3]),buf,len);
                    break;
            }
            //if (ret>=0)
            {
                //print what we got
                int n;
                char tmpbuf[255];
                char newbuf[255];
                newbuf[0] ='\0';

                for (n=0;n<len;n++)
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
            // Get the second letter as the option
            switch (argv[1][1])
            {
                case '1':
                    //get the command line data and compile into a character array
                    parse_data( argv, 4, argc - 6,&outdata );

                    OSIF_write_data(0,parse_option( argv[2]),parse_option( argv[3]),outdata,parse_option( argv[argc-2]), parse_option( argv[argc-1]));
                    break;
                case '2':
                                //get the command line data and compile into a character array 
                    parse_data( argv, 3, argc - 5,&outdata );

                    OSIF_writeonly(0,parse_option( argv[2]),outdata,parse_option( argv[argc-2]), parse_option( argv[argc-1]));
                    break;
                default:
                                //get the command line data and compile into a character array 
                    parse_data( argv, 4, argc - 5,&outdata );

                    OSIF_write(0,parse_option( argv[2]),parse_option( argv[3]),outdata,parse_option( argv[argc-1]));
                    break;
            }
            
            printf ("wrote data\n");
            break;
        case 's':       // scan bus. Returns all bus devices in array
            OSIF_scan(0, &devices[0], &dev_count);
            printf("Device count: %d\n",dev_count);
            int n;
            for( n = 0; n< dev_count; n++)
                printf( "device at 0x%02x\n", devices[n]);
            break;
        case 'p':       //probe to see if the i2c_addr exists at address
            if (argv[1][1] == 'w') // pwm mode
            {
                if (OSIF_set_pwm(0, parse_option( argv[2])))
                    printf("PWM Set\n");
                break;
            }
            if (argc < 3) { failcmd = true; break; }
            if (OSIF_probe(0, parse_option( argv[2])))
                printf("probe found device\n");
            break;
        case 'c':       //send a command
            if (argc < 4) { failcmd = true; break; }
            OSIF_command(0, parse_option( argv[2]), parse_option( argv[3]) );
            break;

        case 'g':       // Gpio control
            if (argv[2][0] == 'd') //ddr
            {
                OSIF_io_set_ddr(0, parse_option( argv[3] ), parse_option( argv[4] ));
            }
            else if (argv[2][0] == 'r') //read
            {
                int io = 0xFF;
                io = OSIF_io_get_in(0, io);
                printf("io is %d\n", io);
            }
            else if (argv[2][0] == 'w') //write
            {
                OSIF_io_set_out(0, parse_option( argv[3] ));
            }
            else if (argv[2][0] == 's') //set 1 gpio
            {
                   OSIF_io_set_out1(0, parse_option( argv[3] ), parse_option( argv[4] ));
            }
            break;
        case 'm':        //More help
            printf( "  commands: w1   [i2c_addr] [byte]- write data\n");
            printf( "  commands: w2   [i2c_addr] [byte]- write only\n");
            printf( "  commands: r1   [i2c_addr] [address]- read data\n");
            printf( "  commands: r2   [i2c_addr] [address]- read only\n");
	    printf( "  commands: pwm  [value 0-255]\n");
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
//    int freeResult = FreeLibrary(hdll); 
#else
printf(libhandle);
    //dlclose(libhandle);
#endif
    return 0;
}

//gets all of the command line data and puts into a character array.
int parse_data( char *argv[], int count, int len, char *data )
{
    int n;
    printf("count %d outdata: ", count);
    for (n = 0; n< len; n++)
    {
        *data = parse_option(argv[count]);
        printf("%d ",data[n]);
        data++;
        count++;
    }

    printf("\n len %d\n", len);
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
