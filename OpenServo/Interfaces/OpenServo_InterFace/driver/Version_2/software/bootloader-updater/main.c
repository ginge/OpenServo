// ======================================================================
// USBtiny Bootloader flash
//
// Copyright (C) 2007 Barry Carter
// Copyright (C) 2008 Alistair Buxton
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

#define MAX_PAGE 65536
#define MAX_MEMORY 65536
#define MAX_RETRY 3

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

void write_page(int page, unsigned char *page_buffer, int page_size) {

    int address = page * page_size;
    int retval;

    retval = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
                             USBTINYBL_FUNC_WRITE, address, 0, 
                             (char *)page_buffer, page_size, 10000);

    if (retval != page_size) {
        fprintf(stderr, "Error: wrong byte count in write_page: %d !\n", retval);
//        exit(1);
    }
}

void read_page(int page, unsigned char *page_buffer, int page_size) 
{
    int address = page * page_size;
    int retval;

    retval = usb_control_msg(usbhandle, 
                             USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                             USBTINYBL_FUNC_READ, address, 0, 
                             (char *)page_buffer, page_size, 10000);

    if (retval != page_size) 
    {
        fprintf(stderr, "Error: wrong byte count in read_page: %d !\n", retval);
  //      exit(1);
    }
}

// compare two char buffers. return number of characters which are equal.
int verify_page(unsigned char *page_wr, unsigned char *page_rd, int page_size) 
{
    int i;
    for(i=0;i<page_size&&page_wr[i]==page_rd[i];i++) ;
    return i;
}

int process_firmware(unsigned char *firmware, int firmware_length, int page_size, char mode) {

    unsigned char page_buffer[MAX_PAGE]; // stores the read back page for verification

    int whole_pages = 0; // number of pages we need to process
    int page = 0;        // current page
    int verify_tmp = 0;  // result of verify
    int fail_count = 0;  // failure count

    whole_pages = firmware_length / page_size;
    if((firmware_length % page_size) > 0) whole_pages++;

    for(page=0;page<whole_pages;page++) {
usleep(100);
        if(mode == 'w') {
            printf("Writing page %d...", page);
            write_page(page, firmware+(page*page_size), page_size);        
        }

        printf("Verifying page %d...", page);
        read_page(page, page_buffer, page_size);

        verify_tmp=verify_page(page_buffer, firmware+(page*page_size), page_size);
        if(verify_tmp < page_size)
        {
            printf("Failed at page %d, byte %d, (address 0x%x)\n", page, verify_tmp, (page*page_size)+verify_tmp);

            if(mode == 'v') return -1;
            else {
                fail_count++;
                if(fail_count >= MAX_RETRY) {
                    printf("Failed too many times, giving up.\n");
                    return -1;
                }
                printf("Retrying...");
                page--;
            }
        } else {
            printf("OK\n");
            fail_count = 0;
        }
    }
    return 0;
}


int main(int argc, char **argv) 
{

    int memory[MAX_MEMORY];
    unsigned char real_memory[MAX_MEMORY];
    int max_addr;
    int page_size;
    int firmware_ver;
    int i;

    if (argc < 3 || argv[1][1] != 0 || 
        (argv[1][0] != 'w' && argv[1][0] != 'v')) {
        fprintf(stderr, "Usage: osifupdate w filename.hex (Write firmware)\n");
        fprintf(stderr, "Usage: osifupdate v filename.hex (Verify firmware)\n");
        exit(1);
    }

    // TODO: Needs checking for errors
    init();

    page_size = get_page_size();       // Load the page size
    printf("Page size: %d\n", page_size);

    firmware_ver = get_firmware_version();       // Get the firmware version
    printf("Firmware Version: %02d.%02d\n", firmware_ver>>8, firmware_ver&0xFF);

    // load file into memory array
    if (load_file(argv[2], memory, &max_addr) <0)
    {
        printf("Failed to load file %s. Check path.\n", argv[1]);
        return -1;
    }

    // convert to byte array
    memset(real_memory, 0, MAX_MEMORY);
    for (i=0; i<= max_addr+1; i++) 
    {
        //copy the temporary page into the page array
        real_memory[i]=memory[i]&0xff;
    }

    printf("Firmware size: %d bytes (%d pages)\n", max_addr, (max_addr%page_size)?(max_addr/page_size)+1:(max_addr/page_size));

    // process task
    process_firmware(real_memory, max_addr, page_size, argv[1][0]);

    // clean up
    usb_close(usbhandle);

    return 0;
}
