#pragma once


// CRegistersToolWnd

#define RTW_ID_PROPLIST	100
#define RTW_ID_SERVO	101
#define RTW_ID_UPDATE	102


class CRegistersToolWnd : public CWnd
{
	DECLARE_DYNAMIC(CRegistersToolWnd)

public:
	CRegistersToolWnd();
	virtual ~CRegistersToolWnd();

	IServoControllerPtr GetController();

	CComboBox m_cb_servo;
	CXTPPropertyGrid m_properties;
	CXTButton m_btn_update;

	BOOL Refresh();

protected:
	CFont m_font;
	IServoPtr m_servo;


	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnControllerConnected( WPARAM wp, LPARAM lp );
	afx_msg LRESULT OnControllerDisconnecting( WPARAM wp, LPARAM lp );
	afx_msg void OnLoadServos();
	afx_msg void OnServoChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	inline afx_msg void OnUpdate() { Refresh(); }
};


