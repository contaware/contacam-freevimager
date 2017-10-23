// RedEyeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "RedEyeDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "Dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRedEyeDlg dialog


CRedEyeDlg::CRedEyeDlg(CWnd* pParent, CPoint point)
	: CDialog(CRedEyeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRedEyeDlg)
	m_bShowMask = FALSE;
	m_bShowOriginal = FALSE;
	m_uiHueStart = (UINT)REDEYE_HUERANGE_START;
	m_uiHueEnd = (UINT)REDEYE_HUERANGE_END;
	//}}AFX_DATA_INIT
	
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;
	ASSERT(pSrcDib && pSrcDib->IsValid());

	// Adjust Point Offset
	point += pView->GetScrollPosition();
	point -= pView->m_ZoomRect.TopLeft();
	point.x = (int)((double)point.x / pDoc->m_dZoomFactor);
	point.y = (int)((double)point.y / pDoc->m_dZoomFactor);
	point.y = pSrcDib->GetHeight() - point.y - 1; // Top-Down to Bottom-Up
	m_ptRedEyeCenter = point;

	// Undo & Mask Dib Offset
	m_ptOffset.x = m_ptRedEyeCenter.x - (REDEYE_REGION_MAXSIZE / 2);
	m_ptOffset.y = m_ptRedEyeCenter.y - (REDEYE_REGION_MAXSIZE / 2);

	// Reset Vars
	m_bInitialized = FALSE;
	m_pMaskDib = NULL;
	m_pSmoothMaskDib = NULL;
	m_pPrevMaskDib = NULL;
	m_pUndoDib = NULL;
	m_nGoodStartRatio = REDEYE_DEFAULT_REDRATIO;
	m_uiHueStartFoundValue = (UINT)REDEYE_HUERANGE_START;
	m_uiHueEndFoundValue = (UINT)REDEYE_HUERANGE_END;
	m_nPrevRatio = -1;
	m_uiPrevHueStart = 0xFFFFFFFF;
	m_uiPrevHueEnd = 0xFFFFFFFF;
	m_bMaskManuallyModified = FALSE;

	// Create
	CDialog::Create(CRedEyeDlg::IDD, pParent);
}

void CRedEyeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRedEyeDlg)
	DDX_Check(pDX, IDC_CHECK_SHOWMASK, m_bShowMask);
	DDX_Check(pDX, IDC_CHECK_SHOWORIGINAL, m_bShowOriginal);
	DDX_Text(pDX, IDC_EDIT_HUE_START, m_uiHueStart);
	DDV_MinMaxUInt(pDX, m_uiHueStart, 0, 359);
	DDX_Text(pDX, IDC_EDIT_HUE_END, m_uiHueEnd);
	DDV_MinMaxUInt(pDX, m_uiHueEnd, 0, 359);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRedEyeDlg, CDialog)
	//{{AFX_MSG_MAP(CRedEyeDlg)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_UNDO_SIZE, OnButtonUndoSize)
	ON_BN_CLICKED(IDC_BUTTON_UNDO_COLOR, OnButtonUndoColor)
	ON_BN_CLICKED(IDC_CHECK_SHOWMASK, OnCheckShowmask)
	ON_BN_CLICKED(IDC_CHECK_SHOWORIGINAL, OnCheckShoworiginal)
	ON_EN_CHANGE(IDC_EDIT_HUE_START, OnChangeEditHueStart)
	ON_EN_CHANGE(IDC_EDIT_HUE_END, OnChangeEditHueEnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRedEyeDlg message handlers

BOOL CRedEyeDlg::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	CDialog::OnInitDialog();

	// Allocate 32 bpp Mask Dibs
	m_pMaskDib = new CDib;
	if (!m_pMaskDib)
	{
		PostMessage(WM_CLOSE, 0, 0);
		return TRUE;
	}
	if (!m_pMaskDib->AllocateBitsFast(	32,
										BI_RGB,
										REDEYE_REGION_MAXSIZE,
										REDEYE_REGION_MAXSIZE))
	{
		delete m_pMaskDib;
		m_pMaskDib = NULL;
		PostMessage(WM_CLOSE, 0, 0);
		return TRUE;
	}
	m_pMaskDib->SetAlpha(TRUE);
	m_pPrevMaskDib = new CDib;
	if (!m_pPrevMaskDib)
	{
		delete m_pMaskDib;
		m_pMaskDib = NULL;
		PostMessage(WM_CLOSE, 0, 0);
		return TRUE;
	}
	m_pSmoothMaskDib = new CDib;
	if (!m_pSmoothMaskDib)
	{
		delete m_pMaskDib;
		m_pMaskDib = NULL;
		delete m_pPrevMaskDib;
		m_pPrevMaskDib = NULL;
		PostMessage(WM_CLOSE, 0, 0);
		return TRUE;
	}

	// Allocate a Dib For Undo
	m_pUndoDib = new CDib;
	if (!m_pUndoDib)
	{
		delete m_pMaskDib;
		m_pMaskDib = NULL;
		delete m_pPrevMaskDib;
		m_pPrevMaskDib = NULL;
		delete m_pSmoothMaskDib;
		m_pSmoothMaskDib = NULL;
		PostMessage(WM_CLOSE, 0, 0);
		return TRUE;
	}
	if (pSrcDib->GetBitCount() <= 8)
	{
		if (!m_pUndoDib->AllocateBitsFast(	pSrcDib->GetBitCount(),
											BI_RGB,
											REDEYE_REGION_MAXSIZE,
											REDEYE_REGION_MAXSIZE,
											pSrcDib->GetColors(),
											pSrcDib->GetNumColors()))
		{
			delete m_pMaskDib;
			m_pMaskDib = NULL;
			delete m_pPrevMaskDib;
			m_pPrevMaskDib = NULL;
			delete m_pSmoothMaskDib;
			m_pSmoothMaskDib = NULL;
			delete m_pUndoDib;
			m_pUndoDib = NULL;
			PostMessage(WM_CLOSE, 0, 0);
			return TRUE;
		}
	}
	else
	{
		if (!m_pUndoDib->AllocateBitsFast(	pSrcDib->GetBitCount(),
											BI_RGB,
											REDEYE_REGION_MAXSIZE,
											REDEYE_REGION_MAXSIZE))
		{
			delete m_pMaskDib;
			m_pMaskDib = NULL;
			delete m_pPrevMaskDib;
			m_pPrevMaskDib = NULL;
			delete m_pSmoothMaskDib;
			m_pSmoothMaskDib = NULL;
			delete m_pUndoDib;
			m_pUndoDib = NULL;
			PostMessage(WM_CLOSE, 0, 0);
			return TRUE;
		}
	}
	
	// Init Undo Dib
	InitUndoRedEyeRegion(pSrcDib);

	// Calculate the Hue Range
	float fHueRangeStart;
	float fHueRangeEnd;
	FindHueRange(	m_ptRedEyeCenter.x,
					m_ptRedEyeCenter.y,
					&fHueRangeStart,
					&fHueRangeEnd,
					pSrcDib);
	m_uiHueStart = (UINT)fHueRangeStart;
	m_uiHueEnd = (UINT)(fHueRangeEnd + 1.0f);
	if (m_uiHueEnd >= 360)
		m_uiHueEnd = m_uiHueEnd - 360;
	m_uiHueStartFoundValue = m_uiHueStart;
	m_uiHueEndFoundValue = m_uiHueEnd;

	// Find Good Start Ratio
	m_nGoodStartRatio = FindGoodRatio(pSrcDib);

	CSpinButtonCtrl* pSpin;

	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_HUE_START);
	pSpin->SetRange(0, 359);
	pSpin->SetPos(m_uiHueStart);
	
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_HUE_END);
	pSpin->SetRange(0, 359);
	pSpin->SetPos(m_uiHueEnd);

	CSliderCtrl* pSlider;

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_RATIO);
	pSlider->SetRange(REDEYE_LOWEST_REDRATIO, REDEYE_HIGHEST_REDRATIO, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(1);
	pSlider->SetPos(m_nGoodStartRatio);
	
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_REDCHANNEL);
	pSlider->SetRange(-5, 60, TRUE);
	pSlider->SetLineSize(5);
	pSlider->SetPageSize(5);
	pSlider->SetPos(REDEYE_DEFAULT_REDCHANNEL);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GREENCHANNEL);
	pSlider->SetRange(-50, 50, TRUE);
	pSlider->SetLineSize(5);
	pSlider->SetPageSize(5);
	pSlider->SetPos(0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BLUECHANNEL);
	pSlider->SetRange(-50, 50, TRUE);
	pSlider->SetLineSize(5);
	pSlider->SetPageSize(5);
	pSlider->SetPos(0);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	pSlider->SetRange(-75, 25, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(1);
	pSlider->SetPos(REDEYE_DEFAULT_BRIGHTNESS);

	AdjustRedEye(pSrcDib, m_bShowMask);

	m_bInitialized = TRUE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRedEyeDlg::InitUndoRedEyeRegion(CDib* pSrcDib)
{
	for (int y = 0 ; y < (int)m_pUndoDib->GetHeight() ; y++)
	{
		for (int x = 0 ; x < (int)m_pUndoDib->GetWidth() ; x++)
		{
			if (pSrcDib->GetBitCount() <= 8)
			{
				m_pUndoDib->SetPixelIndex(x, y,
						pSrcDib->GetPixelIndex(	x + m_ptOffset.x,
												y + m_ptOffset.y));

			}
			else
			{
				m_pUndoDib->SetPixelColor(x, y,
						pSrcDib->GetPixelColor(	x + m_ptOffset.x,
												y + m_ptOffset.y));
			}
		}
	}
}

void CRedEyeDlg::UndoRedEyeRegion(CDib* pSrcDib)
{
	if (m_pUndoDib && pSrcDib)
	{
		for (int y = 0 ; y < (int)m_pUndoDib->GetHeight() ; y++)
		{
			for (int x = 0 ; x < (int)m_pUndoDib->GetWidth() ; x++)
			{
				if (pSrcDib->GetBitCount() <= 8)
				{
					pSrcDib->SetPixelIndex(	x + m_ptOffset.x,
											y + m_ptOffset.y,
											m_pUndoDib->GetPixelIndex(x, y));

				}
				else
				{
					pSrcDib->SetPixelColor(	x + m_ptOffset.x,
											y + m_ptOffset.y,
											m_pUndoDib->GetPixelColor(x, y));
				}
			}
		}
	}
}

void CRedEyeDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	if (m_pUndoDib)
	{
		delete m_pUndoDib;
		m_pUndoDib = NULL;
	}
	if (m_pMaskDib)
	{
		delete m_pMaskDib;
		m_pMaskDib = NULL;
	}
	if (m_pPrevMaskDib)
	{
		delete m_pPrevMaskDib;
		m_pPrevMaskDib = NULL;
	}
	if (m_pSmoothMaskDib)
	{
		delete m_pSmoothMaskDib;
		m_pSmoothMaskDib = NULL;
	}
	pDoc->m_pRedEyeDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;
	CDialog::PostNcDestroy();
}

