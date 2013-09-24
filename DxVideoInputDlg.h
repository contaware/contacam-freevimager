#if !defined(AFX_DXVIDEOINPUTDLG_H__806DA57A_23E2_4087_B09F_DF15F6936401__INCLUDED_)
#define AFX_DXVIDEOINPUTDLG_H__806DA57A_23E2_4087_B09F_DF15F6936401__INCLUDED_

#pragma once

// DxVideoInputDlg.h : header file
//

#ifdef VIDEODEVICEDOC

class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CDxVideoInputDlg dialog

class CDxVideoInputDlg : public CDialog
{
// Construction
public:
	CDxVideoInputDlg(CVideoDeviceDoc* pDoc);

// Dialog Data
	//{{AFX_DATA(CDxVideoInputDlg)
	enum { IDD = IDD_DX_VIDEOINPUT };
	CComboBox	m_VideoInput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDxVideoInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	CStringArray m_Inputs;
	void Apply();
	void NumberDuplicateNames();
	BOOL IsAlreadyInInputs(int nIndex);

	// Generated message map functions
	//{{AFX_MSG(CDxVideoInputDlg)
	afx_msg void OnApply();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DXVIDEOINPUTDLG_H__806DA57A_23E2_4087_B09F_DF15F6936401__INCLUDED_)
