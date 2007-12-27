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
#include "graph.h"
#include "commondefines.h"
#include <qpicture.h>
#include <qpainter.h>
#include <qnamespace.h>
#include <math.h>
#include <iostream>
#include <iomanip>

QPgraph::QPgraph()
{
	nodeCount = 0;
	nodeSelected = -1;
	nodeHighlight = -1;

	gridMaxx = 4500.0;				//grid max size
	gridMinx = -0.2;				//grid min size
	gridMaxy = 1024.0;				//max y of grid. Set to 10 bit servo resolution
	gridMiny = -5.0;				//min y

	maxServoPos = 1023;				//initial values for the max/min lines
	minServoPos = 1;

	posHistoryCount = 0;
	readResolution = 100;

	tick = 0;

	//initialise points on graph
	cPoint[0].graphPointX = 10.0;
	cPoint[0].graphPointY = 200.0;
	cPoint[1].graphPointX = 100.0;
	cPoint[1].graphPointY = 200.0;
	
	
	cPoint[2].graphPointX = 2000.0;
	cPoint[2].graphPointY = 908.0;
	cPoint[3].graphPointX = 1920.0;
	cPoint[3].graphPointY = 908.0;

	cPoint[4].graphPointX = 4000.0;
	cPoint[4].graphPointY = 200.0;
	cPoint[5].graphPointX = 4090.0;
	cPoint[5].graphPointY = 200.0;
	
	//each line (including the tangent) has a node
	nodeCount = 6;

	//start in scaling mode
	isScaled = true;
}


QPgraph::~QPgraph()
{
}

/*
 * Sets the viewport and scaling for the graph.
 */
void QPgraph::drawGraph(QPainter *paint)
{
	vp = paint->viewport();					// set viewport
	w=vp.width();						//define public width and height of viewport
	h=vp.height();

	double tlgx,tlgy;
	tlgx = 0.5;//double(gridMaxx-gridMinx)/16;		//grid division factor
	tlgy = 0.5;//double(gridMaxy-gridMiny)/16;
	lx=(int)((gridMaxx-gridMinx)*100.*1.0/tlgx);		//calculate scale
	ly=(int)((gridMaxy-gridMiny)*100.*1.0/tlgy);	
	ref=QPoint(120, 100);					//store a reference point for the border
	//Scales into widget. scaling on x is fit to widget size
	//Scaling Change suggested by A Buxton.
	paint->scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
	if(paint->xForm(QPoint(lx+2*ref.x(), ly)).x() > paint->viewport().right())
	{
		paint->resetXForm();
		paint->scale((float)w/(float)(lx+2*ref.x()), (float)h/(float)(ly+2*ref.y()));
	}

	//if we have scale x disabled make the x scale fixed
	if( !isScaled )	//check we can fit that in and we want to
	{
		paint->resetXForm();
		paint->scale((float)h/(float)(ly+2*ref.y()), (float)h/(float)(ly+2*ref.y()));
	}

	
	paint->setViewport(-viewport, paint->viewport().y(),w,h);
	vp = paint->viewport();					// set viewport

	worldMatrix=paint->worldMatrix();			//store the world matrix trnsformations
	plotArea.setRect(ref.x(),ref.y(),lx,ly);		//store the area which we are plotting in
	QRect area;

	Scale(tlgx,tlgy);					//scale the image
	
	area = paint->xForm(plotArea);				//calculate area

}

/*
 * Scale the coordinates to ceil round. Stores tics per division scaling
 */
void QPgraph::Scale( double ex, double ey )
{
	QPgraph::ticx=ex;
	QPgraph::ticy=ey;
	tsx=ceil(gridMinx/ex)*ex;
	tsy=ceil(gridMiny/ey)*ey;
}

/* Draw the main bezier hermite curve onto the graph.
 *   draws a curve for each segment of the motion
 */
