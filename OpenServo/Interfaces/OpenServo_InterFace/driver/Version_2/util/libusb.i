// ======================================================================
// SWIG interface description for libusb
//
// Copyright (C) 2006 Dick Streefland
// Copyright (C) 2006 Stefan Siegl <stesie@brokenpipe.de>
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

%module libusb

%{

#include "usb.h"

void	usb_control_in ( usb_dev_handle* dev, int requesttype,
	                 int request, int value, int index,
	                 char* inbuf, int* psize, int timeout )
{
	int	r;

	Py_BEGIN_ALLOW_THREADS
	r = usb_control_msg( dev, USB_ENDPOINT_IN | requesttype, request,
	                     value, index, inbuf, *psize, timeout );
	Py_END_ALLOW_THREADS
	*psize = (r < 0 ? 0 : r);
}

int	usb_control_out ( usb_dev_handle* dev, int requesttype,
	                  int request, int value, int index,
	                  char* outbuf, int sz_outbuf, int timeout )
{
	int	r;

	Py_BEGIN_ALLOW_THREADS
	r = usb_control_msg( dev, USB_ENDPOINT_OUT | requesttype, request,
	                     value, index, outbuf, sz_outbuf, timeout );
	Py_END_ALLOW_THREADS
	return r;
}

void	usb_string ( usb_dev_handle* dev, int index, char* strbuf )
{
	int	r;

	Py_BEGIN_ALLOW_THREADS
	r = usb_get_string_simple( dev, index, strbuf, 255 );
	Py_END_ALLOW_THREADS
	if	( r < 0 )
	{
		r = 0;
	}
	strbuf[r] = '\0';
}

%}

%include cstring.i

%cstring_output_withsize( char* inbuf, int* psize );
void	usb_control_in ( usb_dev_handle *dev, int requesttype,
	                 int request, int value, int index,
	                 char* inbuf, int* psize, int timeout );

%cstring_input_binary( char* outbuf, int sz_outbuf )
int	usb_control_out ( usb_dev_handle *dev, int requesttype,
	                  int request, int value, int index,
	                  char* outbuf, int sz_outbuf, int timeout );

%cstring_bounded_output( char* strbuf, 255 );
void	usb_string ( usb_dev_handle* dev, int index, char* strbuf );

typedef unsigned short u_int16_t;
typedef unsigned char u_int8_t;

%include usb.h
