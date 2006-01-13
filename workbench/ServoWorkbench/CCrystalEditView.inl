////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalEditView.inl
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

#ifndef __CCrystalEditView_INL_INCLUDED
#define __CCrystalEditView_INL_INCLUDED

#include "CCrystalEditView.h"

CE_INLINE BOOL CCrystalEditView::GetOverwriteMode() const
{
	return m_bOvrMode;
}

CE_INLINE void CCrystalEditView::SetOverwriteMode(BOOL bOvrMode /*= TRUE*/)
{
	m_bOvrMode = bOvrMode;
}

CE_INLINE BOOL CCrystalEditView::GetDisableBSAtSOL() const
{
	return m_bDisableBSAtSOL;
}

CE_INLINE BOOL CCrystalEditView::GetAutoIndent() const
{
	return m_bAutoIndent;
}

CE_INLINE void CCrystalEditView::SetAutoIndent(BOOL bAutoIndent)
{
	m_bAutoIndent = bAutoIndent;
}

#endif
