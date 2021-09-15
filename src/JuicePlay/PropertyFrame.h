#pragma once


// CPropertyFrame frame

class CPropertyFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CPropertyFrame)
protected:
	CPropertyFrame();           // protected constructor used by dynamic creation
	virtual ~CPropertyFrame();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


