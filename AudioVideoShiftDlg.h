#if !defined(AFX_AUDIOVIDEOSHIFTDLG_H__3A45285B_FFE3_40F7_BFED_00AD4F24D6DF__INCLUDED_)
#define AFX_AUDIOVIDEOSHIFTDLG_H__3A45285B_FFE3_40F7_BFED_00AD4F24D6DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioVideoShiftDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAudioVideoShiftDlg dialog

class CAudioVideoShiftDlg : public CDialog
{
// Construction
public:
	CAudioVideoShiftDlg(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(CAudioVideoShiftDlg)
	enum { IDD = IDD_AUDIOVIDEO_SHIFT };
	CSliderCtrl	m_SliderVideoFrameRate;
	CSliderCtrl	m_SliderAudioVideoShift;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioVideoShiftDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DisplayShiftValue(int nShiftValue);
	void DisplayFrameRateValue(int nFrameRateDelta);
	BOOL AreAVSameLength();
	void UpdateDisplay();
	// Generated message map functions
	//{{AFX_MSG(CAudioVideoShiftDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonShiftSavetoavi();
	afx_msg void OnButtonUndoShift();
	afx_msg void OnButtonUndoFramerate();
	afx_msg void OnButtonZeroshift();
	afx_msg void OnButtonFramerateSavetoavi();
	afx_msg void OnButtonAvSamelength();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOVIDEOSHIFTDLG_H__3A45285B_FFE3_40F7_BFED_00AD4F24D6DF__INCLUDED_)
