#if !defined(AFX_SNAPSHOTNAMESDLG_H__D7727516_3840_4F3A_866C_48C9B38298E9__INCLUDED_)
#define AFX_SNAPSHOTNAMESDLG_H__D7727516_3840_4F3A_866C_48C9B38298E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SnapshotNamesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSnapshotNamesDlg dialog

class CSnapshotNamesDlg : public CDialog
{
// Construction
public:
	CSnapshotNamesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSnapshotNamesDlg)
	enum { IDD = IDD_SNAPSHOT_NAMES };
	CString	m_sSnapshotLiveJpegName;
	CString	m_sSnapshotLiveJpegThumbName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSnapshotNamesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSnapshotNamesDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNAPSHOTNAMESDLG_H__D7727516_3840_4F3A_866C_48C9B38298E9__INCLUDED_)
