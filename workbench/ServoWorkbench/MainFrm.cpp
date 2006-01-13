// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"

#define INITGUID
const GUID CATID_ActiveScript = { 0xf0b7a1a1, 0x9847, 0x11cf, { 0x8f, 0x20, 0x00, 0x80, 0x5f, 0x2c, 0xd0, 0x64 } };
#include <comcat.h>

#include "ServoWorkbench.h"
#include "MainFrm.h"
#include "defines.h"
#include "servoworkbenchdoc.h"

#include "../ax-servo/servocat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CXTPMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTPMDIFrameWnd)
	ON_WM_CREATE()
	
	ON_COMMAND(ID_SCRIPT_EXECUTE, OnScriptExecute)
//	ON_COMMAND(ID_SCRIPT_PAUSE, OnScriptPause)
	ON_COMMAND(ID_SCRIPT_STOP, OnScriptStop)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_EXECUTE, OnUpdateIfScriptRunnable)
//	ON_UPDATE_COMMAND_UI(ID_SCRIPT_PAUSE, OnUpdateIfScriptRunning)
	ON_UPDATE_COMMAND_UI(ID_SCRIPT_STOP, OnUpdateIfScriptStoppable)

	ON_COMMAND(ID_SERVO_CONNECT, OnServoConnect)
	ON_COMMAND(ID_SERVO_SCAN, OnServoScan)
	ON_UPDATE_COMMAND_UI(ID_SERVO_CONNECT, OnUpdateControllerConnect)
	ON_UPDATE_COMMAND_UI(ID_SERVO_SCAN, OnUpdateIfControllerConnected)

	ON_MESSAGE(XTPWM_DOCKINGPANE_NOTIFY, OnDockingPaneNotify)
//	ON_MESSAGE(XTPWM_POPUPCONTROL_NOTIFY, OnPopUpStatusChange)
END_MESSAGE_MAP()


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
	: m_pIActiveScript(NULL), m_pIActiveScriptParse(NULL), m_scriptState(ScriptStopped)
{
	EnableAutomation();
//	AfxOleLockApp();
}

