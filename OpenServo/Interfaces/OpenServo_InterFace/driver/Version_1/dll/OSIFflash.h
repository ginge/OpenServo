/***************************************************************************
                        OSIF_flash.h  -  Application uploader header
                             -------------------
    begin                : Jan 17 2006
    email                : barry.carter@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef OSIF_FLASH_H
#define OSIF_FLASH_H     //stop recursion

//bootloader defines
#define MAGIC_REBOOT         0xFFFF
//address for the openservo bootloader
#define OPENSERVO_BOOTLDR_ADDR 0x7F

//The command to reset the OpenServo
#define OS_RESET 0x80

#define MAX_BOOTLDR_SIZE 8192
#define PAGE_SIZE 128
#define MAX_I2C_DEVICES 128

//structure to store loaded bootload pages
typedef struct Sbootpages
{
	unsigned char pg[MAX_BOOTLDR_SIZE][PAGE_SIZE+10];
} bootpages;


//bootloader
int ParseIHexLine( const char *line, unsigned char *data );
int load_file(char *filename, int *memory, int *maxaddr);

//programming the servo from the bootloader
int OSIF_bootloader_init( int adapter, int servo, char *filename );
int OSIF_bootloader_reboot( int adapter );
int OSIF_verify_page( int adapter, int servo, unsigned char *reg_addr, unsigned char *page );


#endif