int CRedEyeDlg::FindGoodRatio(CDib* pSrcDib)
{
	int nRatio = REDEYE_LOWEST_REDRATIO;
	int nGoodRatio;
	double dRadiusRatio;
	double dLowestGoodRadiusRatio;

	// Find a good one
	do
	{
		// A negative radius ratio means that it's not a good one
		dRadiusRatio = MakeRedEyeMask(	m_ptRedEyeCenter.x,
										m_ptRedEyeCenter.y,
										(int)m_pMaskDib->GetWidth() / 2,
										(int)m_pMaskDib->GetHeight() / 2,
										(float)m_uiHueStart,
										(float)m_uiHueEnd,
										nRatio,
										pSrcDib,
										m_pMaskDib);
		nRatio += 2;
	
	}
	while (nRatio <= REDEYE_HIGHEST_REDRATIO && dRadiusRatio < 0.0);

	// If a good one found
	if (dRadiusRatio > 0.0)
	{
		// Init Good Ratio
		nGoodRatio = nRatio - 2;
		dLowestGoodRadiusRatio = dRadiusRatio;

		// Go till the good one ends
		while (nRatio <= REDEYE_HIGHEST_REDRATIO && dRadiusRatio > 0.0)
		{
			// A negative radius ratio means that it's not a good one
			dRadiusRatio = MakeRedEyeMask(	m_ptRedEyeCenter.x,
											m_ptRedEyeCenter.y,
											(int)m_pMaskDib->GetWidth() / 2,
											(int)m_pMaskDib->GetHeight() / 2,
											(float)m_uiHueStart,
											(float)m_uiHueEnd,
											nRatio,
											pSrcDib,
											m_pMaskDib);

			if (dRadiusRatio > 0.0 && dRadiusRatio < dLowestGoodRadiusRatio)
			{
				dLowestGoodRadiusRatio = dRadiusRatio;
				nGoodRatio = nRatio;
			}
			nRatio += 2;
		}

		if (nRatio <= REDEYE_HIGHEST_REDRATIO && dRadiusRatio < 0.0)
		{
			// Find next good ones if any
			do
			{
				// A negative radius ratio means that it's not a good one
				dRadiusRatio = MakeRedEyeMask(	m_ptRedEyeCenter.x,
												m_ptRedEyeCenter.y,
												(int)m_pMaskDib->GetWidth() / 2,
												(int)m_pMaskDib->GetHeight() / 2,
												(float)m_uiHueStart,
												(float)m_uiHueEnd,
												nRatio,
												pSrcDib,
												m_pMaskDib);
				nRatio += 2;
			
			}
			while (nRatio <= REDEYE_HIGHEST_REDRATIO && dRadiusRatio < 0.0);

			// If a good one found
			if (dRadiusRatio > 0.0)
			{
				// Set Good Ratio if it's the case
				if (dRadiusRatio < dLowestGoodRadiusRatio)
				{
					nGoodRatio = nRatio - 2;
					dLowestGoodRadiusRatio = dRadiusRatio;
				}

				// Go till the good one ends
				while (nRatio <= REDEYE_HIGHEST_REDRATIO && dRadiusRatio > 0.0)
				{
					// A negative radius ratio means that it's not a good one
					dRadiusRatio = MakeRedEyeMask(	m_ptRedEyeCenter.x,
													m_ptRedEyeCenter.y,
													(int)m_pMaskDib->GetWidth() / 2,
													(int)m_pMaskDib->GetHeight() / 2,
													(float)m_uiHueStart,
													(float)m_uiHueEnd,
													nRatio,
													pSrcDib,
													m_pMaskDib);

					if (dRadiusRatio > 0.0 && dRadiusRatio < dLowestGoodRadiusRatio)
					{
						dLowestGoodRadiusRatio = dRadiusRatio;
						nGoodRatio = nRatio;
					}
					nRatio += 2;
				}
			}
		}
		
		return nGoodRatio;				// Retun good one
	}
	else
		return REDEYE_DEFAULT_REDRATIO;	// Return default
}

