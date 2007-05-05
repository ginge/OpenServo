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
#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include "node.h"

/**
	@author Barry Carter <barry.carter@gmail.com>
*/
class controlPoint{
public:
    controlPoint();

    ~controlPoint();

	// Set the position of the control point at x y
	void setControl(float x, float y);

	// check to see if the coordinates given contain a control point
	int isControlPoint(int x,int y);	//in widget coords

	float pointX;
	float pointY;

	//graph coordinates
	float graphPointX;
	float graphPointY;

	int width;  // width of bounding box around a node point
};

#endif
