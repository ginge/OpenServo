#!/usr/bin/python
# ======================================================================
# crc16.py - Prototype implementations of CRC16 calculation
#
# Copyright (C) 2006 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

import sys

polynomial = 0xA001	# X^16 + X^15 + X^2 + 1 - lower 16 bits, reversed

def crc_update(crc, value, bits):
	for bit in range(bits):
		xor = (crc ^ (value >> bit)) & 1
		crc >>= 1
		if xor:
			crc ^= polynomial
	return crc

def crc1(crc, byte):
	return crc_update(crc, byte, 8)

def crc4(crc, byte):
	crc = (crc >> 4) ^ tab4[(crc ^ byte) & 0xf]
	crc = (crc >> 4) ^ tab4[(crc ^ (byte >> 4)) & 0xf]
	return crc

def crc8(crc, byte):
	return (crc >> 8) ^ tab8[(crc ^ byte) & 0xff]

def crc_file(file, crcfunc):
	crc = 0xffff
	for byte in open(file).read():
		crc = crcfunc(crc, ord(byte))
	crc ^= 0xffff
	print "%02x %02x %s" % (crc & 0xff, crc >> 8, file)

tab4 = [crc_update(i, 0, 4) for i in range(2**4)]
tab8 = [crc_update(i, 0, 8) for i in range(2**8)]
for file in sys.argv[1:]:
	crc_file(file, crc1)
	crc_file(file, crc4)
	crc_file(file, crc8)
