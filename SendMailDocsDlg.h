#if !defined(AFX_SENDMAILDOCSDLG_H__9BFBF245_5EF0_434B_BD34_855B457EDE2A__INCLUDED_)
#define AFX_SENDMAILDOCSDLG_H__9BFBF245_5EF0_434B_BD34_855B457EDE2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendMailDocsDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSendMailDocsDlg dialog

class CSendMailDocsDlg : public CDialog
{
// Construction
public:
	CSendMailDocsDlg(int nID, CWnd* pParent);

	enum OPTIMIZATION_SELECTION
						{	EMAIL_OPT=0,
							NO_OPT=1,
							ADV_OPT=2};

// Dialog Data
	//{{AFX_DATA(CSendMailDocsDlg)
	enum { IDD = IDD_SENDMAIL_OPEN_DOCS };
	int		m_nOptimizationSelection;
	int		m_nPixelsPercentSel;
	BOOL	m_bShrinkingPictures;
	BOOL	m_bZipFile;
	int		m_nShrinkingPercent;
	int		m_nShrinkingPixels;
	int		m_nJpegQuality;
	CString	m_sZipFileName;
	BOOL	m_bShrinkingVideos;
	BOOL	m_bPictureExtChange;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMailDocsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nID;
	// Generated message map functions
	//{{AFX_MSG(CSendMailDocsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioOptimizeAdvanced();
	afx_msg void OnRadioOptimizeEmail();
	afx_msg void OnRadioOptimizeUnchanged();
	afx_msg void OnCheckShrinkingpictures();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDMAILDOCSDLG_H__9BFBF245_5EF0_434B_BD34_855B457EDE2A__INCLUDED_)
