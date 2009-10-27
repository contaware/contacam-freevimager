// MonochromeConversionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MonochromeConversionDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMonochromeConversionDlg dialog


CMonochromeConversionDlg::CMonochromeConversionDlg(CWnd* pParent)
	: CDialog(CMonochromeConversionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMonochromeConversionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CMonochromeConversionDlg::IDD, pParent);
}


void CMonochromeConversionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMonochromeConversionDlg)
	DDX_Control(pDX, IDC_COLOR_DARK, m_ColorDark);
	DDX_Control(pDX, IDC_COLOR_BRIGHT, m_ColorBright);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMonochromeConversionDlg, CDialog)
	//{{AFX_MSG_MAP(CMonochromeConversionDlg)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_COLOR_DARK, OnColorDark)
	ON_BN_CLICKED(IDC_COLOR_BRIGHT, OnColorBright)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMonochromeConversionDlg message handlers

BOOL CMonochromeConversionDlg::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	CDialog::OnInitDialog();
	
	// Init Slider
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BW_THRESHOLD);
	pSlider->SetRange(0, 256, TRUE);
	pSlider->SetLineSize(5);
	pSlider->SetPageSize(5);
	pSlider->SetPos(128);

	// Init Edit Ctrl
	CEdit* pEdit;
	pEdit = (CEdit*)GetDlgItem(IDC_BW_THRESHOLD);
	pEdit->SetWindowText(_T("128"));

	// Set Button's Color
	m_ColorDark.SetColor(	CDib::HighlightColor(RGB(0,0,0)),
							RGB(0,0,0));
	m_ColorBright.SetColor(	CDib::HighlightColor(RGB(255,255,255)),
							RGB(255,255,255));

	if (pDib)
	{
		// Init Preview Undo Dib
		m_PreviewUndoDib = *pDib;

		// Convert
		BeginWaitCursor();
		if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
		{
			pDib->RenderAlphaWithSrcBackground();
			pDib->SetAlpha(FALSE);
		}
		pDib->ConvertTo1bit(128, RGB(0,0,0), RGB(255,255,255), pView, TRUE);
		pDoc->SetDocumentTitle();
		pDoc->UpdateImageInfo();
		EndWaitCursor();
		pDoc->InvalidateAllViews(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMonochromeConversionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	// Better to use Slider Pos Directly than the nPos parameter,
	// this because with the Line and Page Message nPos is always 0...
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			CEdit* pEdit;
			pEdit = (CEdit*)GetDlgItem(IDC_BW_THRESHOLD);
			CString Str;
			Str.Format(_T("%u"), pSlider->GetPos());
			pEdit->SetWindowText(Str);
		}

		if ((SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pDib)
			{
				BeginWaitCursor();
				Undo();
				if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
				{
					pDib->RenderAlphaWithSrcBackground();
					pDib->SetAlpha(FALSE);
				}
				pDib->ConvertTo1bit(256 - pSlider->GetPos(), m_ColorDark.GetBkgColor(), m_ColorBright.GetBkgColor(), pView, TRUE);
				EndWaitCursor();
				pDoc->InvalidateAllViews(FALSE);
			}
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMonochromeConversionDlg::Close()
{
	OnClose();
}

void CMonochromeConversionDlg::OnClose() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	Undo();
	pDoc->UpdateAlphaRenderedDib();
	pDoc->SetDocumentTitle();
	pDoc->UpdateImageInfo();
	pDoc->InvalidateAllViews(FALSE);

	DestroyWindow();
}

BOOL CMonochromeConversionDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
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

void CMonochromeConversionDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	pDoc->m_pMonochromeConversionDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;
	CDialog::PostNcDestroy();
}

void CMonochromeConversionDlg::Undo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	if (pDib && m_PreviewUndoDib.IsValid())
		*pDib = m_PreviewUndoDib;
}

void CMonochromeConversionDlg::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	
	pDoc->AddUndo(&m_PreviewUndoDib);
	pDoc->SetModifiedFlag();
	pDoc->SetDocumentTitle();
	pDoc->UpdateImageInfo();
	DestroyWindow();
}

void CMonochromeConversionDlg::OnColorDark() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	COLORREF cr = m_ColorDark.GetBkgColor();
	if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(cr, this))
	{
		m_ColorDark.SetColor(CDib::HighlightColor(cr), cr);
		CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BW_THRESHOLD);
		if (pSlider && pDib)
		{
			BeginWaitCursor();
			Undo();
			if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
			{
				pDib->RenderAlphaWithSrcBackground();
				pDib->SetAlpha(FALSE);
			}
			pDib->ConvertTo1bit(256 - pSlider->GetPos(), m_ColorDark.GetBkgColor(), m_ColorBright.GetBkgColor(), pView, TRUE);
			EndWaitCursor();
			pDoc->InvalidateAllViews(FALSE);
		}
	}
}

void CMonochromeConversionDlg::OnColorBright() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	COLORREF cr = m_ColorBright.GetBkgColor();
	if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(cr, this))
	{
		m_ColorBright.SetColor(CDib::HighlightColor(cr), cr);
		CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_BW_THRESHOLD);
		if (pSlider && pDib)
		{
			BeginWaitCursor();
			Undo();
			if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
			{
				pDib->RenderAlphaWithSrcBackground();
				pDib->SetAlpha(FALSE);
			}
			pDib->ConvertTo1bit(256 - pSlider->GetPos(), m_ColorDark.GetBkgColor(), m_ColorBright.GetBkgColor(), pView, TRUE);
			EndWaitCursor();
			pDoc->InvalidateAllViews(FALSE);
		}
	}
}
