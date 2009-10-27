#if !defined(AFX_SAVEFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_)
#define AFX_SAVEFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveFileDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveFileDlg dialog

class CSaveFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CSaveFileDlg)

public:
	CSaveFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CSaveFileDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual void OnTypeChange();
};

/////////////////////////////////////////////////////////////////////////////
// CAviSaveFileDlg dialog

class CAviSaveFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CAviSaveFileDlg)

public:
	CAviSaveFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CAviSaveFileDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual void OnTypeChange();
};

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveFileDlg dialog

class CAnimGifSaveFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CAnimGifSaveFileDlg)

public:
	CAnimGifSaveFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CAnimGifSaveFileDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual void OnTypeChange();
};

/////////////////////////////////////////////////////////////////////////////
// CBatchProcSaveFileDlg dialog

class CBatchProcSaveFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CBatchProcSaveFileDlg)

public:
	CBatchProcSaveFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CBatchProcSaveFileDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual void OnTypeChange();
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_)
