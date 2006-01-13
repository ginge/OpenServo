// ServoListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ServoWorkbench.h"
#include "ServoListCtrl.h"

#include "mainfrm.h"
#include "defines.h"
#include ".\servolistctrl.h"


// CServoListCtrl

IMPLEMENT_DYNAMIC(CServoListCtrl, CListBox)
CServoListCtrl::CServoListCtrl()
{
}

CServoListCtrl::~CServoListCtrl()
{
}


BEGIN_MESSAGE_MAP(CServoListCtrl, CListBox)
	ON_WM_TIMER()
	ON_MESSAGE( WM_SN_CONNECTED, OnControllerConnected )
	ON_MESSAGE( WM_SN_DISCONNECTING, OnControllerDisconnecting )
END_MESSAGE_MAP()

IServoControllerPtr CServoListCtrl::GetController()
{
	CMainFrame* pmainframe = (CMainFrame*)AfxGetMainWnd();
	ASSERT_KINDOF(CMainFrame, pmainframe);
	return pmainframe->m_pcontroller;
}

BOOL CServoListCtrl::LoadServos()
{
	ResetContent();
	IServoControllerPtr pcontroller = GetController();
	for(int i = 0, c = pcontroller->count; i<c; i++) {
		InsertServo( -1, pcontroller->item[i] );
	}
	return TRUE;
}

BOOL CServoListCtrl::Refresh()
{
	for(int i=0, c = GetCount(); i<c; i++) {
		ServoInfo* pservoinfo = (ServoInfo*)GetItemDataPtr(i);
		try {
			pservoinfo->position = pservoinfo->ptr->position;
			pservoinfo->targetPosition = pservoinfo->ptr->targetPosition;
			pservoinfo->current = pservoinfo->ptr->current;
			pservoinfo->valid = TRUE;
		} catch(HRESULT) {
			pservoinfo->valid = FALSE;
		}
	}
	Invalidate();
	return TRUE;
}

int CServoListCtrl::InsertServo(int index, IServoPtr pservo)
{
	CString s;
	ServoInfo* pservoinfo = new ServoInfo;
	ASSERT(pservoinfo);

	try {
		pservoinfo->ptr = pservo;
		pservoinfo->address = pservo->address;
		pservoinfo->name = (LPCTSTR)pservo->name;
		pservoinfo->position = pservo->position;
		pservoinfo->targetPosition = pservo->targetPosition;
		pservoinfo->current = pservo->current;
		pservoinfo->valid = TRUE;
	} catch(HRESULT) {
		pservoinfo->valid = FALSE;
	}

	int id = InsertString(index, NULL);
	if(id!=CB_ERR)
		SetItemDataPtr(id, pservoinfo);
	return id;
}



// CServoListCtrl message handlers


LRESULT CServoListCtrl::OnControllerConnected( WPARAM wp, LPARAM lp ) {
	return LoadServos();
}

LRESULT CServoListCtrl::OnControllerDisconnecting( WPARAM wp, LPARAM lp ) {
	ResetContent();
	return TRUE;
}

BOOL CServoListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= LBS_OWNERDRAWFIXED;
	dwStyle &= ~LBS_HASSTRINGS;
	if( CListBox::Create(dwStyle, rect, pParentWnd, nID) ) {
		SetItemHeight( 0, 36 );
		SetTimer( 0, 500, NULL );
		return TRUE;
	} else
		return FALSE;
}


void CServoListCtrl::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	delete ((ServoInfo*)lpDeleteItemStruct->itemData);
	CListBox::DeleteItem(lpDeleteItemStruct);
}

int CServoListCtrl::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	ServoInfo	*lhs = (ServoInfo*)lpCompareItemStruct->itemData1,
				*rhs = (ServoInfo*)lpCompareItemStruct->itemData2;

	// TODO:  Add your code to determine the sorting order of the specified items
	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2

	return  (lhs->address < rhs->address) ? -1 : ((lhs->address > rhs->address) ? 1 : 0);
		
}

void CServoListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(FALSE);	// not variable height list box
}

void CServoListCtrl::DrawProgress(CDC* pdc, CRect bounds, int from, int length, COLORREF color)
{
	COLORREF border_color = ::GetSysColor(COLOR_BTNSHADOW);
	pdc->Draw3dRect( bounds, border_color, border_color );
	bounds.DeflateRect(2,2);
	bounds.left += from;
	bounds.right = bounds.left + length;
	pdc->FillSolidRect( bounds, color );
}

void CServoListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	COLORREF clr_background;

	if( lpDrawItemStruct->itemID == LB_ERR )
		return;

	CDC pdc;
	pdc.Attach( lpDrawItemStruct->hDC );
	
	ServoInfo* pservoinfo = (ServoInfo*)lpDrawItemStruct->itemData;

	if(lpDrawItemStruct->itemState & ODS_SELECTED )
		clr_background = ::GetSysColor(COLOR_BTNFACE)-0x00002020;
	else
		clr_background = ::GetSysColor(COLOR_BTNFACE);

	CRect bounds( lpDrawItemStruct->rcItem );

	pdc.Draw3dRect( bounds, ::GetSysColor(COLOR_BTNHILIGHT), ::GetSysColor(COLOR_BTNSHADOW) );
	bounds.DeflateRect(1,1);

	pdc.FillSolidRect(bounds, clr_background );
	bounds.DeflateRect(1,1);

	CRect rcaddr( bounds.left+3, bounds.top+1, bounds.left+40, bounds.top+16 );
	CString saddr;
	saddr.Format("0x%02x", pservoinfo->address);

	pdc.DrawText(saddr, rcaddr, DT_SINGLELINE|DT_LEFT|DT_TOP);

	CRect rcposition( bounds );
	rcposition.DeflateRect( 4,4 );
	rcposition.left += 30;
	rcposition.bottom = rcposition.top + 10;

	CRect rccurrent( rcposition );
	rccurrent.OffsetRect( 0, rcposition.Height() + 4 );

	if(pservoinfo->valid) {
		DrawProgress(&pdc, rcposition, pservoinfo->position*(rcposition.Width()-6)/255, 2, 0x00aa2020 );
		DrawProgress(&pdc, rccurrent, 0, pservoinfo->current*(rcposition.Width()-4)/255, 0x002020aa );
	}

	pdc.Detach();
}

void CServoListCtrl::OnTimer(UINT nIDEvent)
{
	Refresh();
	CListBox::OnTimer(nIDEvent);
}
