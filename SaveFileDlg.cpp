// SaveFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "SaveFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <dlgs.h>
/* The id's of the controls are defined in the header file <dlgs.h> as      
stc3, stc2                  The two label controls ("File name" and "Files of type")
edt1, cmb1                  The edit control and the drop-down box.
IDOK, IDCANCEL The OK and Cancel button.
lst1                        The window that is used to browse the namespace
*/

/////////////////////////////////////////////////////////////////////////////
// CSaveFileDlg

IMPLEMENT_DYNAMIC(CSaveFileDlg, CFileDialog)

// For VS2008 disable Vista Style because OnTypeChange() not working!
#if _MFC_VER >= 0x0900
CSaveFileDlg::CSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
#else
CSaveFileDlg::CSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
#endif
{
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.Flags |= OFN_EXPLORER;
	m_ofn.lpstrFilter = 
				_T("Windows Bitmap (*.bmp)\0*.bmp\0")
				_T("Graphics Interchange Format (*.gif)\0*.gif\0")
				_T("Portable Network Graphics (*.png)\0*.png\0")
				_T("JPEG File Interchange Format (*.jpg)\0*.jpg\0")
				_T("Tag Image File Format (*.tif)\0*.tif\0")
				_T("PC Paintbrush (*.pcx)\0*.pcx\0")
				_T("Enhanced Metafile (*.emf)\0*.emf\0");
}


BEGIN_MESSAGE_MAP(CSaveFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CSaveFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSaveFileDlg::OnTypeChange()
{
	// Get Needed Dialog Items
	CWnd* typeNameBox = GetParent()->GetDlgItem(cmb1);
	CWnd* fileNameBox = GetParent()->GetDlgItem(edt1);
	
	// New Versions of the ComCtl32.dll have a combobox
	// for file name instead of an edit box!
	if (fileNameBox == NULL)
		fileNameBox = GetParent()->GetDlgItem(cmb13);

	// ASSERT if ComCtl32.dll changed again!
	ASSERT(fileNameBox);
	ASSERT(typeNameBox);

	// Get File Name
	CString fileName; 
	fileNameBox->GetWindowText(fileName);

	int pos;
	switch (((CComboBox *)typeNameBox)->GetCurSel())
	{
		case 0 : // bmp
			if (fileName.IsEmpty())
				fileName = _T("*.bmp");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".bmp");
			}
		break;

		case 1 : // gif
			if (fileName.IsEmpty())
				fileName = _T("*.gif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".gif");
			}
		break;

		case 2 : // png
			if (fileName.IsEmpty())
				fileName = _T("*.png");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".png");
			}
		break;

		case 3 : // jpg
			if (fileName.IsEmpty())
				fileName = _T("*.jpg");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".jpg");
			}
		break;

		case 4 : // tif
			if (fileName.IsEmpty())
				fileName = _T("*.tif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".tif");
			}
		break;

		case 5 : // pcx
			if (fileName.IsEmpty())
				fileName = _T("*.pcx");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".pcx");
			}
		break;

		case 6 : // emf
			if (fileName.IsEmpty())
				fileName = _T("*.emf");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".emf");
			}
		break;
		
		default:
		
		break;
	}
	fileNameBox->SetWindowText(fileName);
}

/////////////////////////////////////////////////////////////////////////////
// CAviSaveFileDlg

IMPLEMENT_DYNAMIC(CAviSaveFileDlg, CFileDialog)

// For VS2008 disable Vista Style because OnTypeChange() not working!
#if _MFC_VER >= 0x0900
CAviSaveFileDlg::CAviSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
#else
CAviSaveFileDlg::CAviSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
#endif
{
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.Flags |= OFN_EXPLORER;
	m_ofn.lpstrFilter =
				_T("Avi File (*.avi)\0*.avi\0")
#ifdef SUPPORT_LIBAVCODEC
				_T("Swf File (*.swf)\0*.swf\0")
#endif
				_T("Animated GIF (*.gif)\0*.gif\0")						
				_T("BMP Sequence (*.bmp)\0*.bmp\0")
				_T("PNG Sequence (*.png)\0*.png\0")
				_T("JPEG Sequence (*.jpg)\0*.jpg\0");
}


