// SoftBordersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SoftBordersDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_SOFTBORDER		0
#define MAX_SOFTBORDER		255
#define DEFAULT_SOFTBORDER	20

/////////////////////////////////////////////////////////////////////////////
// CSoftBordersDlg dialog


CSoftBordersDlg::CSoftBordersDlg(CWnd* pParent)
	: CDialog(CSoftBordersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoftBordersDlg)
	m_bBlur = FALSE;
	//}}AFX_DATA_INIT
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		m_bBlur = ::AfxGetApp()->GetProfileInt(_T("PictureDoc"), _T("SoftBorderBlur"), FALSE);
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CSoftBordersDlg::IDD, pParent);
}


void CSoftBordersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoftBordersDlg)
	DDX_Check(pDX, IDC_CHECK_BLUR, m_bBlur);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoftBordersDlg, CDialog)
	//{{AFX_MSG_MAP(CSoftBordersDlg)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_BLUR, OnCheckBlur)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoftBordersDlg message handlers

BOOL CSoftBordersDlg::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	CDialog::OnInitDialog();
	
	// Init Slider
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SOFT_BORDERS);
	pSlider->SetRange(MIN_SOFTBORDER, MAX_SOFTBORDER, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(1);
	int nSoftborder = DEFAULT_SOFTBORDER;
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		nSoftborder = ::AfxGetApp()->GetProfileInt(_T("PictureDoc"), _T("SoftBorder"), DEFAULT_SOFTBORDER);
	pSlider->SetPos(nSoftborder);

	// Init Edit Ctrl
	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_TEXT_SOFT_BORDERS);
	CString Str;
	Str.Format(_T("%d"), nSoftborder);
	pEdit->SetWindowText(Str);

	// Init Preview Undo Dib
	if (pDoc->m_pDib)
	{
		m_PreviewUndoDib = *pDoc->m_pDib;

		// Soft Borders
		BeginWaitCursor();
		pDoc->m_pDib->SoftBorders(	nSoftborder,
									m_bBlur,
									pView, TRUE);
		pDoc->UpdateAlphaRenderedDib();
		EndWaitCursor();
		pDoc->InvalidateAllViews(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSoftBordersDlg::Close()
{
	OnClose();
}

void CSoftBordersDlg::OnClose() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	Undo();
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);

	DestroyWindow();
}

BOOL CSoftBordersDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
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

void CSoftBordersDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	pDoc->m_pSoftBordersDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}

void CSoftBordersDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	// Better to use Slider Pos Directly than the nPos parameter,
	// this because with the Line and Page Message nPos is always 0...
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
		int nSoftborder;
		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			CEdit* pEdit;
			pEdit = (CEdit*)GetDlgItem(IDC_TEXT_SOFT_BORDERS);
			CString Str;
			nSoftborder = pSlider->GetPos();
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
				::AfxGetApp()->WriteProfileInt(_T("PictureDoc"), _T("SoftBorder"), nSoftborder);
			Str.Format(_T("%d"), nSoftborder);
			pEdit->SetWindowText(Str);
		}

		if ((SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pDoc->m_pDib)
			{
				BeginWaitCursor();
				Undo();
				pDoc->m_pDib->SoftBorders(	nSoftborder,
											m_bBlur,
											pView,
											TRUE);
				pDoc->UpdateAlphaRenderedDib();
				EndWaitCursor();
				pDoc->InvalidateAllViews(FALSE);
			}
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSoftBordersDlg::Undo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	if (pDoc->m_pDib && m_PreviewUndoDib.IsValid())
		*pDoc->m_pDib = m_PreviewUndoDib;
}

void CSoftBordersDlg::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SOFT_BORDERS);

	if (!pSlider || pSlider->GetPos() <= 0)
		Close();
	else
	{
		if (pSlider->GetPos() == 0)
		{
			Undo();
			pDoc->UpdateAlphaRenderedDib();
			pDoc->InvalidateAllViews(FALSE);
		}
		else
		{
			pDoc->AddUndo(&m_PreviewUndoDib);
			pDoc->SetModifiedFlag();
			pDoc->SetDocumentTitle();
			pDoc->UpdateImageInfo();
		}
		
		DestroyWindow();
	}
}

void CSoftBordersDlg::OnCheckBlur() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SOFT_BORDERS);
	if (!pSlider)
		return;

	UpdateData(TRUE);
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		::AfxGetApp()->WriteProfileInt(_T("PictureDoc"), _T("SoftBorderBlur"), m_bBlur);

	if (pSlider->GetPos() <= 0)
		return;

	if (pDoc->m_pDib)
	{
		BeginWaitCursor();
		Undo();
		pDoc->m_pDib->SoftBorders(	pSlider->GetPos(),
									m_bBlur,
									pView,
									TRUE);
		pDoc->UpdateAlphaRenderedDib();
		EndWaitCursor();

		pDoc->InvalidateAllViews(FALSE);
	}
}
