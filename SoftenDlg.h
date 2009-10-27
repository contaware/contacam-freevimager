#if !defined(AFX_SOFTENDLG_H__3D6462DC_50E7_4CDE_8262_D4183F1222A6__INCLUDED_)
#define AFX_SOFTENDLG_H__3D6462DC_50E7_4CDE_8262_D4183F1222A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SoftenDlg.h : header file
//

#include "Dib.h"

/////////////////////////////////////////////////////////////////////////////
// CSoftenDlg dialog

class CSoftenDlg : public CDialog
{
// Construction
public:
	CSoftenDlg(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(CSoftenDlg)
	enum { IDD = IDD_SOFTEN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoftenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DoIt();
	BOOL DoItBigPicture();
	BOOL Soften(CDib* pDib, CDib* pSrcDib, int nSoftness);
	void Undo();
	CDib m_PreviewUndoDib;

	// Generated message map functions
	//{{AFX_MSG(CSoftenDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOFTENDLG_H__3D6462DC_50E7_4CDE_8262_D4183F1222A6__INCLUDED_)
