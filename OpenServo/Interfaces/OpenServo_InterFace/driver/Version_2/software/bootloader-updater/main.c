// ======================================================================
// USBtiny Bootloader flash
//
// Copyright (C) 2007 Barry Carter
//
//
// This is free software, licensed under the terms of the GNU General
// Public License V2 as published by the Free Software Foundation.
// ======================================================================


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "usb.h"

#define USBTINYBL_FUNC_APP_INIT 1
#define USBTINYBL_FUNC_READ     2
#define USBTINYBL_FUNC_WRITE    3
#define USBTINYBL_FUNC_GET_PAGE 4
#define USBTINYBL_FUNC_GET_VER  5

#define USBTINY_VENDOR_ID       0x0403
#define USBTINY_PRODUCT_ID      0x6666

usb_dev_handle *usbhandle;
static usb_dev_handle *find_device();

int load_file(char *filename, int *memory, int *maxaddr);

int init()
{
    usb_init();
    usbhandle = find_device();
    if(usbhandle == NULL){
        fprintf(stderr, "Could not find USB device \"osifupdate\" with vid=0x%x pid=0x%x\n", USBTINY_VENDOR_ID, USBTINY_PRODUCT_ID);
        exit(1);
    }
    return 1;
}

static usb_dev_handle   *find_device(void)
{
    struct usb_bus      *bus;
    struct usb_device   *dev;
    usb_dev_handle      *handle = 0;

    usb_find_busses();
    usb_find_devices();
    for(bus=usb_busses; bus; bus=bus->next){
        for(dev=bus->devices; dev; dev=dev->next){
            if(dev->descriptor.idVendor == USBTINY_VENDOR_ID && dev->descriptor.idProduct == USBTINY_PRODUCT_ID){

                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle){
                    fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }

                printf("USBtiny Bootloader on OSIF\n");

            }
        }
        if(handle)
            break;
    }
    if(!handle)
        fprintf(stderr, "Could not find USB device osifupdater\n");
    return handle;
}

unsigned int get_page_size() {
    char       buf[8];
    int        retlen;

    retlen = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                             USBTINYBL_FUNC_GET_PAGE, 0, 0, 
                             buf, sizeof(buf), 5000);

    if (retlen != 2) {
        fprintf(stderr, "Error: wrong page size. Got: %d !\n", retlen);
        exit(1);
    }

    return (buf[0] << 8) | buf[1];
}

unsigned int get_firmware_version() {
    char       buf[8];
    int        retlen;

    retlen = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                             USBTINYBL_FUNC_GET_VER, 0, 0, 
                             buf, sizeof(buf), 5000);

    if (retlen != 2) {
        fprintf(stderr, "Error: wrong page size. Got: %d !\n", retlen);
        exit(1);
    }

    return (buf[0] << 8) | buf[1];
}

void start_application() {
    char       buf[8];
    int        retlen;

    retlen = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                             USBTINYBL_FUNC_APP_INIT, 0, 0, 
                             buf, sizeof(buf), 5000);

    if (retlen != 0) {
        fprintf(stderr, "Error: wrong return val in start_application: %d !\n", retlen);
        exit(1);
    }
}

void write_page(int address, unsigned char *page, int page_size) {

    int retval;

    retval = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
                             USBTINYBL_FUNC_WRITE, address, 0, 
                             (char *)page, page_size, 5000);

    if (retval != page_size) {
        fprintf(stderr, "Error: wrong byte count in read_page: %d !\n", retval);
        exit(1);
    }
}

void read_page(int address, unsigned char *page, int page_size) 
{

    int retval;

    retval = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                             USBTINYBL_FUNC_READ, address, 0, 
                             (char *)page, page_size, 5000);
    printf("page %d\n",address);

    if (retval != page_size) 
    {
        fprintf(stderr, "Error: wrong byte count in read_page: %d !\n", retval);
        exit(1);
    }
}

int verify_page(int address, unsigned char *page_wr, unsigned char *page_rd, int page_size) 
{
    int i;

    if( (i=strncmp((char *)page_rd, (char *)page_wr, page_size)) != 0) 
    {
        printf("failed %x\n", i);
        return i;
    }

    return -1;
}

int main(int argc, char **argv) 
{

    int memory[65536];
    unsigned char page[8192][512];
    int max_addr=0;
    int reg_addr=0;
    int n=0;
    int page_size;
    int i;
    int page_count=0;
    int firmware_ver;
    int fail_count=0;

    if (argc < 2) {
        fprintf(stderr, "Usage: osifupdate filename.hex\n");
        exit(1);
    }

    // TODO: Needs checking for errors
    init();

    page_size = get_page_size();       // Load the page size
    printf("Page size: %d\n", page_size);

    firmware_ver = get_firmware_version();       // Get the firmware version
    printf("Firmware Version: %02d.%02d\n", firmware_ver>>8, firmware_ver&0xFF);

    // load file into memory array
    if (load_file(argv[1], memory, &max_addr) <0)
    {
        printf("Failed to load file %s. Check path.\n", argv[1]);
        return -1;
    }

    // Break the memory array into each page
    for (i=0; i<= max_addr+1; i++) 
    {
        //copy the temporary page into the page array
        page[page_count][n]=memory[i];
        n++;
        if (n==page_size||max_addr == i) 
        {
            reg_addr+=(page_size);
            page_count++;
            n=0; //reset the counter pointer.
        }
    }
    printf( "page count %d max byte %d\n", page_count, max_addr);

    unsigned char page_readfrom[255];
    reg_addr=0;

    for (i=0; i< page_count; i++) 
    {
        int j=0;
        write_page( reg_addr, page[i], page_size);
        printf("\nWrote. Page %d\n", i);

        read_page(reg_addr, page_readfrom, page_size);
        for(n=0; n<page_size; n++)
        {
            if (j>5)
            {
                printf("\n");
                j=0;
            }
            printf("|0x%02x 0x%02x|", (unsigned char)page_readfrom[n], page[i][n]);
            j++;
        }

        int err;

        if ((err = verify_page( reg_addr, page[i], page_readfrom, page_size)) >=0)
        {
            if (fail_count > 2)
            {
                printf("Failed verify 3 times. Hard Fail. Sorry.\n");
                return -1;
            }
            printf("Verify page error at byte %d. Will resend.\n", err+(page_size*i));
            --i;
            fail_count++;
            continue;
        }

/*        for(n=0; n<page_size; n++)
        {
            printf("0x%02x ", (unsigned char)page[i][n]);
        }*/
        reg_addr+=(page_size);
    }
    printf("\nFlash completed OK\n");

    usb_close(usbhandle);

    return 0;
}
