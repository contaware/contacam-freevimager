#if !defined(AFX_REDEYEDLG_H__DA2AC880_77BB_4C21_A192_A9CE728EA2AE__INCLUDED_)
#define AFX_REDEYEDLG_H__DA2AC880_77BB_4C21_A192_A9CE728EA2AE__INCLUDED_

#pragma once

// RedEyeDlg.h : header file
//

#include "Dib.h"

#define REDEYE_REGION_MAXSIZE							201		// Must be Odd!
#define REDEYE_PICK_REDRATIO							26		// R / G * 20
#define REDEYE_DEFAULT_REDRATIO							32		// R / G * 20
#define REDEYE_LOWEST_REDRATIO							22		// R / G * 20 (Must be Even)
#define REDEYE_HIGHEST_REDRATIO							60		// R / G * 20 (Must be Even)

#define REDEYE_NEIGHBOURS_THRESHOLD_CLEAN_AND_CHECK		3
#define REDEYE_NEIGHBOURS_THRESHOLD						5

#define REDEYE_HUERANGE_START							280.0f
#define REDEYE_HUERANGE_END								50.0f
#define REDEYE_HUERANGE_SEARCHRADIUS					(REDEYE_REGION_MAXSIZE / 4)
#define REDEYE_HUERANGE_REDRATIO_1						26		// R / G * 20
#define REDEYE_HUERANGE_REDRATIO_2						27		// R / G * 20
#define REDEYE_HUERANGE_REDRATIO_3						29		// R / G * 20
#define REDEYE_HUERANGE_REDRATIO_4						32		// R / G * 20

#define REDEYE_BRIGHTWHITE_MAX							180
#define REDEYE_BRIGHTWHITE_MEDIUM						140
#define REDEYE_BRIGHTWHITE_MIN							110
#define REDEYE_DEFAULT_REDCHANNEL						20
#define REDEYE_DEFAULT_BRIGHTNESS						-1


/////////////////////////////////////////////////////////////////////////////
// CRedEyeDlg dialog

class CRedEyeDlg : public CDialog
{
// Construction
public:
	CRedEyeDlg(CWnd* pParent, CPoint point);
	void Close();
	void DoIt();
	void AdjustRedEye(	CDib* pSrcDib,
						BOOL bShowMask);
	void InitUndoRedEyeRegion(CDib* pSrcDib);
	void UndoRedEyeRegion(CDib* pSrcDib);

	// This Function calls MakeRedEyeMask()
	// till an acceptable mask is found.
	// For each failed mask construction the ratio
	// is incremented by 2.
	int FindGoodRatio(CDib* pSrcDib);

