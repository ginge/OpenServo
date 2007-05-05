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
#ifndef GRAPH_H
#define GRAPH_H

#include <qpainter.h>
#include <qpicture.h>

#include "node.h"
#include "commondefines.h"
#include "controlpoint.h"
#include "motion.h"
/**
	@author Barry Carter <barry.carter@gmail.com>
*/

class QPgraph
{

public:
    QPgraph();

    ~QPgraph();

	//local functions
	void drawGraph(QPainter *paint);
	void drawGrid(QPainter *p);
	void renderBezier(QPainter *p1a);
	void Scale( double ex, double ey );
	int translateX( double x);		//graph to painter coords
	int translateY( double y);
	double translateX( int x );		//painter to graph
	double translateY( int y );
	void setScale(double scale);		//set the scale of the grid
	void setGridMax(int maxTime);		//set the maximun size of the grid.
	void setServoPosMin(int minPos);	//set the minimum size of the servo range
	void setServoPosMax(int maxPos);	//set the maximun size of the servo range
	void addControlPoint();			//add a new control point. adds midway between graph end and last point
	void delControlPoint(int point);	//delte selected control point
	void getPosSpdVal(float *x, float *dx, int t);	// get the speed and pos of the graph at time t
	void drawControl(QPainter *p);			// draw the widget
	void drawServoData( QPainter *p);
	int nodeCount;				//store a list of the nodes. Each node and tangent is counted from >1

	// grid settings
	int tick;
	double gridMaxx;
	double gridMinx;
	double gridMaxy;
	double gridMiny;
	int maxServoPos;
	int minServoPos;

	// widgets window transformations
	int w,h;
	QRect	vp;				// viewport
	int viewport;
	QWMatrix worldMatrix;
	QRect plotArea;
	bool isScaled;
	int lx,ly;
	QPoint ref;
	//control point info
	int nodeHighlight;
	int nodeSelected;
	controlPoint cPoint[MAX_NODES];

	// Servo information
	int posHistory[65535];
	int curHistory[65535];
	int posHistoryCount;

	// frequency of the read
	int readResolution;

private:

	double originX;
	double originY;

	// scale xy
	double tsx,tsy;
	//tic divisions
	double ticx;
	double ticy;
	//scale
	double scalex,scaley;
	double mainScale;

	//a list of curves
	motion curve[(MAX_NODES/2)+1];

};

#endif
