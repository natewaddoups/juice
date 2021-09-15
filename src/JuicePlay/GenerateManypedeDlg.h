#pragma once


// CGenerateManypedeDlg dialog

class CGenerateManypedeDlg : public CJuiceDlg
{
	DECLARE_DYNAMIC(CGenerateManypedeDlg)

public:
	CGenerateManypedeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGenerateManypedeDlg();

// Dialog Data
	enum { IDD = IDD_GENERATE_MANYPEDE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	Juice::Model *m_pModel;

	// Number of segments in the Manypede
	UINT m_uSegments;

	// Number of contact points at any one time
	UINT m_uConcurrentCycles;

	// Relative phase of the horizonal and vertical hinges
	int m_iPhaseOffset;

	// Range of the horizontal joints, in degrees
	INT m_iHorizontal;

	// Range of the vertical joints, in degrees
	INT m_iVertical;


	afx_msg void OnBnClickedOk();
};

