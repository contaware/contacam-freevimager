// RotationFlippingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "RotationFlippingDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "PaletteWnd.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotationFlippingDlg dialog


CRotationFlippingDlg::CRotationFlippingDlg(CWnd* pParent)
	: CDialog(CRotationFlippingDlg::IDD, pParent)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);
	m_bDlgInitialized = FALSE;

	//{{AFX_DATA_INIT(CRotationFlippingDlg)
	m_TransformationType = 0;
	m_uiAngle = 0;
	m_uiAngleMinutes = 0;
	m_bAntiAliasing = pDoc->m_pDib->GetBitCount() >= 8 ? TRUE : FALSE;
	//}}AFX_DATA_INIT

	m_PreviewTransformationType = -1;
	m_uiPreviewAngle = 0;
	m_uiPreviewAngleMinutes = 0;
	m_bPreviewAntiAliasing = FALSE;

	m_crBackgroundColor = RGB(0xFF,0xFF,0xFF);
	m_pWndPalette = NULL;
	pView->ForceCursor();
	CDialog::Create(CRotationFlippingDlg::IDD, pParent);
}


void CRotationFlippingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRotationFlippingDlg)
	DDX_Control(pDX, IDC_SPIN_ANGLE_MINUTES, m_SpinAngleMinutes);
	DDX_Control(pDX, IDC_PICK_COLOR_FROM_IMAGE, m_PickColorFromImage);
	DDX_Control(pDX, IDC_PICK_COLOR_FROM_DIALOG, m_PickColorFromDlg);
	DDX_Control(pDX, IDC_SPIN_ANGLE, m_SpinAngle);
	DDX_Radio(pDX, IDC_RADIO_TRANSFORMATION0, m_TransformationType);
	DDX_Text(pDX, IDC_EDIT_ANGLE, m_uiAngle);
	DDV_MinMaxUInt(pDX, m_uiAngle, 0, 359);
	DDX_Text(pDX, IDC_EDIT_ANGLE_MINUTES, m_uiAngleMinutes);
	DDV_MinMaxUInt(pDX, m_uiAngleMinutes, 0, 59);
	DDX_Check(pDX, IDC_CHECK_ANTIALIASING, m_bAntiAliasing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRotationFlippingDlg, CDialog)
	//{{AFX_MSG_MAP(CRotationFlippingDlg)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	ON_BN_CLICKED(IDC_PICK_COLOR_FROM_DIALOG, OnPickColorFromDlg)
	ON_BN_CLICKED(IDC_UNDO, OnUndo)
	ON_BN_CLICKED(IDC_PICK_COLOR_FROM_IMAGE, OnPickColorFromImage)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_LOSSLESS, OnCheckLossless)
	ON_EN_CHANGE(IDC_EDIT_ANGLE, OnChangeEditAngle)
	ON_EN_CHANGE(IDC_EDIT_ANGLE_MINUTES, OnChangeEditAngleMinutes)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_COLOR_PICKED, OnColorPicked)
	ON_MESSAGE(WM_COLOR_PICKER_CLOSED, OnColorPickerClosed)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRotationFlippingDlg message handlers

BOOL CRotationFlippingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	m_SpinAngle.SetRange(0, 359);
	m_SpinAngleMinutes.SetRange(0, 59);
	CButton* pButton = (CButton*)GetDlgItem(IDC_UNDO);
	pButton->EnableWindow(FALSE);
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_LOSSLESS);
	if (pDoc->IsJPEG() && !pDoc->IsModified() && !pDoc->m_bPrintPreviewMode)
	{
		if (pDoc->m_bForceLossyTrafo)
			pCheck->SetCheck(0);
		else
			pCheck->SetCheck(1);
	}
	else
		pCheck->ShowWindow(SW_HIDE);

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CRotationFlippingDlg::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	UpdateData(TRUE);

	pDoc->BeginWaitCursor();

	// Try Lossless Transformation
#ifdef SUPPORT_LIBJPEG
	switch (pDoc->LossLessRotateFlip(TRUE, this))
	{
		case 0 :
			if (m_PreviewUndoDib.IsValid())
			{
				*pDoc->m_pDib = m_PreviewUndoDib; // Restore Original Dib
				pDoc->UpdateAlphaRenderedDib();
			}
			pDoc->EndWaitCursor();
			::AfxMessageBox(ML_STRING(1404, "Please Save Before Doing A Lossless Jpeg Transformation."));
			return FALSE;
		case 1 : 
			pDoc->LoadPicture(&pDoc->m_pDib, pDoc->m_sFileName);  
			pDoc->EndWaitCursor();
			return TRUE; // Ok Done
		case 2 :
			break; // Do Lossy Transformation
		default :
			if (m_PreviewUndoDib.IsValid())
			{
				*pDoc->m_pDib = m_PreviewUndoDib; // Restore Original Dib
				pDoc->UpdateAlphaRenderedDib();
			}
			pDoc->EndWaitCursor();
			return FALSE;
	}
