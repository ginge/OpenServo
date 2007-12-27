/*
 * i2c usb driver - 0.1
 *
 * Copyright (C) 2006 Til Harbaum (Till@Harbaum.org)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-skeleton.c 
 * but has been rewritten to be easy to read and use, as no locks are now
 * needed anymore.
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <asm/uaccess.h>

/* include interfaces to usb layer */
#include <linux/usb.h>

/* include interface to i2c layer */
#include <linux/i2c.h>

/* Version Information */
#define DRIVER_VERSION "v0.9"
#define DRIVER_AUTHOR  "Till Harbaum <Till@Harbaum.org>"
#define DRIVER_DESC    "i2c-tiny-usb driver"
#define DRIVER_URL     "http://www.harbaum.org/till/i2c_tiny_usb"

/* commands via USB, must e.g. match command ids firmware */
#define CMD_ECHO       0
#define CMD_GET_FUNC   1
#define CMD_SET_DELAY  2
#define CMD_GET_STATUS 3

#define CMD_I2C_IO     4
#define CMD_I2C_BEGIN  1  // flag to I2C_IO
#define CMD_I2C_END    2  // flag to I2C_IO

#define DEFAULT_DELAY  10    // default is 10us -> 100kHz
static int delay;

static int usb_read(struct i2c_adapter *i2c_adap, int cmd, 
		    int value, int index, void *data, int len);

static int usb_write(struct i2c_adapter *adapter, int cmd, 
		     int value, int index, void *data, int len);

/* ----- begin of i2c layer ---------------------------------------------- */

#define STATUS_IDLE          0
#define STATUS_ADDRESS_ACK   1
#define STATUS_ADDRESS_NAK   2

static int usb_xfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int num)
{
  unsigned char status;
  struct i2c_msg *pmsg;
  int i;
  int ret=0;

  dbg("master xfer %d messages:", num);

  for (i = 0;ret >= 0 && i < num; i++) {
    int cmd = CMD_I2C_IO;
    if(i == 0)     cmd |= CMD_I2C_BEGIN;
    if(i == num-1) cmd |= CMD_I2C_END;
    
    pmsg = &msgs[i];

    dbg("  %d: %s (flags %d) %d bytes to 0x%02x",
	i, pmsg->flags & I2C_M_RD ? "read" : "write", pmsg->flags,
	pmsg->len, pmsg->addr);

    /* and directly send the message */
    if(pmsg->flags & I2C_M_RD) {
      /* read data */
      if(usb_read(adapter, cmd, 
		  pmsg->flags, pmsg->addr, 
		  pmsg->buf, pmsg->len) != pmsg->len) {

	err("failure reading data");
	return -EREMOTEIO;
      }
#ifdef DEBUG_IO
      { 
	char str[32];
	int j; 
	str[0] = 0;
	for(j=0;j<pmsg->len;j++)
	  sprintf(str+strlen(str), "%x ", pmsg->buf[i]);
	info("   < %s", str);
      }
#endif
    } else {
#ifdef DEBUG_IO
      { 
	char str[32];
	int j; 
	str[0] = 0;
	for(j=0;j<pmsg->len;j++)
	  sprintf(str+strlen(str), "%x ", pmsg->buf[i]);
	info("   > %s", str);
      }
#endif
      
      /* write data */
      if(usb_write(adapter, cmd, 
		   pmsg->flags, pmsg->addr, 
		   pmsg->buf, pmsg->len) != pmsg->len) {
	err("failure writing data");
	return -EREMOTEIO;
      }
    }

    /* read status */
    if(usb_read(adapter, CMD_GET_STATUS, 0, 0, &status, 1) != 1) {
      err("failure reading status");
      return -EREMOTEIO;
    }

    dbg("  status = %d", status);
    if(status == STATUS_ADDRESS_NAK)
      return -EREMOTEIO;
  }
  
  return i;
}

static u32 usb_func(struct i2c_adapter *adapter)
{
  u32 func;

  /* get functionality from adapter */
  if(usb_read(adapter, CMD_GET_FUNC, 0, 0, &func, sizeof(func)) != 
     sizeof(func)) {

    err("failure reading functionality\n");
    return 0;
  }

  info("got adapter functionality %x", func);
  return func;
}

/* This is the actual algorithm we define */
static struct i2c_algorithm usb_algorithm = {
  .master_xfer	  = usb_xfer,
  .functionality  = usb_func,
};

/* ----- end of i2c layer ---------------------------------------------- */

/* ----- begin of usb layer ---------------------------------------------- */

/* the usb i2c interface uses a vid/pid pair donated by ftdi */
#define USB_I2C_TINY_USB_VENDOR_ID	0x0403
#define USB_I2C_TINY_USB_PRODUCT_ID	0xc631

/* table of devices that work with this driver */
static struct usb_device_id i2c_tiny_usb_table [] = {
  { USB_DEVICE(USB_I2C_TINY_USB_VENDOR_ID, USB_I2C_TINY_USB_PRODUCT_ID) },
  { }					/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, i2c_tiny_usb_table);

