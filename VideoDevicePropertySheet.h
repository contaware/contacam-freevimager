#if !defined(AFX_VIDEODEVICEPROPERTYSHEET_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
#define AFX_VIDEODEVICEPROPERTYSHEET_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoDevicePropertySheet.h : header file
//

#include "GeneralPage.h"
#include "SnapshotPage.h"
#include "NetworkPage.h"
#include "MovementDetectionPage.h"

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

class CVideoDevicePropertySheet : public CPropertySheet
{
public:
	DECLARE_DYNAMIC(CVideoDevicePropertySheet)
	CVideoDevicePropertySheet(CVideoDeviceDoc* pDoc);
	void Close();
	void UpdateTitle();
	BOOL IsVisible();
	void Toggle();
	void Show();
	void Hide();

protected:
	virtual ~CVideoDevicePropertySheet();
	static CString MakeTitle(CVideoDeviceDoc* pDoc);

// Attributes
	CGeneralPage m_GeneralPropertyPage;
	CSnapshotPage m_SnapshotPropertyPage;
	CNetworkPage m_NetworkPropertyPage;
	CMovementDetectionPage m_MovementDetectionPropertyPage;
	CVideoDeviceDoc* m_pDoc;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDevicePropertySheet)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Message Handlers
protected:
	//{{AFX_MSG(CVideoDevicePropertySheet)
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEODEVICEPROPERTYSHEET_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
