#if !defined(AFX_GENERALPAGE_H__0C566865_8717_497C_BFF4_68AC9FB23F18__INCLUDED_)
#define AFX_GENERALPAGE_H__0C566865_8717_497C_BFF4_68AC9FB23F18__INCLUDED_

#pragma once

// GeneralPage.h : header file
//

#ifdef VIDEODEVICEDOC

#include "resource.h"
#include "NumSpinCtrl.h"
#include "NumEdit.h"

// Forward Declaration
class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CGeneralPage dialog

#define FRAMERATE_CHANGE_TIMEOUT			4	// In GENERALDLG_TIMER_MS units

class CGeneralPage : public CPropertyPage
{
// Construction
public:
	CGeneralPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CGeneralPage();

// Dialog Data
	//{{AFX_DATA(CGeneralPage)
	enum { IDD = IDD_GENERAL };
	CSliderCtrl	m_VideoRecQuality;
	CNumEdit	m_FrameRate;
	CNumSpinCtrl m_SpinFrameRate;
	BOOL	m_bAutorun;
	BOOL	m_bRotate180;
	BOOL	m_bAudioListen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeneralPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateVideoQualityInfo();

	// Generated message map functions
	//{{AFX_MSG(CGeneralPage)
	afx_msg void OnVideoFormat();
	afx_msg void OnVideoSource();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFrameRate();
	afx_msg void OnRecAudio();
	afx_msg void OnRecAudioFromStream();
	afx_msg void OnRecAudioFromSource();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnAudioInput();
	afx_msg void OnAudioMixer();
	afx_msg void OnVideoInput();
	afx_msg void OnVideoTuner();
	afx_msg void OnCheckAutorun();
	afx_msg void OnCheckLiveRotate180();
	afx_msg void OnCheckAudioListen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CVideoDeviceDoc* m_pDoc;
	CDWordArray m_VideoCompressionFcc;
	CDWordArray m_VideoCompressionFastEncodeAndKeyframesRateSupport;
	CDWordArray m_VideoCompressionQualitySupport;
	BOOL m_bDoChangeFrameRate;
	int m_nFrameRateChangeTimeout;
	BOOL m_bDlgInitialized;
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERALPAGE_H__0C566865_8717_497C_BFF4_68AC9FB23F18__INCLUDED_)