BEGIN_MESSAGE_MAP(CAviSaveFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CAviSaveFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAviSaveFileDlg::OnTypeChange()
{
	// Get Needed Dialog Items
	CWnd* typeNameBox = GetParent()->GetDlgItem(cmb1);
	CWnd* fileNameBox = GetParent()->GetDlgItem(edt1);
	
	// New Versions of the ComCtl32.dll have a combobox
	// for file name instead of an edit box!
	if (fileNameBox == NULL)
		fileNameBox = GetParent()->GetDlgItem(cmb13);

	// ASSERT if ComCtl32.dll changed again!
	ASSERT(fileNameBox);
	ASSERT(typeNameBox);
	
	// Get File Name
	CString fileName; 
	fileNameBox->GetWindowText(fileName);

	int pos;
	switch(((CComboBox *)typeNameBox)->GetCurSel())
	{
		case 0 : // avi
			if (fileName.IsEmpty())
				fileName = _T("*.avi");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".avi");
			}
		break;

#ifdef SUPPORT_LIBAVCODEC
		case 1 : // swf
			if (fileName.IsEmpty())
				fileName = _T("*.swf");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".swf");
			}
		break;

		case 2 : // gif
			if (fileName.IsEmpty())
				fileName = _T("*.gif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".gif");
			}
		break;

		case 3 : // bmp
			if (fileName.IsEmpty())
				fileName = _T("*.bmp");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".bmp");
			}
		break;

		case 4 : // png
			if (fileName.IsEmpty())
				fileName = _T("*.png");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".png");
			}
		break;

		case 5 : // jpg
			if (fileName.IsEmpty())
				fileName = _T("*.jpg");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".jpg");
			}
		break;
#else
		case 1 : // gif
			if (fileName.IsEmpty())
				fileName = _T("*.gif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".gif");
			}
		break;

		case 2 : // bmp
			if (fileName.IsEmpty())
				fileName = _T("*.bmp");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".bmp");
			}
		break;

		case 3 : // png
			if (fileName.IsEmpty())
				fileName = _T("*.png");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".png");
			}
		break;

		case 4 : // jpg
			if (fileName.IsEmpty())
				fileName = _T("*.jpg");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".jpg");
			}
		break;
#endif
		default:	
		break;
	}
	fileNameBox->SetWindowText(fileName);
}

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveFileDlg

IMPLEMENT_DYNAMIC(CAnimGifSaveFileDlg, CFileDialog)

// For VS2008 disable Vista Style because OnTypeChange() not working!
#if _MFC_VER >= 0x0900
CAnimGifSaveFileDlg::CAnimGifSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
#else
CAnimGifSaveFileDlg::CAnimGifSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
#endif
{
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.Flags |= OFN_EXPLORER;
	m_ofn.lpstrFilter =
				_T("Animated GIF (*.gif)\0*.gif\0")	
				_T("Avi File (*.avi)\0*.avi\0")
				_T("BMP Sequence (*.bmp)\0*.bmp\0");
}


BEGIN_MESSAGE_MAP(CAnimGifSaveFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CAnimGifSaveFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAnimGifSaveFileDlg::OnTypeChange()
{
	// Get Needed Dialog Items
	CWnd* typeNameBox = GetParent()->GetDlgItem(cmb1);
	CWnd* fileNameBox = GetParent()->GetDlgItem(edt1);
	
	// New Versions of the ComCtl32.dll have a combobox
	// for file name instead of an edit box!
	if (fileNameBox == NULL)
		fileNameBox = GetParent()->GetDlgItem(cmb13);

	// ASSERT if ComCtl32.dll changed again!
	ASSERT(fileNameBox);
	ASSERT(typeNameBox);
	
	// Get File Name
	CString fileName; 
	fileNameBox->GetWindowText(fileName);

	int pos;
	switch(((CComboBox *)typeNameBox)->GetCurSel())
	{
		case 0 : // gif
			if (fileName.IsEmpty())
				fileName = _T("*.gif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".gif");
			}
		break;

		case 1 : // avi
			if (fileName.IsEmpty())
				fileName = _T("*.avi");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".avi");
			}
		break;

		case 2 : // bmp
			if (fileName.IsEmpty())
				fileName = _T("*.bmp");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".bmp");
			}
		break;
		
		default:
		
		break;
	}
	fileNameBox->SetWindowText(fileName);
}

