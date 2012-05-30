// HLSDlgModeless.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "HLSDlgModeless.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "Quantizer.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHLSDlgModeless dialog


CHLSDlgModeless::CHLSDlgModeless(CWnd* pParent)
	: CDialog(CHLSDlgModeless::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHLSDlgModeless)
	m_bShowOriginal = FALSE;
	//}}AFX_DATA_INIT

	m_bFast = FALSE; // Better Quality for Brightness & Contrast
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CHLSDlgModeless::IDD, pParent);
}


void CHLSDlgModeless::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHLSDlgModeless)
	DDX_Control(pDX, IDC_BANDMAX, m_BandMax);
	DDX_Control(pDX, IDC_BAND7, m_Band7);
	DDX_Control(pDX, IDC_BAND6, m_Band6);
	DDX_Control(pDX, IDC_BAND5, m_Band5);
	DDX_Control(pDX, IDC_BAND4, m_Band4);
	DDX_Control(pDX, IDC_BAND3, m_Band3);
	DDX_Control(pDX, IDC_BAND2, m_Band2);
	DDX_Control(pDX, IDC_BAND1, m_Band1);
	DDX_Control(pDX, IDC_BAND0, m_Band0);
	DDX_Control(pDX, IDC_BANDMIN, m_BandMin);
	DDX_Check(pDX, IDC_CHECK_SHOW_ORIGINAL, m_bShowOriginal);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHLSDlgModeless, CDialog)
	//{{AFX_MSG_MAP(CHLSDlgModeless)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_UNDO, OnButtonUndo)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_SHOW_ORIGINAL, OnCheckShowOriginal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHLSDlgModeless message handlers

