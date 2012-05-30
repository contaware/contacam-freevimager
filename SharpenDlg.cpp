// SharpenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SharpenDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_SHARPNESS		0
#define MAX_SHARPNESS		10
#define DEFAULT_SHARPNESS	4

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
	CDib* pDib = pDoc->m_pDib;

	CDialog::OnInitDialog();
	
	// Init Slider
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SHARPNESS);
	pSlider->SetRange(MIN_SHARPNESS, MAX_SHARPNESS, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(1);
	int nSharpen = DEFAULT_SHARPNESS;
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		nSharpen = ::AfxGetApp()->GetProfileInt(_T("PictureDoc"), _T("Sharpen"), DEFAULT_SHARPNESS);
	pSlider->SetPos(nSharpen);

	// Init Preview Undo Dib
	if (pDib)
		m_PreviewUndoDib = *pDib;

	// Sharpen
	BeginWaitCursor();
	Sharpen(pDib, NULL, nSharpen);
	pDoc->UpdateAlphaRenderedDib();
	EndWaitCursor();
	pDoc->InvalidateAllViews(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSharpenDlg::Sharpen(CDib* pDib, CDib* pSrcDib, int nSharpness)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	switch (nSharpness)
	{
		case 1 : nSharpness = 52; break;
		case 2 : nSharpness = 36; break;
		case 3 : nSharpness = 27; break;
		case 4 : nSharpness = 21; break;
		case 5 : nSharpness = 17; break;
		case 6 : nSharpness = 14; break;
		case 7 : nSharpness = 12; break;
		case 8 : nSharpness = 11; break;
		case 9 : nSharpness = 10; break;
		case 10: nSharpness = 9;  break;
		default: return FALSE;
	}

	int Kernel[] = {-1,-1,-1,
					-1,nSharpness,-1,
					-1,-1,-1};

	return pDib->FilterFast(Kernel,
							nSharpness - 8,
							pSrcDib,
							pView,
							TRUE);
}

void CSharpenDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib = pDoc->m_pDib;

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
			if (pDib)
			{
				BeginWaitCursor();
				Undo();
				int nSharpen = pSlider->GetPos();
				Sharpen(pDib, NULL, nSharpen);
				if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
					::AfxGetApp()->WriteProfileInt(_T("PictureDoc"), _T("Sharpen"), nSharpen);
				pDoc->UpdateAlphaRenderedDib();
				EndWaitCursor();
				pDoc->InvalidateAllViews(FALSE);
			}
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

	Undo();
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

void CSharpenDlg::Undo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib = pDoc->m_pDib;

	if (pDib && m_PreviewUndoDib.IsValid())
		*pDib = m_PreviewUndoDib;
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
		pDoc->AddUndo(&m_PreviewUndoDib);
		pDoc->SetModifiedFlag();
		pDoc->SetDocumentTitle();
		pDoc->UpdateImageInfo();
		DestroyWindow();
	}
}