void CRedEyeDlg::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Undo
	UndoRedEyeRegion(pSrcDib);

	// Add pDoc->m_pDib To Undo Array
	pDoc->AddUndo();
	
	// Adjust Red-Eye
	InitUndoRedEyeRegion(pDoc->m_pDib);
	AdjustRedEye(pDoc->m_pDib, FALSE);

	pDoc->UpdateAlphaRenderedDib();
	pDoc->SetModifiedFlag();
	pDoc->SetDocumentTitle();
	pDoc->UpdateImageInfo();
}

BOOL CRedEyeDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
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

void CRedEyeDlg::OnClose() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	UndoRedEyeRegion(pSrcDib);
	pView->Invalidate(FALSE);
	DestroyWindow();
}

void CRedEyeDlg::Close()
{
	OnClose();
}

void CRedEyeDlg::OnButtonUndoSize() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Undo
	UndoRedEyeRegion(pSrcDib);
	
	// Reset Spins
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_HUE_START);
	pSpin->SetPos(m_uiHueStartFoundValue);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_HUE_END);
	pSpin->SetPos(m_uiHueEndFoundValue);

	// Reset Slider
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_RATIO);
	pSlider->SetPos(m_nGoodStartRatio);

	// Force Mask Recalc. with next AdjustRedEye() call
	if (m_bMaskManuallyModified)
		m_nPrevRatio = -1;

	// Adjust Red-Eye
	if (!m_bShowOriginal)
		AdjustRedEye(pSrcDib, m_bShowMask);
}

void CRedEyeDlg::OnButtonUndoColor() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Undo
	UndoRedEyeRegion(pSrcDib);

	// Reset Sliders
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_REDCHANNEL);
	pSlider->SetPos(REDEYE_DEFAULT_REDCHANNEL);
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GREENCHANNEL);
	pSlider->SetPos(0);
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BLUECHANNEL);
	pSlider->SetPos(0);
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	pSlider->SetPos(REDEYE_DEFAULT_BRIGHTNESS);

	// Adjust Red-Eye
	if (!m_bShowOriginal)
		AdjustRedEye(pSrcDib, m_bShowMask);
}

void CRedEyeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

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
			AdjustRedEye(pSrcDib, m_bShowMask);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CRedEyeDlg::AdjustRedEye(CDib* pSrcDib, BOOL bShowMask)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;

	CSliderCtrl* pSliderRatio = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_RATIO);
	int nCurrentRatio = pSliderRatio->GetPos();
	CSliderCtrl* pSliderRedChannel = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_REDCHANNEL);
	CSliderCtrl* pSliderGreenChannel = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GREENCHANNEL);
	CSliderCtrl* pSliderBlueChannel = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BLUECHANNEL);
	CSliderCtrl* pSliderBrightnessChannel = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BRIGHTNESS);
	
	// Undo Colors
	UndoRedEyeRegion(pSrcDib);

	// Make mask if ratio or hue changed
	if (nCurrentRatio != m_nPrevRatio		||
		m_uiHueStart != m_uiPrevHueStart	||
		m_uiHueEnd != m_uiPrevHueEnd)
	{
		MakeRedEyeMask(	m_ptRedEyeCenter.x,
						m_ptRedEyeCenter.y,
						(int)m_pMaskDib->GetWidth() / 2,
						(int)m_pMaskDib->GetHeight() / 2,
						(float)m_uiHueStart,
						(float)m_uiHueEnd,
						nCurrentRatio,
						pSrcDib,
						m_pMaskDib);
		m_nPrevRatio = nCurrentRatio;
		m_uiPrevHueStart = m_uiHueStart;
		m_uiPrevHueEnd = m_uiHueEnd;
		m_bMaskManuallyModified = FALSE;
		*m_pPrevMaskDib = *m_pMaskDib;
	}
	else
		*m_pMaskDib = *m_pPrevMaskDib;

	// Draw Mask
	if (bShowMask)
	{
		DrawMask(	m_ptRedEyeCenter.x,
					m_ptRedEyeCenter.y,
					(int)m_pMaskDib->GetWidth() / 2,
					(int)m_pMaskDib->GetHeight() / 2,
					pSrcDib,
					m_pMaskDib);
	}
	// Remove Red-Eye
	else
	{
		// Copy
		*m_pSmoothMaskDib = *m_pMaskDib;

		// Smooth Mask Borders
		SmoothMask(	(int)m_pSmoothMaskDib->GetWidth() / 2,
					(int)m_pSmoothMaskDib->GetHeight() / 2,
					m_pSmoothMaskDib);

		// Remove
		RemoveRedEye(	pSliderRedChannel->GetPos(),
						pSliderGreenChannel->GetPos(),
						pSliderBlueChannel->GetPos(),
						pSliderBrightnessChannel->GetPos(),
						m_ptRedEyeCenter.x,
						m_ptRedEyeCenter.y,
						(int)m_pSmoothMaskDib->GetWidth() / 2,
						(int)m_pSmoothMaskDib->GetHeight() / 2,
						pSrcDib,
						m_pSmoothMaskDib);
	}
	
	pView->Invalidate(FALSE);
}

