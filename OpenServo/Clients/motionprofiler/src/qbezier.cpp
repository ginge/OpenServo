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
#include "qbezier.h"
#include <iostream>
#include <sstream>
#include <string>
#include "bezierwidget.h"
#include "qevent.h"
#include "graph.h"
using namespace std;

Qbezier::Qbezier(QWidget* parent = 0, const char* name = 0)
    : QLabel(parent, "", WResizeNoErase | WRepaintNoErase)
{
	//set the background mode of the widget to none. This is to stop the widget clearing the background before
	//repainting causing flicker
	setBackgroundMode(NoBackground);
	name =0;

	mode=0;
	pict.resize(size());
	pict.fill(Qt::white);
	
	//initial draw

	reCalc = true;
	draw();
}

/*
 * Catch the mouse move event for the graph widget.
 */
void Qbezier::mouseMoveEvent(QMouseEvent *event)
{
	QPoint ptd, ptl;
	QPainter DC;
	bool reDraw = false;

	// Start by creating a new view with applied matrix from the grid
	// This is then used to calculate the translated mouse position
	DC.begin(this);
	DC.setWindow(0, 0, maingraph.w, maingraph.h);
	DC.setViewport(maingraph.vp);
	DC.setWorldMatrix(maingraph.worldMatrix);
	// Convert the mouse xy into graph coordinates
	ptl=DC.xFormDev(QPoint(event->x(),event->y()));

	//set the public variable for mouse pos
	mouseX=maingraph.translateX((int)ptl.x());
	mouseY=maingraph.translateY(ptl.y());
	
	//set the widet XY values
	widMouseX=event->x();
	widMouseY=event->y();
	
	//update the label with the mouse position (in translated coordinates) to update the xy pos label
	emit setXY((int)mouseX, (int)mouseY);

	//check to see if the mouse if clicked.
	if (maingraph.nodeSelected != -1)
	{
		//make sure that we dont go past any other points.
		//constrain point between previous point and next
		if (!(maingraph.nodeSelected%2 != 0 )) { 		//are we a control point?
			//make sure that we are not the first point and that we are close the the last point
			if (mouseX < maingraph.cPoint[maingraph.nodeSelected-2].graphPointX+50.0 && ( maingraph.nodeSelected !=0))
			{
				//set the point to last point +50
				maingraph.cPoint[maingraph.nodeSelected].graphPointX = maingraph.cPoint[maingraph.nodeSelected-2].graphPointX+50.0;
				reCalc=true;		//request a recalculation
				update();
				return;
			}
			//make sure we are not the last control point and that we are below the next point 
			else if (mouseX > maingraph.cPoint[maingraph.nodeSelected+2].graphPointX-50.0 && ( maingraph.nodeSelected !=maingraph.nodeCount-2))
			{
				//clip to next point -50
				maingraph.cPoint[maingraph.nodeSelected].graphPointX = maingraph.cPoint[maingraph.nodeSelected+2].graphPointX-50.0;
				reCalc=true;		//request a recalculation
				update();
				return;

			}
		}
		//check to make sure we are not overflowing the set servo bounds, and that it is not a tangent line
		if (mouseY >maingraph.maxServoPos && !(maingraph.nodeSelected%2 != 0 )) {mouseY = maingraph.maxServoPos;}
		if (mouseY <maingraph.minServoPos && !(maingraph.nodeSelected%2 != 0 )) {mouseY = maingraph.minServoPos;}
		//set the mouse ordinates to widget coords
		maingraph.cPoint[maingraph.nodeSelected].graphPointX=mouseX;
		maingraph.cPoint[maingraph.nodeSelected].graphPointY=mouseY;
		reCalc=true;		//request a recalculation
		update();
		return;
	}
	DC.end();
	int n;
	//store a backup of the previously highlighed item.
	int prevhighlight = maingraph.nodeHighlight;
	static int curhighlight;
	//check the mouse position against all control points to see if we are hovering
	for( n = 0 ; n < maingraph.nodeCount; n++ )
	{
		int node;
		//check to see if the mouse is in a control point
		node = maingraph.cPoint[n].isControlPoint(-maingraph.vp.x()+ptl.x(), ptl.y());
		if (node >0)
		{
			//set the current highlighted node
			maingraph.nodeHighlight = n;
			//we dont need to request a redraw if the mouse is still in the same control point
 			if (curhighlight == n ) { break; }
			reDraw=true;
			curhighlight = n;
			break;
		}
		else
		{
			// set higlighed to false
			maingraph.nodeHighlight = -1;
		}
	}
	//reset the current highlighed if not set
	if (maingraph.nodeHighlight== -1 )
	{
		//nothing highlighted
		curhighlight = -1;	
	}
	//If we need to calculate the bezier, call recalc
	if ( reDraw || ( prevhighlight != -1 && maingraph.nodeHighlight == -1 ))
	{
		reCalc=true;
		update();
	}

}

