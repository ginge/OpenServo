#!/usr/bin/python
# ======================================================================
# lsusb.py - Python version of the lsusb command
#
# Copyright 2006-2008 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

from libusb import *

usb_init()
usb_find_busses()
usb_find_devices()
bus = usb_get_busses()
while bus:
	dev = bus.devices
	while dev:
		d = dev.descriptor
		print "Bus %s" % dev.bus.dirname,
		print "Device %s:" % dev.filename,
		print "ID %04x:%04x" % (d.idVendor, d.idProduct),
		h = usb_open(dev)
		str = []
		for i in [d.iManufacturer, d.iProduct, d.iSerialNumber]:
			s = usb_string(h, i)
			if s:
				str.append(s)
		if str:
			print "[%s]" % ', '.join(str),
		usb_close(h)
		print
		dev = dev.next
	bus = bus.next
