// NoVistaFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNoVistaFileDlg

IMPLEMENT_DYNAMIC(CNoVistaFileDlg, CFileDialog)

#if _MFC_VER >= 0x0900
CNoVistaFileDlg::CNoVistaFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
#else
CNoVistaFileDlg::CNoVistaFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
#endif
{
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.Flags |= (OFN_EXPLORER | OFN_ENABLESIZING);
}

BEGIN_MESSAGE_MAP(CNoVistaFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CNoVistaFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()