#if !defined(AFX_REMOTECAMCTL_H__EABB7110_4176_4EEE_A8FC_349928BA81A3__INCLUDED_)
#define AFX_REMOTECAMCTL_H__EABB7110_4176_4EEE_A8FC_349928BA81A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// RemoteCamCtl.h : Declaration of the CRemoteCamCtrl ActiveX Control class.

#include "Dib.h"
#include "NetCom.h"
#include "NetGetFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl : See RemoteCamCtl.cpp for implementation.

class CRemoteCamCtrl : public COleControl
{
	DECLARE_DYNCREATE(CRemoteCamCtrl)

// Constructor
public:
	CRemoteCamCtrl();
	CRITICAL_SECTION m_csDib;
	CDib m_Dib;
	CNetCom m_GetFrameNetCom;
	CGetFrameParseProcess m_GetFrameParseProcess;
	CGetFrameGenerator m_GetFrameGenerator;
	__forceinline DWORD GetMaxFrames() const {return (DWORD)MIN(MAX(NETFRAME_MIN_FRAMES, m_lMaxFrames), NETFRAME_MAX_FRAMES);};
	__forceinline BOOL DoDisableResend() const {return m_bDisableResend;};
	__forceinline CString GetFrameUsername() const {return m_sGetFrameUsername;};
	__forceinline CString GetFramePassword() const {return m_sGetFramePassword;};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteCamCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bFirstConnected;
	virtual ~CRemoteCamCtrl();
	BOOL ConnectGetFrameUDP(LPCTSTR pszHostName, int nPort);

	DECLARE_OLECREATE_EX(CRemoteCamCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CRemoteCamCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CRemoteCamCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CRemoteCamCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CRemoteCamCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CRemoteCamCtrl)
	CString m_sHost;
	afx_msg void OnHostChanged();
	long m_lPort;
	afx_msg void OnPortChanged();
	long m_lMaxFrames;
	afx_msg void OnMaxFramesChanged();
	BOOL m_bDisableResend;
	afx_msg void OnDisableResendChanged();
	CString m_sGetFrameUsername;
	afx_msg void OnUsernameChanged();
	CString m_sGetFramePassword;
	afx_msg void OnPasswordChanged();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CRemoteCamCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CRemoteCamCtrl)
	dispidHost = 1L,
	dispidPort = 2L,
	dispidMaxFrames = 3L,
	dispidDisableResend = 4L,
	dispidUsername = 5L,
	dispidPassword = 6L,
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTECAMCTL_H__EABB7110_4176_4EEE_A8FC_349928BA81A3__INCLUDED)
