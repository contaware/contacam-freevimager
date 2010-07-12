#if !defined(AFX_IMAPI2DLG_H__3CE3CD98_18C3_42A5_9E11_8C84121F13ED__INCLUDED_)
#define AFX_IMAPI2DLG_H__3CE3CD98_18C3_42A5_9E11_8C84121F13ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IMAPI2Dlg.h : header file
//

#include "Imapi2Small.h"

/////////////////////////////////////////////////////////////////////////////
// CIMAPI2Dlg dialog

class CIMAPI2Dlg : public CDialog
{
public:
	// The Worker Thread
	class CIMAPI2DlgThread : public CWorkerThread
	{
		public:
			CIMAPI2DlgThread() {m_pDlg = NULL;};
			virtual ~CIMAPI2DlgThread() {Kill();};
			void SetDlg(CIMAPI2Dlg* pDlg) {m_pDlg = pDlg;};

		protected:
			int Work();
			static BOOL CreateStream(const CString& sPath, IStream** ppStream);
			bool CreateMediaFileSystem(IMAPI_MEDIA_PHYSICAL_TYPE mediaType, IStream** ppDataStream);
			CIMAPI2Dlg* m_pDlg;
	};

// Construction
public:
	CIMAPI2Dlg(CWnd* pParent, const CString& sDir);
	CString m_sDir;

// Dialog Data
	//{{AFX_DATA(CIMAPI2Dlg)
	enum { IDD = IDD_IMAPI2 };
	CStatic	m_ProgressText;
	CProgressCtrl	m_ProgressCtrl;
	CStatic	m_TimeLeft;
	CStatic	m_EstimatedTime;
	CComboBox	m_cbDrive;
	CString	m_sVolumeLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMAPI2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	void AddRecordersToComboBox();
	CString	GetMediaTypeString(int mediaType);
	void EnableUI(BOOL bEnable);
	void UpdateProgress(BOOL bBurn, LONG done, LONG total);
	void UpdateTimes(LONG totalTime, LONG remainingTime);

	CIMAPI2DlgThread m_IMAPI2DlgThread;
	bool	m_isCdromSupported;
	bool	m_isDvdSupported;
	bool	m_isDualLayerDvdSupported;
	bool	m_isBurning;
	bool	m_cancelBurn;
	LONG	m_prevTotalProgress;

	// Generated message map functions
	//{{AFX_MSG(CIMAPI2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	afx_msg void OnBurn();
	afx_msg void OnSelchangeComboDrive();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg LRESULT OnImapiUpdate(WPARAM, LPARAM);
	afx_msg LRESULT OnBurnStatusMessage(WPARAM, LPARAM);
	afx_msg LRESULT OnBurnFinished(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAPI2DLG_H__3CE3CD98_18C3_42A5_9E11_8C84121F13ED__INCLUDED_)
