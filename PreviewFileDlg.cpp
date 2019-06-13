#include "stdafx.h"
#include "resource.h"
#include "uImager.h"
#include "Dib.h"
#include "DibStatic.h"
#include "PreviewFileDlg.h"
#include <dlgs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// The Current Selected View Mode
int g_nPreviewFileDlgViewMode = SHVIEW_Default;

// Original SHELLDLL_DefView window proc
static WNDPROC g_pOldShellDefWndProc = NULL;

// New window procedure for the SHELLDLL_DefView window,
// which is the parent of the SysListView32 control.
LRESULT CALLBACK NewShellDefWndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	WORD wID = LOWORD(wParam);
	if (uiMsg == WM_COMMAND)
	{
		switch (wID)
		{
			case SHVIEW_ICON :
				g_nPreviewFileDlgViewMode = wID;
				break;

			case SHVIEW_LIST :
				g_nPreviewFileDlgViewMode = wID;
				break;

			case SHVIEW_DETAIL :
				g_nPreviewFileDlgViewMode = wID;
				break;

			case SHVIEW_THUMBNAIL :
				g_nPreviewFileDlgViewMode = wID;
				break;

			case SHVIEW_TILE :
				g_nPreviewFileDlgViewMode = wID;
				break;

			default:
				break;
		}
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
														_T("PreviewFileDlgViewMode"),
														g_nPreviewFileDlgViewMode);
	}

	// Pass the message on to the original SHELLDLL_DefView window proc,
	// for standard handling.
	return ::CallWindowProc(g_pOldShellDefWndProc, hwnd, uiMsg, wParam, lParam);
}

// Subclass the window procedure of the SHELLDLL_DefView, diverting all
// message traffic to our 'NewShellDefWndProc' procedure
void CPreviewFileDlg::SetSHELLDefViewWindowProc()
{
	// This is also working: CWnd* pWnd = GetParent()->GetDlgItem(lst2);
	HWND hWnd = ::FindWindowEx(GetParent()->GetSafeHwnd(), NULL, _T("SHELLDLL_DefView"), NULL);
	if (hWnd)
	{
		LONG_PTR lpCurrentWndProc = ::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
		if ((LONG)lpCurrentWndProc != (LONG)NewShellDefWndProc)
			g_pOldShellDefWndProc = (WNDPROC)::SetWindowLongPtr(hWnd,
																GWLP_WNDPROC, 
																(LONG_PTR)NewShellDefWndProc);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPreviewFileDlg

IMPLEMENT_DYNAMIC(CPreviewFileDlg, CFileDialog)

BEGIN_MESSAGE_MAP(CPreviewFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CPreviewFileDlg)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_POST_INITDONE, OnPostInitDone)
	ON_MESSAGE(WM_LOADDONE, OnLoadDone)
END_MESSAGE_MAP()

// Disable Vista Style because otherwise it is not working!
CPreviewFileDlg::CPreviewFileDlg(	BOOL bOpenFileDialog,
									BOOL bPreview,
									LPCTSTR lpszDefExt,
									LPCTSTR lpszFileName,
									LPCTSTR lpszFilter,
									CWnd* pParentWnd) :
									CFileDialog(bOpenFileDialog,
												lpszDefExt,
												lpszFileName,
												OFN_HIDEREADONLY,
												lpszFilter,
												pParentWnd,
												0, FALSE)
{
	m_ofn.Flags |= (OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLESIZING);
	g_pOldShellDefWndProc = NULL;

	// Template Is Attached To the Top-Right of The File Dialog
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPENPREVIEW);

	m_bPreview = bPreview;
	m_sLastFileName = _T("");
	m_DibStaticCtrl.SetDibHdrPointer(&m_DibHdr);
	m_DibStaticCtrl.SetDibFullPointer(&m_DibFull);
	m_DibStaticCtrl.SetAlphaRenderedDibPointer(&m_AlphaRenderedDib);
	m_DibStaticCtrl.SetDibHdrCS(&m_csDibHdr);
	m_DibStaticCtrl.SetDibFullCS(&m_csDibFull);
}

void CPreviewFileDlg::OnDestroy() 
{
	m_DibStaticCtrl.SetNotifyHwnd(NULL);
	m_DibStaticCtrl.GetThumbLoadThread()->Kill();
	m_DibStaticCtrl.GetGifAnimationThread()->Kill();
	CFileDialog::OnDestroy();	
}

