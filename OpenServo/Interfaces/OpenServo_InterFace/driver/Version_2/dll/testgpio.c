/* Open Source InterFace (OSIF) Test application V1.0
  Test routines for gpio functions shared library libOSIFlib.so.1.x.x
    Copyright (C) 2008  Barry Carter <barry.carter@robotfuzz.com>

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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void closeapp(void);
void ltostr(long x, char * s, size_t n);


/* Typedef the OSIF functions */
typedef int  (*OSIF_initfunc   )();
typedef int  (*OSIF_deinitfunc )();

/* GPIOFunctions */
typedef int (*OSIF_io_set_ddrfunc  )(int adapter_no, int ddr, int enabled);
typedef int (*OSIF_io_set_outfunc  )(int adapter_no, int io);
typedef int (*OSIF_io_set_out1func  )(int adapter_no, int gpio, int state);
typedef int (*OSIF_io_get_infunc   )(int adapter_no);
typedef int (*OSIF_io_get_currentfunc   )(int adapter_no);

/* Pointers to the functions */
OSIF_deinitfunc OSIF_deinit;
OSIF_initfunc OSIF_init;

OSIF_io_set_ddrfunc OSIF_io_set_ddr;
OSIF_io_set_outfunc OSIF_io_set_out;
OSIF_io_get_infunc  OSIF_io_get_in;
OSIF_io_set_out1func OSIF_io_set_out1;
OSIF_io_get_currentfunc OSIF_io_get_current;

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)
  
int main ( int argc, char *argv[] ) {


#ifdef WIN32
    /*Windows handle*/
    HANDLE hdll;

    /*LoadLibrary*/
    hdll = LoadLibrary("OSIFdll.dll");

    if (!hdll) { printf("Error loading dll\n"); exit(0); }

    /*GetProcAddress*/
    OSIF_init    = (OSIF_initfunc)GetProcAddress(hdll, "OSIF_init");
    OSIF_deinit  = (OSIF_deinitfunc)GetProcAddress(hdll, "OSIF_deinit");

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

    OSIF_io_set_ddr = (OSIF_io_set_ddrfunc)dlsym(libhandle, "OSIF_io_set_ddr");
    OSIF_io_set_out = (OSIF_io_set_outfunc)dlsym(libhandle, "OSIF_io_set_out");
    OSIF_io_set_out1= (OSIF_io_set_out1func)dlsym(libhandle, "OSIF_io_set_out1");
    OSIF_io_get_in  = (OSIF_io_get_infunc)dlsym(libhandle, "OSIF_io_get_in");
    OSIF_io_get_current  = (OSIF_io_get_currentfunc)dlsym(libhandle, "OSIF_io_get_current");
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
    int dev_count=0;
    bool failcmd = false;
    unsigned char buf[255];
    int ret = 0;
    int len = 1;
    printf("Welcome to GPIO test app! Press 0 - 5 to change a gpio, 6 to read gpio inputs or 9 to quit\n\n");

    // set ports as output excluding I2C
    OSIF_io_set_ddr(0, 0xCF, 0xCF);
    
    bool quit = false;
    bool disabled_i2c = false;
    while (!quit)
    {
        int a, b;
        printf("Enter GPIO (0-5): ");
        scanf("%d", &a);
        if (a == 9)
        {
            quit = true;
            closeapp();
            continue;
        }
        
        // read
        if (a == 6)
        {
            int inv = 0;
            // set ports as input excluding i2c
            OSIF_io_set_ddr(0, 0x00, 0x0F);
            inv = OSIF_io_get_in(0);
	    
	    printf("Port         543210\n");
	    printf("Input Values "BYTETOBINARYPATTERN"\n", BYTETOBINARY(inv));
	    
	    continue;
        }

        if ((a == 4) || (a == 5) && !disabled_i2c)
        {
            char c;
            printf("Warning! Disabling TWI module and convering to GPIO %d\n", a);
            printf("Are you sure? (y/n): ");
            c = getchar();
            c = getchar();
            printf("\n");
            if (c != 'y')
            {
                printf("Nothing done\n");
                disabled_i2c = true;
                continue;
            }
            // set all as outputs.
            OSIF_io_set_ddr(0, 0xFF, 0xFF);

        }
        
        int current_regs = OSIF_io_get_current(0);
        printf("** GPIO %d is currently %s\n", a, (current_regs  & (1<<a)) ? "high" : "low");
        printf("Set High or low? (0|1): ");
        scanf("%d", &b);
        if (b > 1 || b < 0)
            b = 0;
        printf("** GPIO %d set to %s\n", a, b ? "high" : "low");
        
        // set that as an output
        OSIF_io_set_out1(0,a,b);
        
        int j, k;
        current_regs = OSIF_io_get_current(0);
        
	printf("Current pin outputs:\n\n");
        printf("Port         543210\n");
	printf("Input Values "BYTETOBINARYPATTERN"\n", BYTETOBINARY(current_regs));
	    
    }

    return;

}

void ltostr(long x, char * s, size_t n)
{
    memset( s, 0, n );
    int pos = n - 2;

    while( x && (pos >= 0) )
    {
      s[ pos-- ] = (x & 0x1) ? '1' : '0'; // Check LSb of x
      x >>= 1;
    }
}

void closeapp(void)
{
    OSIF_deinit();
    //Free the library:
#ifdef WIN32
    int freeResult = FreeLibrary(hdll);
#else
    dlclose(libhandle);
#endif
}