BOOL CRedEyeDlg::AddMaskPoint(CPoint point)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Check
	if (!m_pMaskDib || !m_pMaskDib->IsValid()			||
		!m_pPrevMaskDib || !m_pPrevMaskDib->IsValid()	||
		!pSrcDib || !pSrcDib->IsValid())
		return FALSE;

	// Adjust Point Offset
	point += pView->GetScrollPosition();
	point -= pView->m_ZoomRect.TopLeft();
	point.x = (int)((double)point.x / pDoc->m_dZoomFactor);
	point.y = (int)((double)point.y / pDoc->m_dZoomFactor);
	point.y = pSrcDib->GetHeight() - point.y - 1; // Top-Down to Bottom-Up
	point.x -=m_ptOffset.x;
	point.y -=m_ptOffset.y;

	// Set Mask Pixel
	if (point.x >= 0 && point.x < (int)m_pMaskDib->GetWidth() &&
		point.y >= 0 && point.y < (int)m_pMaskDib->GetHeight())
	{
		m_pMaskDib->SetPixelColor32Alpha(point.x, point.y, 0xFF000000);
		m_pPrevMaskDib->SetPixelColor32Alpha(point.x, point.y, 0xFF000000);
		if (!m_bShowOriginal)
			AdjustRedEye(pSrcDib, m_bShowMask);
		m_bMaskManuallyModified = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CRedEyeDlg::RemoveMaskPoint(CPoint point)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Check
	if (!m_pMaskDib || !m_pMaskDib->IsValid()			||
		!m_pPrevMaskDib || !m_pPrevMaskDib->IsValid()	||
		!pSrcDib || !pSrcDib->IsValid())
		return FALSE;

	// Adjust Point Offset
	point += pView->GetScrollPosition();
	point -= pView->m_ZoomRect.TopLeft();
	point.x = (int)((double)point.x / pDoc->m_dZoomFactor);
	point.y = (int)((double)point.y / pDoc->m_dZoomFactor);
	point.y = pSrcDib->GetHeight() - point.y - 1; // Top-Down to Bottom-Up
	point.x -=m_ptOffset.x;
	point.y -=m_ptOffset.y;

	// Clear Mask Pixel
	if (point.x >= 0 && point.x < (int)m_pMaskDib->GetWidth() &&
		point.y >= 0 && point.y < (int)m_pMaskDib->GetHeight())
	{
		m_pMaskDib->SetPixelColor32Alpha(point.x, point.y, 0);
		m_pPrevMaskDib->SetPixelColor32Alpha(point.x, point.y, 0);
		if (!m_bShowOriginal)
			AdjustRedEye(pSrcDib, m_bShowMask);
		m_bMaskManuallyModified = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

// Check to see if all of the 8 neighbours are
// red-eye or bright-white pixels
BOOL CRedEyeDlg::CheckNeighbours(	int nXRedEyeCenter,
									int nYRedEyeCenter,
									CDib* pSrcDib,
									int* pRedEyeCount,
									int* pBrightWhiteCount,
									int x,
									int y,
									float fHueRangeStart,
									float fHueRangeEnd,
									int nRatio)
{
	if (!pRedEyeCount || !pBrightWhiteCount)
		return FALSE;

	for (int j = -1 ; j <= 1 ; j++)
	{
		for (int i = -1 ; i <= 1 ; i++)
		{
			if (i != 0 && j != 0)
			{
				if (IsRedEyePixel(pSrcDib->GetPixelColor(	nXRedEyeCenter + x + i,
															nYRedEyeCenter + y + j),
															fHueRangeStart,
															fHueRangeEnd,
															nRatio))			
					(*pRedEyeCount)++;
				else if (IsBrightWhite(pSrcDib->GetPixelColor(	nXRedEyeCenter + x + i,
																nYRedEyeCenter + y + j)))
					(*pBrightWhiteCount)++;
			}
		}
	}

	return TRUE;
}

// Check the mask value of all 8 neighbours,
// return the number of set masks.
__forceinline int CRedEyeDlg::GetNeighboursMaskCount(	int nXMaskCenter,
														int nYMaskCenter,
														CDib* pMaskDib,
														int x,
														int y)
{
	if (!pMaskDib || !pMaskDib->IsValid())
		return 0;

	int nCount = 0;
	COLORREF mask;

	for (int j = -1 ; j <= 1 ; j++)
	{
		for (int i = -1 ; i <= 1 ; i++)
		{
			if (i != 0 && j != 0)
			{
				mask = pMaskDib->GetPixelColor32Alpha(	nXMaskCenter + x + i,
														nYMaskCenter + y + j);
				if (mask)
					nCount++;
			}
		}
	}

	return nCount;
}

// From the passed source Dib, given the approximate
// center of the red-eye, find good values for the
// hue range.
BOOL CRedEyeDlg::FindHueRange(	int nXRedEyeCenter,
								int nYRedEyeCenter,
								float* pHueRangeStart,
								float* pHueRangeEnd,
								CDib* pSrcDib)
{
	int i, x, y;
	float fMinHue = 360.0f + REDEYE_HUERANGE_END;
	float fMaxHue = REDEYE_HUERANGE_START;
	CColor c;
	
	// Check
	if (!pSrcDib || !pSrcDib->IsValid() ||
		!pHueRangeStart || !pHueRangeEnd)
		return FALSE;

	// Center Pixel
	if (IsRedEyePixel(c = pSrcDib->GetPixelColor(	nXRedEyeCenter,
													nYRedEyeCenter),
													REDEYE_HUERANGE_START,
													REDEYE_HUERANGE_END,
													REDEYE_HUERANGE_REDRATIO_1))
	{
		// 0° - REDEYE_HUERANGE_END°
		if (c.GetHue() <= REDEYE_HUERANGE_END)
		{
			if ((c.GetHue() + 360.0f) > fMaxHue)
				fMaxHue = (c.GetHue() + 360.0f);
			else if ((c.GetHue() + 360.0f) < fMinHue)
				fMinHue = (c.GetHue() + 360.0f);
		}
		// REDEYE_HUERANGE_START° - 359°
		else
		{
			if (c.GetHue() > fMaxHue)
				fMaxHue = c.GetHue();
			else if (c.GetHue() < fMinHue)
				fMinHue = c.GetHue();
		}
	}

	// Other Pixels
	for (i = 1 ; i <= REDEYE_HUERANGE_SEARCHRADIUS ; i++)
	{
		// Ratio depends from the current square
		int nRatio;
		if (i <= REDEYE_HUERANGE_SEARCHRADIUS / 4)
			nRatio = REDEYE_HUERANGE_REDRATIO_1;
		else if (i <= REDEYE_HUERANGE_SEARCHRADIUS / 2)
			nRatio = REDEYE_HUERANGE_REDRATIO_2;
		else if (i <= 3 * REDEYE_HUERANGE_SEARCHRADIUS / 4)
			nRatio = REDEYE_HUERANGE_REDRATIO_3;
		else
			nRatio = REDEYE_HUERANGE_REDRATIO_4;

		// Bottom Edge (Remember that the dib is bottom-up)
		y = -i;
		for (x = -i ; x <= i ; x++)
		{
			if (IsRedEyePixel(c = pSrcDib->GetPixelColor(nXRedEyeCenter + x,
														nYRedEyeCenter + y),
														REDEYE_HUERANGE_START,
														REDEYE_HUERANGE_END,
														nRatio))
			{
				// 0° - REDEYE_HUERANGE_END°
				if (c.GetHue() <= REDEYE_HUERANGE_END)
				{
					if ((c.GetHue() + 360.0f) > fMaxHue)
						fMaxHue = (c.GetHue() + 360.0f);
					else if ((c.GetHue() + 360.0f) < fMinHue)
						fMinHue = (c.GetHue() + 360.0f);
				}
				// REDEYE_HUERANGE_START° - 359°
				else
				{
					if (c.GetHue() > fMaxHue)
						fMaxHue = c.GetHue();
					else if (c.GetHue() < fMinHue)
						fMinHue = c.GetHue();
				}
			}
		}

		// Top Edge (Remember that the dib is bottom-up)
		y = i;
		for (x = -i ; x <= i ; x++)
		{
			if (IsRedEyePixel(c = pSrcDib->GetPixelColor(nXRedEyeCenter + x,
														nYRedEyeCenter + y),
														REDEYE_HUERANGE_START,
														REDEYE_HUERANGE_END,
														nRatio))
			{
				// 0° - REDEYE_HUERANGE_END°
				if (c.GetHue() <= REDEYE_HUERANGE_END)
				{
					if ((c.GetHue() + 360.0f) > fMaxHue)
						fMaxHue = (c.GetHue() + 360.0f);
					else if ((c.GetHue() + 360.0f) < fMinHue)
						fMinHue = (c.GetHue() + 360.0f);
				}
				// REDEYE_HUERANGE_START° - 359°
				else
				{
					if (c.GetHue() > fMaxHue)
						fMaxHue = c.GetHue();
					else if (c.GetHue() < fMinHue)
						fMinHue = c.GetHue();
				}
			}
		}

		// Left Edge (Remember that the dib is bottom-up)
		x = -i;
		for (y = -i + 1 ; y < i ; y++)
		{
			if (IsRedEyePixel(c = pSrcDib->GetPixelColor(nXRedEyeCenter + x,
														nYRedEyeCenter + y),
														REDEYE_HUERANGE_START,
														REDEYE_HUERANGE_END,
														nRatio))
			{
				// 0° - REDEYE_HUERANGE_END°
				if (c.GetHue() <= REDEYE_HUERANGE_END)
				{
					if ((c.GetHue() + 360.0f) > fMaxHue)
						fMaxHue = (c.GetHue() + 360.0f);
					else if ((c.GetHue() + 360.0f) < fMinHue)
						fMinHue = (c.GetHue() + 360.0f);
				}
				// REDEYE_HUERANGE_START° - 359°
				else
				{
					if (c.GetHue() > fMaxHue)
						fMaxHue = c.GetHue();
					else if (c.GetHue() < fMinHue)
						fMinHue = c.GetHue();
				}
			}
		}

		// Right Edge (Remember that the dib is bottom-up)
		x = i;
		for (y = -i + 1 ; y < i ; y++)
		{
			if (IsRedEyePixel(c = pSrcDib->GetPixelColor(nXRedEyeCenter + x,
														nYRedEyeCenter + y),
														REDEYE_HUERANGE_START,
														REDEYE_HUERANGE_END,
														nRatio))
			{
				// 0° - REDEYE_HUERANGE_END°
				if (c.GetHue() <= REDEYE_HUERANGE_END)
				{
					if ((c.GetHue() + 360.0f) > fMaxHue)
						fMaxHue = (c.GetHue() + 360.0f);
					else if ((c.GetHue() + 360.0f) < fMinHue)
						fMinHue = (c.GetHue() + 360.0f);
				}
				// REDEYE_HUERANGE_START° - 359°
				else
				{
					if (c.GetHue() > fMaxHue)
						fMaxHue = c.GetHue();
					else if (c.GetHue() < fMinHue)
						fMinHue = c.GetHue();
				}
			}
		}
	}

	// Return Values
	if (fMinHue >= 360.0f)
		fMinHue = fMinHue - 360.0f;
	if (fMaxHue >= 360.0f)
		fMaxHue = fMaxHue - 360.0f;
	*pHueRangeStart = fMinHue;
	*pHueRangeEnd = fMaxHue;

	return TRUE;
}

BOOL CRedEyeDlg::CalcRadiuses(	int nXMaskCenter,
								int nYMaskCenter,
								double* pMinRadius,
								double* pMaxRadius,
								double* pAvgRadius,
								CDib* pMaskDib)
{
	int x, y;
	COLORREF mask;
	CPoint Top(0,0);
	CPoint Bottom(0,0);
	CPoint Left(0,0);
	CPoint Right(0,0);
	CPoint TopLeft(0,0);
	CPoint TopRight(0,0);
	CPoint BottomLeft(0,0);
	CPoint BottomRight(0,0);

	// Check
	if (!pMaskDib || !pMaskDib->IsValid())
		return FALSE;

	x = 0;

	// Top (Remember that the dib is bottom-up)
	for (y = (int)pMaskDib->GetHeight() / 2 ; y >= 1 ; y--)
	{
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			Top.x = x;
			Top.y = y;
			break;
		}
	}

	// Bottom (Remember that the dib is bottom-up)
	for (y = -((int)pMaskDib->GetHeight() / 2) ; y <= -1  ; y++)
	{
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			Bottom.x = x;
			Bottom.y = y;
			break;
		}
	}

	y = 0;

	// Left (Remember that the dib is bottom-up)
	for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= -1 ; x++)
	{
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			Left.x = x;
			Left.y = y;
			break;
		}
	}

	// Right (Remember that the dib is bottom-up)
	for (x = (int)pMaskDib->GetWidth() / 2 ; x >= 1 ; x--)
	{
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			Right.x = x;
			Right.y = y;
			break;
		}
	}

	// Bottom Left (Remember that the dib is bottom-up)
	for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= -1 ; x++)
	{
		y = x;
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			BottomLeft.x = x;
			BottomLeft.y = y;
			break;
		}
	}

	// Bottom Right (Remember that the dib is bottom-up)
	for (x = (int)pMaskDib->GetWidth() / 2 ; x >= 1 ; x--)
	{
		y = -x;
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			BottomRight.x = x;
			BottomRight.y = y;
			break;
		}
	}

	// Top Right (Remember that the dib is bottom-up)
	for (x = (int)pMaskDib->GetWidth() / 2 ; x >= 1 ; x--)
	{
		y = x;
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			TopRight.x = x;
			TopRight.y = y;
			break;
		}
	}

	// Top Left (Remember that the dib is bottom-up)
	for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= -1 ; x++)
	{
		y = -x;
		mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
		if (mask)
		{
			TopLeft.x = x;
			TopLeft.y = y;
			break;
		}
	}

	// Radiuses
	double dMaxRadius = (double)pMaskDib->GetWidth() / 2.0;
	double dLeftRadius = min(dMaxRadius, (double)-Left.x);
	double dRightRadius = min(dMaxRadius, (double)Right.x);
	double dTopRadius = min(dMaxRadius, (double)Top.y);
	double dBottomRadius = min(dMaxRadius, (double)-Bottom.y);
	double dTopLeftRadius = min(dMaxRadius, sqrt((double)(TopLeft.x*TopLeft.x + TopLeft.y*TopLeft.y)));
	double dTopRightRadius = min(dMaxRadius, sqrt((double)(TopRight.x*TopRight.x + TopRight.y*TopRight.y)));
	double dBottomLeftRadius = min(dMaxRadius, sqrt((double)(BottomLeft.x*BottomLeft.x + BottomLeft.y*BottomLeft.y)));
	double dBottomRightRadius = min(dMaxRadius, sqrt((double)(BottomRight.x*BottomRight.x + BottomRight.y*BottomRight.y)));
	if (pMinRadius)							
		*pMinRadius =	min(
							min(min(dLeftRadius,dRightRadius),min(dTopRadius,dBottomRadius)),
							min(min(dTopLeftRadius,dTopRightRadius),min(dBottomLeftRadius,dBottomRightRadius))
						);
	if (pMaxRadius)
		*pMaxRadius =	max(
							max(max(dLeftRadius,dRightRadius),max(dTopRadius,dBottomRadius)),
							max(max(dTopLeftRadius,dTopRightRadius),max(dBottomLeftRadius,dBottomRightRadius))
						);
	
	if (pAvgRadius)
		*pAvgRadius = (dLeftRadius + dRightRadius + dTopRadius + dBottomRadius +
					dTopLeftRadius + dTopRightRadius + dBottomLeftRadius + dBottomRightRadius) / 8.0;

	return TRUE;
}