void QPgraph::renderBezier( QPainter *p )
{
	//initialise pointx and y from graph to paint coords
	float px = translateX(cPoint[0].graphPointX);
	float py = translateY(cPoint[0].graphPointY);

	int n,c;
	c=0;

	//for each node calculate basis then render graph
	for (n=2;n<nodeCount; n+=2)
	{	

		//initialise the hermite function with the curve
		curve[c].hermiteInit(&curve[c], cPoint, n);
		curve[c].steps=(int)curve[c].duration;
		//scale and step though the curve
		for (int t=(int)curve[c].t0; t <= (int)curve[c].t0+ curve[c].steps; t++)
		{
			float gx, gy, dx;

			// scale to pixmap coordinates
			QPen pena;
			pena = p->pen();

			curve[c].hermiteSolve(&curve[c], t, &gy, &dx );

			//float s = curve[c].duration / curve[c].steps;
			// get the next tick position, calculating offset
			float s = ((float)t); //*s);
			gx = translateX((float)s);
			gy = translateY((float)gy);

			p->setPen( QPen( QColor( 0, 0, 255 ) ) );
			p->drawLine((int)px,(int)py,(int)gx,(int)gy);
			//reset the pen transformations
			p->setPen( pena );
			//store the last point positions for the interpolated line
			px = gx; py = gy;
		}
		c++;
	}

}

void QPgraph::drawServoData( QPainter *p)
{
	float bckpx = 1;
	float bckpy = 1;
	float bckcx = 1;
	float bckcy = 1;
	float px, py, cx, cy;
	QPen pena,pen;
	pena = p->pen(); //backup the pen style

	pen = p->pen();
	pen.setStyle(Qt::SolidLine);
	pen.setColor( "red" );
	pen.setWidth(2);
	
	p->setPen(pen);

	// draw the graph lines in two loops so we dont torture the poor paint and pen buffers
	for( int n=0; n< posHistoryCount; n++ )
	{
		// Draw a line for the position
		px = translateX((float)(n*readResolution)+(float)cPoint[0].graphPointX);
		py = translateY((float)posHistory[n]);
		if (n>0)
			p->drawLine((int)bckpx,(int)bckpy,(int)px,(int)py);
	
		bckpx = px; bckpy = py;
	}
		// Draw a line for the Current

	pen = p->pen();
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(2);
	pen.setColor( "green" );		
	p->setPen(pen);

	for( int n=0; n< posHistoryCount; n++ )
	{
		cx = translateX((float)(n*readResolution)+(float)cPoint[0].graphPointX);
		cy = translateY((float)curHistory[n]);
		if (n>0)
			p->drawLine((int)bckcx,(int)bckcy,(int)cx,(int)cy);
	
		bckcx = cx; bckcy = cy;
	}
	//reset the pen transformations
	p->setPen( pena );

}

/* Draw the control points and tangent lines
   Loops through each control point adding the transformation for each, then joins the lines
*/
void QPgraph::drawControl(QPainter *p)
{
	int n;
	QPen pen;
	//Draw the tangent lines and draw the boxes.
	for (n=2; n <=nodeCount;n+=2)
	{
		// store shorthand for the transformations grom graph to map coordinates
		float transX = translateX(cPoint[n-2].graphPointX);
		float transY = translateY(cPoint[n-2].graphPointY);
		float transTanX = translateX(cPoint[n-1].graphPointX);
		float transTanY = translateY(cPoint[n-1].graphPointY);

		pen = p->pen();							//store the cerrent pen
		p->setPen( QPen( QColor( 255, 94, 88 ) ) );
		p->drawLine( (int)transX, (int)transY,(int)transTanX,(int)transTanY);		//draw the tangent lines
		p->setPen( pen );
		cPoint[n-2].setControl(transX,transY);				//set the control point
		cPoint[n-1].setControl(transTanX,transTanY);			//set tangent

		QBrush brush( "yellow" );					//yellow for all points
		p->setBrush(brush);

		//check to see if the control is currently highlighted
		if (n-2 == nodeHighlight) {
			p->setPen( QPen( QColor(255,0,0), 1 ) );
			p->setBrush(QBrush("red"));
		}

		float aa = (float)w/(float)(lx+2*ref.x());
		float ab = (float)h/(float)(ly+2*ref.y());
//		std::cout << "scale  " << aa << "scaleb " << ab << "\n" << endl;
		p->drawRect( (int)cPoint[n-2].pointX-(cPoint[n-2].width/2), (int)cPoint[n-2].pointY-(cPoint[n-2].width/2),
 (int)((float)cPoint[n-2].width), (int)cPoint[n-2].width );	//draw control point box
		p->setPen( pen );
		p->setBrush( QBrush("blue"));
		//see if the tangent point is highlighted
		if (n-1 == nodeHighlight) {
			p->setPen( QPen( QColor(255,0,0), 1 ) );
			p->setBrush(QBrush("red"));
		}
		p->drawRect( (int)cPoint[n-1].pointX-(cPoint[n-1].width/2), (int)cPoint[n-1].pointY-(cPoint[n-1].width/2), (int)cPoint[n-1].width, (int)cPoint[n-1].width );	//draw tangent box
		p->setPen( pen );
	}
}

