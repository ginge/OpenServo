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
#ifndef MOTION_H
#define MOTION_H

#include "controlpoint.h"
#include "commondefines.h"
/**
	@author Barry Carter <barry.carter@gmail.com>
*/
class motion{
public:
    motion();

    ~motion();
	void hermiteInit(motion *curve, controlPoint cPoint[], int point);
	void hermiteSolve(motion *curve, int tick, float *x, float *dx );
	
	// store the basis function for each curve
	float h1;
	float h2;
	float h3;
	float h4;

	//store the time for each node
	float t0;
	float t1;
	//vector
	float v0;
	float v1;
	//position
	float p0;
	float p1;

	//calculated duration of movement after init
	float duration;

	//refinement of the resolution on the curve. 128 is fine
	int steps;
};

#endif
