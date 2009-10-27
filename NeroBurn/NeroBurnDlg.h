/******************************************************************************
|* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
|* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
|* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
|* PARTICULAR PURPOSE.
|* 
|* Copyright 1995-2005 Nero AG. All Rights Reserved.
|*-----------------------------------------------------------------------------
|* PROJECT: NeroBurn NeroAPI Example
|*
|* FILE: NeroBurnDlg.h
|*
|* PURPOSE: Declaration file for the user interaction dialog.
******************************************************************************/


#if !defined(AFX_NEROBURNDLG_H__78BE85B9_87B6_11D4_8056_00105ABB05E6__INCLUDED_)
#define AFX_NEROBURNDLG_H__78BE85B9_87B6_11D4_8056_00105ABB05E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnDlg dialog

class CNeroBurnDlg : public CDialog
{
// Construction
public:
	static void NERO_CALLBACK_ATTR SetPhaseCallback (void* pUserData, const char* text);
	static void NERO_CALLBACK_ATTR AddLogLine(void* pUserData, NERO_TEXT_TYPE type, const char* text);
	static BOOL NERO_CALLBACK_ATTR AbortedCallback (void* pUserData);
	static BOOL NERO_CALLBACK_ATTR ProgressCallback (void* pUserData, DWORD dwProgressInPercent);
	static NeroUserDlgInOut NERO_CALLBACK_ATTR UserDialog(void* pUserData, NeroUserDlgInOut type, void* data);
	static BOOL NERO_CALLBACK_ATTR IdleCallback (void* pUserData);
	CNeroBurnDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CNeroBurnDlg)
	enum { IDD = IDD_NEROBURN_DIALOG };
	CButton	mOK;
	CButton	mbtnAbort;
	CProgressCtrl	mpgsProgress;
	CEdit	medtMessages;
	CEdit	medtFileName;
	CComboBox	mcbxDevices;
	CButton	mbtnBurnCD;
	CButton	mbtnBurnDVD;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeroBurnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	BOOL m_bBurning;
	BOOL m_bBurningCD;
	CString m_sLastAppended;

	// Generated message map functions
	//{{AFX_MSG(CNeroBurnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBurnCD();
	afx_msg void OnBurnDVD();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAbort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	char mpcImage [256];
	bool mbAborted;
	void Burn(BOOL bBurnCD); // TRUE burn CD, FALSE burn DVD
	void NeroAPIFree();
	NERO_SETTINGS nsSettings;
	void AppendString(CString str);
	void NeroAPIInit();
    char pcLanguageFile [128];
    char pcSoftware [128];
    char pcVendor [128];
	char pcNeroFilesPath [128];
	char pcDriveName [128];
	NERO_PROGRESS npProgress;
	NERO_SCSI_DEVICE_INFOS* pndiDeviceInfos;
	NERO_DEVICEHANDLE ndhDeviceHandle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEROBURNDLG_H__78BE85B9_87B6_11D4_8056_00105ABB05E6__INCLUDED_)
