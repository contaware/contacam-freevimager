#if !defined(AFX_UIMAGERCAPDEVDLG_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_UIMAGERCAPDEVDLG_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef VIDEODEVICEDOC

#include "resource.h"

class CCaptureDeviceDlg : public CDialog
{
public:
	CCaptureDeviceDlg(CWnd* pParent = NULL);   // standard constructor

	//{{AFX_DATA(CCaptureDeviceDlg)
	enum { IDD = IDD_DEVICE };
	CListBox	m_ctrlDevices;
	//}}AFX_DATA
	WORD m_wDeviceID;

	//{{AFX_VIRTUAL(CCaptureDeviceDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CCaptureDeviceDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIMAGERCAPDEVDLG_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)