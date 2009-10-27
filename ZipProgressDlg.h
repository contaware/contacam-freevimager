#if !defined(AFX_ZIPPROGRESSDLG_H__D29C6CAF_A7A7_4226_91EE_FF48531B84A2__INCLUDED_)
#define AFX_ZIPPROGRESSDLG_H__D29C6CAF_A7A7_4226_91EE_FF48531B84A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ZipProgressDlg.h : header file
//

#include "WorkerThread.h"

/////////////////////////////////////////////////////////////////////////////
// CZipProgressDlg dialog

class CZipProgressDlg : public CDialog
{
public:
	class CExtractThread : public CWorkerThread
	{
		public:
			CExtractThread(){m_pDlg = NULL;};
			virtual ~CExtractThread() {Kill();};
			void SetDlg(CZipProgressDlg* pDlg) {m_pDlg = pDlg;};
			
		protected:
			int Work();
			CZipProgressDlg* m_pDlg;
	};

	class CCompressThread : public CWorkerThread
	{
		public:
			CCompressThread(){m_pDlg = NULL;};
			virtual ~CCompressThread() {Kill();};
			void SetDlg(CZipProgressDlg* pDlg) {m_pDlg = pDlg;};
			
		protected:
			int Work();
			CZipProgressDlg* m_pDlg;
	};

public:
	CZipProgressDlg(CWnd* pParent, BOOL bExtract);
	volatile int* m_pPictureFilesCount;
	CString m_sZipFileName;
	CString m_sPath;
	BOOL m_bExtract;

// Dialog Data
	//{{AFX_DATA(CZipProgressDlg)
	enum { IDD = IDD_ZIP_PROGRESS };
	CProgressCtrl m_ZipProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZipProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CExtractThread m_ExtractThread;
	CCompressThread m_CompressThread;
	// Generated message map functions
	//{{AFX_MSG(CZipProgressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZIPPROGRESSDLG_H__D29C6CAF_A7A7_4226_91EE_FF48531B84A2__INCLUDED_)
