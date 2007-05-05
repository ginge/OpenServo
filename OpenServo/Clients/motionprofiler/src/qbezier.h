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

#ifndef QBEZIER_H
#define QBEZIER_H

#include "graph.h"
#include "qlabel.h"
#include "bezierwidget.h"

class Qbezier : public QLabel
{
Q_OBJECT
public:
	Qbezier(QWidget* parent, const char* name);
	QWidget* parent;

	QPgraph maingraph;		//handle on graph
 	float mouseX; 			//virtual mousex
 	float mouseY;
	int widMouseX, widMouseY;	//mouse widget corrdinates
	int mode;			//current mode for node deletion
	bool reCalc; 			//request a recalculation of all

	void draw();			//redraw the graph


signals:
	void setXY(int x, int y);
	void calcparams();
	void setInfo(QString &string);

private slots:
	// catch out events on this custom widget. We need to handle all painting outselves
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *);
	void paintEvent(QPaintEvent *e);
private:
	QPixmap pict;                          // our pixmap

	void setGraph(QPgraph *graph);
};

#endif