BOOL CRedEyeDlg::GetRedEyeOffsets(	int nXMaskCenter,
									int nYMaskCenter,
									CPoint& MinOffset,
									CDib* pMaskDib)
{
	// Check
	if (!pMaskDib || !pMaskDib->IsValid())
		return FALSE;

	// Clear
	MinOffset = CPoint(0,0);

	for (int nRedEyeNeighbours = REDEYE_NEIGHBOURS_THRESHOLD ; nRedEyeNeighbours >= 1 ; nRedEyeNeighbours--)
	{
		int x, y;
		COLORREF mask;
		CPoint Top(0,0);
		CPoint Bottom(0,0);
		CPoint Left(0,0);
		CPoint Right(0,0);
		CPoint TopLeft(0,0);
		CPoint TopRight(0,0);
		CPoint BottomLeft(0,0);
		CPoint BottomRight(0,0);
		BOOL bTopSet = FALSE;
		BOOL bBottomSet = FALSE;
		BOOL bLeftSet = FALSE;
		BOOL bRightSet = FALSE;
		BOOL bTopLeftSet = FALSE;
		BOOL bTopRightSet = FALSE;
		BOOL bBottomLeftSet = FALSE;
		BOOL bBottomRightSet = FALSE;

		x = 0;

		// Top (Remember that the dib is bottom-up)
		for (y = 1 ; y <= (int)pMaskDib->GetHeight() / 2 ; y++)
		{
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bTopSet = TRUE;
				Top.x = x;
				Top.y = y;
				break;
			}
		}

		// Bottom (Remember that the dib is bottom-up)
		for (y = -1 ; y >= -((int)pMaskDib->GetHeight() / 2) ; y--)
		{
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bBottomSet = TRUE;
				Bottom.x = x;
				Bottom.y = y;
				break;
			}
		}

		y = 0;

		// Left (Remember that the dib is bottom-up)
		for (x = -1 ; x >= -((int)pMaskDib->GetWidth() / 2) ; x--)
		{
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bLeftSet = TRUE;
				Left.x = x;
				Left.y = y;
				break;
			}
		}

		// Right (Remember that the dib is bottom-up)
		for (x = 1 ; x <= (int)pMaskDib->GetWidth() / 2 ; x++)
		{
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bRightSet = TRUE;
				Right.x = x;
				Right.y = y;
				break;
			}
		}

		// Bottom Left (Remember that the dib is bottom-up)
		for (x = -1 ; x >= -((int)pMaskDib->GetWidth() / 2) ; x--)
		{
			y = x;
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bBottomLeftSet = TRUE;
				BottomLeft.x = x;
				BottomLeft.y = y;
				break;
			}
		}

		// Bottom Right (Remember that the dib is bottom-up)
		for (x = 1 ; x <= (int)pMaskDib->GetWidth() / 2 ; x++)
		{
			y = -x;
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bBottomRightSet = TRUE;
				BottomRight.x = x;
				BottomRight.y = y;
				break;
			}
		}

		// Top Right (Remember that the dib is bottom-up)
		for (x = 1 ; x <= (int)pMaskDib->GetWidth() / 2 ; x++)
		{
			y = x;
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bTopRightSet = TRUE;
				TopRight.x = x;
				TopRight.y = y;
				break;
			}
		}

		// Top Left (Remember that the dib is bottom-up)
		for (x = -1 ; x >= -((int)pMaskDib->GetWidth() / 2) ; x--)
		{
			y = -x;
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask && GetNeighboursMaskCount(	nXMaskCenter + x,
												nYMaskCenter + y,
												pMaskDib,
												0,
												0) >= nRedEyeNeighbours)
			{
				bTopLeftSet = TRUE;
				TopLeft.x = x;
				TopLeft.y = y;
				break;
			}
		}

		// Min offset
		double dLeftRadius, dRightRadius, dTopRadius, dBottomRadius;
		double dTopLeftRadius, dTopRightRadius, dBottomLeftRadius, dBottomRightRadius;
		double dBigValue = (double)pMaskDib->GetWidth();
		double dMaxRadius = (double)pMaskDib->GetWidth() / 2.0;
		if (bLeftSet)
			dLeftRadius =			min(dMaxRadius, (double)-Left.x);
		else
			dLeftRadius =			dBigValue;
		if (bRightSet)
			dRightRadius =			min(dMaxRadius, (double)Right.x);
		else
			dRightRadius =			dBigValue;
		if (bTopSet)
			dTopRadius =			min(dMaxRadius, (double)Top.y);
		else
			dTopRadius =			dBigValue;
		if (bBottomSet)
			dBottomRadius =			min(dMaxRadius, (double)-Bottom.y);
		else
			dBottomRadius =			dBigValue;
		if (bTopLeftSet)
			dTopLeftRadius =		min(dMaxRadius, sqrt((double)(TopLeft.x*TopLeft.x + TopLeft.y*TopLeft.y)));
		else
			dTopLeftRadius =		dBigValue;
		if (bTopRightSet)
			dTopRightRadius =		min(dMaxRadius, sqrt((double)(TopRight.x*TopRight.x + TopRight.y*TopRight.y)));
		else
			dTopRightRadius =		dBigValue;
		if (bBottomLeftSet)
			dBottomLeftRadius =		min(dMaxRadius, sqrt((double)(BottomLeft.x*BottomLeft.x + BottomLeft.y*BottomLeft.y)));
		else
			dBottomLeftRadius =		dBigValue;
		if (bBottomRightSet)
			dBottomRightRadius =	min(dMaxRadius, sqrt((double)(BottomRight.x*BottomRight.x + BottomRight.y*BottomRight.y)));
		else
			dBottomRightRadius =	dBigValue;
		double dMinRadius =			min(
										min(min(dLeftRadius,dRightRadius),min(dTopRadius,dBottomRadius)),
										min(min(dTopLeftRadius,dTopRightRadius),min(dBottomLeftRadius,dBottomRightRadius))
									);
		if (dMinRadius == dLeftRadius)
			MinOffset = Left;
		else if (dMinRadius == dRightRadius)
			MinOffset = Right;
		else if (dMinRadius == dTopRadius)
			MinOffset = Top;
		else if (dMinRadius == dBottomRadius)
			MinOffset = Bottom;
		else if (dMinRadius == dTopLeftRadius)
			MinOffset = TopLeft;
		else if (dMinRadius == dTopRightRadius)
			MinOffset = TopRight;
		else if (dMinRadius == dBottomLeftRadius)
			MinOffset = BottomLeft;
		else
			MinOffset = BottomRight;

		// Return if the offset differs from the origin
		// and if the minimum radius is not to big!
		if (MinOffset != CPoint(0,0) && dMinRadius < (dMaxRadius / 2.0))
			return TRUE;
	}

	return TRUE;
}

