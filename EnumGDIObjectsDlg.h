#if !defined(AFX_ENUMGDIOBJECTSDLG_H__2E21BF71_8BAA_4F97_8780_DE7AF5735CE6__INCLUDED_)
#define AFX_ENUMGDIOBJECTSDLG_H__2E21BF71_8BAA_4F97_8780_DE7AF5735CE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnumGDIObjectsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEnumGDIObjectsDlg dialog

class CEnumGDIObjectsDlg : public CDialog
{
// Construction
public:
	CEnumGDIObjectsDlg(CWnd* pParent);
	void Close();
// Dialog Data
	//{{AFX_DATA(CEnumGDIObjectsDlg)
	enum { IDD = IDD_ENUMGDIOBJECTS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnumGDIObjectsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL GDIHandleSeemsValid(DWORD hGDI, DWORD ObjectType);
	void EnumGDIs();
	// Generated message map functions
	//{{AFX_MSG(CEnumGDIObjectsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENUMGDIOBJECTSDLG_H__2E21BF71_8BAA_4F97_8780_DE7AF5735CE6__INCLUDED_)