CMainFrame::~CMainFrame()
{
	// Release the language engine, since it may hold on to us
	if (m_pIActiveScriptParse)
		m_pIActiveScriptParse->Release();
	if (m_pIActiveScript) {
		m_pIActiveScript->Close();
		m_pIActiveScript->Release();
    }
//	AfxOleUnlockApp();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CXTPMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	EnableDocking(CBRS_ALIGN_ANY);

	if (!InitCommandBars())
	{
		return -1;      // fail to create
	}

	CXTPCommandBars* pCommandBars = GetCommandBars();
	pCommandBars->SetMenu(_T("Menu Bar"), IDR_MAINFRAME);
//	XTPImageManager()->SetMaskColor(RGB(0xff,0x00,0xff)); // #1DFF28

	/* create our command toolbar */
	CXTPToolBar* pCommandBar = (CXTPToolBar*)pCommandBars->Add(_T("Standard"), xtpBarTop, RUNTIME_CLASS(CXTPToolBar) );
	if (!pCommandBar || !pCommandBar->LoadToolBar(IDR_MAINFRAME)) {
		TRACE0("Failed to create Standard toolbar\n");
	}

	/* create our command toolbar */
	CXTPToolBar* pServoBar = (CXTPToolBar*)pCommandBars->Add(_T("Servos"), xtpBarTop, RUNTIME_CLASS(CXTPToolBar) );
	if (!pServoBar || !pServoBar->LoadToolBar(IDR_TOOLBAR_SERVO)) {
		TRACE0("Failed to create Script toolbar\n");
	}

	/* create our command toolbar */
	CXTPToolBar* pScriptBar = (CXTPToolBar*)pCommandBars->Add(_T("Script"), xtpBarTop, RUNTIME_CLASS(CXTPToolBar) );
	if (!pScriptBar || !pScriptBar->LoadToolBar(IDR_TOOLBAR_SCRIPT)) {
		TRACE0("Failed to create Script toolbar\n");
	}

	CXTPPaintManager::SetTheme(xtpThemeOfficeXP  );

	// Initialize the docking pane manager and set the
	// initial them for the docking panes.  Do this only after all
	// control bars objects have been created and docked.
	m_paneManager.InstallDockingPanes(this);
    // Set Office 2003 Theme
	m_paneManager.SetTheme(xtpPaneThemeOffice);


	// Create register pane
	CXTPDockingPane* pdpRegisters = m_paneManager.CreatePane(
		IDR_PANE_REGISTERS, CRect(0, 0,200, 120), xtpPaneDockLeft);
	OnDockingPaneNotify(XTP_DPN_SHOWWINDOW, (LPARAM)pdpRegisters );

	// Create register pane
	CXTPDockingPane* pdpServos = m_paneManager.CreatePane(
		IDR_PANE_SERVOS, CRect(0, 0,200, 120), xtpPaneDockLeft, pdpRegisters);
	OnDockingPaneNotify(XTP_DPN_SHOWWINDOW, (LPARAM)pdpServos );

	// Create console pane
	CXTPDockingPane* pdpConsole = m_paneManager.CreatePane(
		IDR_PANE_CONSOLE, CRect(0, 0,200, 120), xtpPaneDockBottom);
	OnDockingPaneNotify(XTP_DPN_SHOWWINDOW, (LPARAM)pdpConsole );

		// Create trace pane
	CXTPDockingPane* pdpTrace = m_paneManager.CreatePane(
		IDR_PANE_TRACE, CRect(0, 0,200, 120), xtpPaneDockBottom, pdpConsole);
	OnDockingPaneNotify(XTP_DPN_SHOWWINDOW, (LPARAM)pdpTrace );

	m_paneManager.AttachPane( pdpRegisters, pdpServos);
	m_paneManager.ShowPane( pdpServos );

	m_paneManager.AttachPane( pdpTrace, pdpConsole);
	m_paneManager.ShowPane( pdpTrace );

	/* attach tab frame */
	VERIFY(m_MTIClientWnd.Attach(this));
/*
	CXTPTabPaintManager* tpm = new CXTPTabPaintManager();
	tpm->SetAppearance( xtpTabAppearanceVisualStudio );
	tpm->DisableLunaColors( TRUE );
	tpm->SetColor( xtpTabColorWhidbey );
	tpm->m_bShowIcons = FALSE;
	m_MTIClientWnd.SetPaintManager( tpm );
*/

	DockRightOf( pServoBar, pCommandBar );
	DockRightOf( pScriptBar, pServoBar );
	pCommandBars->RecalcFrameLayout();


	m_pcontroller.CreateInstance( __uuidof(CServoController) );

	DisplayPopup("Welcome to Servo Workbench", "Click on the Connect toolbar button to connect to the servo controller. Samples are available from the File menu.", MB_ICONINFORMATION);

	return 0;
}