/*
 * Draw a grid on the painter
 */
void QPgraph::drawGrid(QPainter *p)
{
	int a,b;
	double xmd = gridMaxx+1e-6;		//store the grid max X
	double ymd = gridMaxy+1e-6;		//store the grid max Y
	double d;
	d=tsx;
	scalex = (lx/(gridMaxx-gridMinx));	//calculate the scale X
	scaley = (ly/(gridMaxy-gridMiny));	//calculate the scale Y
	originX=ref.x()-scalex*gridMinx+0.5;	//Calculate and store the origin
	originY=ref.y()+scaley*gridMaxy+0.5;

	p->setPen( QPen( QColor(229,221,255), 1 ) );
	//draw the vertical lines
	while(d<xmd)
	{
		p->Linev(translateX(d), plotArea.bottom(), plotArea.top());
		d+=10.0; //ticx;
	}
	d=tsy;
	//and now the horizontal
	while(d<ymd)
	{
		p->Lineh(plotArea.left(), translateY(d), plotArea.right());
		d+=10.0;//ticy;
	}
	p->setPen( QPen( QColor(0,0,0), 1 ) );
	//Draw the arrows on the Y axis
	b = translateY(0.);
	a = plotArea.right();
	//draw the horizontal axis
	p->Lineh(plotArea.left(), b, a);
	int  dx=4000;
	int dy=1500;
	p->drawLine(a, b, a-dx, b+dy);
	p->drawLine(a, b, a-dx, b-dy);

	//Draw the arrows on the X axis
        a = translateX(0.);
	b = plotArea.top();
	//draw vertical axis
	p->Linev(a, plotArea.bottom(), b);
	dx=1500;
	dy=4000;
	p->drawLine(a, b, a-dx, b+dy);
	p->drawLine(a, b, a+dx, b+dy);

	//Draw the graph mid point
	b = translateY(0.) + ((plotArea.top()-translateY(0.))/2);
	a = plotArea.right();
	//draw the horizontal ine
	QPen pen, bckpen;
	bckpen = p->pen();

	p->setPen((QColor("grey")));
	pen = p->pen();
	pen.setStyle(Qt::DashLine);
	p->setPen(pen);
	p->Lineh(plotArea.left(), b, a);
	p->setPen(bckpen);

	//Draw the servo upper and lower bounds
	b = translateY((float)maxServoPos);
	a = plotArea.right();
	//draw the horizontal ine
//	QPen pen, bckpen;
	bckpen = p->pen();

	p->setPen((QColor("grey")));
	pen = p->pen();
	pen.setStyle(Qt::DashLine);
	p->setPen(pen);
	p->Lineh(plotArea.left(), b, a);
	p->setPen(bckpen);

	//Draw the servo upper and lower bounds
	b = translateY((float)minServoPos);
	a = plotArea.right();
	//draw the horizontal ine
	bckpen = p->pen();
	p->setPen((QColor("grey")));
	pen = p->pen();
	pen.setStyle(Qt::DashLine);
	p->setPen(pen);
	p->Lineh(plotArea.left(), b, a);
	p->setPen(bckpen);
}

//Translate from graph to widget coordinates
double QPgraph::translateX( int x)
{
   return (x-originX)/scalex;
}

