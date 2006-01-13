
#include "stdafx.h"
#include "MainFrm.h"

#include "defines.h"



HRESULT CMainFrame::Connect()
{
	try{
		HRESULT hr=S_OK;
		if(!m_pcontroller->isConnected && SUCCEEDED(hr=m_pcontroller->Connect(""))) {
			m_pcontroller->Scan();

			CXTPDockingPaneInfoList& panes = m_paneManager.GetPaneList();
			POSITION pos = panes.GetHeadPosition();
			for (int i=0;i < panes.GetCount();i++) {

				panes.GetNext(pos)->GetChild()->SendMessage( WM_SN_CONNECTED, 0, (LPARAM)this);
			}

			DisplayPopup("Connected!", "You are now connected to the servo controller. An bus scan was performed to search for any servos attached to the controller.", MB_ICONINFORMATION);
		}
		return hr;
	} catch(HRESULT hr) {
			DisplayPopup("Connect failed!", "An error occured trying to connected to the servo controller.", MB_ICONERROR);
		return hr;
	}
}

HRESULT CMainFrame::Disconnect()
{
	try{
		HRESULT hr=S_OK;

		/* notify windows of intent to disconnect */
		CXTPDockingPaneInfoList& panes = m_paneManager.GetPaneList();
		POSITION pos = panes.GetHeadPosition();
		for (int i=0;i < panes.GetCount();i++)
			panes.GetNext(pos)->GetChild()->SendMessage( WM_SN_DISCONNECTING, 0, (LPARAM)this);

		/* perform disconnection */
		if(m_pcontroller->isConnected && SUCCEEDED(hr=m_pcontroller->Disconnect())) {
	
			/* notify windows of disconnection */
			CXTPDockingPaneInfoList& panes = m_paneManager.GetPaneList();
			POSITION pos = panes.GetHeadPosition();
			for (int i=0;i < panes.GetCount();i++)
				panes.GetNext(pos)->GetChild()->SendMessage( WM_SN_DISCONNECTED, 0, (LPARAM)this);

			DisplayPopup("Disconnected!", "You are now disconnected from the servo controller.", MB_ICONINFORMATION);
		}
		return hr;
	} catch(HRESULT hr) {
		m_pcontroller->Release();
		return S_OK;
	}
}

HRESULT CMainFrame::sleep(LONG msecs)
{
	::Sleep(msecs);
	return S_OK;
}

HRESULT CMainFrame::alert(LPCTSTR msg)
{
	::AfxMessageBox( msg );
	return S_OK;
}

HRESULT CMainFrame::print(LPCTSTR msg)
{
	m_wndConsole.Write( msg );
	m_wndConsole.Write( "\r\n" );
	return S_OK;
}

HRESULT CMainFrame::trace(LPCTSTR msg)
{
	m_wndTrace.Write( msg );
	m_wndTrace.Write( "\r\n" );
	return S_OK;
}
