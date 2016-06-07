#if !defined(AFX_SHARPENDLG_H__414863E8_CA8A_477E_949E_FA089FAC56DE__INCLUDED_)
#define AFX_SHARPENDLG_H__414863E8_CA8A_477E_949E_FA089FAC56DE__INCLUDED_

#pragma once

// SharpenDlg.h : header file
//

#include "Dib.h"

/////////////////////////////////////////////////////////////////////////////
// CSharpenDlg dialog

class CSharpenDlg : public CDialog
{
// Construction
public:
	CSharpenDlg(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(CSharpenDlg)
	enum { IDD = IDD_SHARPEN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSharpenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Unsharp mask algorithm
	//		diff = orig_pix - blurred_pix
	//		sharp_pix = orig_pix + (amount * diff)
	//
	// nAmount (increasing sharpness)
	//		1: amount = 1/10 (0.1)
	//		2: amount = 2/10 (0.2)
	//		..
	BOOL UnsharpMask(int nAmount, CDib* pOrigDib, CDib* pDib);
	void Sharpen(int nAmount);
	void DoIt();
	CDib m_OrigDib;
	CDib m_BlurredDib;

	// Generated message map functions
	//{{AFX_MSG(CSharpenDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHARPENDLG_H__414863E8_CA8A_477E_949E_FA089FAC56DE__INCLUDED_)
