#if !defined(AFX_COLORDETECTIONPAGE_H__0FAD23A3_274F_49DB_BEBF_4946D969B2ED__INCLUDED_)
#define AFX_COLORDETECTIONPAGE_H__0FAD23A3_274F_49DB_BEBF_4946D969B2ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorDetectionPage.h : header file
//

#ifdef VIDEODEVICEDOC

#include "resource.h"
#include "ColorButton.h"
#include "MacProgressCtrl.h"
#include "VideoDeviceDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CColorDetectionPage dialog

class CColorDetectionPage : public CPropertyPage
{
public:
	enum {	REGULATION_TYPE_THRESHOLD_1 = 0,
			REGULATION_TYPE_THRESHOLD_5 = 1,
			REGULATION_TYPE_THRESHOLD_10 = 2,
			REGULATION_TYPE_THRESHOLD_50 = 3,
			REGULATION_TYPE_THRESHOLD_100 = 4,
			REGULATION_TYPE_HUE = 5,
			REGULATION_TYPE_SATURATION = 6,
			REGULATION_TYPE_VALUE  = 7};
// Construction
public:
	CColorDetectionPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CColorDetectionPage();
	void UpdateControls();
	int RegulationTypeToMaxDetectionLevel(int nIndex);
	void EnableDisableCriticalControls(BOOL bEnable);
	CSliderCtrl* GetSlider(DWORD dwIndex);
	CMacProgressCtrl* GetProgress(DWORD dwIndex);
	void SetHueRadius(DWORD dwIndex, int hue)	{	
														if (dwIndex < COLDET_MAX_COLORS)
														{
															m_HueRadius[dwIndex] = hue;
															UpdateDataToSlider(dwIndex);
														}
													};
	void SetSaturationRadius(DWORD dwIndex, int saturation)
													{
														if (dwIndex < COLDET_MAX_COLORS)
														{
															m_SaturationRadius[dwIndex] = saturation;
															UpdateDataToSlider(dwIndex);
														}
													};
	void SetValueRadius(DWORD dwIndex, int value)
													{
														if (dwIndex < COLDET_MAX_COLORS)
														{
															m_ValueRadius[dwIndex] = value;
															UpdateDataToSlider(dwIndex);
														}
													};
	int GetHueRadius(DWORD dwIndex) const {return dwIndex < COLDET_MAX_COLORS ? m_HueRadius[dwIndex] : -1;};
	int GetSaturationRadius(DWORD dwIndex) const {return dwIndex < COLDET_MAX_COLORS ? m_SaturationRadius[dwIndex] : -1;};
	int GetValueRadius(DWORD dwIndex) const {return dwIndex < COLDET_MAX_COLORS ? m_ValueRadius[dwIndex] : -1;};

// Dialog Data
	//{{AFX_DATA(CColorDetectionPage)
	enum { IDD = IDD_COLOR_DETECTION };
	CSpinButtonCtrl	m_SpinWaitBetweenCountup;
	CSliderCtrl	m_SliderAccuracy;
	CSliderCtrl	m_SliderColor7;
	CSliderCtrl	m_SliderColor6;
	CSliderCtrl	m_SliderColor5;
	CSliderCtrl	m_SliderColor4;
	CSliderCtrl	m_SliderColor3;
	CSliderCtrl	m_SliderColor2;
	CSliderCtrl	m_SliderColor1;
	CSliderCtrl m_SliderColor0;
	CMacProgressCtrl m_ProgressColor7;
	CMacProgressCtrl m_ProgressColor6;
	CMacProgressCtrl m_ProgressColor5;
	CMacProgressCtrl m_ProgressColor4;
	CMacProgressCtrl m_ProgressColor3;
	CMacProgressCtrl m_ProgressColor2;
	CMacProgressCtrl m_ProgressColor1;
	CMacProgressCtrl m_ProgressColor0;
	CColorButton m_PickColor7;
	CColorButton m_PickColor6;
	CColorButton m_PickColor5;
	CColorButton m_PickColor4;
	CColorButton m_PickColor3;
	CColorButton m_PickColor2;
	CColorButton m_PickColor1;
	CColorButton m_PickColor0;
	DWORD	m_dwWaitBetweenCountup;
	//}}AFX_DATA
	int m_RegulationType[COLDET_MAX_COLORS];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorDetectionPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateDataToSlider(DWORD dwIndex);
	void UpdateDataFromSlider(DWORD dwIndex, int nPos);
	void OnRegulationTypeChanged(	DWORD dwIndex,
									CSliderCtrl& Slider,
									CMacProgressCtrl& Progress);

	CVideoDeviceDoc* m_pDoc;
	CBrush m_RedBrush, m_GreenBrush, m_BlueBrush;
	COLORREF m_crRedColor, m_crGreenColor, m_crBlueColor, m_crWhiteTextColor;
	int m_DetectionThreshold[COLDET_MAX_COLORS];
	int m_HueRadius[COLDET_MAX_COLORS];
	int m_SaturationRadius[COLDET_MAX_COLORS];
	int m_ValueRadius[COLDET_MAX_COLORS];

	// Generated message map functions
	//{{AFX_MSG(CColorDetectionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnPickcolor0();
	afx_msg void OnPickcolor1();
	afx_msg void OnPickcolor2();
	afx_msg void OnPickcolor3();
	afx_msg void OnPickcolor4();
	afx_msg void OnPickcolor5();
	afx_msg void OnPickcolor6();
	afx_msg void OnPickcolor7();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckDetectionPreview();
	afx_msg void OnButtonResetcounter();
	afx_msg void OnChangeEditWaitbetweencountup();
	afx_msg void OnSelchangeComboRegulationType0();
	afx_msg void OnSelchangeComboRegulationType1();
	afx_msg void OnSelchangeComboRegulationType2();
	afx_msg void OnSelchangeComboRegulationType3();
	afx_msg void OnSelchangeComboRegulationType4();
	afx_msg void OnSelchangeComboRegulationType5();
	afx_msg void OnSelchangeComboRegulationType6();
	afx_msg void OnSelchangeComboRegulationType7();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnCheckVideoDetectionColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORDETECTIONPAGE_H__0FAD23A3_274F_49DB_BEBF_4946D969B2ED__INCLUDED_)
