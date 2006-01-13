// RegistersToolWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ServoWorkbench.h"
#include "RegistersToolWnd.h"
#include "defines.h"

#include "MainFrm.h"
#include ".\registerstoolwnd.h"

#undef CREATEFONT
#define CREATEFONT(var,fontname,size,bold,italic)		if( !var.m_hObject ) var.CreateFont(  \
			-MulDiv(size, GetDC()->GetDeviceCaps(LOGPIXELSY), 72), 0, 0 /* vertical escapement */, 0, \
			bold, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, \
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fontname);


// CRegistersToolWnd

IMPLEMENT_DYNAMIC(CRegistersToolWnd, CWnd)
CRegistersToolWnd::CRegistersToolWnd()
{
}

CRegistersToolWnd::~CRegistersToolWnd()
{
}


BEGIN_MESSAGE_MAP(CRegistersToolWnd, CWnd)
	ON_WM_SIZE()
	ON_MESSAGE( WM_SN_CONNECTED, OnControllerConnected )
	ON_MESSAGE( WM_SN_DISCONNECTING, OnControllerDisconnecting )
	ON_CBN_DROPDOWN( RTW_ID_SERVO, OnLoadServos )
	ON_CBN_SELENDOK( RTW_ID_SERVO, OnServoChange )
	ON_COMMAND( RTW_ID_UPDATE, OnUpdate )
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

IServoControllerPtr CRegistersToolWnd::GetController()
{
	CMainFrame* pmainframe = (CMainFrame*)AfxGetMainWnd();
	ASSERT_KINDOF(CMainFrame, pmainframe);
	return pmainframe->m_pcontroller;
}

BOOL CRegistersToolWnd::Refresh()
{
	try {
		IRegisterSetPtr pregisters = m_servo->registers;
		m_servo->ScanRegisters( pregisters );

		m_properties.ResetContent();

		CXTPPropertyGridItems* pcategories = m_properties.GetCategories();
		for(int i=0, c=pregisters->count; i<c; i++) {
			IRegisterPtr pregister = pregisters->byIndex[i];
			CXTPPropertyGridItem* pgroup = pcategories->FindItem( (LPCTSTR)pregister->category );
			if( !pgroup ) {
				/* create group */
				pgroup = m_properties.AddCategory( (LPCTSTR)pregister->category );
				pgroup->Expand();
			}

			/* get the servo value */
			_variant_t value = pregister->value;
			CString svalue;
			svalue.Format("0x%04x", value.uiVal);

			CXTPPropertyGridItem* pitem = new CXTPPropertyGridItem( (LPCTSTR)pregister->name, (LPCTSTR)svalue );
			pitem->SetDescription( (LPCTSTR)pregister->description );
			pgroup->AddChildItem( pitem );
		}

		return TRUE;
	} catch(HRESULT) {
		return FALSE;
	}
}



// CRegistersToolWnd message handlers


BOOL CRegistersToolWnd::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	CString lpszClassName = AfxRegisterWndClass(CS_DBLCLKS, NULL, NULL, NULL);
	if( CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext) ) {

		CREATEFONT(m_font, "arial", 8, FALSE, FALSE); 

		if( !m_cb_servo.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS, CRect(), this, RTW_ID_SERVO) )
			return FALSE;
		m_cb_servo.SetFont(&m_font);

		if( !m_btn_update.Create("Update", WS_CHILD|WS_VISIBLE, CRect(), this, RTW_ID_UPDATE ) )
			return FALSE;
		m_btn_update.SetFont(&m_font);

		if( !m_properties.Create(CRect(), this, RTW_ID_PROPLIST) )
			return FALSE;

		m_properties.SetTheme( xtpGridThemeNativeWinXP  );
#if 0
				m_wndphrases.SetCustomColors( 
					::GetSysColor(COLOR_BTNFACE), // clrHelpBack
					RGB( 0x00, 0x00, 0x00), // clrHelpFore
					RGB( 0xFF, 0xFD, 0xAB ), // clrViewLine
					RGB( 0xff, 0xff, 0xff ), // clrViewBack
					RGB( 0x00, 0x00, 0x00 ), // clrViewFore
					RGB( 0x00, 0x00, 0x00 )
					);
#endif


		return TRUE;
	}
	return FALSE;
}

void CRegistersToolWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if( m_properties.m_hWnd ) {
		CRect bounds;
		GetClientRect(bounds);

		/* bonds for servo drop down */
		CRect rcservo(bounds);
		rcservo.bottom = rcservo.top + 25;
		rcservo.DeflateRect(1,1);
		
		/* bounds for prop list */
		bounds.top = rcservo.bottom+2;

		/* bounds for update button */
		CRect rcupdate(rcservo);
		rcupdate.left = rcupdate.right - 50;
		rcupdate.DeflateRect(1,1);
		rcservo.right = rcupdate.left - 4;
		rcservo.bottom += 150;	// extend height for drop height!
		
		/* update rects */
		m_cb_servo.MoveWindow( rcservo );
		m_btn_update.MoveWindow( rcupdate );
		m_properties.MoveWindow(bounds);
	}
}

LRESULT CRegistersToolWnd::OnControllerConnected( WPARAM wp, LPARAM lp ) {
	return FALSE;
}

LRESULT CRegistersToolWnd::OnControllerDisconnecting( WPARAM wp, LPARAM lp ) {
	m_properties.ResetContent();
	m_cb_servo.ResetContent();
	return FALSE;
}


void CRegistersToolWnd::OnLoadServos()
{
	m_cb_servo.ResetContent();

	IServoControllerPtr pcontroller = GetController();
	for(int i = 0, c = pcontroller->count; i<c; i++) {
		IServoPtr pservo = pcontroller->item[i];
		CString s;
		s.Format("0x%02x - %s", pservo->address, (LPCTSTR)pservo->typeName );
		m_cb_servo.InsertString( -1, s);
	}
}

void CRegistersToolWnd::OnServoChange()
{
	try {
		int i = m_cb_servo.GetCurSel();
		ASSERT(i != CB_ERR);

		m_servo = GetController()->item[i];
		
		Refresh();

	} catch(HRESULT) {
		/* this shouldnt happen as the servo list was just loaded when the drop down occured, unless a rescan happened and this servo
			dissappeared in between the servo list drop down and the selection event. */
		AfxMessageBox("Unexpected error: this servo is no longer valid!");
	}
}

BOOL CRegistersToolWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect bounds;
	GetClientRect(bounds);
	pDC->FillSolidRect(bounds, ::GetSysColor(COLOR_BTNFACE) );
	return TRUE;
}
