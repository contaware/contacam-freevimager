#if !defined(AFX_PREVIEWFILEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_)
#define AFX_PREVIEWFILEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PreviewFileDlg.h : header file
//
#include "resource.h"
#include "DibStatic.h"
#include "TryEnterCriticalSection.h"
#include "Dib.h"
#include "AviPlay.h"

/*
Comctl32.dll Versions:
4.0   Microsoft Windows 95/Microsoft Windows NT 4.0. 
4.7   Microsoft Internet Explorer 3.x. 
4.71  Internet Explorer 4.0. 
4.72  Internet Explorer 4.01 and Windows 98.
5.8   Internet Explorer 5.
5.81  Windows 2000 and Windows Me.
6.0   Windows XP.
*/

#ifndef MAX_FILEDLG_PATH
#define MAX_FILEDLG_PATH			65535 // If you put 65536 it's not working on win2000...
#endif
#define WM_POST_INITDONE			WM_USER + 3000 
#define WM_POST_CHANGEFILEVIEWMODE5	WM_USER + 3001

extern int g_nPreviewFileDlgViewMode;

enum SHVIEW_ListViewModes 
{
	SHVIEW_Default		= 0,
	SHVIEW_ICON			= 0x7029,	// Style: LVS_SMALLICON
	SHVIEW_LIST			= 0x702B,	// Style: LVS_LIST
	SHVIEW_DETAIL		= 0x702C,	// Style: LVS_REPORT
	SHVIEW_THUMBNAIL	= 0x702D,	// Style: LVS_REPORT + 0x10000000 not set!
	SHVIEW_TILE			= 0x702E	// Style: LVS_ICON
};

enum SHVIEW581_ListViewModes
{
	SHVIEW581_LARGEICON	= 0x7029,	// Style: LVS_ICON
	SHVIEW581_SMALLICON	= 0x702A,	// Style: LVS_SMALLICON
	SHVIEW581_LIST		= 0x702B,	// Style: LVS_LIST
	SHVIEW581_DETAIL	= 0x702C,	// Style: LVS_REPORT
	SHVIEW581_THUMBNAIL	= 0x7031	// Style: LVS_REPORT + 0x10000000 not set!
};

/////////////////////////////////////////////////////////////////////////////
// CPreviewFileDlg dialog
class CPreviewFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CPreviewFileDlg)

	typedef struct _DllVersionInfo
	{
			DWORD cbSize;
			DWORD dwMajorVersion;                   // Major version
			DWORD dwMinorVersion;                   // Minor version
			DWORD dwBuildNumber;                    // Build number
			DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
	} DLLVERSIONINFO;

public:
	CPreviewFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
					BOOL bPreview = TRUE,
					LPCTSTR lpszDefExt = NULL,
					LPCTSTR lpszFileName = NULL,
					LPCTSTR lpszFilter = NULL,
					CWnd* pParentWnd = NULL);
	void SetInfoText(CString str);
	void ChangeFileViewMode(int nFileViewMode);
	CString GetLastSelectedFilePath();

// Attributes
public:
	BOOL m_bPreview;
	CDibStatic m_DibStaticCtrl;
	CStatic m_PreviewInfo;

protected:
	//{{AFX_MSG(CPreviewFileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPreview();
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LONG OnPostInitDone(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnPostChangeFileViewMode5(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnLoadDone(WPARAM wparam, LPARAM lparam);

	virtual void OnInitDone();
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	void Load(BOOL bOnlyHeader = FALSE);
	BOOL ParseSubstrings(CString s, CStringArray& a);
	void SendChangeFileViewMode(int nFileViewMode);
	void ChangeFileViewMode581(int nFileViewMode);
	CString GetLongFileName() const;
	void SetSHELLDefViewWindowProc();
	CString GetCreationFileTime(CString sFileName);
	int GetComCtl32MajorVersion();
	int GetComCtl32MinorVersion();

protected:
	BOOL m_bNewSizeDlg;
	CString m_sLastFileName;
	CDib m_DibHdr;
	CDib m_DibFull;
	CDib m_AlphaRenderedDib;
	CTryEnterCriticalSection m_csDibHdr;
	CTryEnterCriticalSection m_csDibFull;
	CAVIPlay m_AVIPlay;

// Implementation
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWFILEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_)