/* Structure to hold all of our device specific stuff */
struct i2c_tiny_usb {
  struct usb_device *udev;		/* the usb device for this device */
  struct usb_interface *interface;	/* the interface for this device */
  
  /* i2c related things */
  struct i2c_adapter      i2c_adap;
};

static int usb_read(struct i2c_adapter *adapter, int cmd, 
		    int value, int index, void *data, int len) {
  struct i2c_tiny_usb *dev = (struct i2c_tiny_usb *)adapter->algo_data;
  int retval;

//  printk("reading %d bytes\n", len);

  /* do control transfer */
  retval = usb_control_msg(dev->udev,
			   usb_rcvctrlpipe(dev->udev, 0),
			   cmd,
			   USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_IN,
			   value, index,
			   data,	
			   len,
			   2000);

//  printk("data = %x %x %x %x\n", 
//	 *(char*)data,*(char*)(data+1),*(char*)(data+2),*(char*)(data+3));

  return retval;
}

static int usb_write(struct i2c_adapter *adapter, int cmd, 
		    int value, int index, void *data, int len) {
  struct i2c_tiny_usb *dev = (struct i2c_tiny_usb *)adapter->algo_data;
  int retval;

  /* do control transfer */
  retval = usb_control_msg(dev->udev,
			   usb_sndctrlpipe(dev->udev, 0),
			   cmd,
			   USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			   value, index,
			   data,	
			   len,
			   2000);

  return retval;
}

static void i2c_tiny_usb_free(struct i2c_tiny_usb *dev) {	
  usb_put_dev(dev->udev);
  kfree (dev);
}

static int i2c_tiny_usb_probe(struct usb_interface *interface, 
			 const struct usb_device_id *id) {
  struct i2c_tiny_usb *dev = NULL;
  int retval = -ENOMEM;
  u16 version;
  
  dbg("probing usb device");
  
  /* allocate memory for our device state and initialize it */
  dev = kmalloc(sizeof(*dev), GFP_KERNEL);
  if (dev == NULL) {
    err("Out of memory");
    goto error;
  }

  /* clear memory */
  memset(dev, 0, sizeof(*dev));

  dev->udev = usb_get_dev(interface_to_usbdev(interface));
  dev->interface = interface;
  /* save our data pointer in this interface device */
  usb_set_intfdata(interface, dev);

  version = le16_to_cpu(dev->udev->descriptor.bcdDevice);
  info("version %x.%02x found at bus %03d address %03d", 
       version>>8, version&0xff, 
       dev->udev->bus->busnum, dev->udev->devnum);
   
  /* setup i2c adapter description */
  dev->i2c_adap.owner	  = THIS_MODULE;
  dev->i2c_adap.class	  = I2C_CLASS_HWMON;
  dev->i2c_adap.algo	  = &usb_algorithm;
  dev->i2c_adap.algo_data = dev;
  sprintf(dev->i2c_adap.name, "i2c-tiny-usb at bus %03d device %03d", 
	  dev->udev->bus->busnum, dev->udev->devnum);

  /* set delay value */
  if(!delay) delay = DEFAULT_DELAY;
  if(usb_write(&(dev->i2c_adap), CMD_SET_DELAY, 
	       le16_to_cpu(delay), 0, NULL, 0) != 0) {
    err("failure setting delay to %dus", delay);
    retval = -EIO;
    goto error;
  }
  
  /* and finally attach to i2c layer */
  i2c_add_adapter(&(dev->i2c_adap));

  /* inform user about successful attachment to i2c layer */
  dev_info(&dev->i2c_adap.dev, "connected i2c-tiny-usb device\n");
  
  return 0;
  
 error:

  if(dev)
    i2c_tiny_usb_free(dev);
  
  return retval;
}

static void i2c_tiny_usb_disconnect(struct usb_interface *interface)
{
  struct i2c_tiny_usb *dev = usb_get_intfdata(interface);
  
  i2c_del_adapter(&(dev->i2c_adap));
  
  usb_set_intfdata(interface, NULL);
  
  i2c_tiny_usb_free(dev);
  
  dbg("disconnected");
}

static struct usb_driver i2c_tiny_usb_driver = {
  .name =	"i2c-tiny-usb",
  .probe =	i2c_tiny_usb_probe,
  .disconnect =	i2c_tiny_usb_disconnect,
  .id_table =	i2c_tiny_usb_table,
};

static int __init usb_i2c_tiny_usb_init(void)
{
  int result;
  
  info(DRIVER_DESC " " DRIVER_VERSION " of " __DATE__);

  /* register this driver with the USB subsystem */
  result = usb_register(&i2c_tiny_usb_driver);
  if (result)
    err("usb_register failed. Error number %d", result);
  
  return result;
}

static void __exit usb_i2c_tiny_usb_exit(void)
{
  /* deregister this driver with the USB subsystem */
  usb_deregister(&i2c_tiny_usb_driver);
}

module_init (usb_i2c_tiny_usb_init);
module_exit (usb_i2c_tiny_usb_exit);

/* ----- end of usb layer ---------------------------------------------- */

module_param(delay, int, 0);

MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("GPL");