// From the passed source Dib, given the approximate
// center of the red-eye, create the red-eye mask,
// which specifies the red-eye region.
double CRedEyeDlg::MakeRedEyeMask(	int nXRedEyeCenter,
									int nYRedEyeCenter,
									int nXMaskCenter,
									int nYMaskCenter,
									float fHueRangeStart,
									float fHueRangeEnd,
									int nRatio,
									CDib* pSrcDib,
									CDib* pMaskDib)
{
	int x, y;
	double dMinRadius;
	double dMaxRadius;
	double dAvgRadius;

	// Check
	if (!pMaskDib || !pMaskDib->IsValid() ||
		!pSrcDib || !pSrcDib->IsValid())
		return FALSE;

	// Clear Mask to RGBA(0,0,0,0)
	pMaskDib->SetBitColors(0);
	
	// Find all red-eye pixels
	for (y = -((int)pMaskDib->GetHeight() / 2) ; y <= (int)pMaskDib->GetHeight() / 2 ; y++)
	{
		for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= (int)pMaskDib->GetWidth() / 2 ; x++)
		{
			if (IsRedEyePixel(pSrcDib->GetPixelColor(nXRedEyeCenter + x, nYRedEyeCenter + y), fHueRangeStart, fHueRangeEnd, nRatio))			
				pMaskDib->SetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y, 0xFF000000);
		}
	}

	// Calc. Radiuses
	CalcRadiuses(	nXMaskCenter,
					nYMaskCenter,
					&dMinRadius,
					&dMaxRadius,
					&dAvgRadius,
					pMaskDib);

	// 1. Clean Outside of Max Radius.
	// 2. Check Inside Average Radius to detect not red-eye
	//    regions which should be red-eye.
	CleanAndCheck(	nXMaskCenter,
					nYMaskCenter,
					dMaxRadius,
					dAvgRadius,
					pMaskDib);

	// Clean Not Connected Regions.
	CleanNotConnectedRegions(	nXMaskCenter,
								nYMaskCenter,
								pMaskDib);

	// Recalc. Radiuses
	CalcRadiuses(	nXMaskCenter,
					nYMaskCenter,
					&dMinRadius,
					&dMaxRadius,
					&dAvgRadius,
					pMaskDib);

	// 1. Clean Outside of Max Radius.
	// 2. Check Inside Average Radius to detect not red-eye
	//    regions which should be red-eye.
	CleanAndCheck(	nXMaskCenter,
					nYMaskCenter,
					dMaxRadius,
					dAvgRadius,
					pMaskDib);

	// Mask is ok?
	BOOL bOk = (dAvgRadius > 1.0	&&									// More than 1 pixel
				dMaxRadius > 1.5	&&									// More than 1.5 pixels
				dMaxRadius < (double)(8 * pMaskDib->GetWidth() / 20));	// And less than 80% of max radius
	if (bOk)
		return dMaxRadius / dAvgRadius;
	else
		return -1.0; // Not Ok
}

