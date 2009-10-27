#if !defined(AFX_BATCHPROCSHRINKTAB_H__E529BF3B_3B2F_4668_A7AD_B84EC761BB13__INCLUDED_)
#define AFX_BATCHPROCSHRINKTAB_H__E529BF3B_3B2F_4668_A7AD_B84EC761BB13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BatchProcShrinkTab.h : header file
//

#include "TabPageSSL.h"

/////////////////////////////////////////////////////////////////////////////
// CBatchProcShrinkTab dialog

class CBatchProcShrinkTab : public CTabPageSSL
{
// Construction
public:
	CBatchProcShrinkTab(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CBatchProcShrinkTab)
	enum { IDD = IDD_BATCH_PROC_SHRINKTAB };
	BOOL	m_bShrinkingPictures;
	BOOL	m_bSharpen;
	int		m_nPixelsPercentSel;
	int		m_nShrinkingPercent;
	int		m_nShrinkingPixels;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchProcShrinkTab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBatchProcShrinkTab)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckShrinkingpictures();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPROCSHRINKTAB_H__E529BF3B_3B2F_4668_A7AD_B84EC761BB13__INCLUDED_)
