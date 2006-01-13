#pragma once


// CServoListCtrl

class CServoListCtrl : public CListBox
{
	DECLARE_DYNAMIC(CServoListCtrl)

	/* our data structure storing the properties we will display for each servo */
	typedef struct {
		IServoPtr ptr;
		DWORD address;
		CString name;
		int position;
		int targetPosition;
		int current;
		BOOL valid;
	} ServoInfo;

public:
	CServoListCtrl();
	virtual ~CServoListCtrl();

	/* get the controller, defaults to locating it in the mainframe */
	virtual IServoControllerPtr GetController();

	/* refresh servo values */
	BOOL Refresh();

	/* loads the list of servos to display */
	virtual BOOL LoadServos();

	/* adds the given servo to the list */
	virtual int InsertServo(int index, IServoPtr pservo);

protected:
	void DrawProgress(CDC* pdc, CRect bounds, int from, int length, COLORREF color);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnControllerConnected( WPARAM wp, LPARAM lp );
	afx_msg LRESULT OnControllerDisconnecting( WPARAM wp, LPARAM lp );

public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT /*lpCompareItemStruct*/);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
	afx_msg void OnTimer(UINT nIDEvent);
};


