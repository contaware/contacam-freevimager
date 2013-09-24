#if !defined(AFX_HTTPVIDEOFORMATDLG_H__007C0A4D_5AE0_47F0_ADE6_2B0EE14F002D__INCLUDED_)
#define AFX_HTTPVIDEOFORMATDLG_H__007C0A4D_5AE0_47F0_ADE6_2B0EE14F002D__INCLUDED_

#pragma once

// HttpVideoFormatDlg.h : header file
//

#ifdef VIDEODEVICEDOC

class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CHttpVideoFormatDlg dialog

class CHttpVideoFormatDlg : public CDialog
{
// Construction
public:
	CHttpVideoFormatDlg(CVideoDeviceDoc* pDoc);

// Dialog Data
	//{{AFX_DATA(CHttpVideoFormatDlg)
	enum { IDD = IDD_HTTP_VIDEOFORMAT };
	CComboBox	m_VideoSize;
	CSliderCtrl	m_VideoQualitySlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHttpVideoFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	// Generated message map functions
	//{{AFX_MSG(CHttpVideoFormatDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnOK();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif

#endif // !defined(AFX_HTTPVIDEOFORMATDLG_H__007C0A4D_5AE0_47F0_ADE6_2B0EE14F002D__INCLUDED_)