	// Returns the current selected ratio
	__forceinline int GetCurrentRatio()
	{
		CSliderCtrl* pSliderRatio = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_RATIO);
		if (pSliderRatio)
			return pSliderRatio->GetPos();
		else
			return REDEYE_DEFAULT_REDRATIO;
	}

	BOOL AddMaskPoint(CPoint point);
	BOOL RemoveMaskPoint(CPoint point);

	// Is the given color a possible red-eye pixel?
	static __forceinline BOOL IsRedEyePixel(COLORREF crColor,
											float fHueRangeStart,
											float fHueRangeEnd,
											int nRatio);

	// Is the given color the possible shiny
	// (flash reflection) part of the red-eye?
	static __forceinline BOOL IsBrightWhite(COLORREF crColor);

	// Check to see if all of the 8 neighbours are
	// red-eye or bright-white pixels
	static BOOL CheckNeighbours(int nXRedEyeCenter,
								int nYRedEyeCenter,
								CDib* pSrcDib,
								int* pRedEyeCount,
								int* pBrightWhiteCount,
								int x,
								int y,
								float fHueRangeStart,
								float fHueRangeEnd,
								int nRatio);

	// Check the mask value of all 8 neighbours,
	// return the number of set masks.
	static __forceinline int GetNeighboursMaskCount(int nXMaskCenter,
													int nYMaskCenter,
													CDib* pMaskDib,
													int x,
													int y);

	static BOOL SmoothMask(	int nXMaskCenter,
								int nYMaskCenter,
								CDib* pMaskDib);

	static BOOL DrawMask(	int nXRedEyeCenter,
							int nYRedEyeCenter,
							int nXMaskCenter,
							int nYMaskCenter,
							CDib* pSrcDib,
							CDib* pMaskDib);

	// From the passed source Dib, given the approximate
	// center of the red-eye, find good values for the
	// hue range.
	static BOOL FindHueRange(	int nXRedEyeCenter,
								int nYRedEyeCenter,
								float* pHueRangeStart,
								float* pHueRangeEnd,
								CDib* pSrcDib);

	// Calculate Radiuses
	static BOOL CalcRadiuses(	int nXMaskCenter,
								int nYMaskCenter,
								double* pMinRadius,
								double* pMaxRadius,
								double* pAvgRadius,
								CDib* pMaskDib);

	static BOOL GetRedEyeOffsets(	int nXMaskCenter,
									int nYMaskCenter,
									CPoint& MinOffset,
									CDib* pMaskDib);

	// 1. Clean Outside of Max Radius.
	// 2. Check Inside Average Radius to detect not red-eye 
	//    regions which should be red-eye.
	static BOOL CleanAndCheck(	int nXMaskCenter,
								int nYMaskCenter,
								double dMaxRadius,
								double dAvgRadius,
								CDib* pMaskDib);

	// Clean Not Connected Regions.
	static BOOL CleanNotConnectedRegions(	int nXMaskCenter,
											int nYMaskCenter,
											CDib* pMaskDib);

	// From the passed source Dib, given the approximate
	// center of the red-eye, create the red-eye mask,
	// which specifies the red-eye region.
	// It's a 32 bpp dib with alpha flag set and with values:
	// Red-eye:					RGBA(0,0,0,255)
	// Otherwise:				RGBA(0,0,0,0)
	static double MakeRedEyeMask(int nXRedEyeCenter,
								int nYRedEyeCenter,
								int nXMaskCenter,
								int nYMaskCenter,
								float fHueRangeStart,
								float fHueRangeEnd,
								int nRatio,
								CDib* pSrcDib,
								CDib* pMaskDib);

	// From the passed source Dib and the given red-eye mask,
	// update the destination Dib.
	static BOOL RemoveRedEye(int nRedChannel,
							int nGreenChannel,
							int nBlueChannel,
							int nBrightness,
							int nXRedEyeCenter,
							int nYRedEyeCenter,
							int nXMaskCenter,
							int nYMaskCenter,
							CDib* pSrcDib,
							CDib* pMaskDib);

	static __forceinline void Brightness(CColor& c, short brightness);

// Dialog Data
	//{{AFX_DATA(CRedEyeDlg)
	enum { IDD = IDD_REDEYE };
	BOOL	m_bShowMask;
	BOOL	m_bShowOriginal;
	UINT	m_uiHueStart;
	UINT	m_uiHueEnd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRedEyeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ShowAdvanced(BOOL bShowAdvanced);

	CPoint m_ptRedEyeCenter;
	CPoint m_ptOffset;
	CDib* m_pMaskDib;
	CDib* m_pPrevMaskDib;
	CDib* m_pSmoothMaskDib;
	CDib* m_pUndoDib;
	int m_nPrevRatio;
	UINT m_uiPrevHueStart;
	UINT m_uiPrevHueEnd;
	int m_nGoodStartRatio;
	UINT m_uiHueStartFoundValue;
	UINT m_uiHueEndFoundValue;
	BOOL m_bInitialized;
	BOOL m_bShowAdvanced;
	BOOL m_bMaskManuallyModified;
	WINDOWPLACEMENT m_DlgPlacement;
	// Generated message map functions
	//{{AFX_MSG(CRedEyeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnButtonUndoSize();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckShowmask();
	afx_msg void OnCheckShoworiginal();
	afx_msg void OnButtonAdvanced();
	afx_msg void OnChangeEditHueStart();
	afx_msg void OnChangeEditHueEnd();
	afx_msg void OnButtonUndoColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

