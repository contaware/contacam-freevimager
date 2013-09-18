#include "stdafx.h"
#include "resource.h"
#include "uImager.h"
#include "Dib.h"
#include "AviPlay.h"
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
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		{
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
															_T("PreviewFileDlgViewMode"),
															g_nPreviewFileDlgViewMode);
		}
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
	m_DibStaticCtrl.SetAVIPlayPointer(&m_AVIPlay);
}

void CPreviewFileDlg::OnDestroy() 
{
	m_DibStaticCtrl.SetNotifyHwnd(NULL);
	m_DibStaticCtrl.GetThumbLoadThread()->Kill();
#ifdef SUPPORT_GIFLIB
	m_DibStaticCtrl.GetGifAnimationThread()->Kill();
#endif
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
	// Vista or higher remember automatically the selected view type
	if (!g_bWinVistaOrHigher)
	{
		// This is necessary because the list view is not yet created!
		PostMessage(WM_POST_INITDONE, 0, 0);
	}
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
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		{
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
															_T("PreviewFileDlgViewMode"),
															nFileViewMode);
		}
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
	if (::GetFileStatus(sFileName, FileStatus))
		return	::MakeDateLocalFormat(FileStatus.m_ctime) +
				_T(" ") +
				::MakeTimeLocalFormat(FileStatus.m_ctime, TRUE);
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

		// Avi File
		if (::GetFileExt(sLastFileName) == _T(".avi") ||
			::GetFileExt(sLastFileName) == _T(".divx"))
		{
			CAVIPlay* pAVIPlay = m_DibStaticCtrl.GetAVIPlayPointer();
			CAVIPlay::CAVIVideoStream* pVideoStream = pAVIPlay->GetVideoStream(0);
			CAVIPlay::CAVIAudioStream* pAudioStream = pAVIPlay->GetAudioStream(0);

			if (pVideoStream && (pAVIPlay->GetVideoStreamsCount() >= 1))
			{
				t.Format(_T("%d x %d   %.1ff/s\r\n"),pVideoStream->GetWidth(),
													pVideoStream->GetHeight(),
													pVideoStream->GetFrameRate());
				s += t;

				t.Format(_T("%s\r\n"), CDib::GetCompressionName(pVideoStream->GetFormat(true)));
				s += t;
			}

			if (pAudioStream && (pAVIPlay->GetAudioStreamsCount() >= 1))
			{
				if (CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)) != _T(""))
				{
					if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)
					{
						if (pAudioStream->IsVBR())
							t = _T("Mp3 VBR\r\n");
						else
							t = _T("Mp3 CBR\r\n");
					}
					else if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_PCM)
						t = _T("Uncompressed Audio\r\n");
					else
						t.Format(_T("%s\r\n"),
							CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));		
				}
				else
					t.Format(_T("Audio 0x%04x\r\n"), pAudioStream->GetFormatTag(true));
				s += t;
			}

			if (pVideoStream && (pAVIPlay->GetVideoStreamsCount() >= 1))
			{
				double dLength;		// Total Length in Seconds
				int nLengthHour;	// Hours Part
				int nLengthMin;		// Minutes Part
				double dLengthSec;	// Seconds Part
				dLength		=	(double)pVideoStream->GetTotalFrames() /
										pVideoStream->GetFrameRate();
				nLengthHour	= (int)(dLength / 3600.0);
				nLengthMin	= (int)((dLength - nLengthHour * 3600.0) / 60.0);
				dLengthSec	= dLength - nLengthHour * 3600.0 - nLengthMin * 60.0;
				t.Format(_T("%02d:%02d:%02d"),	nLengthHour,
												nLengthMin,
												Round(dLengthSec));
				s += t;
			}
		}
		else
		{
			CDib* pDib = m_DibStaticCtrl.GetDibHdrPointer();

			// Dpi
			int nXDpi = pDib->GetXDpi();
			int nYDpi = pDib->GetYDpi();
			BOOL bDpi = TRUE;
			if (nXDpi == 0 || nYDpi == 0)
				bDpi = FALSE;

			// Create Info Text
			s.Format(_T("Size:\t%d x %d\r\n"),	pDib->GetWidth(),
												pDib->GetHeight());

#ifdef SUPPORT_LIBJPEG
			if (pDib->GetExifInfo()->bHasExif)
			{
				t.Format(_T("Depth:\t%s\r\n"), pDib->m_FileInfo.GetDepthName());
				s += t;

				if (bDpi)
				{
					t.Format(_T("Dpi:\t%d x %d\r\n"), nXDpi, nYDpi);
					s += t;
				}
					
				if (pDib->GetExifInfo()->CameraModel[0])
				{
					t.Format(_T("Model:\t%s\r\n"), CString(pDib->GetExifInfo()->CameraModel));
					s+=t;
				}

				if (pDib->GetExifInfo()->Flash >= 0)
				{
					if (pDib->GetExifInfo()->Flash & 1)
						t = ML_STRING(1714, "Flash:\tyes\r\n");
					else
						t = ML_STRING(1715, "Flash:\tno\r\n");
					s+=t;
				}

				if (pDib->GetExifInfo()->ExposureTime)
				{
					t.Format(_T("Exp.:\t%.3f s "), (double)pDib->GetExifInfo()->ExposureTime); s+=t;
					if (pDib->GetExifInfo()->ExposureTime <= 0.5)
					{
						t.Format(_T(" (1/%d)"), Round(1.0 / pDib->GetExifInfo()->ExposureTime));
						s+=t;
					}
					t.Format(_T("\r\n"));
					s+=t;
				}

				if (pDib->GetExifInfo()->ApertureFNumber)
				{
					t.Format(_T("Aperture:\tf/%.1f\r\n"), (double)pDib->GetExifInfo()->ApertureFNumber);
					s+=t;
				}

				if (pDib->GetExifInfo()->DateTime[0])
				{
					CTime Time = CMetadata::GetDateTimeFromExifString(CString(pDib->GetExifInfo()->DateTime));
					CString sTime = ::MakeDateLocalFormat(Time) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE);
					t.Format(_T("Taken:\t%s"), sTime);
					s+=t;
				}
				else
					s += (_T("Created:\t") + GetCreationFileTime(sLastFileName));
			}
			else
#endif
			{
				t.Format(_T("Depth:\t%s\r\n"), pDib->m_FileInfo.GetDepthName());
				s += t;

				t.Format(_T("Image:\t%d %s\r\n"),		(pDib->GetImageSize() >= 1024) ? pDib->GetImageSize() >> 10 : pDib->GetImageSize(),
														(pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				s += t;

				if (bDpi)
				{
					t.Format(_T("Dpi:\t%d x %d\r\n"), nXDpi, nYDpi);
					s += t;
				}

#ifdef SUPPORT_GIFLIB
				if (::GetFileExt(sLastFileName) == _T(".gif"))
				{
					t.Format(_T("Ver:\t%s\r\n"), CDib::GIFGetVersion(sLastFileName, FALSE));
					s += t;
					if (m_DibStaticCtrl.GetGifAnimationThread()->IsRunning() &&
						m_DibStaticCtrl.GetGifAnimationThread()->m_dwDibAnimationCount > 1)
					{
						t.Format(_T("Frames:\t%d\r\n"),	m_DibStaticCtrl.GetGifAnimationThread()->m_dwDibAnimationCount > 1 ?
														m_DibStaticCtrl.GetGifAnimationThread()->m_dwDibAnimationCount : 1);
						s += t;
					}
				}
#endif

				s += (_T("Created:\t") + GetCreationFileTime(sLastFileName));
			}
		}

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
