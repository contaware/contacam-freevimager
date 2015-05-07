#if !defined(AFX_AUDIOINSOURCEDLG_H__A0A94175_A768_4BC5_8679_03CC3D4328A0__INCLUDED_)
#define AFX_AUDIOINSOURCEDLG_H__A0A94175_A768_4BC5_8679_03CC3D4328A0__INCLUDED_

#pragma once

// AudioInSourceDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAudioInSourceDlg dialog

class CAudioInSourceDlg : public CDialog
{
// Construction
public:
	CAudioInSourceDlg(UINT uiDeviceID);   // standard constructor
	static CString DevIDToName(UINT uiID);

// Dialog Data
	//{{AFX_DATA(CAudioInSourceDlg)
	enum { IDD = IDD_AUDIOIN_SOURCE };
	CComboBox	m_AudioInSource;
	//}}AFX_DATA
	UINT m_uiDeviceID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioInSourceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAudioInSourceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOINSOURCEDLG_H__A0A94175_A768_4BC5_8679_03CC3D4328A0__INCLUDED_)
