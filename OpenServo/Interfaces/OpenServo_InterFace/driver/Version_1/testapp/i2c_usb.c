/*
 * i2c_usb.c - test application for the i2c-tiby-usb interface
 *             http://www.harbaum.org/till/i2c_tiny_usb
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>

/* ds1621 chip address (A0-A2 tied low) */
#define OPENSERVO_ADDR  0x10

#define LOOPS 100

#define USB_CTRL_IN    (USB_TYPE_CLASS | USB_ENDPOINT_IN)
#define USB_CTRL_OUT   (USB_TYPE_CLASS)

/* the vendor and product id was donated by ftdi ... many thanks!*/
#define I2C_TINY_USB_VID  0x0403
#define I2C_TINY_USB_PID  0xc631

#ifdef WIN
#include <windows.h>
#include <winbase.h>
#define usleep(t) Sleep((t) / 1000)
#endif

#define I2C_M_RD		0x01

/* commands via USB, must e.g. match command ids firmware */
#define CMD_ECHO       0
#define CMD_GET_FUNC   1
#define CMD_SET_DELAY  2
#define CMD_GET_STATUS 3
#define CMD_I2C_IO     4
#define CMD_I2C_BEGIN  1  // flag to I2C_IO
#define CMD_I2C_END    2  // flag to I2C_IO

#define STATUS_IDLE          0
#define STATUS_ADDRESS_ACK   1
#define STATUS_ADDRESS_NAK   2

usb_dev_handle      *handle = NULL;

/* write a set of bytes to the i2c_tiny_usb device */
int i2c_tiny_usb_write(int request, int value, int index) {
  if(usb_control_msg(handle, USB_CTRL_OUT, request, 
		      value, index, NULL, 0, 1000) < 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  }
  return 1;
}

/* read a set of bytes from the i2c_tiny_usb device */
int i2c_tiny_usb_read(unsigned char cmd, void *data, int len) {
  int                 nBytes;

  /* send control request and accept return value */
  nBytes = usb_control_msg(handle, 
	   USB_CTRL_IN, 
	   cmd, 0, 0, data, len, 1000);

  if(nBytes < 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return nBytes;
  }

  return 0;
}

/* get i2c usb interface firmware version */
void i2c_tiny_usb_get_func(void) {
  unsigned long func;
  
  if(i2c_tiny_usb_read(CMD_GET_FUNC, &func, sizeof(func)) == 0)
    printf("Functionality = %lx\n", func);
}

/* set a value in the I2C_USB interface */
void i2c_tiny_usb_set(unsigned char cmd, int value) {
  if(usb_control_msg(handle, 
	     USB_TYPE_VENDOR, cmd, value, 0, 
	     NULL, 0, 1000) < 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
  }
}

/* get the current transaction status from the i2c_tiny_usb interface */
int i2c_tiny_usb_get_status(void) {
  int i;
  unsigned char status;
  
  if((i=i2c_tiny_usb_read(CMD_GET_STATUS, &status, sizeof(status))) < 0) {
    fprintf(stderr, "Error reading status\n");
    return i;
  }

  return status;
}

/* write command and read an 8 or 16 bit value from the given chip */
int i2c_read_with_cmd(unsigned char addr, char cmd, int length) {
  unsigned char result[2];

  if((length < 0) || (length > sizeof(result))) {
    fprintf(stderr, "request exceeds %d bytes\n", sizeof(result));
    return -1;
  } 

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN
		     + ((!length)?CMD_I2C_END:0),
		     0, addr, &cmd, 1, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }

  // just a test? return ok
  if(!length) return 0;

  if(usb_control_msg(handle, 
		     USB_CTRL_IN, 
		     CMD_I2C_IO + CMD_I2C_END,
		     I2C_M_RD, addr, (char*)result, length, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "read data status failed\n");
    return -1;
  }

  // return 16 bit result
  if(length == 2)
    return 256*result[0] + result[1];

  // return 8 bit result
  return result[0];  
}

