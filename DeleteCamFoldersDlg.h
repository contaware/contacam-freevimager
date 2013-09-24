#if !defined(AFX_DELETECAMFOLDERSDLG_H__83A2088E_A0D9_4E23_B343_158C7EB75A81__INCLUDED_)
#define AFX_DELETECAMFOLDERSDLG_H__83A2088E_A0D9_4E23_B343_158C7EB75A81__INCLUDED_

#pragma once

// DeleteCamFoldersDlg.h : header file
//

#ifdef VIDEODEVICEDOC

#include "HScrollListBox.h"
#include "SortableFileFind.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteCamFoldersDlg dialog

class CDeleteCamFoldersDlg : public CDialog
{
// Construction
public:
	CDeleteCamFoldersDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeleteCamFoldersDlg)
	enum { IDD = IDD_DELETE_CAM_FOLDERS };
	CHScrollListBox	m_CamFolders;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteCamFoldersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStringArray m_DevicePathNames;
	CSortableFileFind m_DirFind;
	// Generated message map functions
	//{{AFX_MSG(CDeleteCamFoldersDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonListSelectall();
	afx_msg void OnButtonListSelectnone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DELETECAMFOLDERSDLG_H__83A2088E_A0D9_4E23_B343_158C7EB75A81__INCLUDED_)