BOOL CHLSDlgModeless::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	CDialog::OnInitDialog();
	
	CSliderCtrl* pSlider;

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	pSlider->SetRange(-80, 80, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	pSlider->SetPos(0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
	pSlider->SetRange(-80, 80, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	pSlider->SetPos(0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
	pSlider->SetRange(-100, 100, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	pSlider->SetPos(0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
	pSlider->SetRange(-180, 180, TRUE);
	pSlider->SetTicFreq(60);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	pSlider->SetPos(0);

	/*
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_LIGHTNESS);
	pSlider->SetRange(-80, 80, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	pSlider->SetPos(0);
	*/

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA);
	pSlider->SetRange(1, 50, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(5);
	pSlider->SetPos(51 - 10);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_GAMMA);
	CString sGamma;
	sGamma.Format(_T("%0.1f"), 5.1 - (double)((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA))->GetPos() / 10.0);
	pEdit->SetWindowText(sGamma);

	// Band Colors and Range
	m_BandMin.SetColor(RGB(0, 0, 0));
	m_Band0.SetColor(RGB(4, 4, 4));
	m_Band1.SetColor(RGB(28, 28, 28));
	m_Band2.SetColor(RGB(52, 52, 52));
	m_Band3.SetColor(RGB(84, 84, 84));
	m_Band4.SetColor(RGB(124, 124, 124));
	m_Band5.SetColor(RGB(168, 168, 168));
	m_Band6.SetColor(RGB(208, 208, 208));
	m_Band7.SetColor(RGB(240, 240, 240));
	m_BandMax.SetColor(RGB(252, 252, 252));
	m_BandMin.SetRange(0, 30);
	m_Band0.SetRange(0, 30);
	m_Band1.SetRange(0, 30);
	m_Band2.SetRange(0, 30);
	m_Band3.SetRange(0, 30);
	m_Band4.SetRange(0, 30);
	m_Band5.SetRange(0, 30);
	m_Band6.SetRange(0, 30);
	m_Band7.SetRange(0, 30);
	m_BandMax.SetRange(0, 30);

	// Current Monitor's Max Edge
	CSize szMonitor = ::AfxGetMainFrame()->GetMonitorSize();
	int nMaxSizeX = szMonitor.cx;
	int nMaxSizeY = szMonitor.cy;

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&pDoc->m_AlphaRenderedDib :
					pDoc->m_pDib;

	// Store Preview Dib
	if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
		m_OldPreviewDib = *(pDib->GetPreviewDib());

	// Convert to 8 bpp
	if (pDib->GetBitCount() > 8)
	{
		BeginWaitCursor();

		// Create Preview Dib
		if ((int)pDib->GetWidth() <= nMaxSizeX &&
			(int)pDib->GetHeight() <= nMaxSizeY)
			pDib->CreatePreviewDib(pDib->GetWidth(), pDib->GetHeight()); // Same Size
		else
			pDib->CreatePreviewDib(nMaxSizeX, nMaxSizeY);
		
		RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[256];
		CQuantizer Quantizer(256, 8);
		Quantizer.ProcessImage(pDib->GetPreviewDib(), pView, TRUE);
		Quantizer.SetColorTable(pColors);
		pDib->GetPreviewDib()->CreatePaletteFromColors(256, pColors); 
		pDib->GetPreviewDib()->ConvertTo8bitsErrDiff(pDib->GetPreviewDib()->GetPalette(), pView, TRUE);
		delete [] pColors;

		EndWaitCursor();
	}
	else
	{
		// Create Preview Dib
		if ((int)pDib->GetWidth() <= nMaxSizeX &&
			(int)pDib->GetHeight() <= nMaxSizeY)
			pDib->CreatePreviewDib(pDib->GetWidth(), pDib->GetHeight()); // Same Size
		else
			pDib->CreatePreviewDib(nMaxSizeX, nMaxSizeY);

		// Convert to 8bpp
		if (pDib->GetPreviewDib()->GetBitCount() < 8)
			pDib->GetPreviewDib()->ConvertTo8bits(pDib->GetPreviewDib()->GetPalette(), pView, TRUE);
	}
	
	pDoc->InvalidateAllViews(FALSE);

	UpdateHysto();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHLSDlgModeless::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
		(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
		(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
		(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
		(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
		(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
		(SB_LEFT == nSBCode)	||		// Home Button
		(SB_RIGHT == nSBCode))			// End Button
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_GAMMA);
		CString sGamma;
		sGamma.Format(_T("%0.1f"), 5.1 - (double)((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA))->GetPos() / 10.0);
		pEdit->SetWindowText(sGamma);
		if (!m_bShowOriginal)
			AdjustColor(TRUE); // Adjust Colors of the Preview Dib
		UpdateHysto();
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CHLSDlgModeless::OnButtonUndo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&pDoc->m_AlphaRenderedDib :
					pDoc->m_pDib;

	if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
	{
		CSliderCtrl* pSlider;
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
		pSlider->SetPos(0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
		pSlider->SetPos(0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
		pSlider->SetPos(0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
		pSlider->SetPos(0);
		//pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_LIGHTNESS);
		//pSlider->SetPos(0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA);
		pSlider->SetPos(51 - 10);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_GAMMA);
		CString sGamma;
		sGamma.Format(_T("%0.1f"), 5.1 - (double)((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA))->GetPos() / 10.0);
		pEdit->SetWindowText(sGamma);

		pDib->GetPreviewDib()->UndoColor();
		
		pDoc->InvalidateAllViews(FALSE);

		UpdateHysto();
	}		
}

BOOL CHLSDlgModeless::UpdateHysto()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&pDoc->m_AlphaRenderedDib :
					pDoc->m_pDib;

	// Assert
	ASSERT(pDib->GetPreviewDib()->GetBitCount() == 8);

	// Check
	if (!pDib->GetPreviewDib() || !pDib->GetPreviewDib()->IsValid())
		return FALSE;

	DWORD indexes[256];
	DWORD grayscales[256];
	memset(indexes, 0, sizeof(DWORD) * 256);
	memset(grayscales, 0, sizeof(DWORD) * 256);
	int i, k;
	DWORD band;
	double percentage;
	CString sText;
	CEdit* pEdit;
	int nWidth = pDib->GetPreviewDib()->GetWidth();
	int nHeight = pDib->GetPreviewDib()->GetHeight();
	int pixcount = nWidth * nHeight;
	LPBYTE p = pDib->GetPreviewDib()->GetBits();
	int nWidthDWAligned = DWALIGNEDWIDTHBYTES(nWidth * 8); // DWORD aligned (in bytes)

	// Count indexes
	for (k = 0 ; k < nHeight ; k++)
	{
		for (i = 0 ; i < nWidth ; i++)
			indexes[p[i]]++;
		p += nWidthDWAligned;
	}

	// Indexes count to grayscales count
	RGBQUAD* pColors = pDib->GetPreviewDib()->GetColors();
	for (i = 0 ; i < pDib->GetPreviewDib()->GetNumColors() ; i++)
	{
		BYTE gray = CDib::RGBToGray(pColors[i].rgbRed,
									pColors[i].rgbGreen,
									pColors[i].rgbBlue);
		grayscales[gray] += indexes[i];
	}

	// Using the 11 Zones System of Ansel Adams.
	// Brightest two zones are toghether because we can
	// only distinguish 4 bit increments. This low
	// resolution comes from the 8 bpp color conversion
	// which uses a 5-6-5 (16 bpp) lookup table.

	// Band Min: 0..3
	band = 0;
	for (k = 0 ; k < 4 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_BandMin.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BANDMIN);
	pEdit->SetWindowText(sText);

	// Band 0: 4..27
	band = 0;
	for (k = 4 ; k < 28 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band0.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND0);
	pEdit->SetWindowText(sText);
	
	// Band 1: 28..51
	band = 0;
	for (k = 28 ; k < 52 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band1.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND1);
	pEdit->SetWindowText(sText);

	// Band 2: 52..83
	band = 0;
	for (k = 52 ; k < 84 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band2.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND2);
	pEdit->SetWindowText(sText);

	// Band 3: 84..123
	band = 0;
	for (k = 84 ; k < 124 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band3.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND3);
	pEdit->SetWindowText(sText);

	// Band 4: 124..167
	band = 0;
	for (k = 124 ; k < 168 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band4.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND4);
	pEdit->SetWindowText(sText);

	// Band 5: 168..207
	band = 0;
	for (k = 168 ; k < 208 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band5.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND5);
	pEdit->SetWindowText(sText);

	// Band 6: 208..239
	band = 0;
	for (k = 208 ; k < 240 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band6.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND6);
	pEdit->SetWindowText(sText);

	// Band 7: 240..251
	band = 0;
	for (k = 240 ; k < 252 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_Band7.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BAND7);
	pEdit->SetWindowText(sText);

	// Band Max: 252..255
	band = 0;
	for (k = 252 ; k < 256 ; k++)
		band += grayscales[k];
	percentage = 100.0 * (double)band / (double)pixcount;
	m_BandMax.SetPos(Round(percentage));
	if (percentage == 0.0)
		sText = _T("0%");
	else if (percentage == 100.0)
		sText = _T("100%");
	else
		sText.Format(_T("%0.1f%%"), percentage);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_BANDMAX);
	pEdit->SetWindowText(sText);

	return TRUE;
}

BOOL CHLSDlgModeless::AdjustColor(BOOL bAdjustPreviewDib)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	BOOL res = FALSE;

	if (bAdjustPreviewDib)
	{
		// Alpha
		CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
						&pDoc->m_AlphaRenderedDib :
						pDoc->m_pDib;

		if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
		{
			// Adjust Hue
			int nHue = -((CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE))->GetPos();
			if (nHue < 0) nHue += 360;
			res = pDib->GetPreviewDib()->AdjustImage(-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS))->GetPos(),
													-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST))->GetPos(),
													0/*-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_LIGHTNESS))->GetPos()*/,
													-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION))->GetPos(),
													(unsigned short)nHue,
													5.1 - (double)((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA))->GetPos() / 10.0,
													m_bFast,
													TRUE); // Enable Undo

			pDoc->InvalidateAllViews(FALSE);
		}
	}
	else
	{
		if (pDoc->m_pDib && pDoc->m_pDib->IsValid())
		{
			// Adjust Hue
			int nHue = -((CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE))->GetPos();
			if (nHue < 0) nHue += 360;
			res = pDoc->m_pDib->AdjustImage(-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS))->GetPos(),
											-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST))->GetPos(),
											0/*-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_LIGHTNESS))->GetPos()*/,
											-((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION))->GetPos(),
											(unsigned short)nHue,
											5.1 - (double)((CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA))->GetPos() / 10.0,
											m_bFast,
											FALSE, // Disable Undo
											pView, TRUE);
			
			pDoc->InvalidateAllViews(FALSE);
		}
	}

	return res;
}

