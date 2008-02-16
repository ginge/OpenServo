// ======================================================================
// SWIG interface description for libusb
//
// Copyright 2006-2008 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

%module libusb

%{

#include "usb.h"
#include <errno.h>

static	void	check_status ( int status )
{
	if	( status < 0 )
	{
		fprintf( stderr, "libusb: %s\n", strerror( errno ) );
		exit( 1 );
	}
}

static	void	usb_control_in ( usb_dev_handle* dev, int requesttype,
                                 int request, int value, int index,
                                 char* inbuf, int* psize, int timeout )
{
	int	r;

	Py_BEGIN_ALLOW_THREADS
	r = usb_control_msg( dev, USB_ENDPOINT_IN | requesttype, request,
	                     value, index, inbuf, *psize, timeout );
	Py_END_ALLOW_THREADS
	check_status( r );
	*psize = r;
}

static	int	usb_control_out ( usb_dev_handle* dev, int requesttype,
                                  int request, int value, int index,
                                  char* outbuf, int bufsize, int timeout )
{
	int	r;

	Py_BEGIN_ALLOW_THREADS
	r = usb_control_msg( dev, USB_ENDPOINT_OUT | requesttype, request,
	                     value, index, outbuf, bufsize, timeout );
	Py_END_ALLOW_THREADS
	check_status( r );
	return r;
}

static	void	usb_string ( usb_dev_handle* dev, int index, char* strbuf )
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
static	void	usb_control_in ( usb_dev_handle *dev, int requesttype,
				 int request, int value, int index,
				 char* inbuf, int* psize, int timeout );

%cstring_input_binary( char* outbuf, int bufsize )
static	int	usb_control_out ( usb_dev_handle *dev, int requesttype,
				  int request, int value, int index,
				  char* outbuf, int bufsize, int timeout );

%cstring_bounded_output( char* strbuf, 255 );
static	void	usb_string ( usb_dev_handle* dev, int index, char* strbuf );

typedef unsigned short uint16_t;
typedef unsigned short u_int16_t;
typedef unsigned char uint8_t;
typedef unsigned char u_int8_t;

%include usb.h