/////////////////////////////////////////////////////////////////////////////
// CBatchProcSaveFileDlg

IMPLEMENT_DYNAMIC(CBatchProcSaveFileDlg, CFileDialog)

// For VS2008 disable Vista Style because OnTypeChange() not working!
#if _MFC_VER >= 0x0900
CBatchProcSaveFileDlg::CBatchProcSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
#else
CBatchProcSaveFileDlg::CBatchProcSaveFileDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
#endif
{
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.Flags |= OFN_EXPLORER;

	if (((CUImagerApp*)::AfxGetApp())->IsMODIAvailable())
	{
		m_ofn.lpstrFilter =
					_T("Zip File (*.zip)\0*.zip\0")
					_T("Avi File (*.avi)\0*.avi\0")
					_T("Animated GIF (*.gif)\0*.gif\0")						
					_T("Multi-Page TIFF (*.tif)\0*.tif\0")
					_T("Pdf Document (*.pdf)\0*.pdf\0")
					_T("Text Document (*.txt)\0*.txt\0");
	}
	else
	{
		m_ofn.lpstrFilter =
					_T("Zip File (*.zip)\0*.zip\0")
					_T("Avi File (*.avi)\0*.avi\0")
					_T("Animated GIF (*.gif)\0*.gif\0")						
					_T("Multi-Page TIFF (*.tif)\0*.tif\0")
					_T("Pdf Document (*.pdf)\0*.pdf\0");
	}
}


BEGIN_MESSAGE_MAP(CBatchProcSaveFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CBatchProcSaveFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBatchProcSaveFileDlg::OnTypeChange()
{
	// Get Needed Dialog Items
	CWnd* typeNameBox = GetParent()->GetDlgItem(cmb1);
	CWnd* fileNameBox = GetParent()->GetDlgItem(edt1);
	
	// New Versions of the ComCtl32.dll have a combobox
	// for file name instead of an edit box!
	if (fileNameBox == NULL)
		fileNameBox = GetParent()->GetDlgItem(cmb13);

	// ASSERT if ComCtl32.dll changed again!
	ASSERT(fileNameBox);
	ASSERT(typeNameBox);
	
	// Get File Name
	CString fileName; 
	fileNameBox->GetWindowText(fileName);

	int pos;
	switch(((CComboBox *)typeNameBox)->GetCurSel())
	{
		case 0 : // zip
			if (fileName.IsEmpty())
				fileName = _T("*.zip");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".zip");
			}
		break;

		case 1 : // avi
			if (fileName.IsEmpty())
				fileName = _T("*.avi");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".avi");
			}
		break;

		case 2 : // gif
			if (fileName.IsEmpty())
				fileName = _T("*.gif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".gif");
			}
		break;

		case 3 : // tif
			if (fileName.IsEmpty())
				fileName = _T("*.tif");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".tif");
			}
		break;

		case 4 : // pdf
			if (fileName.IsEmpty())
				fileName = _T("*.pdf");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".pdf");
			}
		break;

		case 5 : // txt
			if (fileName.IsEmpty())
				fileName = _T("*.txt");
			else
			{
				pos = fileName.ReverseFind(_T('.'));
				if (pos != -1)
					fileName.Delete(pos, fileName.GetLength() - pos);
				fileName += _T(".txt");
			}
		break;
		
		default:
		
		break;
	}
	fileNameBox->SetWindowText(fileName);
}
