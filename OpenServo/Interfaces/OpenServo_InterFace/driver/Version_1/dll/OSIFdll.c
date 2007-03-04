
#include "OSIFdll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
	   cmd, 0, 0, (char *)data, len, 1000);

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
EXPORT int OSIF_readbytes(int adapter, int servo, unsigned char data, int length) 
{
  unsigned char result[2];

  if((length < 0) || (length > sizeof(result))) {
    fprintf(stderr, "request exceeds %d bytes\n", sizeof(result));
    return -1;
  } 

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN
		     + ((!length)?CMD_I2C_END:0),
		     0, servo, &data, 1, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }
  
  if(usb_control_msg(handle, 
		     USB_CTRL_IN, 
		     CMD_I2C_IO + CMD_I2C_END,
		     I2C_M_RD, servo, (char*)result, length, 
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


/* write a command byte and a single byte to the i2c client */
EXPORT int OSIF_write8(int adapter, int servo, unsigned char addr, int data)
{
  char msg[2];

  msg[0] = addr;
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
EXPORT int OSIF_write16(int adapter, int servo, unsigned char addr, int data) 
{
  char msg[3];

  msg[0] = addr;
  msg[1] = data >> 8;
  msg[2] = data & 0xff;

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, servo, msg, 3, 
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

EXPORT int OSIF_init(void)
{

handle = NULL;

#ifndef WIN
  int ret;
#endif

  usb_init();
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus = usb_get_busses(); bus; bus = bus->next) {
    for(dev = bus->devices; dev; dev = dev->next) {
      if((dev->descriptor.idVendor == I2C_TINY_USB_VID) && 
	 (dev->descriptor.idProduct == I2C_TINY_USB_PID)) {
		
	/* open device */
	if(!(handle = usb_open(dev))) 
		return -1;

	break;
      }
    }
  }
  
  if(!handle) {
    return -1;
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
printf("initialised OK\n");
  	return 0;
 }
 
EXPORT int OSIF_deinit(void)
 {
 #ifndef WIN
  ret = usb_release_interface(handle, 0);
  if (ret)
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    
   return 1;
#endif

  usb_close(handle);
}

EXPORT int OSIF_write(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen )
{
	if (check_params( servo )<0)
	{
		printf("Data outside bounds. Use 0 -> 127 range\n");
		return -1;
	}

	char msg[65];
	  
	if ( buflen > 64 )
		return -1;
		
  msg[0] = addr;

	int n=0;
	for (n=0; n<buflen;n++)
	{
		msg[n+1]=data[n];
	}
	printf("adapter %d, servo %d, addr %d, data %x, buflen %d, msg %s\n",adapter,servo,addr,data,buflen,msg);

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, servo, msg, buflen, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }
	printf("write okay\n");
  return 0;  
	
}

/* write command and read an an arbitary length value from the given chip */
EXPORT int OSIF_read(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen )
{
 	if (check_params( servo )<0)
	{
		printf("Data outside bounds. Use 0 -> 127 range\n");
		return -1;
	}

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN
		     + ((!buflen)?CMD_I2C_END:0),
		     0, servo, &addr, 1, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }


  if(usb_control_msg(handle, 
		     USB_CTRL_IN, 
		     CMD_I2C_IO + CMD_I2C_END,
		     I2C_M_RD, servo, data, buflen, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "read data status failed\n");
    return -1;
  }

  return 1;  
}

EXPORT int OSIF_scan(int adapter, int devices[], int *dev_count )
{
	
  int i;
	*dev_count = 0;
  //search all addresses for a response
  for (i=0x02; i<=MAX_I2C_DEVICES; i++) 
  {
    int res;

    char addr;
    addr=0x00;

	  /* write one byte register address to chip */
	  if(usb_control_msg(handle, USB_CTRL_OUT, 
			     CMD_I2C_IO + CMD_I2C_BEGIN
			     + CMD_I2C_END,
			     0, i, &addr, 1, 
			     1000) < 1) {
	    fprintf(stderr, "USB error: %s\n", usb_strerror());
	    return -1;
	  }
	  
		if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
  		;//
  	}
	  else
	  {
	  	printf("found device 0x%02x\n", i);
	  	devices[*dev_count] = i;
	  	*dev_count+=1;
	  	printf ("%x\n", *dev_count);
	  }
		
	}	
  return 1;
}

EXPORT bool OSIF_probe(int adapter, int servo )
{
  char addr;
  addr=0x00;

	if (check_params( servo )<0)
	{
		printf("Data outside bounds. Use 0 -> 127 range\n");
		return -1;
	}

  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN
		     + CMD_I2C_END,
		     0, servo, &addr, 1, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return false;
  }
  
	if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
		;//
	}
  else
  {
  	printf("found device 0x%02x\n", servo);
		return true;
  }
	
  return false;
}

/*
 * Send a command to the OpenServo
 */
EXPORT int OSIF_command(int adapter, int servo, unsigned char command)
{
	if (check_params( servo )<0)
	{
		printf("Data outside bounds. Use 0 -> 127 range\n");
		return -1;
	}
  /* write one byte register address to chip */
  if(usb_control_msg(handle, USB_CTRL_OUT, 
		     CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END,
		     0, servo, &command, 1, 
		     1000) < 1) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());
    return -1;
  } 

  if(i2c_tiny_usb_get_status() != STATUS_ADDRESS_ACK) {
    fprintf(stderr, "write command status failed\n");
    return -1;
  }
	
	printf( "Send command\n" );
	return 0;
}

int check_params( int val )
{
	if (val > 127 || val <0 )
		return -1;
	return 1;
}
