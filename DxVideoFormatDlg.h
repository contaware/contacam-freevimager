#if !defined(AFX_DXVIDEOFORMATDLG_H__915D4803_A70F_4F8D_87EF_BDDB323ED6C5__INCLUDED_)
#define AFX_DXVIDEOFORMATDLG_H__915D4803_A70F_4F8D_87EF_BDDB323ED6C5__INCLUDED_

#pragma once

// DxVideoFormatDlg.h : header file
//

#ifdef VIDEODEVICEDOC

#include "DxCapture.h"

// Window Message IDs
#define WM_DX_APPLY_VIDEOFORMAT_CHANGE			WM_USER + 650

class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CDxVideoFormatDlg dialog

class CDxVideoFormatDlg : public CDialog
{
// Construction
public:
	CDxVideoFormatDlg(CVideoDeviceDoc* pDoc);

// Dialog Data
	//{{AFX_DATA(CDxVideoFormatDlg)
	enum { IDD = IDD_DX_VIDEOFORMAT };
	CComboBox	m_VideoSize;
	CComboBox	m_VideoCompression;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDxVideoFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	BOOL m_bChangingFormat;
	CStringArray m_Formats;
	CDWordArray m_Compressions;
	CDWordArray m_Bits;
	CStringArray m_CompressionStrings;
	CArray<CSize,CSize> m_Sizes;
	void Apply();
	void FormatToCurSel(AM_MEDIA_TYPE* pmtConfig);
	void ReStart();

	// Generated message map functions
	//{{AFX_MSG(CDxVideoFormatDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnApply();
	//}}AFX_MSG
	afx_msg LONG OnApplyVideoFormatChange(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DXVIDEOFORMATDLG_H__915D4803_A70F_4F8D_87EF_BDDB323ED6C5__INCLUDED_)
