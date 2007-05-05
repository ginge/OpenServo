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


#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qcheckbox.h>
#include <qscrollbar.h>
#include <qscrollview.h>
#include <qlistview.h>
#include <qstring.h>

#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#include "qservo.h"
#include "graph.h"
#include "bezierwidget.h"
#include "qbezier.h"
#include "qmessagebox.h"


using namespace std;
template<typename T> std::string toString(const T& x) {
	std::ostringstream oss;
	oss << x;
	return oss.str();
}

BezierWidget::BezierWidget(QWidget* parent, const char* name, WFlags fl)
        : BezierWidgetBase(parent,name,fl)
{
	QListViewItem *listItem;

	//initialise the timer
        timer = new QTimer( this );
	//connect the timer to the timeout function
        connect( timer, SIGNAL(timeout()), this, SLOT(timerDone()) );

	//draw the servo sim image checks png then bmp
	if (!servoPixmap.load("servotoppng.png")) { !servoPixmap.load("servotoppng.bmp"); }
	//set the pixmap to the widget
	ServoSim->setPixmap(servoPixmap);
	//initialise the rotation matrix
	servoSimRotate(0.0);
	//initialise the variables from the form
	servoMaxPosChange(FormMaxPos->text());
	servoMinPosChange(FormMinPos->text());
	//disable sorting on the register output lists
	listRegOut->setSorting( -1);
	listRegOutHex->setSorting( -1);

	// See if w can locate an OSIF on the USB bus
	osifIO = new OSIF();
	printf( "isinit %d\n", osifIO->isOSIFinitialised());
	if (osifIO->isOSIFinitialised() != false )
	{
		// Scan for devices
		if ( osifIO->scanBus() >0 )
		{
			osifIO->scanDevices(0);
			printf( "OSIF count %d\n", osifIO->OpenServoCount);	
			for( int i=0; i<osifIO->OpenServoCount; i++)
			{
				// Add the servo to the list
				listItem = new QListViewItem( servoList, QString().sprintf("0x%-2x", osifIO->OpenServo[i]) );
			}
			// Select the last item in the list (the top one)
			servoList->setSelected(listItem, true);
			
			// Create a new timer for the curve send buffer.
			curveTimer = new QTimer( this );
			connect( curveTimer, SIGNAL(timeout()), SLOT(curveProcessor()) );
			
			// Create a timer for the input feedback
			readPosTimer = new QTimer( this );
			connect( readPosTimer, SIGNAL(timeout()), SLOT(readProcessor()) );
	
			timePos = 0;
		}
	}
}

BezierWidget::~BezierWidget()
{
	osifIO->deinit();
}

/*$SPECIALIZATION$*/

/*
 * Set the label on the status button
 */
void BezierWidget::setLabelXY(int x, int y)
{
	QString newlabel=QString("%1,%2")
			.arg(x) .arg(y);
	label->setText(newlabel);
}

/*
 * if the max size of the grid is changed, then the graph is updated and 
 * the scrollbar updated if enabled
 */
void BezierWidget::gridTimeChange(const QString &String)
{
	int t = String.toInt();
	if (t>30000) { return; } //too big 

	QPoint ptd, ptl;
	QPainter DC;

	qbezier->maingraph.setGridMax(t);	//write the new grid

	qbezier->reCalc = true;			// request a reclulation and ten do a redraw to flush the new viewport data
	qbezier->draw();			// for when we reclaulate the slider position below

	// Start by creating a new view with applied matrix from the grid
	// This is then used to calculate the translated position
	DC.begin(qbezier);
	DC.setWindow(0, 0, qbezier->maingraph.w, qbezier->maingraph.h);
	DC.setViewport(qbezier->maingraph.vp);
	DC.setWorldMatrix(qbezier->maingraph.worldMatrix);
	
	// Convert the new width into widget coordinates. This will be larger than the widget
	ptl=DC.xForm(QPoint(qbezier->maingraph.lx+2*qbezier->maingraph.ref.x(), qbezier->maingraph.ly));
	DC.end();

	//subtract the new calculated widget width from the actual widget width
	int base = ptl.x();
	int scaled = base-qbezier->width();
	if (scaled<0) {scaled=0;}
	//see if scrolling is disabled
	if (qbezier->maingraph.isScaled)
	{
		bezierScroll->setEnabled(false);
		bezierScroll->setMaxValue(0);
	}
	else
	{
		//set the scrollbar width
		bezierScroll->setEnabled(true);
		bezierScroll->setMaxValue((int)((float)scaled));
	}
	qbezier->reCalc = true; 		//request a redraw
	qbezier->update();
}