//Translate from widget to graph coordinates
int QPgraph::translateX( double x)
{

        int xi;
	static double lastx;		//Store the last x input for later calc
	
	//check to see is it is a valid number
	if(isnan(x))
	{
		if(lastx<1. && lastx>-1.)
			xi=(int)(originX-scalex*lastx);
		else
			xi=(lastx<0)? plotArea.left(): plotArea.right();
	}
	//make sure that x is not infinity. clip to left
	else if(isinf(x)==-1)
	{

		xi=plotArea.left();
	}
	//make sure that x is not infinity. clip to right
	else if(isinf(x)==1)
	{

		xi=plotArea.right();
                
	}
	//bounds checking
	else if(x<gridMinx)
	{
		xi=plotArea.left();
	}
	//bounds checking
	else if(x>gridMaxx)
	{
		xi=plotArea.right();
	}
	//default to converting from graph to picmap coordinates
	else
	{
		xi=(int)(originX+scalex*x);
	}

	lastx=x;
	return xi;
}

double QPgraph::translateY( int y)
{
	return (originY-y)/scaley;
}

int QPgraph::translateY( double y)
{
	int yi;
	static double lasty;

	if(isnan(y))
	{

		if(lasty<1. && lasty>-1.)
			yi=(int)(originY-scaley*lasty);
		else
			yi=(lasty<0)? plotArea.bottom(): plotArea.top();
	}
	else if(isinf(y)==-1)
	{
		yi=plotArea.bottom();
                
	}
	else if(isinf(y)==1)
	{
		yi=plotArea.top();
                
	}
	else if(y<gridMiny)
	{
		yi=plotArea.bottom();
	}
	else if(y>gridMaxy)
	{
		yi=plotArea.top();
	}
	else
	{
		yi=(int)(originY-scaley*y);
	}

	lasty=y;
	return yi;
}

/*
 * Set the time t maximum
 */
void QPgraph::setGridMax(int maxTime)
{
	gridMaxx = (double)maxTime;
	
}

/*
 * Set the servo bounds max
 */
void QPgraph::setServoPosMax(int maxPos)
{
	maxServoPos = maxPos;
}

/*
 * Set the servo bounds min
 */
void QPgraph::setServoPosMin(int minPos)
{
	minServoPos = minPos;
}

/*
 * Add a new control point half way between the last point and the end of the grid (time t)
 */
void QPgraph::addControlPoint()
{
	//no more control points!
	if (nodeCount == MAX_NODES ){return;}
	//adds a new control point half way between the grid max and the last point
	//initialise points on graph

	cPoint[nodeCount].graphPointX = cPoint[nodeCount-2].graphPointX+((gridMaxx - cPoint[nodeCount-2].graphPointX)/2);
	cPoint[nodeCount].graphPointY = cPoint[nodeCount-2].graphPointY;
	cPoint[nodeCount+1].graphPointX = cPoint[nodeCount].graphPointX+20.0;
	cPoint[nodeCount+1].graphPointY = cPoint[nodeCount-1].graphPointY;
	
	nodeCount +=2;	
}

/*
 * delete selected control point
 */
void QPgraph::delControlPoint(int point)
{	
	int n;

	if( nodeCount == 4) { return; }

	for (n=0;n<=nodeCount;n++)
	{
		// delete the point
		if (  n >= point )
		{
			//check to see if this is the last point, or there is only onw curve available
			if (point == nodeCount )
			{
				break;
			}
			//check if tangent line
			if (point%2 != 0)
			{
				n--;
			}
			//refill the rest of the array
			for (n=n;n<=nodeCount;n++)
			{
				cPoint[n] = cPoint[n+2];
			}
			break;
					
		}

	}
	//decrease the count of the nodes
	nodeCount -=2;
}

/*
 * Get the position x and the derivitive dx at time t. Works over the whole range of curves
 */
void QPgraph::getPosSpdVal(float *x, float *dx, int t)
{
	int n;
	//check all curves to find the timestep in the curve queue
	//for each node calculate basis then render graph
	for (n=0;n<nodeCount/2; n++)
	{	
		//find which segment we are in
		if (t <=curve[n].t1)
		{
			
//			float s = (( curve[n].steps / curve[n].duration) * (t- curve[n].t0 )) ;

			curve[n].hermiteSolve(&curve[n], (int)t, x, dx );
			break;
		}

	}
	
}
