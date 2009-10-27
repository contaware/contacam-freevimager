// AddBordersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "AddBordersDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddBordersDlg dialog


CAddBordersDlg::CAddBordersDlg(CWnd* pParent)
	: CDialog(CAddBordersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddBordersDlg)
	m_uiLeftBorder = 0;
	m_uiBottomBorder = 0;
	m_uiRightBorder = 0;
	m_uiTopBorder = 0;
	//}}AFX_DATA_INIT
	m_crBorder = 0;
	m_uiBorderIndex = 0;
	m_pWndPalette = NULL;
}


void CAddBordersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddBordersDlg)
	DDX_Control(pDX, IDC_PICK_COLOR, m_PickColor);
	DDX_Text(pDX, IDC_EDIT_LEFTBORDER, m_uiLeftBorder);
	DDX_Text(pDX, IDC_EDIT_BOTTOMBORDER, m_uiBottomBorder);
	DDX_Text(pDX, IDC_EDIT_RIGHTBORDER, m_uiRightBorder);
	DDX_Text(pDX, IDC_EDIT_TOPBORDER, m_uiTopBorder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddBordersDlg, CDialog)
	//{{AFX_MSG_MAP(CAddBordersDlg)
	ON_BN_CLICKED(IDC_PICK_COLOR, OnPickColor)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_COLOR_PICKED, OnColorPicked)
	ON_MESSAGE(WM_COLOR_PICKER_CLOSED, OnColorPickerClosed)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddBordersDlg message handlers

void CAddBordersDlg::OnPickColor() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	if (pDoc->m_pDib->GetBitCount() <= 8)
	{
		if (!m_pWndPalette)
		{
			m_pWndPalette = (CPaletteWnd*)new CPaletteWnd;
			if (!m_pWndPalette)
				return;
			CButton* pButton = (CButton*)GetDlgItem(IDC_PICK_COLOR);
			CRect rcButton;
			pButton->GetWindowRect(&rcButton);
			m_pWndPalette->Create(	CPoint(	rcButton.left,
											rcButton.top),
									this,
									pDoc->m_pDib->GetPalette());
			m_pWndPalette->SetCurrentColor(m_crBorder);
			CRect rcCurrent;
			m_pWndPalette->GetWindowRect(&rcCurrent);

			// Size
			int w = rcCurrent.Width();
			int h = rcCurrent.Height();

			// Move Coordinates
			rcCurrent.left = rcButton.right - w;
			rcCurrent.top = rcButton.bottom - h;
			rcCurrent.right = rcCurrent.left + w;
			rcCurrent.bottom = rcCurrent.top + h;

			// Clip
			::AfxGetMainFrame()->ClipToWorkRect(rcCurrent, pButton);
			m_pWndPalette->MoveWindow(	rcCurrent.left,
										rcCurrent.top,
										w,
										h);

			// Show
			m_pWndPalette->ShowWindow(SW_SHOW);
		}
	}
	else
	{
		if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(m_crBorder, this))
		{
			// Set Color
			m_PickColor.SetColor(	CDib::HighlightColor(m_crBorder),
									m_crBorder);

			// Make opaque for images with alpha
			if (pDoc->m_pDib->GetBitCount() == 32 && pDoc->m_pDib->HasAlpha())
			{
				m_crBorder = RGBA(	GetRValue(m_crBorder),
									GetGValue(m_crBorder),
									GetBValue(m_crBorder),
									255);
			}
		}
	}	
}

LRESULT CAddBordersDlg::OnColorPicked(WPARAM wParam, LPARAM lParam)
{
	m_uiBorderIndex = wParam;
	m_crBorder = lParam;
	m_PickColor.SetColor(	CDib::HighlightColor(m_crBorder),
							m_crBorder);
	if (m_pWndPalette)
		m_pWndPalette->Close();
	return 0;
}

LRESULT CAddBordersDlg::OnColorPickerClosed(WPARAM wParam, LPARAM lParam)
{
	m_pWndPalette = NULL;
	return 0;
}

BOOL CAddBordersDlg::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	CDialog::OnInitDialog();
	
	// For images with alpha init with transparent borders (-> show no color)
	if (pDoc->m_pDib->GetBitCount() != 32 || !pDoc->m_pDib->HasAlpha())
	{
		m_PickColor.SetColor(	CDib::HighlightColor(m_crBorder),
								m_crBorder);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddBordersDlg::OnOK() 
{
	if (m_pWndPalette)
		m_pWndPalette->Close();
	CDialog::OnOK();
}

void CAddBordersDlg::OnCancel() 
{
	if (m_pWndPalette)
		m_pWndPalette->Close();
	CDialog::OnCancel();
}