/*
 * Set the graph max servo pos
 */
void BezierWidget::servoMaxPosChange(const QString &String)
{
	int t = String.toInt();
	if (t>1024) { t = 1024; } //too big
	qbezier->maingraph.setServoPosMax(t);  //write the new grid
	qbezier->reCalc = true; 		//request a redraw
	qbezier->update();
}

/*
 * Set the servo min pos
 */
void BezierWidget::servoMinPosChange(const QString &String)
{
	int t = String.toInt();
	if (t>1024) { t=1024; } //too big
	qbezier->maingraph.setServoPosMin(t);  //write the new grid
	qbezier->reCalc = true; 		//request a redraw
	qbezier->update();
}

/*
 * Paused pressed. stop the timer and toggle the on off states
 */
void BezierWidget::BtnSimPause()
{
	if (timer->isActive() )
	{
		timer->stop(); // stop 10ms timer
		FormBtnSimPause->setEnabled(FALSE);
		FormBtnSimStart->setEnabled(TRUE);
	}
}

/*
 * Stop pressed. Reset the timer to 0
 */
void BezierWidget::BtnSimStop()
{
	timer->stop(); // 10ms timer
	FormBtnSimStart->setEnabled(TRUE);
	qbezier->maingraph.tick = 0;
	timerval = 0;
	qbezier->update(); //flush tick pointer
}

/*
 * Start pressed. Resumes if in pause mode
 */
void BezierWidget::BtnSimStart()
{
	timer->start(10); // 10ms timer
	FormBtnSimStart->setEnabled(FALSE);
	//A quick check to see if the motion is paused
	if (FormBtnSimPause->isEnabled())
	{
		timerval = 0;
		qbezier->maingraph.tick = 0;
	}
	else
	{
		FormBtnSimPause->setEnabled(TRUE);
	}
}

/*
 * Timer widget. Calculates all paramers for simulation of the curve.
 */
void BezierWidget::timerDone()
{
	//bounds checking the time widget the last curve
	if ( timerval >= qbezier->maingraph.gridMaxx || timerval >=qbezier->maingraph.cPoint[qbezier->maingraph.nodeCount-2].graphPointX )
	{
		//If we are not looping bail
		if (!ServoSimLoop->isChecked())
		{
			//stop simulation and enable all buttons
			FormBtnSimStart->setEnabled(TRUE);
			timer->stop(); // 10ms timer
			timerval = (int)qbezier->maingraph.cPoint[qbezier->maingraph.nodeCount-2].graphPointX;
		}
		else
			timerval = 0;

		qbezier->maingraph.tick = 0;
		qbezier->update();		//do one last update to flush the timer points
	}
	else
	{

		//increment the timer
		timerval+=10;
		qbezier->maingraph.tick = timerval;

	}
	
	//update the time label
	QString newlabel=QString("%1ms")
		.arg(timerval);
	FormTimePos->setText( newlabel);
	qbezier->update();
	//get the currect tick position and update the display
	float x,dx;
	qbezier->maingraph.getPosSpdVal(&x,&dx, timerval);

	//clip to max servo range
	if (x > 1023 ) {x=1023;}
	//clip to designated servo range
	if (x > FormMaxPos->text().toInt() ) {x=FormMaxPos->text().toInt();}
	if (x < FormMinPos->text().toInt() ) {x=FormMinPos->text().toInt();}
	newlabel=QString("%1")
		.arg(x);
	FormMotorPos->setText( newlabel);
	//update the current motor speed.
	newlabel=QString("%1")
		.arg(dx);
	FormMotorSpeed->setText( newlabel);
	//update the servo rotation simulation
	servoSimRotate((float)x);	
}

/*
 * Delete a control point on the graph
 */