void CHLSDlgModeless::OnCheckShowOriginal() 
{
	UpdateData(TRUE);
	if (m_bShowOriginal)
	{
		CPictureView* pView = (CPictureView*)m_pParentWnd;
		CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

		// Alpha
		CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
						&pDoc->m_AlphaRenderedDib :
						pDoc->m_pDib;

		if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
			pDib->GetPreviewDib()->UndoColor();

		pDoc->InvalidateAllViews(FALSE);
	}
	else
		AdjustColor(TRUE); // Adjust Colors of the Preview Dib
	UpdateHysto();
}

BOOL CHLSDlgModeless::IsModified()
{
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	if (pSlider->GetPos() != 0)
		return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
	if (pSlider->GetPos() != 0)
		return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
	if (pSlider->GetPos() != 0)
		return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
	if (pSlider->GetPos() != 0)
		return TRUE;
	//pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_LIGHTNESS);
	//if (pSlider->GetPos() != 0)
	//	return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAMMA);
	if (pSlider->GetPos() != 51 - 10)
		return TRUE;
	else
		return FALSE;
}

BOOL CHLSDlgModeless::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&pDoc->m_AlphaRenderedDib :
					pDoc->m_pDib;

	// Do Nothing if Regulators are all reset
	if (!IsModified())
	{
		// Restore Original Preview Dib
		if (m_OldPreviewDib.IsValid())
			*(pDib->GetPreviewDib()) = m_OldPreviewDib;
		// Delete 8bpp Preview Dib
		else
			pDib->DeletePreviewDib();

		// Update Alpha Rendered Dib
		pDoc->UpdateAlphaRenderedDib();

		// Invalidate
		pDoc->InvalidateAllViews(FALSE);

		return TRUE;
	}

	// Restore Original Preview Dib
	if (m_OldPreviewDib.IsValid())
		*(pDib->GetPreviewDib()) = m_OldPreviewDib;
	// Delete 8bpp Preview Dib
	else
		pDib->DeletePreviewDib();

	// Add pDoc->m_pDib To Undo Array
	pDoc->AddUndo();
	
	// Adjust Color:
	BeginWaitCursor();
	AdjustColor(TRUE); // Of Preview (if any)
	BOOL res = AdjustColor(FALSE); // Of Picture

	EndWaitCursor();

	// Update Alpha Rendered Dib
	pDoc->UpdateAlphaRenderedDib();

	// Set Modified Flag
	pDoc->SetModifiedFlag();

	// Document Title
	pDoc->SetDocumentTitle();

	// Invalidate
	pDoc->InvalidateAllViews(FALSE);

	// Update Image Information
	pDoc->UpdateImageInfo();

	return res;
}

BOOL CHLSDlgModeless::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				DoIt();
				DestroyWindow();
				return TRUE;
			case IDCANCEL:
				Close();
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CHLSDlgModeless::OnClose() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&pDoc->m_AlphaRenderedDib :
					pDoc->m_pDib;

	// Restore Original Preview Dib
	if (m_OldPreviewDib.IsValid())
		*(pDib->GetPreviewDib()) = m_OldPreviewDib;
	// Delete 8bpp Preview Dib
	else
		pDib->DeletePreviewDib();

	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);
	DestroyWindow();
}

void CHLSDlgModeless::Close()
{
	OnClose();
}

void CHLSDlgModeless::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	pDoc->m_pHLSDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;
	CDialog::PostNcDestroy();
}
