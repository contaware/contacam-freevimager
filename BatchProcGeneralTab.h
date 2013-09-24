#if !defined(AFX_BATCHPROCGENERALTAB_H__C05D0871_354A_4B9A_949F_88B6F42D3E7A__INCLUDED_)
#define AFX_BATCHPROCGENERALTAB_H__C05D0871_354A_4B9A_949F_88B6F42D3E7A__INCLUDED_

#pragma once

// BatchProcGeneralTab.h : header file
//

#include "TabPageSSL.h"

/////////////////////////////////////////////////////////////////////////////
// CBatchProcGeneralTab dialog

class CBatchProcGeneralTab : public CTabPageSSL
{
public:
	enum EXTCHANGETYPE	{	NO_CHANGE=0,
							AUTO_CHANGE=1,
							ALL_JPEG=2,
							ALL_PNG=3,
							ALL_GIF=4,
							ALL_TIFF=5,
							ALL_BMP=6,
							ALL_PCX=7,
							ALL_EMF=8};

// Construction
public:
	CBatchProcGeneralTab(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CBatchProcGeneralTab)
	enum { IDD = IDD_BATCH_PROC_GENERALTAB };
	BOOL	m_bMergeXmp;
	int m_nIptcPriority;
	double	m_dFrameRate;
	BOOL	m_bExifTimeOffset;
	int		m_nExifTimeOffsetDays;
	int		m_nExifTimeOffsetHours;
	int		m_nExifTimeOffsetMin;
	int		m_nExifTimeOffsetSec;
	int		m_nExifTimeOffsetSign;
	//}}AFX_DATA
	int m_nExtChangeType;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchProcGeneralTab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBatchProcGeneralTab)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonMergeXmp();
	afx_msg void OnCheckMergeXmp();
	afx_msg void OnCheckExifTimeoffset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPROCGENERALTAB_H__C05D0871_354A_4B9A_949F_88B6F42D3E7A__INCLUDED_)