#endif

	// Do Transformation if the angle is not 0°
	if (m_TransformationType != 5 || m_uiAngle != 0 || m_uiAngleMinutes != 0)
	{
		// Add Undo
		pDoc->AddUndo(m_PreviewUndoDib.IsValid() ? &m_PreviewUndoDib : pDoc->m_pDib);

		// Do Transformation if not already Done by Preview,
		// or do it also if Preview Done, but settings have been changed after Preview.
		// If Preview Done, Transformation is the same and it is number 5 (free rotation)
		// check whether the angle has been changed!
		if (!m_PreviewUndoDib.IsValid() ||
			m_TransformationType != m_PreviewTransformationType ||
			(m_TransformationType == 5 &&
			(m_uiAngle != m_uiPreviewAngle ||
			m_uiAngleMinutes != m_uiPreviewAngleMinutes ||
			m_bAntiAliasing != m_bPreviewAntiAliasing)))
		{
			if (m_PreviewUndoDib.IsValid())
				*pDoc->m_pDib = m_PreviewUndoDib; // Restore Original Dib

			switch (m_TransformationType)
			{
				case 0 : pDoc->m_pDib->Rotate90CW(); break;
				case 1 : pDoc->m_pDib->Rotate90CCW(); break;
				case 2 : pDoc->m_pDib->Rotate180(); break;
				case 3 : pDoc->m_pDib->FlipLeftRight(); break;
				case 4 : pDoc->m_pDib->FlipTopDown(); break;
				case 5 : 
				{
					if (m_uiAngle == 90 && m_uiAngleMinutes == 0)
						pDoc->m_pDib->Rotate90CW();
					else if (m_uiAngle == 180 && m_uiAngleMinutes == 0)
						pDoc->m_pDib->Rotate180();
					else if (m_uiAngle == 270 && m_uiAngleMinutes == 0)
						pDoc->m_pDib->Rotate90CCW();
					else if (m_uiAngle != 0 || m_uiAngleMinutes != 0)
						pDoc->m_pDib->RotateCW(((double)m_uiAngle + (double)m_uiAngleMinutes / 60.0) * PI / 180.0,
												GetRValue(m_crBackgroundColor),
												GetGValue(m_crBackgroundColor),
												GetBValue(m_crBackgroundColor),
												GetAValue(m_crBackgroundColor),
												m_bAntiAliasing,
												NULL,
												FALSE,
												pView,
												TRUE);
					break;
				}
				default: break;
			}
		}
	}
	else
	{
		if (m_PreviewUndoDib.IsValid())
			*pDoc->m_pDib = m_PreviewUndoDib; // Restore Original Dib
	}

	pDoc->m_DocRect.bottom = pDoc->m_pDib->GetHeight();
	pDoc->m_DocRect.right = pDoc->m_pDib->GetWidth();

	pDoc->EndWaitCursor();

	pDoc->UpdateAlphaRenderedDib();
	if (m_TransformationType != 5 || m_uiAngle != 0 || m_uiAngleMinutes != 0)
		pDoc->SetModifiedFlag();
	pDoc->SetDocumentTitle();
	pDoc->UpdateAllViews(NULL);
	pDoc->UpdateImageInfo();

	return TRUE;
}

void CRotationFlippingDlg::OnPreview()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib = pDoc->m_pDib;
	if (!pDib)
		return;
	
	UpdateData(TRUE);

	pDoc->BeginWaitCursor();

	if (m_PreviewUndoDib.IsValid())
		*pDib = m_PreviewUndoDib; // Second, Third,... Time Preview
	else
		m_PreviewUndoDib = *pDib; // First Time Preview or just after undo

	switch (m_TransformationType)
	{
		case 0 : pDib->Rotate90CW(); break;
		case 1 : pDib->Rotate90CCW(); break;
		case 2 : pDib->Rotate180(); break;
		case 3 : pDib->FlipLeftRight(); break;
		case 4 : pDib->FlipTopDown(); break;
		case 5 : 
		{
			if (m_uiAngle == 90 && m_uiAngleMinutes == 0)
				pDib->Rotate90CW();
			else if (m_uiAngle == 180 && m_uiAngleMinutes == 0)
				pDib->Rotate180();
			else if (m_uiAngle == 270 && m_uiAngleMinutes == 0)
				pDib->Rotate90CCW();
			else if (m_uiAngle != 0 || m_uiAngleMinutes != 0)
				pDib->RotateCW(((double)m_uiAngle + (double)m_uiAngleMinutes / 60.0) * PI / 180.0,
								GetRValue(m_crBackgroundColor),
								GetGValue(m_crBackgroundColor),
								GetBValue(m_crBackgroundColor),
								GetAValue(m_crBackgroundColor),
								m_bAntiAliasing,
								NULL,
								FALSE,
								pView,
								TRUE);

			break;
		}
		default: break;
	}

	// Store Preview Data
	m_PreviewTransformationType = m_TransformationType; 
	m_uiPreviewAngle = m_uiAngle;
	m_uiPreviewAngleMinutes = m_uiAngleMinutes;
	m_bPreviewAntiAliasing = m_bAntiAliasing;
	pDoc->m_DocRect.bottom = pDib->GetHeight();
	pDoc->m_DocRect.right = pDib->GetWidth();
	pDoc->EndWaitCursor();
	if (m_TransformationType != 5 || m_uiAngle != 0 || m_uiAngleMinutes != 0)
	{
		CButton* pButton = (CButton*)GetDlgItem(IDC_UNDO);
		pButton->EnableWindow(TRUE);
	}
	else
	{
		m_PreviewUndoDib.Free();
		CButton* pButton = (CButton*)GetDlgItem(IDC_UNDO);
		pButton->EnableWindow(FALSE);
	}
	pDoc->UpdateAlphaRenderedDib();
	pDoc->UpdateAllViews(NULL);
	pDoc->UpdateImageInfo();
}

