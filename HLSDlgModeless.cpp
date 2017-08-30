// HLSDlgModeless.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "HLSDlgModeless.h"
#include "PictureDoc.h"
#include "PictureView.h"

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
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CHLSDlgModeless::IDD, pParent);
}


void CHLSDlgModeless::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHLSDlgModeless)
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
	pSlider->SetRange(-200, 200, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	::SetRevertedPos(pSlider, 0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
	pSlider->SetRange(-80, 80, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	::SetRevertedPos(pSlider, 0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
	pSlider->SetRange(-100, 100, TRUE);
	pSlider->SetTicFreq(20);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	::SetRevertedPos(pSlider, 0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
	pSlider->SetRange(-180, 180, TRUE);
	pSlider->SetTicFreq(60);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(10);
	::SetRevertedPos(pSlider, 0);

	// Current Monitor's Max Edge
	CSize szMonitor = ::AfxGetMainFrame()->GetMonitorSize();
	int nMaxSizeX = szMonitor.cx;
	int nMaxSizeY = szMonitor.cy;

	// Work on Preview Dib if the Full Size Dib is too big
	if ((int)pDoc->m_pDib->GetWidth() > nMaxSizeX || (int)pDoc->m_pDib->GetHeight() > nMaxSizeY)
	{
		BeginWaitCursor();
		pDoc->m_pDib->CreatePreviewDib(nMaxSizeX, nMaxSizeY);
		EndWaitCursor();
	}
	// Work on Preview Dib if a bit depth conversion is performed in AdjustImage()
	else if (pDoc->m_pDib->GetBitCount() < 24 ||
			(pDoc->m_pDib->GetBitCount() == 32 && !pDoc->m_pDib->HasAlpha() && !pDoc->m_pDib->IsFast32bpp()))
	{
		BeginWaitCursor();
		pDoc->m_pDib->CreatePreviewDib(pDoc->m_pDib->GetWidth(), pDoc->m_pDib->GetHeight());
		EndWaitCursor();
	}
	// Work only on Full Size Dib
	else
		pDoc->m_pDib->DeletePreviewDib();

	// Update Alpha Rendered Dib and Invalidate
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);

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
		if (!m_bShowOriginal)
		{
			// Adjust colors with undo
			AdjustColor(TRUE);

			// Update Alpha Rendered Dib and Invalidate
			pDoc->UpdateAlphaRenderedDib();
			pDoc->InvalidateAllViews(FALSE);
		}
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CHLSDlgModeless::OnButtonUndo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Reset sliders
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	::SetRevertedPos(pSlider, 0);
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
	::SetRevertedPos(pSlider, 0);
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
	::SetRevertedPos(pSlider, 0);
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
	::SetRevertedPos(pSlider, 0);

	// Undo Preview Dib
	if (pDoc->m_pDib->GetPreviewDib())
		pDoc->m_pDib->GetPreviewDib()->UndoColor();
	// Undo Full Size Dib
	else
		pDoc->m_pDib->UndoColor();
		
	// Update Alpha Rendered Dib and Invalidate
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);		
}

void CHLSDlgModeless::AdjustColor(BOOL bEnableUndo)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Work on Preview Dib
	if (pDoc->m_pDib->GetPreviewDib())
	{
		pDoc->m_pDib->GetPreviewDib()->AdjustImage(	::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS)),
													::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST)),
													::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION)),
													::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE)),
													bEnableUndo);
	}
	// Work on Full Size Dib
	else
	{
		pDoc->m_pDib->AdjustImage(	::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS)),
									::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST)),
									::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION)),
									::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE)),
									bEnableUndo);
	}
}

void CHLSDlgModeless::OnCheckShowOriginal() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	UpdateData(TRUE);

	if (m_bShowOriginal)
	{
		// Undo Preview Dib
		if (pDoc->m_pDib->GetPreviewDib())
			pDoc->m_pDib->GetPreviewDib()->UndoColor();
		// Undo Full Size Dib
		else
			pDoc->m_pDib->UndoColor();
	}
	else
	{
		// Adjust colors with undo
		AdjustColor(TRUE);
	}

	// Update Alpha Rendered Dib and Invalidate
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);
}

BOOL CHLSDlgModeless::IsModified()
{
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	if (::GetRevertedPos(pSlider) != 0)
		return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
	if (::GetRevertedPos(pSlider) != 0)
		return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
	if (::GetRevertedPos(pSlider) != 0)
		return TRUE;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
	if (::GetRevertedPos(pSlider) != 0)
		return TRUE;
	return FALSE;
}

void CHLSDlgModeless::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Work only on Full Size Dib
	if (pDoc->m_pDib->GetPreviewDib())
		pDoc->m_pDib->DeletePreviewDib();
	// Undo Full Size Dib and clean-up
	else
	{
		pDoc->m_pDib->UndoColor();
		pDoc->m_pDib->ResetColorUndo();
	}

	// Are sliders set?
	if (IsModified())
	{
		// Add pDoc->m_pDib To Undo Array
		pDoc->AddUndo();

		// Adjust colors without undo (of the Full Size Dib)
		BeginWaitCursor();
		AdjustColor(FALSE);
		EndWaitCursor();

		// Set Modified Flag
		pDoc->SetModifiedFlag();

		// Document Title
		pDoc->SetDocumentTitle();

		// Update Image Information
		pDoc->UpdateImageInfo();
	}

	// Update Alpha Rendered Dib and Invalidate
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);
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

	// Work only on Full Size Dib
	if (pDoc->m_pDib->GetPreviewDib())
		pDoc->m_pDib->DeletePreviewDib();
	// Undo Full Size Dib and clean-up
	else
	{
		pDoc->m_pDib->UndoColor();
		pDoc->m_pDib->ResetColorUndo();
	}

	// Update Alpha Rendered Dib and Invalidate
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);

	// Destroy dialog window
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
