#if !defined(AFX_OUTVOLDLG_H__D0252745_C340_485E_865E_A3BE2F0CCB8F__INCLUDED_)
#define AFX_OUTVOLDLG_H__D0252745_C340_485E_865E_A3BE2F0CCB8F__INCLUDED_

#pragma once

// OutVolDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// COutVolDlg dialog

class COutVolDlg : public CDialog
{
// Construction
public:
	COutVolDlg(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(COutVolDlg)
	enum { IDD = IDD_OUTVOL };
	CSliderCtrl	m_SliderWaveVolRight;
	CSliderCtrl	m_SliderWaveVolLeft;
	CSliderCtrl	m_SliderMasterVolRight;
	CSliderCtrl	m_SliderMasterVolLeft;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutVolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COutVolDlg)
	afx_msg void OnClose();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckMastervol();
	afx_msg void OnCheckWavevol();
	afx_msg void OnAudioDestination();
	//}}AFX_MSG
	afx_msg LRESULT OnMixerCtrlChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	DWORD m_dwMasterVolLeftLastPos;
	DWORD m_dwMasterVolRightLastPos;
	DWORD m_dwWaveVolLeftLastPos;
	DWORD m_dwWaveVolRightLastPos;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTVOLDLG_H__D0252745_C340_485E_865E_A3BE2F0CCB8F__INCLUDED_)
