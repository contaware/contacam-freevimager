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
CSaveFileDlg::CSaveFileDlg(BOOL bShowJPEGCompression, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE)
#else
CSaveFileDlg::CSaveFileDlg(BOOL bShowJPEGCompression, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
							DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
	CFileDialog(FALSE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
#endif
{
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.Flags |= (OFN_EXPLORER | OFN_ENABLESIZING);
	m_bShowJPEGCompression = bShowJPEGCompression;
	m_nJpegCompressionQuality = DEFAULT_JPEGCOMPRESSION;
}

BEGIN_MESSAGE_MAP(CSaveFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CSaveFileDlg)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSaveFileDlg::OnInitDone()
{
	if (m_bShowJPEGCompression)
	{
		// Resize dialog
		CRect rcDialog;
		GetParent()->GetWindowRect(&rcDialog);
		GetParent()->SetWindowPos(	NULL, 0, 0, rcDialog.Width(),
									rcDialog.Height() + SAVEFILEDLG_SLIDER_HEIGHT + SAVEFILEDLG_BOTTOM_OFFSET,
									SWP_NOMOVE | SWP_NOZORDER);
		ScreenToClient(rcDialog);

		// Slider Label
		CRect rcSliderLabel(SAVEFILEDLG_LEFT_OFFSET,
							rcDialog.bottom,
							SAVEFILEDLG_LEFT_OFFSET + SAVEFILEDLG_SLIDERLABEL_WIDTH,
							rcDialog.bottom + SAVEFILEDLG_SLIDER_HEIGHT);
		m_SliderLabel.Create(ML_STRING(1864, "JPEG Quality\n(0: worst, 100: best)"), WS_CHILD | WS_VISIBLE, rcSliderLabel, GetParent(), ctl1+16);
		m_SliderLabel.SetFont(GetParent()->GetFont(), FALSE);

		// Slider Ctrl
		CRect rcSlider(		rcSliderLabel.right,
							rcSliderLabel.top,
							rcDialog.Width() - SAVEFILEDLG_SLIDERTEXT_WIDTH - SAVEFILEDLG_RIGHT_OFFSET,
							rcSliderLabel.bottom);
		m_SliderCtrl.Create(WS_CHILD | WS_VISIBLE | TBS_NOTICKS | WS_TABSTOP | TBS_HORZ, rcSlider, GetParent(), ctl1+17);
		m_SliderCtrl.SetRange(0, 100);
		m_SliderCtrl.SetTicFreq(10);
		m_SliderCtrl.SetPageSize(5);
		m_SliderCtrl.SetLineSize(5);
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			m_nJpegCompressionQuality = ::AfxGetApp()->GetProfileInt(_T("PictureDoc"), _T("JpegSaveAsCompressionQuality"), DEFAULT_JPEGCOMPRESSION);
		m_SliderCtrl.SetPos(m_nJpegCompressionQuality);

		// Slider JPEG Quality Text
		CRect rcSliderText(	rcSlider.right,
							rcSlider.top,
							rcSlider.right + SAVEFILEDLG_SLIDERTEXT_WIDTH,
							rcSlider.bottom);
		CString sSliderText;
		sSliderText.Format(_T("%d"), m_nJpegCompressionQuality);
		m_SliderText.Create(sSliderText, WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE, rcSliderText, GetParent(), ctl1+18);
		m_SliderText.SetFont(GetParent()->GetFont(), FALSE);

		// Init Timer
		SetTimer(ID_TIMER_SAVEFILEDLG, SAVEFILEDLG_TIMER_MS, NULL);
	}
}

// Using polling because ON_WM_HSCROLL_REFLECT() is not working
void CSaveFileDlg::OnTimer(UINT nIDEvent) 
{
	CString sNewText, sCurrentText;
	sNewText.Format(_T("%d"), m_SliderCtrl.GetPos());
	m_SliderText.GetWindowText(sCurrentText);
	if (sCurrentText != sNewText)
		m_SliderText.SetWindowText(sNewText);
	CFileDialog::OnTimer(nIDEvent);
}

void CSaveFileDlg::OnDestroy() 
{
	CFileDialog::OnDestroy();
	if (m_bShowJPEGCompression)
	{
		m_nJpegCompressionQuality = m_SliderCtrl.GetPos();
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			::AfxGetApp()->WriteProfileInt(_T("PictureDoc"), _T("JpegSaveAsCompressionQuality"), m_nJpegCompressionQuality);
		KillTimer(ID_TIMER_SAVEFILEDLG);
	}
}

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

	// Update File Name
	CString fileName; 
	fileNameBox->GetWindowText(fileName);
	int pos;
	CString sCurExt;
	((CComboBox *)typeNameBox)->GetLBText(((CComboBox *)typeNameBox)->GetCurSel(), sCurExt);
	int nIndexStart = sCurExt.ReverseFind(_T('.'));
	int nIndexEnd = sCurExt.ReverseFind(_T(')'));
	sCurExt = sCurExt.Mid(nIndexStart, nIndexEnd - nIndexStart);
	if (fileName.IsEmpty())
		fileName = _T("*") + sCurExt;
	else
	{
		pos = fileName.ReverseFind(_T('.'));
		if (pos != -1)
			fileName.Delete(pos, fileName.GetLength() - pos);
		fileName += sCurExt;
	}
	fileNameBox->SetWindowText(fileName);
}