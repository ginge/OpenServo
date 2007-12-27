/***************************************************************************
 *   Copyright (C) 2006 by Barry Carter   *
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


#ifndef _BEZIERWIDGET_H_
#define _BEZIERWIDGET_H_

#include "bezierwidgetbase.h"
#include "graph.h"
#include "osif.h"
#include <qtimer.h>
//#include <qscrollarea.h>
#include <iostream>
#include <sstream>
#include <string>

class OSIFcurve
{
public:
	int Td;
	int Pos;
	int Tin;
	int Tout;
private:

};



using namespace std;
class BezierWidget : public BezierWidgetBase
{
    Q_OBJECT

public:
    BezierWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~BezierWidget();
    /*$PUBLIC_FUNCTIONS$*/
	OSIF *osifIO;			// pointer to the dll handle
	OSIFcurve curveList[65535];	// curve buffer TODO define size

public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void setLabelXY(int x, int y);
    virtual void timerDone();
    virtual void BtnSimPause();
    virtual void BtnSimStop();
    virtual void BtnSimStart();
    virtual void servoMinPosChange(const QString &string);
    virtual void servoMaxPosChange(const QString &String);
    virtual void gridTimeChange(const QString &String);
    virtual void BtnGridDel();
    virtual void BtnGridAdd();
    virtual void FormBezierScroll(int val);
    virtual void UpdateRegisters();
    virtual void servoChanged(QListViewItem*);
    virtual void enableScroll(bool val);
    virtual void infoUpdate(QString &label);
    virtual void BtnOSsendCurve();
    virtual void curveProcessor(void);
    virtual void readProcessor(void);
protected:
    /*$PROTECTED_FUNCTIONS$*/
	
	//void paintEvent( QPaintEvent * );
protected slots:
    /*$PROTECTED_SLOTS$*/
	
private:
	int timerval;			//stores the simulation counter
	QPixmap servoPixmap;		//the servo pixmap handle
	QTimer *timer;			//hande to the timer object
	void servoSimRotate(float rot);	//rotate the servo pixmap by rot degrees

	int16_t floatToFixed(float a);	//floating point number to signed 6:10 fixed point
	float fixedToFloat(int16_t a);	// 6:10 fixed to float

	int servo;			//current initialised servo
	QTimer *curveTimer;		//pointer to the curve timer
	int curveState;			//current curve event state
	int curveCount;			//number of active curves
	int curveIndex;			//index along the curve playback

	QTimer *readPosTimer;
	int timePos;

};

#endif

