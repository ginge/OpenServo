#!/usr/bin/python
# ======================================================================
# usbtiny.py - USBtiny interface class
#
# Copyright (C) 2006 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

import sys
from libusb import *

USBTINY_ECHO	= 0	# echo test

class USBtiny:
	def __init__(self, vendor, product):
		self.handle = None
		usb_init()
		usb_find_busses()
		usb_find_devices()
		dev = None
		bus = usb_get_busses()
		while bus:
			dev = bus.devices
			bus = bus.next
			while dev:
				d = dev.descriptor
				if d.idVendor == vendor \
				and d.idProduct == product:
					bus = None
					break
				dev = dev.next
		if not dev:
			print 'Cannot find USB device %04x/%04x' % \
				(vendor, product)
			sys.exit(1)
		self.handle = usb_open(dev)
		if not self.handle:
			print 'Cannot open USB device %04x/%04x' % \
				(vendor, product)
			sys.exit(1)
	def __del__(self):
		if self.handle:
			usb_close(self.handle)
	def control_in(self, request, val, index, size):
		return usb_control_in(self.handle,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			request, val, index, size, 500)
	def control_out(self, request, val, index, data):
		return usb_control_out(self.handle, 
			USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			request, val, index, data, 500)
	def echo_test(self):
		for v in [0x0000, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
		          0xffff, 0xaaaa, 0xfffe]:
			for i in range(16):
				r = self.control_in(USBTINY_ECHO, v, 0, 8)
				if not r:
					print >> sys.stderr, \
						"error: no response"
					sys.exit(1)
				print "%04x" % v,
				w = (ord(r[3]) << 8) | ord(r[2])
				if w != v:
					print
					print >> sys.stderr, \
						"error: %04x sent," % v, \
						"%04x received\n" % w
					return
				v = ((v << 1) | (v >> 15)) & 0xffff;
			print

def dump(addr, data):
	if not data:
		return
	for i in range(len(data)):
		if (i % 16) == 0:
			if i > 0:
				print
			print "%04x:" % (addr + i),
		print "%02x" % ord(data[i]),
	print