__forceinline BOOL CRedEyeDlg::IsBrightWhite(COLORREF crColor)
{
	BYTE R = GetRValue(crColor);
	BYTE G = GetGValue(crColor);
	float fRatio = (float)R / (float)G;
	BYTE gray = COLORREFTOGRAY(crColor);
	if (gray >= REDEYE_BRIGHTWHITE_MAX)
	{
		if (fRatio > 0.7f &&
			fRatio < 1.3f)
			return TRUE;
		else
			return FALSE;
	}
	else if (gray >= REDEYE_BRIGHTWHITE_MEDIUM)
	{
		if (fRatio > 0.75f &&
			fRatio < 1.25f)
			return TRUE;
		else
			return FALSE;
	}
	else if (gray >= REDEYE_BRIGHTWHITE_MIN)
	{
		if (fRatio > 0.8f &&
			fRatio < 1.2f)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

// Hue:
// 280° Purple
// ..
// 300° Pink
// ..
// 0°(=360°) Red
// ..
// 10° Dark Orange
// ..
// fHueRangeStart:	from REDEYE_HUERANGE_START (280°)
// fHueRangeEnd:	till REDEYE_HUERANGE_END (50°)
__forceinline BOOL CRedEyeDlg::IsRedEyePixel(COLORREF crColor,
											float fHueRangeStart,
											float fHueRangeEnd,
											int nRatio)
{
	CColor c(crColor);
	float fRatio = (float)nRatio / 20.0f;

	// Clip
	if (fHueRangeEnd > REDEYE_HUERANGE_END && fHueRangeEnd < REDEYE_HUERANGE_START)
		fHueRangeEnd = REDEYE_HUERANGE_END;
	if (fHueRangeStart > REDEYE_HUERANGE_END && fHueRangeStart < REDEYE_HUERANGE_START)
		fHueRangeStart = REDEYE_HUERANGE_START;

	// Both Before 0°
	if (fHueRangeStart >= REDEYE_HUERANGE_START &&
		fHueRangeEnd > REDEYE_HUERANGE_START)
	{
		if (c.GetHue() > (fHueRangeEnd) ||
			c.GetHue() < (fHueRangeStart))
			return FALSE;
	}
	// Both After 0°
	else if (	fHueRangeStart < REDEYE_HUERANGE_END &&
				fHueRangeEnd <= REDEYE_HUERANGE_END)
	{
		if (c.GetHue() > (fHueRangeEnd) ||
			c.GetHue() < (fHueRangeStart))
			return FALSE;
	}
	// Start Before 0°, End After 0°
	else if (	fHueRangeStart >= REDEYE_HUERANGE_START &&
				fHueRangeEnd <= REDEYE_HUERANGE_END)
	{
		if (c.GetHue() > (fHueRangeEnd) &&
			c.GetHue() < (fHueRangeStart))
			return FALSE;
	}
	else
		return FALSE;

	// Get Red and Green Channels
	float fRed = (float)c.GetRed();
	float fGreen = (float)c.GetGreen();
	if (fGreen == 0.0f)	
		fGreen = 1.0f;

	//
	// Check the ratio of red to green
	//

	// Purple Red-Eyes have a lower ratio
	if ((c.GetHue() >= REDEYE_HUERANGE_START) && (c.GetHue() <= 310.0f))
	{
		if (fRed / fGreen < (fRatio - 0.2f))
			return FALSE;
		else
			return TRUE;
	}
	// Pink have also a lower ratio, not as low as purple
	else if ((c.GetHue() > 310.0f) && (c.GetHue() <= 340.0f))
	{
		if (fRed / fGreen < (fRatio - 0.1f))
			return FALSE;
		else
			return TRUE;
	}
	// Red or Orange
	else
	{
		if (fRed / fGreen < fRatio)
			return FALSE;
		else
			return TRUE;
	}
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REDEYEDLG_H__DA2AC880_77BB_4C21_A192_A9CE728EA2AE__INCLUDED_)