/* write a single byte to the i2c client */
int i2c_write_byte(unsigned char addr, char data) {

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, addr, &data, 1, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }

  return 0;  
}

/* write a command byte and a single byte to the i2c client */
int i2c_write_cmd_and_byte(unsigned char addr, char cmd, char data) {
  char msg[2];

  msg[0] = cmd;
  msg[1] = data;

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, addr, msg, 2, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }

  return 0;  
}

/* write a command byte and a 16 bit value to the i2c client */
int i2c_write_cmd_and_word(unsigned char addr, char cmd, int data) {
  char msg[3];

  msg[0] = cmd;
  msg[1] = data >> 8;
  msg[2] = data & 0xff;

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, addr, msg, 3, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }

  return 0;  
}

int main(int argc, char *argv[]) {
  struct usb_bus      *bus;
  struct usb_device   *dev;

#ifndef WIN
  int ret;
#endif
  
  printf("--          i2c-tiny-usb test application         --\n");
  printf("--            (c) 2006 by Till Harbaum            --\n");
  printf("-- (c) 2007 OpenServo Modifications Barry Carter  --\n");
  printf("--   http://www.harbaum.org/till/i2c_tiny_usb     --\n");
  printf("--          http://www.openservo.com/             --\n");

  usb_init();
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus = usb_get_busses(); bus; bus = bus->next) {
    for(dev = bus->devices; dev; dev = dev->next) {
      if((dev->descriptor.idVendor == I2C_TINY_USB_VID) && 
	 (dev->descriptor.idProduct == I2C_TINY_USB_PID)) {
	
	printf("Found i2c_tiny_usb device on bus %s device %s.\n", 
	       bus->dirname, dev->filename);
	
	/* open device */
	if(!(handle = usb_open(dev))) 
	  fprintf(stderr, "Error: Cannot open the device: %s\n", 
		  usb_strerror());

	break;
      }
    }
  }
  
  if(!handle) {
    fprintf(stderr, "Error: Could not find i2c_tiny_usb device\n");

#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif

    exit(-1);
  }

#ifndef WIN
  /* Get exclusive access to interface 0. Does not work under windows. */
  ret = usb_claim_interface(handle, 0);
  if (ret != 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());

    exit(1);
  }
#endif
  
  /* do some testing */
  i2c_tiny_usb_get_func();

  /* try to set i2c clock to 100kHz (10us), will actually result in ~50kHz */
  /* since the software generated i2c clock isn't too exact. in fact setting */
  /* it to 10us doesn't do anything at all since this already is the default */
  i2c_tiny_usb_set(CMD_SET_DELAY, 10);

  /* -------- begin of OpenServo client processing --------- */
  printf("Probing for OpenServo ... ");

  /* try to access ds1621 at address DS1621_ADDR */
  if(usb_control_msg(handle, USB_CTRL_IN, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, OPENSERVO_ADDR, NULL, 0, 
		     1000) < 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    goto quit;
  } 
  
  if(i2c_tiny_usb_get_status() == STATUS_ADDRESS_ACK) {
    int temp;

    printf("success at address 0x%02x\n", OPENSERVO_ADDR);

    /* activate one shot mode */
    if(i2c_write_cmd_and_word(OPENSERVO_ADDR, 0x10, 0x02BC) < 0)
      goto quit;

    /* wait 10ms */
    usleep(10000);

    /* display limits */
    temp = i2c_read_with_cmd(OPENSERVO_ADDR, 0x08, 2);
    printf("Current servo position = %04d\n", temp);
  } else
    printf("failed\n");
  /* -------- end of OpenServo client processing --------- */

 quit:
#ifndef WIN
  ret = usb_release_interface(handle, 0);
  if (ret)
    fprintf(stderr, "USB error: %s\n", usb_strerror());
#endif

  usb_close(handle);

#ifdef WIN
  printf("Press return to quit\n");
  getchar();
#endif

  return 0;
}