void CRotationFlippingDlg::OnPickColorFromDlg() 
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
			CButton* pButton = (CButton*)GetDlgItem(IDC_PICK_COLOR_FROM_DIALOG);
			CRect rcButton;
			pButton->GetWindowRect(&rcButton);
			m_pWndPalette->Create(	CPoint(	rcButton.left,
											rcButton.top),
									this,
									pDoc->m_pDib->GetPalette());
			m_pWndPalette->SetCurrentColor(m_crBackgroundColor);
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
		if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(m_crBackgroundColor, this))
		{
			m_PickColorFromImage.SetColorToWindowsDefault();
			m_PickColorFromDlg.SetColor(CDib::HighlightColor(m_crBackgroundColor),
										m_crBackgroundColor);
		}
	}
}

void CRotationFlippingDlg::OnPickColorFromImage() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	
	// Cancel Zoom
	pDoc->CancelZoomTool();

	pDoc->m_bDoRotationColorPickup = TRUE;
	::AfxGetMainFrame()->StatusText(ML_STRING(1229, "*** Click Inside The Image To Get The Background Color ***"));
	pView->UpdateCursor();
}

void CRotationFlippingDlg::OnUndo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib = pDoc->m_pDib;
	if (!pDib)
		return;

	if (m_PreviewUndoDib.IsValid())
	{
		*pDib = m_PreviewUndoDib;
		m_PreviewUndoDib.Free();

		// Store Reset Data
		m_PreviewTransformationType = -1; 
		m_uiPreviewAngle = 0;
		m_uiPreviewAngleMinutes = 0;
		m_bPreviewAntiAliasing = FALSE;

		pDoc->m_DocRect.bottom = pDoc->m_pDib->GetHeight();
		pDoc->m_DocRect.right = pDoc->m_pDib->GetWidth();
		pDoc->UpdateAlphaRenderedDib();
		pDoc->UpdateAllViews(NULL);
		pDoc->UpdateImageInfo();
		CButton* pButton = (CButton*)GetDlgItem(IDC_UNDO);
		pButton->EnableWindow(FALSE);
	}
}

void CRotationFlippingDlg::OnClose() 
{
	OnUndo();
	DestroyWindow();
}

void CRotationFlippingDlg::Close()
{
	OnClose();
}

BOOL CRotationFlippingDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
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

void CRotationFlippingDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	pDoc->m_bDoRotationColorPickup = FALSE;
	::AfxGetMainFrame()->StatusText();
	pDoc->m_pRotationFlippingDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;
	CDialog::PostNcDestroy();
}

LRESULT CRotationFlippingDlg::OnColorPicked(WPARAM wParam, LPARAM lParam)
{
	wParam;
	m_crBackgroundColor = lParam;
	m_PickColorFromImage.SetColorToWindowsDefault();
	m_PickColorFromDlg.SetColor(CDib::HighlightColor(m_crBackgroundColor),
								m_crBackgroundColor);
	if (m_pWndPalette)
		m_pWndPalette->Close();
	return 0;
}

LRESULT CRotationFlippingDlg::OnColorPickerClosed(WPARAM wParam, LPARAM lParam)
{
	m_pWndPalette = NULL;
	return 0;
}

void CRotationFlippingDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if (m_pWndPalette)
		m_pWndPalette->Close();
}

void CRotationFlippingDlg::OnCheckLossless() 
{	
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_LOSSLESS);
	pDoc->m_bForceLossyTrafo = !pCheck->GetCheck();
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("ForceLossyTrafo"),
										pDoc->m_bForceLossyTrafo);
	}
}

void CRotationFlippingDlg::OnChangeEditAngle() 
{
	if (m_bDlgInitialized)
	{
		m_TransformationType = 5;
		CButton* pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION0);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION1);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION2);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION3);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION4);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION5);
		if (pCheck) pCheck->SetCheck(1);
	}
}

void CRotationFlippingDlg::OnChangeEditAngleMinutes() 
{
	if (m_bDlgInitialized)
	{
		m_TransformationType = 5;
		CButton* pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION0);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION1);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION2);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION3);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION4);
		if (pCheck) pCheck->SetCheck(0);
		pCheck = (CButton*)GetDlgItem(IDC_RADIO_TRANSFORMATION5);
		if (pCheck) pCheck->SetCheck(1);
	}
}