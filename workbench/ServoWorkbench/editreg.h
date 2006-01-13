////////////////////////////////////////////////////////////////////////////
//	File:		editcmd.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Registry keys and values for Crystal Edit - syntax colorig text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#ifndef CRYSEDITREG_H__INCLUDED
#define CRYSEDITREG_H__INCLUDED

//	Registry keys & values
#define REG_FIND_SUBKEY		_T("CrystalEdit\\Find")
#define REG_REPLACE_SUBKEY	_T("CrystalEdit\\Replace")
#define REG_MATCH_CASE		_T("MatchCase")
#define REG_WHOLE_WORD		_T("WholeWord")
#define REG_FIND_WHAT		_T("FindWhat")
#define REG_REPLACE_WITH	_T("ReplaceWith")

#define REG_PAGE_SUBKEY		_T("CrystalEdit\\PageSetup")
#define REG_MARGIN_LEFT		_T("LeftMargin")
#define REG_MARGIN_RIGHT	_T("RightMargin")
#define REG_MARGIN_TOP		_T("TopMargin")
#define REG_MARGIN_BOTTOM	_T("BottomMargin")

#endif
