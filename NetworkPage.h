#if !defined(AFX_NETWORKPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
#define AFX_NETWORKPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NetworkPage.h : header file
//

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

/////////////////////////////////////////////////////////////////////////////
// CNetworkPage dialog

class CNetworkPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CNetworkPage)

// Construction
public:
	CNetworkPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CNetworkPage();
	void EnableDisableCriticalControls(BOOL bEnable);

// Dialog Data
	//{{AFX_DATA(CNetworkPage)
	enum { IDD = IDD_NETWORK };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CNetworkPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVideoDeviceDoc* m_pDoc;
	// Generated message map functions
	//{{AFX_MSG(CNetworkPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnVideoPortEnabled();
	afx_msg void OnChangeEditPort();
	afx_msg void OnChangeEditMtu();
	afx_msg void OnChangeEditDatarate();
	afx_msg void OnSelchangeComboSize();
	afx_msg void OnChangeEditFreqdiv();
	afx_msg void OnChangeEditConnections();
	afx_msg void OnChangeAuthUsername();
	afx_msg void OnChangeAuthPassword();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETWORKPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
