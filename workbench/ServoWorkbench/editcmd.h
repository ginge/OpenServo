////////////////////////////////////////////////////////////////////////////
//	File:		editcmd.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Command defintions and resource IDs for Crystal Edit - syntax coloring
//	text editor.
//	Include this in .RC file in your project.
//
//	You are free to use, distribute or modify this code
//	but LEAVE THIS HEADER INTACT.
////////////////////////////////////////////////////////////////////////////

#ifndef _EDITCMD_H_INCLUDED
#define _EDITCMD_H_INCLUDED

#define		ID_EDIT_FIRST				37000
#define		ID_EDIT_LAST				37999

//	ID_EDIT_COPY, ID_EDIT_PASTE, ID_EDIT_CUT, ID_EDIT_SELECT_ALL
//	are defined in afxres.h

//	Edit commands
#define		ID_EDIT_DELETE				37000
#define		ID_EDIT_DELETE_BACK			37001
#define		ID_EDIT_DELETE_WORD_BACK	37002
#define		ID_EDIT_TAB					37003
#define		ID_EDIT_UNTAB				37004
#define		ID_EDIT_SWITCH_OVRMODE		37005
//
//	[JRT]: Added Identifiers For Additional Features
//
#define		ID_EDIT_DISABLEBSATSOL		37006
#define		ID_EDIT_DISABLEDRAGANDDROP	37007

//	Cursor movement commands
#define		ID_EDIT_CHAR_LEFT			37100
#define		ID_EDIT_EXT_CHAR_LEFT		37101
#define		ID_EDIT_CHAR_RIGHT			37102
#define		ID_EDIT_EXT_CHAR_RIGHT		37103
#define		ID_EDIT_WORD_LEFT			37104
#define		ID_EDIT_EXT_WORD_LEFT		37105
#define		ID_EDIT_WORD_RIGHT			37106
#define		ID_EDIT_EXT_WORD_RIGHT		37107
#define		ID_EDIT_LINE_UP				37108
#define		ID_EDIT_EXT_LINE_UP			37109
#define		ID_EDIT_LINE_DOWN			37110
#define		ID_EDIT_EXT_LINE_DOWN		37111
#define		ID_EDIT_SCROLL_UP			37112
#define		ID_EDIT_SCROLL_DOWN			37113
#define		ID_EDIT_PAGE_UP				37114
#define		ID_EDIT_EXT_PAGE_UP			37115
#define		ID_EDIT_PAGE_DOWN			37116
#define		ID_EDIT_EXT_PAGE_DOWN		37117
#define		ID_EDIT_LINE_END			37118
#define		ID_EDIT_EXT_LINE_END		37119
#define		ID_EDIT_HOME				37120
#define		ID_EDIT_EXT_HOME			37121
#define		ID_EDIT_TEXT_BEGIN			37122
#define		ID_EDIT_EXT_TEXT_BEGIN		37123
#define		ID_EDIT_TEXT_END			37124
#define		ID_EDIT_EXT_TEXT_END		37125

//	Bookmark commands
#define		ID_EDIT_TOGGLE_BOOKMARK0	37200
#define		ID_EDIT_TOGGLE_BOOKMARK1	37201
#define		ID_EDIT_TOGGLE_BOOKMARK2	37202
#define		ID_EDIT_TOGGLE_BOOKMARK3	37203
#define		ID_EDIT_TOGGLE_BOOKMARK4	37204
#define		ID_EDIT_TOGGLE_BOOKMARK5	37205
#define		ID_EDIT_TOGGLE_BOOKMARK6	37206
#define		ID_EDIT_TOGGLE_BOOKMARK7	37207
#define		ID_EDIT_TOGGLE_BOOKMARK8	37208
#define		ID_EDIT_TOGGLE_BOOKMARK9	37209
#define		ID_EDIT_GO_BOOKMARK0		37210
#define		ID_EDIT_GO_BOOKMARK1		37211
#define		ID_EDIT_GO_BOOKMARK2		37212
#define		ID_EDIT_GO_BOOKMARK3		37213
#define		ID_EDIT_GO_BOOKMARK4		37214
#define		ID_EDIT_GO_BOOKMARK5		37215
#define		ID_EDIT_GO_BOOKMARK6		37216
#define		ID_EDIT_GO_BOOKMARK7		37217
#define		ID_EDIT_GO_BOOKMARK8		37218
#define		ID_EDIT_GO_BOOKMARK9		37219
#define		ID_EDIT_CLEAR_BOOKMARKS		37220
#define		ID_EDIT_SET_BREAKPOINT		37221
#define		ID_EDIT_ENABLE_BREAKPOINT	37222

//	Status update
#define		ID_EDIT_INDICATOR_COL		37900
#define		ID_EDIT_INDICATOR_READ		37901
#define		ID_EDIT_INDICATOR_CRLF		37902
#define		ID_EDIT_INDICATOR_POSITION	37903

//	Resource IDs
#define		IDR_MARGIN_CURSOR			22900
#define		IDR_DEFAULT_ACCEL			22901
#define		IDR_MARGIN_ICONS			22902

//	Dialog IDs
#define		IDD_EDIT_FIND				8601
#define		IDD_EDIT_REPLACE			8602

//	Control IDs
#define		IDC_EDIT_WHOLE_WORD			8603
#define		IDC_EDIT_MATCH_CASE			8604
#define		IDC_EDIT_TEXT				8605
#define		IDS_EDIT_TEXT_NOT_FOUND		8606
#define		IDC_EDIT_REPLACE			8607
#define		IDC_EDIT_REPLACE_ALL		8608
#define		IDC_EDIT_REPLACE_WITH		8609
#define		IDC_EDIT_SKIP				8610
#define		IDC_EDIT_DIRECTION_UP		8611
#define		IDC_EDIT_DIRECTION_DOWN		8612
#define		IDC_EDIT_SCOPE_SELECTION	8613
#define		IDC_EDIT_SCOPE_WHOLE_FILE	8614

#endif