void BezierWidget::BtnGridDel()
{
	//check to see if we are alreasy in delete mode. If we are then toggle back to the nor mal mode.
	if (qbezier->mode == M_DEL_POINT )
	{
		qbezier->mode = 0;
		return;
	}
	//Set the mode to delete
	qbezier->mode = M_DEL_POINT;
	//call for a list redraw for servo params
	UpdateRegisters();
	qbezier->reCalc = true; //draw the new graph
	qbezier->update();
	//update the display to show delete mode
	QString string("delete");
	emit infoUpdate(string);
}

/*
 * Add a new control point
 */
void BezierWidget::BtnGridAdd()
{
	//check to make sure we dont overflow the maximum number of nodes allowed.
	if (qbezier->maingraph.nodeCount == MAX_NODES) 
	{
		QString string("max!!");
		emit infoUpdate(string);
		return;
	}
	//add more graph if we need to
	if ( (qbezier->maingraph.gridMaxx - qbezier->maingraph.cPoint[qbezier->maingraph.nodeCount-2].graphPointX ) < 100.0 ) 
		{ qbezier->maingraph.gridMaxx += 100.0; }
	FormGridWidth->setText(QString().sprintf("%d",(int)qbezier->maingraph.gridMaxx));
	//add the point
	qbezier->maingraph.addControlPoint();
	//redraw the servo param list
	UpdateRegisters();
	qbezier->reCalc = true; //draw our new point
	qbezier->update();
}

/*
 * Rotate the servo pixmap by x degrees
 */
void BezierWidget::servoSimRotate(float x)
{
	//set the full servo bounds here.
//	float servoResolution = 270.0;
	float servoResolution = (FormMaxPos->text().toInt()/(1024.0/360.0))-((FormMinPos->text().toInt()/(1024.0/360.0)));
	float rot;
	if (ReverseSeek->isChecked()) { 
		rot = 180.0-((360-servoResolution)/2)+(servoResolution/(1024.0/x));
	} else {
		rot = 180.0+((360-servoResolution)/2)+(servoResolution/(1024.0/x));
	}
	//construct a temporary image for the motor simulation image.
	int maxDim = QMAX(servoPixmap.width(), servoPixmap.height());
	QPixmap tempPixmap(maxDim, maxDim);

	tempPixmap.fill(Qt::white);	
	QPainter p(&tempPixmap);
	//QPainter p(ServoSim);
	QWMatrix wm;

	wm.translate(maxDim / 2, maxDim / 2); // really center
	//calculate the rotation matrix based on current position

	wm.rotate(rot);
	p.setWorldMatrix(wm);
	//apply to the tempory pixmap
	p.drawPixmap(-servoPixmap.width() / 2, -servoPixmap.height() / 2, servoPixmap);	
	//update the display
	//bitBlt(ServoSim,0,0,&tempPixmap);
	ServoSim->servoPixmap = (tempPixmap);
	ServoSim->update();
}

/*
 * Scroll the graph window by val units
 */
void BezierWidget::FormBezierScroll(int val)
{
	qbezier->maingraph.viewport = val;
	qbezier->reCalc = true;
	qbezier->update();
}

/* 
 * Enable the scroll bar.
 */
void BezierWidget::enableScroll(bool val)
{
	if (!val) { qbezier->maingraph.isScaled = true; }
	else { qbezier->maingraph.isScaled = false; }

	gridTimeChange( FormGridWidth->text());
	qbezier->update();
}

/*
 * Calculates the OpenServo output windows, and converts to 6:10 hex where needed
 */