void Qbezier::mousePressEvent(QMouseEvent *event)
{
    	event->accept(); // A no-op, but I will do it anyway!!!
	int n;
	QPoint ptd, ptl;
	QPainter DC;

	// Start by creating a new view with applied matrix from the grid
	// This is then used to calculate the translated mouse position
	DC.begin(this);
	DC.setWindow(0, 0, maingraph.w, maingraph.h);
	DC.setViewport(maingraph.vp);
	DC.setWorldMatrix(maingraph.worldMatrix);

	// Convert the mouse xy into graph coordinates
	ptl=DC.xFormDev(QPoint(event->x(),event->y()));

     // check to see if the left button is pressed
     if (event->button() == Qt::LeftButton) {
	for( n = 0 ; n < maingraph.nodeCount; n++ )
	{
		//if the mouse is clicked inside the node update the selected node		
		if (maingraph.cPoint[n].isControlPoint(-maingraph.vp.x()+ptl.x(), ptl.y()) == 1)
		{
			maingraph.nodeSelected = n;
			//check to see if we are removing a control point
			if ( mode == M_DEL_POINT )
			{
				//delete the control point n
				maingraph.delControlPoint(n);
				//reset the mode in the info window
				QString string(":");
				emit setInfo(string);
				//exit delete mode
				mode = 0;
			}
			break;
		}
	}
     }
}

void Qbezier::mouseReleaseEvent(QMouseEvent *event)
{
	//is left mouse button pressed?
     	if (event->button() == Qt::LeftButton)
	{
		//sanity check the tangent for length NOT WORKING
		if ( maingraph.nodeSelected%2 != 0) //check if even number therefore tangent
		{
			//TODO finish me: i'm supposed to clip the tangents to a fixed length
 			float lny = (maingraph.cPoint[maingraph.nodeSelected-1].graphPointY - mouseY );
 			float lnx = (maingraph.cPoint[maingraph.nodeSelected-1].graphPointX - mouseX );
			//fix a 0 tangent division by zero
 			if(lnx==0) { lnx = 1; }
 			if(lny==0) { lny = 1; }
		} 
		//delelect the node
		maingraph.nodeSelected = -1;
		//update the servo outout parameters
		emit calcparams();
	}
}

/*
 * Catch the resize event. Dispatch signal.
 */
void Qbezier::resizeEvent(QResizeEvent *)
{	
	pict.resize(size());
	reCalc = true;
	emit calcparams();
}

/*
 * On every repaint of the widget call  draw
 */
void Qbezier::paintEvent(QPaintEvent *)
{
	draw();
}

/* Draw the bezier widget only if a recalculation is requested. This saves blitting time and CPU
    Paints to a pixmap that is then bitblt'd to the qbezier custom widget
*/
void Qbezier::draw()
{
	QColor backgroundcolour("white");
	QPainter paint;
	//onyl redraw if something requested a recalc. Saves on cpu time
	if (reCalc)
	{

		pict.fill(backgroundcolour);
		paint.begin( &pict );				// begin painting onto pixmap
		maingraph.drawGraph(&paint);			// set the viewport & matrix for the widget
		maingraph.drawGrid(&paint);			// draw the grid layer
		maingraph.renderBezier(&paint);			// render the bezier`lines
		maingraph.drawServoData(&paint);		// draw the grid layer
		maingraph.drawControl(&paint);			// draw control points and tangent lines
		paint.end();					// painting done
		//someone request a recalc
		reCalc=false;
	}

	//see if we have a timer in motion, if so draw the tick
	if(maingraph.tick >0)
	{
		paint.begin( &pict );				// begin painting onto pixmap
		QPen pen, bckpen;
		bckpen = paint.pen();				//backup our pen
		maingraph.drawGraph(&paint);			//apply our transformation matrix
		paint.setPen((QColor("red")));
		pen = paint.pen();
		pen.setStyle(Qt::DashLine);
		pen.setWidth(2);
		paint.setPen(pen);
		int a,b;
        	a = maingraph.translateX((double)maingraph.tick);
		b = maingraph.plotArea.top();
		//draw vertical axis line representing time
		paint.Linev(a, maingraph.plotArea.bottom(), b);

		paint.setPen(bckpen);				//restore our pen
		paint.end();					//
		// we have to request a redraw to get this to blit. This way we blit every other frame saving CPU
		reCalc = true;
	}
	//update the main widget
	bitBlt(this,0,0,&pict);
}
