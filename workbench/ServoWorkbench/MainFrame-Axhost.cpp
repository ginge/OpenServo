
#include "stdafx.h"
#include "MainFrm.h"
#include <process.h>


BEGIN_DISPATCH_MAP(CMainFrame, CXTPMDIFrameWnd)
	DISP_FUNCTION_ID(CMainFrame, "sleep", dispidSleep, sleep, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CMainFrame, "alert", dispidAlert, alert, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CMainFrame, "print", dispidPrint, print, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CMainFrame, "trace", dispidTrace, trace, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CMainFrame, "Connect", dispidConnect, Connect, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

// Note: we add support for IID_IServoWorkbench to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {7C314D49-8E58-4DC4-83D5-802FEB298443}
static const IID IID_IServoWorkbenchSystem =
{ 0x7C314D49, 0x8E58, 0x4DC4, { 0x83, 0xD5, 0x80, 0x2F, 0xEB, 0x29, 0x84, 0x44 } };

BEGIN_INTERFACE_MAP(CMainFrame, CXTPMDIFrameWnd)
	INTERFACE_PART(CMainFrame, IID_IServoWorkbenchSystem, Dispatch)
END_INTERFACE_MAP()


//DELEGATE_DUAL_INTERFACE(CMainFrame, System)


//const IID LIBID_VBSCRIPT = {0xF3B1C684,0x05C7,0x8BF7,{0xEE,0x51,0x3E,0x5A,0xB5,0x21}};
const IID CLSID_VBScript = {0xb54f3741, 0x5b07, 0x11cf, {0xa4, 0xb0, 0x0, 0xaa, 0x0, 0x4a, 0x55, 0xe8 } };
const IID CLSID_JavaScript = {0xf414c260, 0x6ac0, 0x11cf, {0xb6,0xd1, 0x00,0xaa,0x00,0xbb,0xbb,0x58} };


/////////////////////////////////////////////////////////////////////////////
//COM interface implementation
ULONG FAR EXPORT CMainFrame::XScriptSite::AddRef()
{
    METHOD_PROLOGUE(CMainFrame, ScriptSite)
    return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CMainFrame::XScriptSite::Release()
{
    METHOD_PROLOGUE(CMainFrame, ScriptSite)
    return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CMainFrame::XScriptSite::QueryInterface(
    REFIID iid, void FAR* FAR* ppvObj)
{
    METHOD_PROLOGUE(CMainFrame, ScriptSite)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}




ULONG FAR EXPORT CMainFrame::XScriptSiteWindow::AddRef()
{
    METHOD_PROLOGUE(CMainFrame, ScriptSiteWindow)
    return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CMainFrame::XScriptSiteWindow::Release()
{
    METHOD_PROLOGUE(CMainFrame, ScriptSiteWindow)
    return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CMainFrame::XScriptSiteWindow::QueryInterface(
    REFIID iid, void FAR* FAR* ppvObj)
{
    METHOD_PROLOGUE(CMainFrame, ScriptSiteWindow)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}


#if 0
ULONG FAR EXPORT CMainFrame::XSystem::AddRef()
{
    METHOD_PROLOGUE(CMainFrame, System)
    return pThis->ExternalAddRef();
}

ULONG FAR EXPORT CMainFrame::XSystem::Release()
{
    METHOD_PROLOGUE(CMainFrame, System)
    return pThis->ExternalRelease();
}

HRESULT FAR EXPORT CMainFrame::XSystem::QueryInterface(
    REFIID iid, void FAR* FAR* ppvObj)
{
    METHOD_PROLOGUE(CMainFrame, System)
    return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}
#endif



/////////////////////////////////////////////////////////////////////////////
// ActiveScript hosting

// ##### BEGIN ACTIVEX SCRIPTING SUPPORT #####
//***************************************************************************
// IActiveScriptSite Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::GetLCID(LCID *plcid)
{
  return E_NOTIMPL;     // Use system settings
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::GetItemInfo
(
  LPCOLESTR   pstrName,
  DWORD       dwReturnMask,
  IUnknown**  ppunkItemOut,
  ITypeInfo** pptinfoOut
)
{
	//HRESULT hr;
	METHOD_PROLOGUE(CMainFrame, ScriptSite)
	USES_CONVERSION;

	if (dwReturnMask & SCRIPTINFO_ITYPEINFO) {
		if (!pptinfoOut)
			return E_INVALIDARG;
		*pptinfoOut = NULL;
	}

	if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
	{
		if (!ppunkItemOut)
			return E_INVALIDARG;
		*ppunkItemOut = NULL;

		/* convert the given name */
		LPTSTR lpszName = OLE2T(pstrName);
		IUnknown* pinterface = NULL;
    
		/* get interface of requested object */
		if(!_tcsicmp("controller", lpszName))
			pinterface = pThis->GetController().GetInterfacePtr();
		else if(!_tcsicmp("system", lpszName)) {
			//ASSERT( SUCCEEDED( pThis->QueryInterface(IID_IServoWorkbenchSystem, (LPVOID*)&pinterface) ));
			pinterface = (LPDISPATCH)pThis->GetInterface(&IID_IDispatch);
				//pThis->GetIDispatch(FALSE);
		}

		/* get the dispatch interface on the interface */
		if(pinterface) {
			pinterface->QueryInterface( IID_IDispatch, (void**)ppunkItemOut);
			return (*ppunkItemOut == NULL) ? TYPE_E_ELEMENTNOTFOUND : S_OK;
		}
	}
	
	return TYPE_E_ELEMENTNOTFOUND;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::GetDocVersionString(BSTR *pbstrVersion)
{
	*pbstrVersion = ::SysAllocString( L"CMainFrame-v1.0" );
	return S_OK;   // UNDONE: Implement this method
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::RequestItems(void)
{
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::RequestTypeLibs(void)
{
	METHOD_PROLOGUE(CMainFrame,ScriptSite);
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::OnScriptTerminate(const VARIANT   *pvarResult,const EXCEPINFO *pexcepinfo)
{
	// UNDONE: Put up error dlg here
	return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
	// Don't care about notification
	return S_OK;
}


//---------------------------------------------------------------------------
// Display the error
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::OnScriptError(IActiveScriptError *pse)
{
	METHOD_PROLOGUE(CMainFrame,ScriptSite);
  
	CString   strError;
	CString	  strArrow;
	CString	  strDesc;
	CString	  strLine;
	
	EXCEPINFO ei;
	DWORD     dwSrcContext;
	ULONG     ulLine;
	LONG      ichError;
	BSTR      bstrLine = NULL;
	
	HRESULT   hr;

	pse->GetExceptionInfo(&ei);
	pse->GetSourcePosition(&dwSrcContext, &ulLine, &ichError);
	hr = pse->GetSourceLineText(&bstrLine);
	if (hr)
		hr = S_OK;  // Ignore this error, there may not be source available
  
	if (!hr)
    {
		strError=ei.bstrSource;
		
		strDesc=ei.bstrDescription;
		strLine=bstrLine;

		if(strDesc.IsEmpty())
			strDesc = "No text.";

		if (ichError > 0 && ichError < 255)
		{
			strArrow=CString(_T('-'),ichError);
			strArrow.SetAt(ichError-1,_T('v'));
			
		}

		CString strErrorCopy=strError;
		strError.Format(_T("Error on line %d, char %ld, error 0x%04x : %s\r\n"),
						ulLine,
						ichError,
						(int)ei.wCode,
						LPCTSTR(strDesc),
						LPCTSTR(strArrow),
						LPCTSTR(strLine));
		
		pThis->m_wndTrace.Write(strError);
    }

	if (bstrLine)
		SysFreeString(bstrLine);

	return hr;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::OnEnterScript(void)
{
  // No need to do anything
  return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSite::OnLeaveScript(void)
{
  // No need to do anything
  return S_OK;
}



//***************************************************************************
// IActiveScriptSiteWindow Interface
//***************************************************************************

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSiteWindow::GetWindow(HWND *phwndOut)
{
  if (!phwndOut)
    return E_INVALIDARG;

  METHOD_PROLOGUE(CMainFrame,ScriptSiteWindow);

  *phwndOut = AfxGetMainWnd()->GetSafeHwnd();
  return S_OK;
}


//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
STDMETHODIMP CMainFrame::XScriptSiteWindow::EnableModeless(BOOL fEnable)
{
  return S_OK;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HRESULT CMainFrame::CreateScriptEngine()
{
	HRESULT hr;

//	LPCOLESTR pstrItemName = T2COLE(_T("document"));

	if (m_pIActiveScript)
		return S_FALSE;   // Already created it

	// Create the ActiveX Scripting Engine
	hr = CoCreateInstance(CLSID_JavaScript, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void **)&m_pIActiveScript);
	if (hr)
	{
		return E_FAIL;
	}
  
	// Script Engine must support IActiveScriptParse for us to use it
	hr = m_pIActiveScript->QueryInterface(IID_IActiveScriptParse, (void **)&m_pIActiveScriptParse);
	if (hr)
	{
		return hr;
	}
  
	hr = m_pIActiveScript->SetScriptSite(&m_xScriptSite);
	if (hr)
		return hr;

	return InitScriptEngine();
}

HRESULT CMainFrame::InitScriptEngine()
{
	HRESULT hr;

	// InitNew the object:
	hr=m_pIActiveScriptParse->InitNew();

	/* add xml document */
	USES_CONVERSION;
	if( FAILED( hr=m_pIActiveScript->AddNamedItem(T2COLE(_T("controller")), SCRIPTITEM_ISVISIBLE) ))
		return hr;
	if( FAILED( hr=m_pIActiveScript->AddNamedItem(T2COLE(_T("system")), SCRIPTITEM_ISVISIBLE) ))
		return hr;

	return hr;
}


BOOL CMainFrame::LoadScript()
{
	CFileDialog dlg(TRUE, 
					_T("*.txt"),
					_T("*.txt"),
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					_T("Text files (*.txt)"));

	if(dlg.DoModal()==IDOK)
	{
		CString m_strCode;
		m_strCode.Empty();

		TCHAR szBuffer[BUFSIZ];
		size_t bytes_read=1;
		CFile fh(dlg.GetPathName(), CFile::modeRead );
		if(fh.m_hFile != CFile::hFileNull) {
			while( bytes_read>0 ) {
				bytes_read = fh.Read(szBuffer, sizeof(szBuffer)-1);
				szBuffer[bytes_read] = 0;
				m_strCode+=szBuffer;
			}
		}
		fh.Close();

		// ##### BEGIN ACTIVEX SCRIPTING SUPPORT #####
		USES_CONVERSION;
		EXCEPINFO   ei;
		LPCOLESTR	lpszCode = T2COLE(LPCTSTR(m_strCode));
		HRESULT hr=m_pIActiveScriptParse->ParseScriptText(lpszCode, NULL, NULL, NULL, 0, 0, 0L, NULL, &ei);
		// ##### END ACTIVEX SCRIPTING SUPPORT #####
	
		if(FAILED(hr)) {
			return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

HRESULT CMainFrame::Execute(LPCWSTR lpszCode)
{

	ScriptCookie* sc = new ScriptCookie;
	sc->pThis = this;
	sc->dwScriptLen = wcslen(lpszCode);
	sc->lpszScript = (LPWSTR)malloc(sc->dwScriptLen * 2 + 2);
	wcscpy(sc->lpszScript, lpszCode);

#if 1
	m_tid_runscript = _beginthread( s_runscript, 0, sc);
#else
	runscript(*sc);
	delete sc;
#endif

	return S_OK;
}

void CMainFrame::runscript(ScriptCookie& sc)
{
	HRESULT hr;
	CoInitializeEx(0, COINIT_MULTITHREADED);

	/* take a reference to the servo controller so it wont go out of scope */
	IServoControllerPtr pcontroller = m_pcontroller;

	if(!m_pIActiveScript)
		CreateScriptEngine();

	EXCEPINFO   ei;
	if(FAILED( hr=m_pIActiveScriptParse->ParseScriptText(sc.lpszScript, NULL, NULL, NULL, 0, 0, 0L, NULL, &ei) )) {
		return;
	}

    m_scriptState = ScriptRun;
	while( (m_scriptState&0x0f) == ScriptRun ) {
		m_scriptState = ScriptRunning;
		if(FAILED(hr=m_pIActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED))) {
			AfxMessageBox(_T("Failed to execute file."));
			return;
		}

		/* if state is still 'Running' then program completed, switch to stop state */
		if(m_scriptState == ScriptRunning)
			m_scriptState = ScriptStop;

		if( (m_scriptState&0x0f) == ScriptPause ) {
			m_scriptState = ScriptPaused;
			while( (m_scriptState&0x0f) == ScriptPause ) {
				Sleep(50);
			}
		}
	}

    m_scriptState = ScriptStopped;
	m_pIActiveScriptParse->Release();
	m_pIActiveScript->Release();
	m_pIActiveScriptParse = NULL;
	m_pIActiveScript = NULL;
}
