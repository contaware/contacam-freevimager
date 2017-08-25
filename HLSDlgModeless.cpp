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

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&pDoc->m_AlphaRenderedDib :
					pDoc->m_pDib;

	// Store Preview Dib
	if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
		m_OldPreviewDib = *(pDib->GetPreviewDib());

	// Create Preview Dib
	if ((int)pDib->GetWidth() <= nMaxSizeX &&
		(int)pDib->GetHeight() <= nMaxSizeY)
		pDib->CreatePreviewDib(pDib->GetWidth(), pDib->GetHeight());
	else
		pDib->CreatePreviewDib(nMaxSizeX, nMaxSizeY);

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
			AdjustColor(TRUE); // Adjust Colors of the Preview Dib
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
		::SetRevertedPos(pSlider, 0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST);
		::SetRevertedPos(pSlider, 0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION);
		::SetRevertedPos(pSlider, 0);
		pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE);
		::SetRevertedPos(pSlider, 0);

		pDib->GetPreviewDib()->UndoColor();
		
		pDoc->InvalidateAllViews(FALSE);
	}		
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
			res = pDib->GetPreviewDib()->AdjustImage(::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS)),
													::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST)),
													::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION)),
													::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE)),
													TRUE); // Enable Undo

			pDoc->InvalidateAllViews(FALSE);
		}
	}
	else
	{
		if (pDoc->m_pDib && pDoc->m_pDib->IsValid())
		{
			res = pDoc->m_pDib->AdjustImage(::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS)),
											::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CONTRAST)),
											::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_SATURATION)),
											::GetRevertedPos((CSliderCtrl*)GetDlgItem(IDC_SLIDER_HUE)),
											FALSE); // Disable Undo
			
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

BOOL CHLSDlgModeless::DoIt()
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
	// Delete Preview Dib
	else
		pDib->DeletePreviewDib();

	// Do Nothing if Regulators are all reset
	if (!IsModified())
	{
		// Update Alpha Rendered Dib
		pDoc->UpdateAlphaRenderedDib();

		// Invalidate
		pDoc->InvalidateAllViews(FALSE);

		return TRUE;
	}

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
	// Delete Preview Dib
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
