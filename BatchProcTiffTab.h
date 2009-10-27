#if !defined(AFX_BATCHPROCTIFFTAB_H__E58C74E2_1CCB_41B8_920C_951373FB0CEC__INCLUDED_)
#define AFX_BATCHPROCTIFFTAB_H__E58C74E2_1CCB_41B8_920C_951373FB0CEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BatchProcTiffTab.h : header file
//

#include "TabPageSSL.h"

/////////////////////////////////////////////////////////////////////////////
// CBatchProcTiffTab dialog

class CBatchProcTiffTab : public CTabPageSSL
{
// Construction
public:
	CBatchProcTiffTab(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CBatchProcTiffTab)
	enum { IDD = IDD_BATCH_PROC_TIFFTAB };
	int		m_nCompression;
	int		m_nJpegQuality;
	BOOL	m_bForceCompression;
	BOOL	m_bWorkOnAllPages;
	//}}AFX_DATA
	CString m_sPdfPaperSize;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchProcTiffTab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBatchProcTiffTab)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokComboPaperSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPROCTIFFTAB_H__E58C74E2_1CCB_41B8_920C_951373FB0CEC__INCLUDED_)
