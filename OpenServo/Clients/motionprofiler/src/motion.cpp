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
#include "motion.h"

motion::motion()
{
}


motion::~motion()
{
}

void motion::hermiteInit(motion *curve, controlPoint cPoint[], int point)
{
	float lnx, lny;

	//initialise the points in the curve
	curve->p0 = cPoint[point-2].graphPointY;
	curve->p1 = cPoint[point].graphPointY;
	curve->t0 = cPoint[point-2].graphPointX;
	curve->t1 = cPoint[point].graphPointX;
	//scale v0 and v1 to sane range this is a gradient so multiple not important
	lny = (cPoint[point-2].graphPointY - cPoint[point-1].graphPointY );
	lnx = (cPoint[point-2].graphPointX - cPoint[point-1].graphPointX );
	//catch division by zero error
	if(lnx==0) { lnx = 0.1; }
	if(lny==0) { lny = 0.1; }

	//calculate the tangent velocity
	curve->v0 =  (lny / lnx);

	//length of the tangent points
	lny = (cPoint[point].graphPointY - cPoint[point+1].graphPointY );
	lnx = (cPoint[point].graphPointX - cPoint[point+1].graphPointX );

	//catch division by zero errors
	if(lnx==0) { lnx = 0.1; }
	if(lny==0) { lny = 0.1; }
	curve->v1 =  (lny/lnx);

	//calculate the duration of the movement		
	curve->duration = curve->t1 - curve->t0;

	// Tangents expressed as slope of value/time.  The time span will 
	// be normalized to 0.0 to 1.0 range so correct the tangents by 
	// scaling them by the duration of the curve.
	curve->v0 *= curve->duration;
	curve->v1 *= curve->duration;

	// Determine the cubic coefficients by multiplying the matrix form of
	// the Hermite curve by p0, p1, v0 and v1.
	// 
	// | a |   |  2  -2   1   1 |   |       p0       |
	// | b |   | -3   3  -2  -1 |   |       p1       |
	// | c | = |  0   0   1   0 | . | (t1 - t0) * v0 |
	// | d |   |  1   0   0   0 |   | (t1 - t0) * v1 |
	curve->h1 = (2.0 * curve->p0) - (2.0 * curve->p1) + curve->v0 + curve->v1;
	curve->h2 = -(3.0 * curve->p0) + (3.0 * curve->p1) - (2.0 * curve->v0) - curve->v1;
	curve->h3 = curve->v0;
	curve->h4 = curve->p0;


}

void motion::hermiteSolve(motion *curve, int tick, float *x, float *dx )
{
	// scale the graph step division to fit into the duration
//	float s = (float)curve->duration / (float)curve->steps;
	float t1 = ((float) ( tick - curve->t0) / (float)curve->duration);

	// get the next tick position, calculating offset
//	s = curve->t0 + ((float)tick*s);
	float t2 = t1 * t1;
	float t3 = t2 * t1;

	// scale the point to the grid
//	float ds = (s - curve->t0) / (float)curve->duration;

	// calculate the cubic polynomial function 
	// at^3 + bt^2 +ct +d
	//gx=(float)s;
//	*x = curve->h4 + (ds * (curve->h3 + (ds * (curve->h2 + (ds * curve->h1)))));
	*x = (curve->h1 * t3 ) + (curve->h2 * t2) + (curve->h3 * t1) + curve->h4;

	// Compute the derivative value of the cubic equation.
	// x = au^3 + bu^2 + cu + d = d + u(c + u(b + u(a)))
	//dx = 3au^2 + 2bu + c = c + u(2b + u(3a))
//	*dx = curve->h3 + (ds * ((ds * curve->h2) + (ds * (3.0 * curve->h1))));
	*dx = ( 3.0 * curve->h1 * t2) + (2.0 * curve->h2 * t1) + curve->h3;

	//Normalize the slope back from the 0.0 to 1.0 range to (t1 - t0).
	*dx /= curve->duration;

}

