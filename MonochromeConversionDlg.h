#if !defined(AFX_MONOCHROMECONVERSIONDLG_H__EC591BFE_0F36_48E7_BD8A_4DA23DF5BA79__INCLUDED_)
#define AFX_MONOCHROMECONVERSIONDLG_H__EC591BFE_0F36_48E7_BD8A_4DA23DF5BA79__INCLUDED_

#pragma once

// MonochromeConversionDlg.h : header file
//

#include "ColorButton.h"
#include "Dib.h"

/////////////////////////////////////////////////////////////////////////////
// CMonochromeConversionDlg dialog

class CMonochromeConversionDlg : public CDialog
{
// Construction
public:
	CMonochromeConversionDlg(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(CMonochromeConversionDlg)
	enum { IDD = IDD_MONOCHROME };
	CColorButton	m_ColorDark;
	CColorButton	m_ColorBright;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMonochromeConversionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DoIt();
	void Undo();
	CDib m_PreviewUndoDib;

	// Generated message map functions
	//{{AFX_MSG(CMonochromeConversionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnColorDark();
	afx_msg void OnColorBright();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MONOCHROMECONVERSIONDLG_H__EC591BFE_0F36_48E7_BD8A_4DA23DF5BA79__INCLUDED_)
