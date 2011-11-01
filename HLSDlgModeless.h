#if !defined(AFX_HLSDLGMODELESS_H__B2E689E1_C05D_11D4_B4BC_00010223337E__INCLUDED_)
#define AFX_HLSDLGMODELESS_H__B2E689E1_C05D_11D4_B4BC_00010223337E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HLSDlgModeless.h : header file
//

#include "resource.h"
#include "Dib.h"
#include "MacProgressCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CHLSDlgModeless dialog

class CHLSDlgModeless : public CDialog
{
// Construction
public:
	CHLSDlgModeless(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(CHLSDlgModeless)
	enum { IDD = IDD_HLS };
	CMacProgressCtrl	m_BandMax;
	CMacProgressCtrl	m_Band7;
	CMacProgressCtrl	m_Band6;
	CMacProgressCtrl	m_Band5;
	CMacProgressCtrl	m_Band4;
	CMacProgressCtrl	m_Band3;
	CMacProgressCtrl	m_Band2;
	CMacProgressCtrl	m_Band1;
	CMacProgressCtrl	m_Band0;
	CMacProgressCtrl	m_BandMin;
	BOOL	m_bShowOriginal;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHLSDlgModeless)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL IsModified();
	BOOL DoIt();
	BOOL UpdateHysto();
	BOOL AdjustColor(BOOL bAdjustPreviewDib);
	BOOL m_bFast;
	CDib m_OldPreviewDib;
	// Generated message map functions
	//{{AFX_MSG(CHLSDlgModeless)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonUndo();
	afx_msg void OnClose();
	afx_msg void OnCheckShowOriginal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HLSDLGMODELESS_H__B2E689E1_C05D_11D4_B4BC_00010223337E__INCLUDED_)
