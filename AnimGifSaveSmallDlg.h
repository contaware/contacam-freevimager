#if !defined(AFX_ANIMGIFSAVESMALLDLG_H__081A6559_C503_4DC4_B42C_530C7E8FDBA5__INCLUDED_)
#define AFX_ANIMGIFSAVESMALLDLG_H__081A6559_C503_4DC4_B42C_530C7E8FDBA5__INCLUDED_

#pragma once

// AnimGifSaveSmallDlg.h : header file
//

#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveSmallDlg dialog

class CAnimGifSaveSmallDlg : public CDialog
{
// Construction
public:
	CAnimGifSaveSmallDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimGifSaveSmallDlg)
	enum { IDD = IDD_ANIMGIF_SAVE_SMALL };
	CListCtrlEx	m_ListDelays;
	UINT	m_uiPlayTimes;
	int		m_nLoopInfinite;
	//}}AFX_DATA
	CArray<int, int>* m_pDelaysArray;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimGifSaveSmallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimGifSaveSmallDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLoop();
	afx_msg void OnRadioLoopInfinite();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMGIFSAVESMALLDLG_H__081A6559_C503_4DC4_B42C_530C7E8FDBA5__INCLUDED_)