BOOL CPreviewFileDlg::OnInitDialog() 
{
	CFileDialog::OnInitDialog();

	m_DibStaticCtrl.SubclassDlgItem(IDC_IMAGE, this);
	m_DibStaticCtrl.SetNotifyHwnd(this->GetSafeHwnd());
	GetDlgItem(IDC_PREVIEW)->SendMessage(BM_SETCHECK, (m_bPreview) ? 1 : 0);
	SetInfoText(_T(""));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreviewFileDlg::OnInitDone()
{
	
}

LONG CPreviewFileDlg::OnPostInitDone(WPARAM wparam, LPARAM lparam)
{
	SetSHELLDefViewWindowProc();
	
	if (g_nPreviewFileDlgViewMode != SHVIEW_Default)
		ChangeFileViewMode(g_nPreviewFileDlgViewMode);
	
	return 0;
}

void CPreviewFileDlg::ChangeFileViewMode(int nFileViewMode)
{	
	// This is also working: CWnd* pWnd = GetParent()->GetDlgItem(lst2);
	HWND hShellDllView = ::FindWindowEx(GetParent()->GetSafeHwnd(), NULL, _T("SHELLDLL_DefView"), NULL);
	if (hShellDllView)
	{
		// Send View Mode Change Command
		g_nPreviewFileDlgViewMode = nFileViewMode;
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
														_T("PreviewFileDlgViewMode"),
														nFileViewMode);
		::SendMessage(hShellDllView, WM_COMMAND, nFileViewMode, 0);

		// Sent parent dialog a Refresh command (F5) to force repaint.
		// (The command ID value was obtained by inspection using Spy++)
		const int CMD_REFRESH = 40966;
		::SendMessage(GetParent()->GetSafeHwnd(), WM_COMMAND, CMD_REFRESH, 0);
	}	
}

CString CPreviewFileDlg::GetLongFileName() const
{
	if ((m_ofn.Flags & OFN_EXPLORER) && m_hWnd != NULL)
	{
		ASSERT(::IsWindow(m_hWnd));
		CString strResult;
		if (GetParent()->SendMessage(CDM_GETSPEC, (WPARAM)MAX_FILEDLG_PATH,
			(LPARAM)strResult.GetBuffer(MAX_FILEDLG_PATH)) < 0)
		{
			strResult.Empty();
		}
		else
		{
			strResult.ReleaseBuffer();
			return strResult;
		}
	}
	return m_ofn.lpstrFileTitle;
}

void CPreviewFileDlg::OnFileNameChange() 
{
	CFileDialog::OnFileNameChange();
	CString sLastFileName = GetLastSelectedFilePath();
	BOOL bExisting = ::IsExistingFile(sLastFileName);
	if (!bExisting || (m_sLastFileName != sLastFileName))
		Load(!m_bPreview);
}

void CPreviewFileDlg::OnFolderChange() 
{
	CFileDialog::OnFolderChange();
	CString sLastFileName = GetLastSelectedFilePath();
	BOOL bExisting = ::IsExistingFile(sLastFileName);
	if (!bExisting || (m_sLastFileName != sLastFileName))
		Load(!m_bPreview);

	// We loose the subclassing every time the user navigates to a
	// different folder, since the control gets recreated. So, re-subclass
	// the SHELLDLL_DefView window proc.
	if (g_pOldShellDefWndProc)
		SetSHELLDefViewWindowProc();
}

CString CPreviewFileDlg::GetCreationFileTime(CString sFileName)
{
	CFileStatus FileStatus;
	if (CFile::GetStatus(sFileName, FileStatus))
	{
		return	::MakeDateLocalFormat(FileStatus.m_ctime) +
				_T(" ") +
				::MakeTimeLocalFormat(FileStatus.m_ctime, TRUE);
	}
	else
		return _T("");		
}

// a[0] contains the last selected file name
// a[1] contains file name
// ...
// returns TRUE if multiple files selected
// returns FALSE if only one file selected
// (a is empty in this case!)
BOOL CPreviewFileDlg::ParseSubstrings(CString s, CStringArray& a)
{
	BOOL bHasSubstring = FALSE;
	int nCurrent = 0;
	int nPrevious = 1;
	while ((nCurrent = s.Find(_T('\"'), nPrevious)) > 0)
	{
		CString t = s.Mid(nPrevious, nCurrent - nPrevious);
		if (t != _T(" "))
		{
			a.Add(t);
			bHasSubstring = TRUE;
		}
		nPrevious = nCurrent + 1;
	}
	return bHasSubstring;
}

CString CPreviewFileDlg::GetLastSelectedFilePath()
{
	int nPos;
	CString sFiles = GetLongFileName();
	if ((nPos = sFiles.Find(_T('\"'), 1)) > 0)
		return GetFolderPath() + _T('\\') + sFiles.Mid(1, nPos - 1);
	else
		return GetPathName();
}

void CPreviewFileDlg::Load(BOOL bOnlyHeader/*=FALSE*/) 
{
	m_sLastFileName = GetLastSelectedFilePath();
	m_DibStaticCtrl.Load(m_sLastFileName, bOnlyHeader);
}

