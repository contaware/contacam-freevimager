#if !defined(AFX_HOSTPORTDLG_H__8DA6632A_D989_4EC8_ABD0_9B43877EF0C3__INCLUDED_)
#define AFX_HOSTPORTDLG_H__8DA6632A_D989_4EC8_ABD0_9B43877EF0C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HostPortDlg.h : header file
//

#ifdef VIDEODEVICEDOC

#include "NetCom.h"

#define WM_HOST_OK						WM_USER + 800
#define MAX_HOST_PORT_HISTORY_SIZE		8


/////////////////////////////////////////////////////////////////////////////
// CHostPortDlg dialog

class CHostPortDlg : public CDialog
{
public:
	// The Networking Ping Parser & Processor Class
	class CPingParseProcess : public CNetCom::CParseProcess
	{
		public:
			CPingParseProcess(CHostPortDlg* pDlg, int nHostIndex, DWORD dwEnumCount) {	m_pDlg = pDlg;
																						m_nHostIndex = nHostIndex;
																						m_dwEnumCount = dwEnumCount;
																						m_bPingReceived = FALSE;};
			virtual ~CPingParseProcess() {;};
			virtual BOOL Parse(CNetCom* pNetCom);
			void ResetPingReceived() {m_bPingReceived;};
			BOOL IsPingReceived() const {return m_bPingReceived;};

		protected:
			CHostPortDlg* m_pDlg;
			BOOL m_bPingReceived;
			int m_nHostIndex;
			DWORD m_dwEnumCount;
	};

	// The Networking Ping Generator Class
	class CPingGenerator : public CNetCom::CIdleGenerator
	{
		public:
			CPingGenerator(CHostPortDlg* pDlg) {m_pDlg = pDlg;};
			BOOL Generate(CNetCom* pNetCom);

		protected:
			CHostPortDlg* m_pDlg;
	};

public:
	// Arrrays
	typedef CArray<CNetCom*,CNetCom*> NETCOMARRAY;
	typedef CArray<CPingParseProcess*,CPingParseProcess*> PARSEPROCESSARRAY;
	typedef CArray<CPingGenerator*,CPingGenerator*> GENERATORARRAY;

// Construction
public:
	CHostPortDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHostPortDlg() {;};

// Returned Data
public:
	CString m_sHost;
	int m_nPort;
	DWORD m_dwMaxFrames;

// Dialog Data
	//{{AFX_DATA(CHostPortDlg)
	enum { IDD = IDD_HOSTPORT };
	CComboBox	m_cbBufSize;
	int		m_nDeviceTypeMode;
	BOOL	m_bDisableResend;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHostPortDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL Connect(	CNetCom* pNetCom,
					CPingParseProcess* pPingParseProcess,
					CPingGenerator* pPingGenerator,
					LPCTSTR pszHostName, int nPort);
	void Free();
	void LoadSettings();
	void SaveSettings();

	CStringArray m_HostsHistory;
	CDWordArray m_PortsHistory;
	CDWordArray m_DeviceTypeModesHistory;
	CDWordArray m_MaxFramesHistory;
	CDWordArray m_DisableResendHistory;
	CStringArray m_Hosts;
	NETCOMARRAY m_Connections;
	PARSEPROCESSARRAY m_Parsers;
	GENERATORARRAY m_Generators;
	DWORD m_dwEnumCount;
	// Generated message map functions
	//{{AFX_MSG(CHostPortDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSearchServers();
	virtual void OnCancel();
	afx_msg void OnSelchangeComboHost();
	afx_msg void OnSelchangeComboDevicetypemode();
	//}}AFX_MSG
	afx_msg LONG OnHostOk(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOSTPORTDLG_H__8DA6632A_D989_4EC8_ABD0_9B43877EF0C3__INCLUDED_)
