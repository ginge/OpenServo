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
#include "controlpoint.h"

controlPoint::controlPoint()
{
	width = 4000;
	pointX = 0;
	pointY = 0;
//	tangentX = tx;
//	tangentY = ty;
}


controlPoint::~controlPoint()
{
}



/* 
 * Set the x y coordinates of the control point
 */
void controlPoint::setControl(float x, float y)
{
 	pointX = x;
 	pointY = y;
}

/*
 * Check to see if the corrdintes given contain a control point.
 */
int controlPoint::isControlPoint(int mouseX, int mouseY)
{
	float px = pointX-(width/2)-1;
	float py = pointY-(width/2)-1;
	if (( mouseX > px && mouseX < px+width+2 ) && (mouseY > py && mouseY < py + width + 2))
	{
		return 1;
	}

	return -1;	
}

