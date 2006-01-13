////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextView.inl
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Inline functions of Crystal Edit classes
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef __CCrystalTextView_INL_INCLUDED
#define __CCrystalTextView_INL_INCLUDED

#include "CCrystalTextView.h"

CE_INLINE BOOL CCrystalTextView::IsDraggingText() const
{
	return m_bDraggingText;
}

#endif
