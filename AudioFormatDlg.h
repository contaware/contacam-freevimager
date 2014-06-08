#if !defined(AFX_AUDIOFORMATDLG_H__6B10DA54_67C5_49A9_A505_A1DC76307613__INCLUDED_)
#define AFX_AUDIOFORMATDLG_H__6B10DA54_67C5_49A9_A505_A1DC76307613__INCLUDED_

#pragma once

// AudioFormatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAudioFormatDlg dialog

class CAudioFormatDlg : public CDialog
{
// Construction
public:
	CAudioFormatDlg(CWnd* pParent = NULL);   // standard constructor
	WAVEFORMATEX m_WaveFormat;

// Dialog Data
	//{{AFX_DATA(CAudioFormatDlg)
	enum { IDD = IDD_AUDIOFORMAT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ResetAllCtrls(); 
	void WaveFormatToCtrls();
	void CtrlsToWaveFormat();
	void EnableDisableCtrls();

	// Generated message map functions
	//{{AFX_MSG(CAudioFormatDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioPcm();
	afx_msg void OnRadioAdpcm();
	afx_msg void OnRadioMp3();
	afx_msg void OnSelchangeComboPcmBits();
	afx_msg void OnSelchangeComboPcmChannels();
	afx_msg void OnSelchangeComboPcmSamplingrate();
	afx_msg void OnSelchangeComboAdpcmChannels();
	afx_msg void OnSelchangeComboAdpcmSamplingrate();
	afx_msg void OnSelchangeComboMp3Bitrate();
	afx_msg void OnSelchangeComboMp3Channels();
	afx_msg void OnSelchangeComboMp3Samplingrate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOFORMATDLG_H__6B10DA54_67C5_49A9_A505_A1DC76307613__INCLUDED_)