void BezierWidget::UpdateRegisters()
{
	//redraw the entire listview and insert the new openservo control values
	//0x18 & 0x19 - unsigned 16bit time delta from previous keypoint 
 	//0x1A & 0x1B - unsigned 16bit position 
 	//0x1C & 0x1D - signed fixed point 6:10 bit in-tangent 
 	//0x1E & 0x1F - signed fixed point 6:10 bit out-tangent 
	// duration = c1-c0
	// position = cpointx
	// tan in = 
	// tan out = 
	int n;
	curveCount = (qbezier->maingraph.nodeCount-2)/2;
	//clear the list
	listRegOut->clear();
	listRegOutHex->clear();
	//build a list of curves and points
	//for(n=2;n<qbezier->maingraph.nodeCount;n+=2)
	int i = curveCount;
	// Iterate over the graph points and calculate basis for each
	for(n=qbezier->maingraph.nodeCount-2;n>=0;n-=2)
	{
		int duration = (int)qbezier->maingraph.cPoint[n].graphPointX - (int)qbezier->maingraph.cPoint[n-2].graphPointX;
		int tinlnx = (int)qbezier->maingraph.cPoint[n+1].graphPointX - (int)qbezier->maingraph.cPoint[n].graphPointX;
		int tinlny = (int)qbezier->maingraph.cPoint[n+1].graphPointY - (int)qbezier->maingraph.cPoint[n].graphPointY;
		int toutlnx = (int)qbezier->maingraph.cPoint[n+1].graphPointX - (int)qbezier->maingraph.cPoint[n].graphPointX;
		int toutlny = (int)qbezier->maingraph.cPoint[n+1].graphPointY - (int)qbezier->maingraph.cPoint[n].graphPointY;
		float tanin = (float)tinlny/(float)tinlnx;
//		float tanout = (float)toutlny/(float)toutlnx;
		float tanout = tanin;

		// clip the values to sane ones.
		if (tanin > 31.0) { tanin = 31.0; } 
		if (tanin < -31.0) { tanin = -31.0; } 

		if (tanout > 31.0) { tanout = 31.0; } 
		if (tanout < -31.0) { tanout = -31.0; } 
	
		//update the list view
        	(void) new QListViewItem( listRegOut, QString("%1").arg(duration),
									QString("%1").arg((int)qbezier->maingraph.cPoint[n].graphPointY),
									QString("%1").arg(tanin),
									QString("%1").arg(tanout)
								);
        	(void) new QListViewItem( listRegOutHex, QString().sprintf("0x%04x",duration),
									QString().sprintf("0x%04x",(int)qbezier->maingraph.cPoint[n].graphPointY),
									QString().sprintf("0x%04x", ((int16_t)floatToFixed(tanin))&0x0000FFFF),
									QString().sprintf("0x%04x", ((int16_t)floatToFixed(tanout))&0x0000FFFF)
								);
		// put the information into the curve queue.
		curveList[i].Td = duration;
		curveList[i].Pos = (int)qbezier->maingraph.cPoint[n].graphPointY;
		curveList[i].Tin = ((int16_t)floatToFixed(tanin));
		curveList[i].Tout = ((int16_t)floatToFixed(tanout));
		i--;
	}
}

float BezierWidget::fixedToFloat(int16_t a)
// 6:10 signed fixed point to float.
{
     return ((float) a) / 1024.0;
}


int16_t BezierWidget::floatToFixed(float a)
// Float to 6:10 signed fixed.
{
     return (int16_t) (a * 1024.0);
}

/*
 * Update the status window.
 */
void BezierWidget::infoUpdate(QString &label)
{
	lblDebug->setText(label);
}

/*
 * Send the first part of the curve, and set up for the rest to send in a timer
 */
void BezierWidget::BtnOSsendCurve()
{
	int iter;
	if (curveCount < 8)
	{
		iter = curveCount;
	}
	else
	{
		iter = 8;
	}
	char msg[10];
	if( servo >0 )
	{
//		curveTimer->setEnabled(TRUE);
		osifIO->writeData(0, servo, TWI_CMD_CURVE_MOTION_ENABLE, msg, 1);
		for (int i = 0; i<= iter; i++)
		{
			printf("servo 0x%02x\n", servo);
			// send the curve points in a single message
			msg[0] = (curveList[i].Td>>8)&0x00FF;
			msg[1] = (curveList[i].Td)&0x00FF;
			msg[2] = (curveList[i].Pos>>8)&0x00FF;
			msg[3] = (curveList[i].Pos)&0x00FF;
			msg[4] = (curveList[i].Tin>>8)&0x00FF;
			msg[5] = (curveList[i].Tin)&0x00FF;
			msg[6] = (curveList[i].Tout>>8)&0x00FF;
			msg[7] = (curveList[i].Tout)&0x00FF;
	
			osifIO->writeData(0, servo, REG_CURVE_DELTA_HI, msg, 8);
			osifIO->writeData(0, servo, TWI_CMD_CURVE_MOTION_APPEND, msg, 1);
		}
		// if there are more curves pending, hand off the the curve queue processor
		if ( curveCount > 7 )
		{
			// wait enough time to clear 5 queue elements
			curveIndex = 7;
			printf("start timer %d\n", curveList[0].Td + curveList[1].Td + curveList[2].Td + curveList[3].Td + curveList[4].Td);
			curveTimer->start((curveList[0].Td + curveList[1].Td + curveList[2].Td + curveList[3].Td + curveList[4].Td + curveList[5].Td));
		}
	}
	qbezier->maingraph.readResolution = 100;
	readPosTimer->start(qbezier->maingraph.readResolution);
	qbezier->maingraph.posHistoryCount = 0;
}

