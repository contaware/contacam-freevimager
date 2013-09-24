#if !defined(AFX_AUDIOOUTDESTINATIONDLG_H__3885EACE_A537_4DE3_8579_47BF85383631__INCLUDED_)
#define AFX_AUDIOOUTDESTINATIONDLG_H__3885EACE_A537_4DE3_8579_47BF85383631__INCLUDED_

#pragma once

// AudioOutDestinationDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CAudioOutDestinationDlg dialog

class CAudioOutDestinationDlg : public CDialog
{
// Construction
public:
	CAudioOutDestinationDlg(CWnd* pParent, UINT uiDeviceID);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAudioOutDestinationDlg)
	enum { IDD = IDD_AUDIOOUT_DESTINATION };
	CComboBox	m_AudioOutDestination;
	//}}AFX_DATA

	CArray<CString,CString&> m_Devices;
	UINT m_uiNumDev;
	UINT m_uiDeviceID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioOutDestinationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAudioOutDestinationDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOOUTDESTINATIONDLG_H__3885EACE_A537_4DE3_8579_47BF85383631__INCLUDED_)
