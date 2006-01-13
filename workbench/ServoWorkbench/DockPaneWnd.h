// DockPaneWnd.h : main header file for the SAMPLEAPP application
//

#if !defined(__DOCKPANEWND_H__)
#define __DOCKPANEWND_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<class BASE_CLASS>
class CDockPaneWnd : public BASE_CLASS
{
protected:

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CREATE:
			{
				if (BASE_CLASS::WindowProc(message, wParam, lParam) == -1)
					return -1;

				SetFont(XTPPaintManager()->GetRegularFont());

				return 0;
			}
			
		case WM_NCPAINT:
			{
				CWindowDC dc(this);
				
				CRect rc;
				GetWindowRect(&rc);
				
				int cx = rc.Width(); 
				int cy = rc.Height();
				
				const COLORREF clrFrame =  GetXtremeColor(XPCOLOR_3DSHADOW);
				dc.Draw3dRect(0, 0, cx, cy, clrFrame, clrFrame);
				
				return TRUE;
			}
			
		case WM_NCCALCSIZE:
			{
				NCCALCSIZE_PARAMS FAR* lpncsp = (NCCALCSIZE_PARAMS FAR*)lParam;
				
				// adjust non-client area for border space
				lpncsp->rgrc[0].left   += 1;
				lpncsp->rgrc[0].top    += 1;
				lpncsp->rgrc[0].right  -= 1;
				lpncsp->rgrc[0].bottom -= 1;

				return TRUE;
			}
		}
		
		return BASE_CLASS::WindowProc(message, wParam, lParam);
	}
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__DOCKPANEWND_H__)
