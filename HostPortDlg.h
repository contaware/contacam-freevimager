#if !defined(AFX_HOSTPORTDLG_H__8DA6632A_D989_4EC8_ABD0_9B43877EF0C3__INCLUDED_)
#define AFX_HOSTPORTDLG_H__8DA6632A_D989_4EC8_ABD0_9B43877EF0C3__INCLUDED_

#pragma once

// HostPortDlg.h : header file
//

#ifdef VIDEODEVICEDOC

#define MAX_HOST_PORT_HISTORY_SIZE		8

/////////////////////////////////////////////////////////////////////////////
// CHostPortDlg dialog

class CHostPortDlg : public CDialog
{
// Construction
public:
	CHostPortDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHostPortDlg() {;};
	void ParseUrl(	CString& sGetFrameVideoHost,
					int& nGetFrameVideoPort,
					int& nNetworkDeviceTypeMode,
					CString& sHttpGetFrameLocation);

// Returned Data
public:
	CString m_sHost;
	int m_nPort;
	int	m_nDeviceTypeMode;
	enum { IDD = IDD_HOSTPORT };

// Implementation
protected:
	void EnableDisableCtrls();
	void LoadCredentials();
	void SaveCredentials();
	void LoadSettings();
	void SaveSettings();
	
	CStringArray m_HostsHistory;
	CDWordArray m_PortsHistory;
	CDWordArray m_DeviceTypeModesHistory;
	// Generated message map functions
	//{{AFX_MSG(CHostPortDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeComboHost();
	afx_msg void OnEditchangeComboHost();
	afx_msg void OnChangeEditPort();
	afx_msg void OnSelchangeComboDeviceTypeMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOSTPORTDLG_H__8DA6632A_D989_4EC8_ABD0_9B43877EF0C3__INCLUDED_)
