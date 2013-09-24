#if !defined(AFX_BATCHPROCJPEGTAB_H__6A3F5AD3_B927_492F_979F_B7C5DA3F219F__INCLUDED_)
#define AFX_BATCHPROCJPEGTAB_H__6A3F5AD3_B927_492F_979F_B7C5DA3F219F__INCLUDED_

#pragma once

// BatchProcJpegTab.h : header file
//

#include "TabPageSSL.h"

/////////////////////////////////////////////////////////////////////////////
// CBatchProcJpegTab dialog

class CBatchProcJpegTab : public CTabPageSSL
{
public:
	enum EXIFTHUMBOPERATIONTYPE
						{	THUMB_NONE=0,
							THUMB_UPDATE=1,
							THUMB_ADD_UPDATE=2,
							THUMB_REMOVE=3};

// Construction
public:
	CBatchProcJpegTab(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CBatchProcJpegTab)
	enum { IDD = IDD_BATCH_PROC_JPEGTAB };
	int		m_nJpegQuality;
	BOOL	m_bAutoOrientate;
	BOOL	m_bRemoveCom;
	BOOL	m_bRemoveExif;
	BOOL	m_bRemoveIcc;
	BOOL	m_bRemoveXmp;
	BOOL	m_bRemoveIptc;
	BOOL	m_bRemoveJfif;
	BOOL	m_bRemoveOtherAppSections;
	BOOL	m_bForceJpegQuality;
	//}}AFX_DATA
	int m_nExifThumbOperationType;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchProcJpegTab)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBatchProcJpegTab)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPROCJPEGTAB_H__6A3F5AD3_B927_492F_979F_B7C5DA3F219F__INCLUDED_)
