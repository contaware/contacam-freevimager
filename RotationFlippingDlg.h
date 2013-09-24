#if !defined(AFX_ROTATIONFLIPPINGDLG_H__443F94AF_5CBE_4CD9_8EB3_877A4BEC43AA__INCLUDED_)
#define AFX_ROTATIONFLIPPINGDLG_H__443F94AF_5CBE_4CD9_8EB3_877A4BEC43AA__INCLUDED_

#pragma once

// RotationFlippingDlg.h : header file
//

#include "resource.h"
#include "ColorButton.h"
#include "PaletteWnd.h"
#include "Dib.h"

/////////////////////////////////////////////////////////////////////////////
// CRotationFlippingDlg dialog

class CRotationFlippingDlg : public CDialog
{
// Construction
public:
	CRotationFlippingDlg(CWnd* pParent);
	BOOL DoIt();
	void Close();

// Dialog Data
	//{{AFX_DATA(CRotationFlippingDlg)
	enum { IDD = IDD_ROTATE_FLIP_MODELESS };
	CSpinButtonCtrl	m_SpinAngleMinutes;
	CColorButton	m_PickColorFromImage;
	CColorButton	m_PickColorFromDlg;
	CSpinButtonCtrl	m_SpinAngle;
	int		m_TransformationType;
	UINT	m_uiAngle;
	UINT	m_uiAngleMinutes;
	BOOL	m_bAntiAliasing;
	//}}AFX_DATA

	COLORREF m_crBackgroundColor;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotationFlippingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bDlgInitialized;
	CPaletteWnd* m_pWndPalette;
	CDib m_PreviewUndoDib;
	int  m_PreviewTransformationType;
	UINT m_uiPreviewAngle;
	UINT m_uiPreviewAngleMinutes;
	BOOL m_bPreviewAntiAliasing;

	// Generated message map functions
	//{{AFX_MSG(CRotationFlippingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPreview();
	afx_msg void OnPickColorFromDlg();
	afx_msg void OnUndo();
	afx_msg void OnPickColorFromImage();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnCheckLossless();
	afx_msg void OnChangeEditAngle();
	afx_msg void OnChangeEditAngleMinutes();
	//}}AFX_MSG
	afx_msg LRESULT OnColorPicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnColorPickerClosed(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROTATIONFLIPPINGDLG_H__443F94AF_5CBE_4CD9_8EB3_877A4BEC43AA__INCLUDED_)
