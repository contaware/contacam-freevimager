// SharpenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SharpenDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_SHARPNESS		0
#define MAX_SHARPNESS		10
#define DEFAULT_SHARPNESS	5

/////////////////////////////////////////////////////////////////////////////
// CSharpenDlg dialog

CSharpenDlg::CSharpenDlg(CWnd* pParent)
	: CDialog(CSharpenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSharpenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CSharpenDlg::IDD, pParent);
}

void CSharpenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSharpenDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSharpenDlg, CDialog)
	//{{AFX_MSG_MAP(CSharpenDlg)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSharpenDlg message handlers

BOOL CSharpenDlg::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	CDialog::OnInitDialog();
	
	// Init Slider
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SHARPNESS);
	pSlider->SetRange(MIN_SHARPNESS, MAX_SHARPNESS, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(1);
	int nSharpen = ::AfxGetApp()->GetProfileInt(_T("PictureDoc"), _T("Sharpen"), DEFAULT_SHARPNESS);
	nSharpen = MIN(MAX_SHARPNESS, MAX(MIN_SHARPNESS, nSharpen));
	pSlider->SetPos(nSharpen);

	// Create blurred dib and sharpen
	BeginWaitCursor();
	if (pDoc->m_pDib)
	{
		m_OrigDib = *pDoc->m_pDib;
#ifdef GAUSSIAN_BLUR_3x3
		int Kernel[] = {1, 2, 1,
						2, 4, 2,
						1, 2, 1};
		m_BlurredDib.FilterFast(Kernel,
								16,
								&m_OrigDib,
								pView,
								TRUE);
#elif defined(GAUSSIAN_BLUR_5x5)
		int Kernel[] = {1, 1, 2, 1, 1, 
						1, 2, 4, 2, 1, 
						2, 4, 8, 4, 2,
						1, 2, 4, 2, 1, 
						1, 1, 2, 1, 1};
		m_BlurredDib.Filter(Kernel,
							5,
							52,
							0,
							&m_OrigDib,
							pView,
							TRUE);
#else // Gaussian blur 7X7
		int Kernel[] = {1, 1, 2, 2, 2, 1, 1,
						1, 2, 2, 4, 2, 2, 1,
						2, 2, 4, 8, 4, 2, 2,
						2, 4, 8, 16,8, 4, 2,
						2, 2, 4, 8, 4, 2, 2,
						1, 2, 2, 4, 2, 2, 1,
						1, 1, 2, 2, 2, 1, 1};
		m_BlurredDib.Filter(Kernel,
							7,
							140,
							0,
							&m_OrigDib,
							pView,
							TRUE);
#endif
		Sharpen(nSharpen);
	}
	pDoc->UpdateAlphaRenderedDib();
	EndWaitCursor();
	pDoc->InvalidateAllViews(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSharpenDlg::UnsharpMask(int nAmount, CDib* pOrigDib, CDib* pDib)
{	
	// Check
	if (!pOrigDib || !pOrigDib->IsValid() || !pDib || !pDib->IsValid())
		return FALSE;

	// Sharpen
	int nDiff;
	int nOrigRed, nOrigGreen, nOrigBlue, nOrigAlpha, r, g, b, a;
	COLORREF crOrigColor, crColor;
	if (pOrigDib->GetBitCount() <= 8)
	{
		pDib->InitGetClosestColorIndex();

		for (unsigned int y = 0 ; y < pOrigDib->GetHeight() ; y++)
		{
			for (unsigned int x = 0 ; x < pOrigDib->GetWidth() ; x++)
			{
				// Get pixels
				crOrigColor = pOrigDib->GetPixelColor(x, y);
				crColor = pDib->GetPixelColor(x, y);

				// Red
				nOrigRed = GetRValue(crOrigColor);
				nDiff = nOrigRed - GetRValue(crColor);
				r = nOrigRed + nAmount * nDiff / 10;
				r = MIN(255, MAX(0, r));

				// Green
				nOrigGreen = GetGValue(crOrigColor);
				nDiff = nOrigGreen - GetGValue(crColor);
				g = nOrigGreen + nAmount * nDiff / 10;
				g = MIN(255, MAX(0, g));

				// Blue
				nOrigBlue = GetBValue(crOrigColor);
				nDiff = nOrigBlue - GetBValue(crColor);
				b = nOrigBlue + nAmount * nDiff / 10;
				b = MIN(255, MAX(0, b));

				// Set pixel
				pDib->SetPixelIndex(x, y, pDib->GetClosestColorIndex(RGB(r, g, b)));
			}
		}
	}
	else if (pOrigDib->HasAlpha() && pOrigDib->GetBitCount() == 32)
	{
		for (unsigned int y = 0 ; y < pOrigDib->GetHeight() ; y++)
		{
			for (unsigned int x = 0 ; x < pOrigDib->GetWidth() ; x++)
			{
				// Get pixels
				crOrigColor = pOrigDib->GetPixelColor32Alpha(x, y);
				crColor = pDib->GetPixelColor32Alpha(x, y);

				// Red
				nOrigRed = GetRValue(crOrigColor);
				nDiff = nOrigRed - GetRValue(crColor);
				r = nOrigRed + nAmount * nDiff / 10;
				r = MIN(255, MAX(0, r));

				// Green
				nOrigGreen = GetGValue(crOrigColor);
				nDiff = nOrigGreen - GetGValue(crColor);
				g = nOrigGreen + nAmount * nDiff / 10;
				g = MIN(255, MAX(0, g));

				// Blue
				nOrigBlue = GetBValue(crOrigColor);
				nDiff = nOrigBlue - GetBValue(crColor);
				b = nOrigBlue + nAmount * nDiff / 10;
				b = MIN(255, MAX(0, b));

				// Alpha
				nOrigAlpha = GetAValue(crOrigColor);
				nDiff = nOrigAlpha - GetAValue(crColor);
				a = nOrigAlpha + nAmount * nDiff / 10;
				a = MIN(255, MAX(0, a));

				// Set pixel
				pDib->SetPixelColor32Alpha(x, y, RGBA(r, g, b, a));
			}
		}
	}
	else
	{
		for (unsigned int y = 0 ; y < pOrigDib->GetHeight() ; y++)
		{
			for (unsigned int x = 0 ; x < pOrigDib->GetWidth() ; x++)
			{
				// Get pixels
				crOrigColor = pOrigDib->GetPixelColor(x, y);
				crColor = pDib->GetPixelColor(x, y);

				// Red
				nOrigRed = GetRValue(crOrigColor);
				nDiff = nOrigRed - GetRValue(crColor);
				r = nOrigRed + nAmount * nDiff / 10;
				r = MIN(255, MAX(0, r));

				// Green
				nOrigGreen = GetGValue(crOrigColor);
				nDiff = nOrigGreen - GetGValue(crColor);
				g = nOrigGreen + nAmount * nDiff / 10;
				g = MIN(255, MAX(0, g));

				// Blue
				nOrigBlue = GetBValue(crOrigColor);
				nDiff = nOrigBlue - GetBValue(crColor);
				b = nOrigBlue + nAmount * nDiff / 10;
				b = MIN(255, MAX(0, b));
					
				// Set pixel
				pDib->SetPixelColor(x, y, RGB(r, g, b));
			}
		}
	}

	return TRUE;
}

void CSharpenDlg::Sharpen(int nAmount)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	
	if (pDoc->m_pDib)
	{
		if (nAmount > 0)
		{
			*pDoc->m_pDib = m_BlurredDib;
			UnsharpMask(nAmount, &m_OrigDib, pDoc->m_pDib);
		}
		else
			*pDoc->m_pDib = m_OrigDib;
	}
}

void CSharpenDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Better to use Slider Pos Directly than the nPos parameter,
	// this because with the Line and Page Message nPos is always 0...
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
		if ((SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			BeginWaitCursor();
			int nSharpen = pSlider->GetPos();
			Sharpen(nSharpen);
			::AfxGetApp()->WriteProfileInt(_T("PictureDoc"), _T("Sharpen"), nSharpen);
			pDoc->UpdateAlphaRenderedDib();
			EndWaitCursor();
			pDoc->InvalidateAllViews(FALSE);
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSharpenDlg::Close()
{
	OnClose();
}

void CSharpenDlg::OnClose() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	Sharpen(0);
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);

	DestroyWindow();
}

BOOL CSharpenDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				DoIt();
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

void CSharpenDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	pDoc->m_pSharpenDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}

void CSharpenDlg::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SHARPNESS);

	if (!pSlider || pSlider->GetPos() <= 0)
		Close();
	else
	{
		pDoc->AddUndo(&m_OrigDib);
		pDoc->SetModifiedFlag();
		pDoc->SetDocumentTitle();
		pDoc->UpdateImageInfo();
		DestroyWindow();
	}
}