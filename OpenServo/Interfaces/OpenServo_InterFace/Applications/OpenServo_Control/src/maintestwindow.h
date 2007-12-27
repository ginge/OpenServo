/***************************************************************************
 *   Copyright (C) 2007 by Barry Carter   *
 *   barry.carter@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MAINTESTWINDOW_H
#define MAINTESTWINDOW_H

#include "mainwin.h"
#include "aboutclass.h"



class mainTestWindow: public testMainWin {
Q_OBJECT
public:
    mainTestWindow(QWidget *parent = 0, const char *name = 0);
    ~mainTestWindow();
public slots:
    virtual void readServo();
    virtual void adapterSelectChange(QListViewItem *selitem);
    virtual void genericReadData();
    virtual void genericWriteData();
    virtual void writeServo();
    virtual void timerIntervalChange(int timerval);
    virtual void servoSelectChange(QListViewItem *selItem);
    virtual void liveDataClick();
    virtual void requestVoltage();
    virtual void flashFileLoad();
    virtual void commandFlash();
    virtual void commandDefault();
    virtual void commandRestore();
    virtual void commandSave();
    virtual void commandPWM();
    virtual void commandReboot();
    virtual void scanBus();
    virtual void aboutClicked();
    virtual void logBoxClear(int,int);

private:
	int writeData( int adapter, int servo, int addr, char *val, size_t len );
	int readData(int adapter, int servo, int addr, unsigned char *buf, size_t len);
	int hexarrToInt(unsigned char *ncurrent);
	int parseData( char *argv[], int count, int len, char *data );
	int parseOption( char *arg );
	void logPrint( char *logData);
	void readPids();
	int setPosOut;
	int setupPOut;
	int setupIOut;
	int setupDOut;
	int setupSMaxOut;
	int setupSMinOut;

	//back buffer variables for comparison.
	int bckSetPosOut;
	int bckSetupPOut;
	int bckSetupIOut;
	int bckSetupDOut;
	int bckSetupSMaxOut;
	int bckSetupSMinOut;

	/*Typedef the functions*/
	typedef int  (*OSIF_initfunc   )();
	typedef int  (*OSIF_deinitfunc )();
	typedef int  (*OSIF_writefunc  )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
	typedef int  (*OSIF_readfunc   )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
	typedef int  (*OSIF_reflashfunc)(int adapter, int servo, int bootloader_addr, char *filename);
	typedef int  (*OSIF_commandfunc)(int adapter, int servo, unsigned char command);
	typedef int  (*OSIF_scanfunc   )(int adapter, int devices[], int *dev_count);
	typedef bool (*OSIF_probefunc  )(int adapter, int servo);
	typedef int  (*OSIF_get_adapter_namefunc  )(int adapter, char *name);
	typedef int  (*OSIF_get_adapter_countfunc  )(void);

		
	/*A pointer to a function*/
	OSIF_deinitfunc OSIF_deinit;
	OSIF_initfunc OSIF_init;
	OSIF_writefunc OSIF_write;
	OSIF_readfunc OSIF_read;
	OSIF_reflashfunc OSIF_reflash;
	OSIF_scanfunc OSIF_scan;
	OSIF_probefunc OSIF_probe;
	OSIF_commandfunc OSIF_command;
	OSIF_get_adapter_namefunc OSIF_get_adapter_name;
	OSIF_get_adapter_countfunc OSIF_get_adapter_count;

	int adapters[128];
	int adapterCount;

	void * libhandle;// handle to the shared lib when opened

	int devices[128];
	int devCount;

	bool OSIFinit;
	bool servoPWMenabled;
	QString fileToFlash;
	//timer to read the servo
	QTimer *readTimer;
	int readInterval;
	int servo;
	int adapter;
	QString loggingData;
	aboutBoxWidget *theAboutWin;
};

#endif
