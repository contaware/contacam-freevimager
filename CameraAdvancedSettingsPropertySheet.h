#if !defined(AFX_CAMERAADVANCEDSETTINGSPROPERTYSHEET_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
#define AFX_CAMERAADVANCEDSETTINGSPROPERTYSHEET_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_

#pragma once

// CameraAdvancedSettingsPropertySheet.h : header file
//

#include "MovementDetectionPage.h"
#include "SnapshotPage.h"

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

class CCameraAdvancedSettingsPropertySheet : public CPropertySheet
{
public:
	DECLARE_DYNAMIC(CCameraAdvancedSettingsPropertySheet)
	CCameraAdvancedSettingsPropertySheet(CVideoDeviceDoc* pDoc);
	void Close();
	void UpdateTitle();
	void Show();
	void Hide(BOOL bSaveSettingsOnHiding);

protected:
	virtual ~CCameraAdvancedSettingsPropertySheet();
	static CString MakeTitle(CVideoDeviceDoc* pDoc);

// Attributes
	CVideoPage m_VideoPropertyPage;
	CSnapshotPage m_SnapshotPropertyPage;
	CVideoDeviceDoc* m_pDoc;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCameraAdvancedSettingsPropertySheet)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Message Handlers
protected:
	//{{AFX_MSG(CCameraAdvancedSettingsPropertySheet)
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERAADVANCEDSETTINGSPROPERTYSHEET_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