/*
 * User clicked on a different servo. Set the global selected servo parameter.
 */
void BezierWidget::servoChanged(QListViewItem* listItem)
{
	int byte;
	bool ok;
	QString qarg = QString((char *)listItem->text(0).ascii());
	//automatically convert the type from string int/hex to int
	byte = qarg.toInt(&ok, 0);

	servo = byte;
}

/*
 * Timer event that send a new set of curves before waiting for the next timeout
 * The timeout is set to the value of the duration of the new curves (minus a bit)
 */
void BezierWidget::curveProcessor(void)
{
	int iter=0;
	char msg[10];
	unsigned char bufferleft[1];
	bool done=false;

	printf ("processing curves\n");

	// check to see how much buffer space we have
	osifIO->readData(0, servo, REG_CURVE_BUFFER, bufferleft, 1);
	printf( "buffer left %d\n", bufferleft[0]);
	if (curveIndex + bufferleft[0] < curveCount )
		iter = curveIndex+bufferleft[0];
	else
	{
		iter = curveCount-curveIndex;
		curveTimer->stop();
		done = true;
	}
	
	//send the next 4 and set the timer to wait for 4 more.
	for (int i = curveIndex; i< iter; i++)
	{
		printf("curve %d\n", curveIndex);
		msg[0] = (curveList[i].Td>>8)&0x00FF;
		msg[1] = (curveList[i].Td)&0x00FF;
		msg[2] = (curveList[i].Pos>>8)&0x00FF;
		msg[3] = (curveList[i].Pos)&0x00FF;
		msg[4] = (curveList[i].Tin>>8)&0x00FF;
		msg[5] = (curveList[i].Tin)&0x00FF;
		msg[6] = (curveList[i].Tout>>8)&0x00FF;
		msg[7] = (curveList[i].Tout)&0x00FF;

		osifIO->writeData(0, servo, REG_CURVE_DELTA_HI, msg, 8);
		osifIO->writeData(0, servo, TWI_CMD_CURVE_MOTION_APPEND, msg, 1);
		curveIndex++;
	}
	if (!done)
	{
		curveTimer->stop();
		curveTimer->start((curveList[curveIndex+0].Td + curveList[curveIndex+1].Td + curveList[curveIndex+2].Td + curveList[curveIndex+3].Td)/2);
	}
	else
		printf("All Done!\n");
}

void BezierWidget::readProcessor(void)
{
	unsigned char data[6];
	// if done stop timer
	if (timePos > qbezier->maingraph.cPoint[qbezier->maingraph.nodeCount-2].graphPointX )
	{
		readPosTimer->stop();
		timePos = 0;
	}
	timePos +=qbezier->maingraph.readResolution;
	osifIO->readData(0, servo, REG_POSITION_HI, data, 6);
	qbezier->maingraph.posHistory[qbezier->maingraph.posHistoryCount] = (data[0]<<8)|data[1];	//set the history in the position buffer
	qbezier->maingraph.curHistory[qbezier->maingraph.posHistoryCount] = (data[4]<<8)|data[5];	// current sense buffer
	printf( "%d\n", qbezier->maingraph.posHistory[qbezier->maingraph.posHistoryCount]);
	qbezier->maingraph.posHistoryCount++;
	qbezier->reCalc = true; //draw the new graph
	qbezier->update();			// for when we reclaulate the slider position
}

//#include "bezierwidget.moc"