LRESULT CMainFrame::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam == XTP_DPN_SHOWWINDOW)
    {
        CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;
        
        if (!pPane->IsValid()) {
            switch (pPane->GetID()) {
            case IDR_PANE_SERVOS: {                           
                    if (m_wndServos.GetSafeHwnd() == 0)
                    {
                        m_wndServos.Create(
                            WS_CHILD|WS_CLIPCHILDREN|
                            WS_CLIPSIBLINGS|SS_CENTER,
                            CRect(0, 0, 0, 0), this, 0);
                    }
                    pPane->Attach(&m_wndServos);
                    break;
                }
            case IDR_PANE_REGISTERS: {                           
                    if (m_wndRegisters.GetSafeHwnd() == 0)
                    {
                        m_wndRegisters.Create("\n\nRegisters", 
                            WS_CHILD|WS_CLIPCHILDREN|
                            WS_CLIPSIBLINGS|SS_CENTER,
                            CRect(0, 0, 0, 0), this, 0);
                    }
                    pPane->Attach(&m_wndRegisters);
                    break;
                }
          case IDR_PANE_CONSOLE: {                           
                    if (m_wndConsole.GetSafeHwnd() == 0)
                    {
                        m_wndConsole.Create( 
                            WS_CHILD|WS_CLIPCHILDREN|
                            WS_CLIPSIBLINGS|SS_CENTER,
                            CRect(0, 0, 0, 0), this, 0);
                    }
                    pPane->Attach(&m_wndConsole);
                    break;
                }
          case IDR_PANE_TRACE: {                           
                    if (m_wndTrace.GetSafeHwnd() == 0)
                    {
                        m_wndTrace.Create( 
                            WS_CHILD|WS_CLIPCHILDREN|
                            WS_CLIPSIBLINGS|SS_CENTER,
                            CRect(0, 0, 0, 0), this, 0);
                    }
                    pPane->Attach(&m_wndTrace);
                    break;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CXTPMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

void CMainFrame::DisplayPopup(LPCTSTR text, LPCTSTR desc, UINT type) {
	/* clean up collection of popup wnds */
	CXTPPopupControl* wndPopup = NULL;
	for(PopupControls::iterator p=m_popups.begin(), _p=m_popups.end(); p!=_p; p++) {
		if( (*p)->GetPopupState() == xtpPopupStateClosed ) {
			wndPopup = *p;
			break;
		}
	}
	if( !wndPopup ) {
		/* create a new popup */
		wndPopup = new CXTPPopupControl();
		m_popups.insert( m_popups.end(), wndPopup );
	}

	ASSERT( wndPopup );

	wndPopup->RemoveAllItems();
	wndPopup->SetShowDelay(6000);

	wndPopup->SetPopupAnimation(xtpPopupAnimationSlide);
	wndPopup->SetAnimateDelay(100);

	wndPopup->SetTransparency(220);
	wndPopup->AllowMove(true);

	wndPopup->SetTheme(xtpPopupThemeOffice2003);
	wndPopup->GetPaintManager()->RefreshMetrics();

	/* popup metrics */
	CSize szPopup(230, 90);
	CPoint posPopup(10, 10);
	int leftMargin = 46, rightMargin = 8;
	int topMargin = 16, bottomMargin = 8;

	CXTPPopupItem *pItemText=NULL, *pItemDesc=NULL;
	
	/* create the title text */
	pItemText = wndPopup->AddItem( new CXTPPopupItem(CRect(leftMargin, topMargin, szPopup.cx-rightMargin, szPopup.cy-bottomMargin), text ) );
	pItemText->SetHyperLink(FALSE);
	pItemText->SetTextAlignment(DT_LEFT|DT_WORDBREAK);
	pItemText->CalculateHeight();
	pItemText->SetBold(TRUE);
	
	CRect  rc( pItemText->GetRect() );
	if( rc.right+rightMargin > szPopup.cx ) 
		szPopup.cx = rc.right + rightMargin;
	szPopup.cy = rc.bottom + bottomMargin;

	/* create the descriptive text */
	if( desc ) {
		pItemDesc = wndPopup->AddItem( new CXTPPopupItem(CRect(leftMargin, rc.bottom+10, szPopup.cx-rightMargin, szPopup.cy-bottomMargin), desc ) );
		pItemDesc->SetHyperLink(FALSE);
		pItemDesc->SetTextAlignment(DT_CENTER|DT_WORDBREAK);
		pItemDesc->CalculateHeight();
		CRect  rcDesc( pItemDesc->GetRect() );
		if( rcDesc.right+rightMargin > szPopup.cx ) 
			szPopup.cx = rcDesc.right + rightMargin;
		szPopup.cy = rcDesc.bottom + bottomMargin;
	}

	/* create the icon */
	CXTPPopupItem* pItemIcon = NULL;
	LPSTR idicon;
	switch( type ) {
		case MB_ICONINFORMATION: idicon = IDI_INFORMATION; break;
		case MB_ICONWARNING: idicon = IDI_WARNING; break;
		case MB_ICONERROR: idicon = IDI_ERROR; break;
		default: idicon = IDI_INFORMATION; break;
	}
	HICON hIcon = (HICON)LoadIcon(NULL, idicon );

	if(hIcon) {
		pItemIcon = (CXTPPopupItem*)wndPopup->AddItem( new CXTPPopupItem(CRect(8, topMargin, leftMargin+16, topMargin+16)) );
		pItemIcon->SetIcon( hIcon );
	}

	/* make sure popup reaches a certain height */
	if(szPopup.cy<120)
		szPopup.cy = 120;

	/* position of popup is relative to lower right hand corner of window */
	//posPopup+=szPopup;

	wndPopup->SetPopupSize(szPopup);
//	wndPopup->SetPopupPos( posPopup );
	wndPopup->Show(this);
}

HRESULT CMainFrame::GetControllers()
{
	_COM_SMARTPTR_TYPEDEF(ICatInformation, IID_ICatInformation);

	HRESULT hr;
	ICatInformationPtr ci;
	if( FAILED(hr = ci.CreateInstance( CLSID_StdComponentCategoriesMgr ) ))
		return hr;

	/* get enumerator for class id */
	CATID cat = CATID_SERVOCONTROLLER;
	IEnumGUID* e;
	if( FAILED(hr = ci->EnumClassesOfCategories(1, &cat, 0, NULL, &e ) ))
		return hr;

	/* enumerate over guids */
	GUID g;
	while( SUCCEEDED(hr = e->Next( 1, &g, NULL)) && (hr!=S_FALSE) ) {
		TRACE("ok\r\n");
	}

	e->Release();
	
	return S_OK;
}

HRESULT CMainFrame::GetScriptEngines()
{
	_COM_SMARTPTR_TYPEDEF(ICatInformation, IID_ICatInformation);

	HRESULT hr;
	ICatInformationPtr ci;
	if( FAILED(hr = ci.CreateInstance( CLSID_StdComponentCategoriesMgr ) ))
		return hr;

	/* get enumerator for class id */
	CATID cat = CATID_ActiveScript;
	IEnumGUID* e;
	if( FAILED(hr = ci->EnumClassesOfCategories(1, &cat, 0, NULL, &e ) ))
		return hr;

	/* enumerate over guids */
	GUID g;
	while( SUCCEEDED(hr = e->Next( 1, &g, NULL)) && (hr!=S_FALSE) ) {
		ScriptEngineInfo sei;

		LPBYTE clsidstr;
		if(UuidToString(&g, &clsidstr)==RPC_S_OK) {
				CString skey;
				skey.Format("CLSID\\{%s}", clsidstr);
				HKEY hkey;
				if(RegOpenKey(HKEY_CLASSES_ROOT, skey, &hkey) == ERROR_SUCCESS) {
					char name[BUFSIZ];
					LONG szname = sizeof(name);
					if(RegQueryValue(hkey, NULL, name, &szname) == ERROR_SUCCESS) {
						sei.clsid = g;
						sei.name = name;
						m_scriptEngines.insert(m_scriptEngines.end(), sei);
					}
				}
				RpcStringFree(&clsidstr);
		}
	}

	e->Release();
	
	return S_OK;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CXTPMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CXTPMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnUpdateIfScriptRunning(CCmdUI* pcmdui)
{
	pcmdui->Enable( m_scriptState == ScriptRunning );
}

void CMainFrame::OnUpdateIfScriptStopped(CCmdUI* pcmdui)
{
	pcmdui->Enable( m_scriptState == ScriptStopped );
}

void CMainFrame::OnUpdateIfScriptPaused(CCmdUI* pcmdui)
{
	pcmdui->Enable( m_scriptState == ScriptPaused );
}

void CMainFrame::OnUpdateIfScriptRunnable(CCmdUI* pcmdui)
{
	if(MDIGetActive()) {
		CDocument* pdoc = MDIGetActive()->GetActiveDocument();
		pcmdui->Enable( pdoc && pdoc->IsKindOf(RUNTIME_CLASS(CServoWorkbenchDoc)) && (m_scriptState != ScriptRunning) );
	}
}

void CMainFrame::OnUpdateIfScriptStoppable(CCmdUI* pcmdui)
{
	pcmdui->Enable( (m_scriptState == ScriptRunning) || (m_scriptState == ScriptPaused) );
}

void CMainFrame::OnScriptStop()
{
	if(m_pIActiveScript) {
		m_scriptState = ScriptStop;
		EXCEPINFO ei;
		memset(&ei, 0, sizeof(EXCEPINFO));
		m_pIActiveScript->SetScriptState( SCRIPTSTATE_DISCONNECTED  );
		m_pIActiveScript->InterruptScriptThread(SCRIPTTHREADID_BASE, &ei, /* SCRIPTINTERRUPT_DEBUG */ 0 );
	}
}

void CMainFrame::OnScriptPause()
{
	if(m_pIActiveScript) {
		m_scriptState = ScriptPause;
		EXCEPINFO ei;
		memset(&ei, 0, sizeof(EXCEPINFO));
		m_pIActiveScript->SetScriptState( SCRIPTSTATE_DISCONNECTED  );
		m_pIActiveScript->InterruptScriptThread(SCRIPTTHREADID_BASE, &ei, /* SCRIPTINTERRUPT_DEBUG */ 0 );
	}
}

void CMainFrame::OnScriptExecute()
{
	if( m_scriptState !=ScriptStopped ) {
		m_scriptState = ScriptRun;
	} else if(MDIGetActive()) {
		CServoWorkbenchDoc* pdoc = (CServoWorkbenchDoc*)MDIGetActive()->GetActiveDocument();
		ASSERT_KINDOF(CServoWorkbenchDoc, pdoc);
		CString m_strCode = pdoc->GetCode();

		// ##### BEGIN ACTIVEX SCRIPTING SUPPORT #####
		USES_CONVERSION;
		LPCOLESTR	lpszCode = T2COLE(LPCTSTR(m_strCode));

		Execute(lpszCode);
	}
}


void CMainFrame::OnUpdateControllerConnect(CCmdUI* pcmdui)
{
	pcmdui->SetCheck( m_pcontroller->isConnected );
}

void CMainFrame::OnUpdateIfControllerConnected(CCmdUI* pcmdui)
{
	pcmdui->Enable( m_pcontroller->isConnected );
}

void CMainFrame::OnServoConnect()
{
	if(m_pcontroller!=NULL) {
		if( m_pcontroller->isConnected )
			Disconnect();
		else
			Connect();
	}
}

void CMainFrame::OnServoScan()
{
	try{
		if( m_pcontroller->isConnected ) {
			m_pcontroller->Scan();

			/* notify windows of change */
			CXTPDockingPaneInfoList& panes = m_paneManager.GetPaneList();
			POSITION pos = panes.GetHeadPosition();
			for (int i=0;i < panes.GetCount();i++) {

				panes.GetNext(pos)->GetChild()->SendMessage( WM_SN_CONNECTED, 0, (LPARAM)this);
			}

			/* notify user */
			CString s;
			s.Format("There were %d device(s) found on the bus.", m_pcontroller->count );
			DisplayPopup("Scan completed", s, MB_ICONINFORMATION);
		} else
			DisplayPopup("Scan aborted", "You must be connected to a servo controller to perform a bus scan.", MB_ICONWARNING);
	} catch(HRESULT) {
			DisplayPopup("Scan failed!", "An error occured trying to scan bus devices on the servo controller.", MB_ICONERROR);
	}
}
