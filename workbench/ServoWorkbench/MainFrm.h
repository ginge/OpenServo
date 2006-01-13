// MainFrm.h : interface of the CMainFrame class
//


#pragma once


#include <Activscp.h>

#include "mfcdual.h"
#include "DockPaneWnd.h"
#include "servolistctrl.h"
#include "registersToolWnd.h"
#include "OutputWnd.h"

#include <vector>
using namespace std;


class CMainFrame : public CXTPMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Operations
public:
	inline IServoControllerPtr GetController() { return m_pcontroller; }


	inline bool isConnected() { return (m_pcontroller!=NULL) && m_pcontroller->isConnected; }

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	HRESULT GetControllers();
	
	/* displays an Instant Messenger like popup, this sort of display is more user friendly than AfxMessageBox's */
	void DisplayPopup(LPCTSTR text, LPCTSTR desc, UINT type);


// Attributes
public:
	IServoControllerPtr m_pcontroller;

	CXTPDockingPaneManager m_paneManager;
	CXTPTabClientWnd m_MTIClientWnd;

	CDockPaneWnd<CServoListCtrl> m_wndServos;
	CDockPaneWnd<CRegistersToolWnd> m_wndRegisters;
	CDockPaneWnd<COutputWnd> m_wndConsole;
	CDockPaneWnd<COutputWnd> m_wndTrace;

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CXTPStatusBar  m_wndStatusBar;

	/* stored any active popups */
	typedef vector<CXTPPopupControl*> PopupControls;
	PopupControls m_popups;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnScriptStop();
	afx_msg void OnScriptPause();
	afx_msg void OnScriptExecute();
	
	afx_msg void OnServoConnect();
	afx_msg void OnServoScan();

	void OnUpdateIfScriptRunning(CCmdUI* pcmdui);
	void OnUpdateIfScriptStopped(CCmdUI* pcmdui);
	void OnUpdateIfScriptStoppable(CCmdUI* pcmdui);
	void OnUpdateIfScriptRunnable(CCmdUI* pcmdui);
	void OnUpdateIfScriptPaused(CCmdUI* pcmdui);
	void OnUpdateIfControllerConnected(CCmdUI* pcmdui);
	void OnUpdateControllerConnect(CCmdUI* pcmdui);

	DECLARE_MESSAGE_MAP()

	// ##### BEGIN ACTIVEX SCRIPTING SUPPORT #####
protected:
	DECLARE_INTERFACE_MAP()
	DECLARE_DISPATCH_MAP()

	IActiveScript        *m_pIActiveScript;
	IActiveScriptParse   *m_pIActiveScriptParse;

public:
	BEGIN_INTERFACE_PART(ScriptSite, IActiveScriptSite)
        STDMETHOD(GetLCID)(LCID *plcid);
		STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
		STDMETHOD(GetDocVersionString)(BSTR *pszVersion);
		STDMETHOD(RequestItems)(void);
		STDMETHOD(RequestTypeLibs)(void);
		STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
		STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState);
		STDMETHOD(OnScriptError)(IActiveScriptError *pscripterror);
		STDMETHOD(OnEnterScript)(void);
		STDMETHOD(OnLeaveScript)(void);
    END_INTERFACE_PART(ScriptSite)

	BEGIN_INTERFACE_PART(ScriptSiteWindow, IActiveScriptSiteWindow)
		STDMETHOD(GetWindow)(HWND *phwnd);
		STDMETHOD(EnableModeless)(BOOL fEnable);
    END_INTERFACE_PART(ScriptSiteWindow)


	/* methods in the IServoWorkbenchSystem Inteface */
	afx_msg HRESULT Connect();
	afx_msg HRESULT Disconnect();
	afx_msg HRESULT sleep(LONG msecs);
	afx_msg HRESULT alert(LPCTSTR msg);
	afx_msg HRESULT print(LPCTSTR msg);
	afx_msg HRESULT trace(LPCTSTR msg);

	enum 
	{
		dispidSleep = 1L,
		dispidAlert = 2L,
		dispidPrint = 3L,
		dispidTrace = 4L,
		dispidConnect = 5L
	};

	BOOL LoadScript();
	HRESULT CreateScriptEngine();
	HRESULT InitScriptEngine();
	HRESULT Execute(LPCWSTR lpszCode);

	/* enumerates the system for the list of installed script engines */
	typedef struct {
		GUID clsid;
		CString name;
	} ScriptEngineInfo;
	vector<ScriptEngineInfo> m_scriptEngines;
	HRESULT GetScriptEngines();
	
	/* the thread that will execute our script */
	typedef struct {
		CMainFrame* pThis;
		size_t dwScriptLen;
		LPWSTR lpszScript;
	} ScriptCookie;

	typedef enum {
		ScriptStop = 0x00,
		ScriptPause,
		ScriptRun,
		ScriptStopped = 0x10,
		ScriptPaused,
		ScriptRunning
	} ScriptState;
	ScriptState m_scriptState;

	uintptr_t m_tid_runscript;
	static void s_runscript(void* lpvoid) { ScriptCookie* cookie = (ScriptCookie*)lpvoid; cookie->pThis->runscript(*cookie); free(cookie->lpszScript); delete cookie; }
	virtual void runscript(ScriptCookie& cookie);
	// #####  END  ACTIVEX SCRIPTING SUPPORT #####
};


