/***************************************************************************
                        OSIFflash.c  -  Bootloader functions
                             -------------------
    begin                : Jan 29 2007
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
#include "OSIFdll.h"
//#include "OSIFflash.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN
#include <windows.h>
#endif

/**
*
*   Programs the bootloader. This is done starting at 16bit page 0x0000
*   each page is 128bytes. After each page if flashed, the address is incremented
*   page PAZE_SIZE, and the next page begins.
*   After each page is flashed, it is read and verified. If a page fails verification, it is reflashed
*
*/
int OSIF_bootloader_init(int adapter, int servo, char * filename) {
	int i;
	int n=2;
	int page_count=0;
	int reg_addr=0;
	int	memory[65536];
	unsigned char page[MAX_BOOTLDR_SIZE][PAGE_SIZE+10];
	
	usb_dev_handle *handle;
	handle = get_adapter_handle(adapter);
	
	int page_fail_cnt=0;
	unsigned char verbuf[PAGE_SIZE];
	char buf[255];
	int max_addr=0;
	// load file into memory array
	if (load_file(filename, memory, &max_addr) <0)
	{
		printf("Failed to load file %s. Check path.\n", filename);
		return -1;
	}
	
	max_addr+=20;
	// break into 128 byte pages
	for (i=0; i< max_addr+1; i++) {
	  //copy the temporary page into the page array
	  page[page_count][n]=memory[i];
	  n++;
	  if (n>PAGE_SIZE+1) {
	    //start at offset 2
	    n=2;
	    page_count++;
	  }
	}
	printf( "page count %d\n", page_count);

  //for each page, write to the servo, and verify
  for (i=0; i<=page_count; i++) 
  {
    //convert to uint16
    page[i][0]=(reg_addr>>8)&0x00FF;
    page[i][1]=(reg_addr)&0x00FF;

    //print the data to the log.
    fprintf(stderr, "page %x %x\n", page[i][0], page[i][1] );

		if(usb_control_msg(handle, USB_CTRL_OUT, 
	     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
	     0, servo, page[i], PAGE_SIZE+2, 
	     1000) < 1) 
			{
			  fprintf(stderr, "USB error: %s\n", usb_strerror());
				return -1;
			} 
			//assign some memory for the verify
			memcpy( &verbuf, &page[i][2], PAGE_SIZE );
			//only verify from page 2. lower portion is the bootloader
			if ( i>2) 
			{
			  if ( OSIF_verify_page(adapter, servo, page[i], verbuf )>0) {
			    printf("verify OK\n");
			    page_fail_cnt=0;
				}
      	else 
	      {   //page bad
	        if (page_fail_cnt==2) 
	        {
	          printf("Verify FAIL. Page sent twice. ABORTING\n");
	          return -1;
	      	}
		      printf("Verify FAIL. Resending page\n");
		      i--;
		        page_fail_cnt++;
		        continue;
		    }
    	}
      reg_addr+=(PAGE_SIZE);
  }

  OSIF_bootloader_reboot(adapter);
  return 1;
}


/**
*
*   bootloader verify 1 page starting at address reg_addr. the data to be verified is
*   passed as page
*
*/
int OSIF_verify_page(int adapter, int servo, unsigned char *reg_addr, unsigned char *page) {
	unsigned char read_page[PAGE_SIZE];
	
	usb_dev_handle *handle;
	handle = get_adapter_handle(adapter);
	
  //write the register to read from
	if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, servo, reg_addr, 2, 
		     1000) < 1) 
	{
		fprintf(stderr, "USB error: %s\n", usb_strerror());
		return -1;
	}
  				
 	if(usb_control_msg(handle, 
		     USB_CTRL_IN, 
		     CMD_I2C_IO + CMD_I2C_END,
		     I2C_M_RD, servo, read_page, PAGE_SIZE, 
		     1000) < 1) 
	{
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(OSIF_USB_get_status(handle) != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "read data status failed\n");
    return -1;
  }
  int n;

  printf( "write:\n");
  for( n=0; n<PAGE_SIZE;n++)
  {
    printf( "0x%02x ", page[n]);
  }
  printf("\n");
	
  printf( "read:\n");
  for( n=0; n<PAGE_SIZE;n++)
  {
    printf( "0x%02x ", read_page[n]);
  }
  printf("\n");
	  
  //compare the strings
  if( strncmp(&read_page[0], &page[0], PAGE_SIZE) != 0) { return -1; }
    	
  return 1;
}


/**
*
*  Bootloader Reboot. Sends magic 0xffff to reboot the servo after a flash.
* 
*/
int OSIF_bootloader_reboot( int adapter) {
  //write magic address of 0xFFFF
  unsigned char magic[3];
  magic[0]=0xFF;
  magic[1]=0xFF;

  OSIF_write(adapter, OPENSERVO_BOOTLDR_ADDR, 0xFF, magic, 2 );
  
  return 1;
}

/**
*
*   Programs the bootloader. This is done starting at 16bit page 0x0000
*   each page is 128bytes. After each page if flashed, the address is incremented
*   page PAZE_SIZE, and the next page begins.
*   After each page is flashed, it is read and verified. If a page fails verification, it is reflashed
*
*/
int OSIF_bootloader_getpage(char * filename, bootpages *page, int p_size) {
  int i;
  int n=2;
  int	memory[65536];
  int max_addr;

	int p=0;

	int page_count=0;
	// load file into memory array
	if (load_file(filename, memory, &max_addr) <0 )
		return -1;

  // break into 128 byte pages
  for (i=0; i< max_addr+1; i++) 
  {
    //copy the temporary page into the page array
    page->pg[page_count][n]=memory[i];
    n++;
    if (n>p_size+2) 
    {
      //start at offset 2
      n=2;
			p++;
      page_count = p;
    }
  }
  return 1;
}

/**
*
*   flash the servo. issue a reboot commmand, wait a couple of seconds for 
*   the servo to settle, and then hand off to the flash routines
*
*/
EXPORT int OSIF_reflash(int adapter, int servo, int bootloader_addr, char *filename) {

	//Check is a valid file
	if (!servo) return -1;

  if (OSIF_command(adapter, servo, OS_RESET) <0) {
    printf("Couldn't issue a reset, continuing anyway");
  }

  //wait 2 seconds to be sure
	usleep( 2000);

  if ( OSIF_bootloader_init(adapter, bootloader_addr, filename) <0) {
    printf("Error flashing servo\n");
    return -1;
  }
  return 1;
}