LONG CPreviewFileDlg::OnLoadDone(WPARAM wparam, LPARAM lparam)
{
	CString s, t;
	CString sLastFileName = GetLastSelectedFilePath();

	if (((int)wparam == CDibStatic::HDRLOAD_HDRDONE) ||
		((int)wparam == CDibStatic::FULLLOAD_HDRDONE)||
		((int)wparam == CDibStatic::FULLLOAD_FULLDONE))
	{
		// Enter CS
		m_csDibHdr.EnterCriticalSection();

		CDib* pDib = m_DibStaticCtrl.GetDibHdrPointer();

		// Size
		t.Format(_T("%d x %d px\r\n"), pDib->GetWidth(), pDib->GetHeight());
		s += t;

		// Dpi
		if (pDib->GetXDpi() != 0 && pDib->GetYDpi() != 0)
		{
			t.Format(_T("%d x %d dpi\r\n"), pDib->GetXDpi(), pDib->GetYDpi());
			s += t;
		}

		// Pixel Depth
		t.Format(_T("%s\r\n"), pDib->m_FileInfo.GetDepthName());
		s += t;

		// Exif
		if (pDib->GetExifInfo()->bHasExif)
		{			
			if (pDib->GetExifInfo()->CameraModel[0])
			{
				t.Format(_T("%s\r\n"), CString(pDib->GetExifInfo()->CameraModel));
				s += t;
			}

			if (pDib->GetExifInfo()->ExposureTime)
			{
				t.Format(_T("%.3f s"), (double)pDib->GetExifInfo()->ExposureTime);
				s += t;
				if (pDib->GetExifInfo()->ExposureTime <= 0.5)
				{
					t.Format(_T(" (1/%d)"), Round(1.0 / pDib->GetExifInfo()->ExposureTime));
					s += t;
				}
				s += _T("\r\n");
			}

			if (pDib->GetExifInfo()->ApertureFNumber)
			{
				t.Format(_T("f/%.1f\r\n"), (double)pDib->GetExifInfo()->ApertureFNumber);
				s += t;
			}

			if (pDib->GetExifInfo()->Flash >= 0 && (pDib->GetExifInfo()->Flash & 1))
				s += _T("\u21af\r\n");
		}
		// Gif
		else if (::GetFileExt(sLastFileName) == _T(".gif"))
		{
			t.Format(_T("%s"), CDib::GIFGetVersion(sLastFileName, FALSE));
			s += t;
			if (m_DibStaticCtrl.GetGifAnimationThread()->IsRunning() &&
				m_DibStaticCtrl.GetGifAnimationThread()->m_dwDibAnimationCount > 1)
			{
				// Note: use the white square with black shadow because only Windows 10
				//       supports the correct frame symbol \U0001f39e
				t.Format(_T(", %d\u2750"),	m_DibStaticCtrl.GetGifAnimationThread()->m_dwDibAnimationCount > 1 ?
											m_DibStaticCtrl.GetGifAnimationThread()->m_dwDibAnimationCount : 1);
				s += t;
			}
			s += _T("\r\n");
		}

		// Time
		if (pDib->GetExifInfo()->bHasExif && pDib->GetExifInfo()->DateTime[0])
		{
			CTime Time = CMetadata::GetDateTimeFromExifString(CString(pDib->GetExifInfo()->DateTime));
			CString sTime = ::MakeDateLocalFormat(Time) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE);
			t.Format(_T("%s"), sTime);
			s += t;
		}
		else
			s += GetCreationFileTime(sLastFileName);

		// Leave CS
		m_csDibHdr.LeaveCriticalSection();

		// Set Info Text
		SetInfoText(s);
	}
	else if (	((int)wparam == CDibStatic::HDRLOAD_ERROR)		||
				((int)wparam == CDibStatic::FULLLOAD_HDRERROR))
	{
		// Clear Info Text
		SetInfoText(_T(""));
	}

	return 0;
}

void CPreviewFileDlg::OnPreview() 
{	
	m_bPreview = !m_bPreview;
	Load(!m_bPreview);
}

BOOL CPreviewFileDlg::OnQueryNewPalette() 
{
	m_DibStaticCtrl.SendMessage(WM_QUERYNEWPALETTE);	// redo the palette if necessary
	return CFileDialog::OnQueryNewPalette();
}

void CPreviewFileDlg::OnPaletteChanged(CWnd* pFocusWnd) 
{
	CFileDialog::OnPaletteChanged(pFocusWnd);
	m_DibStaticCtrl.SendMessage(WM_PALETTECHANGED, (WPARAM)pFocusWnd->GetSafeHwnd());	// redo the palette if necessary
}

void CPreviewFileDlg::OnSetFocus(CWnd* pOldWnd) 
{
	CFileDialog::OnSetFocus(pOldWnd);
	m_DibStaticCtrl.SendMessage(WM_QUERYNEWPALETTE);	// redo the palette if necessary
}

void CPreviewFileDlg::SetInfoText(CString str)
{
	GetDlgItem(IDC_PREVIEW_INFO)->SetWindowText(str);
}

#ifdef _DEBUG
void CPreviewFileDlg::Dump(CDumpContext& dc) const
{
	CFileDialog::Dump(dc);
	if (m_bPreview)
		dc << "preview is enabled\n";
	else
		dc << "preview is disabled\n";
}
#endif //_DEBUG