// 1. Clean Outside of Max Radius.
// 2. Check Inside Average Radius to detect not red-eye 
//    regions which should be red-eye.
BOOL CRedEyeDlg::CleanAndCheck(	int nXMaskCenter,
								int nYMaskCenter,
								double dMaxRadius,
								double dAvgRadius,
								CDib* pMaskDib)
{
	// Check
	if (!pMaskDib || !pMaskDib->IsValid())
		return FALSE;

	int x, y;
	COLORREF mask;
	CDib OrigMaskDib(*pMaskDib);

	for (y = -((int)pMaskDib->GetHeight() / 2) ; y <= ((int)pMaskDib->GetHeight() / 2) ; y++)
	{
		for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= ((int)pMaskDib->GetWidth() / 2) ; x++)
		{
			mask = OrigMaskDib.GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			double dRadius = sqrt((double)(x*x + y*y));
			if ((dRadius > dMaxRadius) && mask)
				pMaskDib->SetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y, 0);
			else if ((dRadius <= dAvgRadius) && (mask == 0))
			{
				int nCount = GetNeighboursMaskCount(nXMaskCenter,
													nYMaskCenter,
													&OrigMaskDib,
													x,
													y);
				if (nCount >= REDEYE_NEIGHBOURS_THRESHOLD_CLEAN_AND_CHECK)
					pMaskDib->SetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y, 0xFF000000);
			}
		}
	}

	return TRUE;
}

// Clean Not Connected Regions.
BOOL CRedEyeDlg::CleanNotConnectedRegions(	int nXMaskCenter,
											int nYMaskCenter,
											CDib* pMaskDib)
{
	int x, y;

	// Check
	if (!pMaskDib || !pMaskDib->IsValid())
		return FALSE;

	// Are the mask center plus some neighbours redeye?
	BOOL bMaskCenterIsRedeye = FALSE;
	if (pMaskDib->GetPixelColor32Alpha(nXMaskCenter, nYMaskCenter))
	{
		int nRedEyeCount = GetNeighboursMaskCount(	nXMaskCenter,
													nYMaskCenter,
													pMaskDib,
													0,
													0);
		if (nRedEyeCount >= REDEYE_NEIGHBOURS_THRESHOLD)
			bMaskCenterIsRedeye = TRUE;
	}

	// Flood Fill with RGBA(0,0,0,254) values where they are RGBA(0,0,0,255).
	// The mask center is exactly in the middle -> top-down or bottom-up
	// coordinates are the same for the Center, but the final coordinate has to be
	// converted from bottom-up to top-down because FloodFillColor takes top-down
	// coordinates.
	if (bMaskCenterIsRedeye)
	{
		x = nXMaskCenter;
		y = nYMaskCenter;
		y = (int)pMaskDib->GetHeight() - 1 - y;
		pMaskDib->FloodFillColor(	x,
									y,
									0xFE000000,	// = RGBA(0,0,0,254)
									0xFF000000);// = RGBA(0,0,0,255)
	}
	else
	{
		// Get closest redeye point (we may have clicked inside
		// the bright-white reflection and not inside a redeye point)
		CPoint Offset;
		if (GetRedEyeOffsets(	nXMaskCenter,
								nYMaskCenter,
								Offset,
								pMaskDib))
		{
			x = nXMaskCenter;
			y = nYMaskCenter;
			x += Offset.x;
			y += Offset.y;
			y = (int)pMaskDib->GetHeight() - 1 - y;
			pMaskDib->FloodFillColor(	x,
										y,
										0xFE000000,	// = RGBA(0,0,0,254)
										0xFF000000);// = RGBA(0,0,0,255)
		}
	}

	// Restore the 255 values and clean-up the rest
	LPBYTE lpBits = pMaskDib->GetBits();
	for (int i = 0 ; i < (int)pMaskDib->GetImageSize() ; i += 4)
	{
		if (lpBits[i + 3] == 0xFEU)
			lpBits[i + 3] = 0xFFU;
		else
			lpBits[i + 3] = 0U;
	}

	return TRUE;
}

// Mask Inner:					255
// Mask Before Border:			190
// Mask On Border:				130
// Mask Just Outside Border:	70
BOOL CRedEyeDlg::SmoothMask(	int nXMaskCenter,
								int nYMaskCenter,
								CDib* pMaskDib)
{
	int x, y;
	COLORREF mask, maskleft, maskright, masktop, maskbottom;

	// Check
	if (!pMaskDib || !pMaskDib->IsValid())
		return FALSE;

	// Bottom to Top (Remember that the dib is bottom-up)
	for (y = -((int)pMaskDib->GetHeight() / 2) ; y <= ((int)pMaskDib->GetHeight() / 2) ; y++)
	{
		for (x = -((int)pMaskDib->GetWidth() / 2 - 1) ; x <= ((int)pMaskDib->GetWidth() / 2 - 1) ; x++) // Left to Right (Remember that the dib is bottom-up)
		{
			maskleft = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x - 1, nYMaskCenter + y);
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			maskright = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x + 1, nYMaskCenter + y);
			
			// Left Border
			if ((GetAValue(mask) >= 130) &&
				(GetAValue(maskleft) <= 70) &&
				(GetAValue(maskright) >= 190))
			{
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y,
												RGBA(	GetRValue(mask),
														GetGValue(mask),
														GetBValue(mask),
														130));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x - 1,
												nYMaskCenter + y,
												RGBA(	GetRValue(maskleft),
														GetGValue(maskleft),
														GetBValue(maskleft),
														70));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x + 1,
												nYMaskCenter + y,
												RGBA(	GetRValue(maskright),
														GetGValue(maskright),
														GetBValue(maskright),
														190));
			}

			// Right Border
			if ((GetAValue(mask) >= 130) &&
				(GetAValue(maskleft) >= 190) &&
				(GetAValue(maskright) <= 70))
			{
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y,
												RGBA(	GetRValue(mask),
														GetGValue(mask),
														GetBValue(mask),
														130));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x - 1,
												nYMaskCenter + y,
												RGBA(	GetRValue(maskleft),
														GetGValue(maskleft),
														GetBValue(maskleft),
														190));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x + 1,
												nYMaskCenter + y,
												RGBA(	GetRValue(maskright),
														GetGValue(maskright),
														GetBValue(maskright),
														70));
			}
		}
	}

	// Left to Right (Remember that the dib is bottom-up)
	for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= ((int)pMaskDib->GetWidth() / 2) ; x++)
	{
		for (y = -((int)pMaskDib->GetHeight() / 2 - 1) ; y <= ((int)pMaskDib->GetHeight() / 2 - 1) ; y++) // Bottom to Top (Remember that the dib is bottom-up)
		{
			maskbottom = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y - 1);
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			masktop = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y + 1);
			
			// Bottom Border
			if ((GetAValue(mask) >= 130) &&
				(GetAValue(maskbottom) <= 70) &&
				(GetAValue(masktop) >= 190))
			{
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y,
												RGBA(	GetRValue(mask),
														GetGValue(mask),
														GetBValue(mask),
														130));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y - 1,
												RGBA(	GetRValue(maskbottom),
														GetGValue(maskbottom),
														GetBValue(maskbottom),
														70));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y + 1,
												RGBA(	GetRValue(masktop),
														GetGValue(masktop),
														GetBValue(masktop),
														190));
			}

			// Top Border
			if ((GetAValue(mask) >= 130) &&
				(GetAValue(maskbottom) >= 190) &&
				(GetAValue(masktop) <= 70))
			{
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y,
												RGBA(	GetRValue(mask),
														GetGValue(mask),
														GetBValue(mask),
														130));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y - 1,
												RGBA(	GetRValue(maskbottom),
														GetGValue(maskbottom),
														GetBValue(maskbottom),
														190));
				pMaskDib->SetPixelColor32Alpha(	nXMaskCenter + x,
												nYMaskCenter + y + 1,
												RGBA(	GetRValue(masktop),
														GetGValue(masktop),
														GetBValue(masktop),
														70));
			}
		}
	}

	// Smooth Mask a bit more than only the 255, 190, 130, 70 values
	int Smooth[] = {1,1,1,
					1,1,1,
					1,1,1};
	return pMaskDib->FilterFastAlpha(Smooth, 9);
}

BOOL CRedEyeDlg::DrawMask(	int nXRedEyeCenter,
							int nYRedEyeCenter,
							int nXMaskCenter,
							int nYMaskCenter,
							CDib* pSrcDib,
							CDib* pMaskDib)
{
	int x, y;
	COLORREF mask;

	// Check
	if (!pMaskDib || !pMaskDib->IsValid() ||
		!pSrcDib || !pSrcDib->IsValid())
		return FALSE;

	for (y = -((int)pMaskDib->GetHeight() / 2) ; y <= ((int)pMaskDib->GetHeight() / 2) ; y++)
	{
		for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= ((int)pMaskDib->GetWidth() / 2) ; x++)
		{
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask)
				pSrcDib->SetPixelColor(nXRedEyeCenter + x, nYRedEyeCenter + y, RGB(0,255,0));
		}
	}

	return TRUE;
}

// From the passed source Dib and the given red-eye mask,
// update the destination Dib.
BOOL CRedEyeDlg::RemoveRedEye(	int nRedChannel,
								int nGreenChannel,
								int nBlueChannel,
								int nBrightness,
								int nXRedEyeCenter,
								int nYRedEyeCenter,
								int nXMaskCenter,
								int nYMaskCenter,
								CDib* pSrcDib,
								CDib* pMaskDib)
{
	int x, y;
	COLORREF mask;

	// Check
	if (!pMaskDib	||
		!pSrcDib	||
		!pSrcDib->IsValid())
		return FALSE;

	// Remove
	for (y = -((int)pMaskDib->GetHeight() / 2) ; y <= ((int)pMaskDib->GetHeight() / 2) ; y++)
	{
		for (x = -((int)pMaskDib->GetWidth() / 2) ; x <= ((int)pMaskDib->GetWidth() / 2) ; x++)
		{
			mask = pMaskDib->GetPixelColor32Alpha(nXMaskCenter + x, nYMaskCenter + y);
			if (mask)
			{
				CColor crOriginal(pSrcDib->GetPixelColor(nXRedEyeCenter + x, nYRedEyeCenter + y));
				CColor crNew = crOriginal;
				crNew.SetRed(MAX(0,MIN(255,(crOriginal.GetBlue() + crOriginal.GetGreen()) / 2 + nRedChannel)));
				crNew.SetGreen(MAX(0,MIN(255,(crOriginal.GetGreen() + nGreenChannel))));
				crNew.SetBlue(MAX(0,MIN(255,(crOriginal.GetBlue() + nBlueChannel))));

				// Adjust Brightness
				Brightness(crNew, nBrightness);

				// Blend
				int nAlpha =	GetAValue(mask);
				int nAlphaInv =	255 - nAlpha;
				int nRed = 		crNew.GetRed()			* nAlpha	/ 255 +
								crOriginal.GetRed()		* nAlphaInv	/ 255;
				int nGreen = 	crNew.GetGreen()		* nAlpha	/ 255 +
								crOriginal.GetGreen()	* nAlphaInv	/ 255;
				int nBlue =		crNew.GetBlue()			* nAlpha	/ 255 +
								crOriginal.GetBlue()	* nAlphaInv	/ 255;
				pSrcDib->SetPixelColor(nXRedEyeCenter + x, nYRedEyeCenter + y, RGB(nRed, nGreen, nBlue));
			}
		}
	}

	return TRUE;
}

// brightness: -100 .. +100
__forceinline void CRedEyeDlg::Brightness(CColor& c, short brightness)
{
	// Use the YIQ Color Space. Y is the Brightness.
	int R, G, B;
	R = (int)c.GetRed();
	G = (int)c.GetGreen();
	B = (int)c.GetBlue();
	int Y = 30 * R / 100 + 59 * G / 100 + 11 * B / 100;
	int I = 60 * R / 100 - 28 * G / 100 - 32 * B / 100;
	int Q = 21 * R / 100 - 52 * G / 100 + 31 * B / 100;
	float fNewBrightness = (float)brightness * 2.55f;
	Y = Y + (int)fNewBrightness;
	R = Y + 2401 * I / 2532 + 395 * Q / 633;
	G = Y - 233 * I / 844 - 135 * Q / 211;
	B = Y - 2799 * I / 2532 + 365 * Q / 211;
	if (R > 255) R = 255;
	else if (R < 0) R = 0;
	if (G > 255) G = 255;
	else if (G < 0) G = 0;
	if (B > 255) B = 255;
	else if (B < 0) B = 0;

	c.SetRed((BYTE)R);
	c.SetGreen((BYTE)G);
	c.SetBlue((BYTE)B);
}

void CRedEyeDlg::OnCheckShowmask() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	UpdateData(TRUE);
	if (m_bShowMask)
	{
		m_bShowOriginal = FALSE;
		UpdateData(FALSE);
	}
	AdjustRedEye(pSrcDib, m_bShowMask);
}

void CRedEyeDlg::OnCheckShoworiginal() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	UpdateData(TRUE);
	if (m_bShowOriginal)
	{
		m_bShowMask = FALSE;
		UpdateData(FALSE);

		UndoRedEyeRegion(pSrcDib);
		
		pView->Invalidate(FALSE);
	}
	else
		AdjustRedEye(pSrcDib, m_bShowMask);
}

void CRedEyeDlg::OnChangeEditHueStart() 
{
	if (m_bInitialized)
	{
		CPictureView* pView = (CPictureView*)m_pParentWnd;
		CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
		CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
						&(pDoc->m_AlphaRenderedDib) :
						pDoc->m_pDib;

		UpdateData(TRUE);
		if (!m_bShowOriginal)
			AdjustRedEye(pSrcDib, m_bShowMask);
	}
}

void CRedEyeDlg::OnChangeEditHueEnd() 
{
	if (m_bInitialized)
	{
		CPictureView* pView = (CPictureView*)m_pParentWnd;
		CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
		CDib* pSrcDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
						&(pDoc->m_AlphaRenderedDib) :
						pDoc->m_pDib;

		UpdateData(TRUE);
		if (!m_bShowOriginal)
			AdjustRedEye(pSrcDib, m_bShowMask);
	}
}
