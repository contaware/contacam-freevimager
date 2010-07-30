// ColorDetectionPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ColorDetectionPage.h"
#include "VideoDeviceView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CColorDetectionPage dialog

CColorDetectionPage::CColorDetectionPage()
	: CPropertyPage(CColorDetectionPage::IDD)
{
	// OnInitDialog() is called when first pressing the tab
	// OnInitDialog() inits the property page pointer in the doc
	// -> Move all inits to OnInitDialog() because the assistant
	// may change the doc vars between construction and OnInitDialog() call
	//{{AFX_DATA_INIT(CColorDetectionPage)
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

void CColorDetectionPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CColorDetectionPage::~CColorDetectionPage()
{
}

void CColorDetectionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorDetectionPage)
	DDX_Control(pDX, IDC_SPIN_WAITBETWEENCOUNTUP, m_SpinWaitBetweenCountup);
	DDX_Control(pDX, IDC_SLIDER_ACCURACY, m_SliderAccuracy);
	DDX_Control(pDX, IDC_SLIDER_COLOR7, m_SliderColor7);
	DDX_Control(pDX, IDC_SLIDER_COLOR6, m_SliderColor6);
	DDX_Control(pDX, IDC_SLIDER_COLOR5, m_SliderColor5);
	DDX_Control(pDX, IDC_SLIDER_COLOR4, m_SliderColor4);
	DDX_Control(pDX, IDC_SLIDER_COLOR3, m_SliderColor3);
	DDX_Control(pDX, IDC_PROGRESS_COLOR7, m_ProgressColor7);
	DDX_Control(pDX, IDC_PROGRESS_COLOR6, m_ProgressColor6);
	DDX_Control(pDX, IDC_PROGRESS_COLOR5, m_ProgressColor5);
	DDX_Control(pDX, IDC_PROGRESS_COLOR4, m_ProgressColor4);
	DDX_Control(pDX, IDC_PROGRESS_COLOR3, m_ProgressColor3);
	DDX_Control(pDX, IDC_SLIDER_COLOR2, m_SliderColor2);
	DDX_Control(pDX, IDC_SLIDER_COLOR1, m_SliderColor1);
	DDX_Control(pDX, IDC_PROGRESS_COLOR2, m_ProgressColor2);
	DDX_Control(pDX, IDC_PROGRESS_COLOR1, m_ProgressColor1);
	DDX_Control(pDX, IDC_SLIDER_COLOR0, m_SliderColor0);
	DDX_Control(pDX, IDC_PROGRESS_COLOR0, m_ProgressColor0);
	DDX_Control(pDX, IDC_PICKCOLOR_7, m_PickColor7);
	DDX_Control(pDX, IDC_PICKCOLOR_6, m_PickColor6);
	DDX_Control(pDX, IDC_PICKCOLOR_5, m_PickColor5);
	DDX_Control(pDX, IDC_PICKCOLOR_4, m_PickColor4);
	DDX_Control(pDX, IDC_PICKCOLOR_3, m_PickColor3);
	DDX_Control(pDX, IDC_PICKCOLOR_2, m_PickColor2);
	DDX_Control(pDX, IDC_PICKCOLOR_1, m_PickColor1);
	DDX_Control(pDX, IDC_PICKCOLOR_0, m_PickColor0);
	DDX_Text(pDX, IDC_EDIT_WAITBETWEENCOUNTUP, m_dwWaitBetweenCountup);
	DDV_MinMaxDWord(pDX, m_dwWaitBetweenCountup, 1, 60);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE0, m_RegulationType[0]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE1, m_RegulationType[1]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE2, m_RegulationType[2]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE3, m_RegulationType[3]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE4, m_RegulationType[4]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE5, m_RegulationType[5]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE6, m_RegulationType[6]);
	DDX_CBIndex(pDX, IDC_COMBO_REGULATION_TYPE7, m_RegulationType[7]);
}

BEGIN_MESSAGE_MAP(CColorDetectionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CColorDetectionPage)
	ON_BN_CLICKED(IDC_PICKCOLOR_0, OnPickcolor0)
	ON_BN_CLICKED(IDC_PICKCOLOR_1, OnPickcolor1)
	ON_BN_CLICKED(IDC_PICKCOLOR_2, OnPickcolor2)
	ON_BN_CLICKED(IDC_PICKCOLOR_3, OnPickcolor3)
	ON_BN_CLICKED(IDC_PICKCOLOR_4, OnPickcolor4)
	ON_BN_CLICKED(IDC_PICKCOLOR_5, OnPickcolor5)
	ON_BN_CLICKED(IDC_PICKCOLOR_6, OnPickcolor6)
	ON_BN_CLICKED(IDC_PICKCOLOR_7, OnPickcolor7)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_DETECTION_PREVIEW, OnCheckDetectionPreview)
	ON_BN_CLICKED(IDC_BUTTON_RESETCOUNTER, OnButtonResetcounter)
	ON_EN_CHANGE(IDC_EDIT_WAITBETWEENCOUNTUP, OnChangeEditWaitbetweencountup)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE0, OnSelchangeComboRegulationType0)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE1, OnSelchangeComboRegulationType1)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE2, OnSelchangeComboRegulationType2)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE3, OnSelchangeComboRegulationType3)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE4, OnSelchangeComboRegulationType4)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE5, OnSelchangeComboRegulationType5)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE6, OnSelchangeComboRegulationType6)
	ON_CBN_SELCHANGE(IDC_COMBO_REGULATION_TYPE7, OnSelchangeComboRegulationType7)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_VIDEO_DETECTION_COLOR, OnCheckVideoDetectionColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorDetectionPage message handlers

void CColorDetectionPage::UpdateControls()
{
	// Enable / Disable
	switch (m_pDoc->m_ColorDetection.GetColorsCount())
	{
		case 8 :
		case 7 :
			m_PickColor7.EnableWindow(TRUE);
		case 6 :
			m_PickColor6.EnableWindow(TRUE);
		case 5 :
			m_PickColor5.EnableWindow(TRUE);
		case 4 :
			m_PickColor4.EnableWindow(TRUE);
		case 3 :
			m_PickColor3.EnableWindow(TRUE);
		case 2 :
			m_PickColor2.EnableWindow(TRUE);
		case 1 :
			m_PickColor1.EnableWindow(TRUE);
		case 0 :
			m_PickColor0.EnableWindow(TRUE);
		default :
			break;
	}
	switch (m_pDoc->m_ColorDetection.GetColorsCount())
	{
		case 0 :
			m_PickColor1.EnableWindow(FALSE);
		case 1 :
			m_PickColor2.EnableWindow(FALSE);
		case 2 :
			m_PickColor3.EnableWindow(FALSE);
		case 3 :
			m_PickColor4.EnableWindow(FALSE);
		case 4 :
			m_PickColor5.EnableWindow(FALSE);
		case 5 :
			m_PickColor6.EnableWindow(FALSE);
		case 6 :
			m_PickColor7.EnableWindow(FALSE);
		case 7 :
		case 8 :
			break;
		default :
			break;
	}

	// Set Colors
	switch (m_pDoc->m_ColorDetection.GetColorsCount())
	{
		case 8 :
			m_PickColor7.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(7)),
									m_pDoc->m_ColorDetection.GetColor(7));
			m_ProgressColor7.SetColor(m_pDoc->m_ColorDetection.GetColor(7));
		case 7 :
			m_PickColor6.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(6)),
									m_pDoc->m_ColorDetection.GetColor(6));
			m_ProgressColor6.SetColor(m_pDoc->m_ColorDetection.GetColor(6));
		case 6 :
			m_PickColor5.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(5)),
									m_pDoc->m_ColorDetection.GetColor(5));
			m_ProgressColor5.SetColor(m_pDoc->m_ColorDetection.GetColor(5));
		case 5 :
			m_PickColor4.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(4)),
									m_pDoc->m_ColorDetection.GetColor(4));
			m_ProgressColor4.SetColor(m_pDoc->m_ColorDetection.GetColor(4));
		case 4 :
			m_PickColor3.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(3)),
									m_pDoc->m_ColorDetection.GetColor(3));
			m_ProgressColor3.SetColor(m_pDoc->m_ColorDetection.GetColor(3));
		case 3 :
			m_PickColor2.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(2)),
									m_pDoc->m_ColorDetection.GetColor(2));
			m_ProgressColor2.SetColor(m_pDoc->m_ColorDetection.GetColor(2));
		case 2 :
			m_PickColor1.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(1)),
									m_pDoc->m_ColorDetection.GetColor(1));
			m_ProgressColor1.SetColor(m_pDoc->m_ColorDetection.GetColor(1));
		case 1 :
			m_PickColor0.SetColor(	CDib::HighlightColor(m_pDoc->m_ColorDetection.GetColor(0)),
									m_pDoc->m_ColorDetection.GetColor(0));
			m_ProgressColor0.SetColor(m_pDoc->m_ColorDetection.GetColor(0));
		case 0 :
		default :
			break;
	}
	switch (m_pDoc->m_ColorDetection.GetColorsCount())
	{
		CEdit* pEdit;
		case 0 :
			m_PickColor0.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP0);
			pEdit->SetWindowText(_T(""));
		case 1 :
			m_PickColor1.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP1);
			pEdit->SetWindowText(_T(""));
		case 2 :
			m_PickColor2.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP2);
			pEdit->SetWindowText(_T(""));
		case 3 :
			m_PickColor3.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP3);
			pEdit->SetWindowText(_T(""));
		case 4 :
			m_PickColor4.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP4);
			pEdit->SetWindowText(_T(""));
		case 5 :
			m_PickColor5.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP5);
			pEdit->SetWindowText(_T(""));
		case 6 :
			m_PickColor6.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP6);
			pEdit->SetWindowText(_T(""));
		case 7 :
			m_PickColor7.SetColorToWindowsDefault();
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_COUNTUP7);
			pEdit->SetWindowText(_T(""));
		case 8 :
		default :
			break;
	}
}

CSliderCtrl* CColorDetectionPage::GetSlider(DWORD dwIndex)
{
	switch (dwIndex)
	{
		case 0 : return &m_SliderColor0;
		case 1 : return &m_SliderColor1;
		case 2 : return &m_SliderColor2;
		case 3 : return &m_SliderColor3;
		case 4 : return &m_SliderColor4;
		case 5 : return &m_SliderColor5;
		case 6 : return &m_SliderColor6;
		case 7 : return &m_SliderColor7;
		default: return NULL;
	}
}

CMacProgressCtrl* CColorDetectionPage::GetProgress(DWORD dwIndex)
{
	switch (dwIndex)
	{
		case 0 : return &m_ProgressColor0;
		case 1 : return &m_ProgressColor1;
		case 2 : return &m_ProgressColor2;
		case 3 : return &m_ProgressColor3;
		case 4 : return &m_ProgressColor4;
		case 5 : return &m_ProgressColor5;
		case 6 : return &m_ProgressColor6;
		case 7 : return &m_ProgressColor7;
		default: return NULL;
	}
}

void CColorDetectionPage::UpdateDataToSlider(DWORD dwIndex)
{
	CSliderCtrl* pSlider = GetSlider(dwIndex);

	if (pSlider)
	{
		if (m_RegulationType[dwIndex] <= REGULATION_TYPE_THRESHOLD_100)
		{
			pSlider->SetRange(0, RegulationTypeToMaxDetectionLevel(m_RegulationType[dwIndex]), TRUE);
			pSlider->SetPos(m_DetectionThreshold[dwIndex]);
			m_pDoc->m_ColorDetection.SetDetectionThreshold(dwIndex, m_DetectionThreshold[dwIndex]);
		}
		else if (m_RegulationType[dwIndex] == REGULATION_TYPE_HUE)
		{
			pSlider->SetRange(0, COLDET_MAX_HUE_RADIUS, TRUE);
			pSlider->SetPos(m_HueRadius[dwIndex]);
		}
		else if (m_RegulationType[dwIndex] == REGULATION_TYPE_SATURATION)
		{
			pSlider->SetRange(0, COLDET_MAX_SATURATION_RADIUS, TRUE);
			pSlider->SetPos(m_SaturationRadius[dwIndex]);
		}
		else if (m_RegulationType[dwIndex] == REGULATION_TYPE_VALUE)
		{
			pSlider->SetRange(0, COLDET_MAX_VALUE_RADIUS, TRUE);
			pSlider->SetPos(m_ValueRadius[dwIndex]);
		}
	}
}

void CColorDetectionPage::UpdateDataFromSlider(DWORD dwIndex, int nPos)
{
	if (m_RegulationType[dwIndex] <= REGULATION_TYPE_THRESHOLD_100)
	{
		m_DetectionThreshold[dwIndex] = nPos;
		m_pDoc->m_ColorDetection.SetDetectionThreshold(dwIndex, m_DetectionThreshold[dwIndex]);
	}
	else if (m_RegulationType[dwIndex] == REGULATION_TYPE_HUE)
	{
		m_HueRadius[dwIndex] = nPos;
		m_pDoc->m_ColorDetection.SetHueRadius(dwIndex, m_HueRadius[dwIndex]);
	}
	else if (m_RegulationType[dwIndex] == REGULATION_TYPE_SATURATION)
	{
		m_SaturationRadius[dwIndex] = nPos;
		m_pDoc->m_ColorDetection.SetSaturationRadius(dwIndex, m_SaturationRadius[dwIndex]);
	}
	else if (m_RegulationType[dwIndex] == REGULATION_TYPE_VALUE)
	{
		m_ValueRadius[dwIndex] = nPos;
		m_pDoc->m_ColorDetection.SetValueRadius(dwIndex, m_ValueRadius[dwIndex]);
	}
}

BOOL CColorDetectionPage::OnInitDialog()
{
	int i;

	// Init vars
	m_dwWaitBetweenCountup = m_pDoc->GetColorDetectionWaitTime() / 1000U;
	for (i = 0 ; i < COLDET_MAX_COLORS ; i++)
	{
		m_RegulationType[i] = REGULATION_TYPE_THRESHOLD_10;
		m_DetectionThreshold[i] = 0;
		m_HueRadius[i] = 0;
		m_SaturationRadius[i] = 0;
		m_ValueRadius[i] = 0;
	}

	// Init Combo Boxes
	for (i = 0 ; i < COLDET_MAX_COLORS ; i++)
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_REGULATION_TYPE0 + i);
		if (pComboBox)
		{
			pComboBox->AddString(_T("1%"));
			pComboBox->AddString(_T("5%"));
			pComboBox->AddString(_T("10%"));
			pComboBox->AddString(_T("50%"));
			pComboBox->AddString(_T("100%"));
			pComboBox->AddString(ML_STRING(1503, "Hue"));
			pComboBox->AddString(ML_STRING(1504, "Sat."));
			pComboBox->AddString(ML_STRING(1505, "Value"));
		}
	}

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();
	
	m_crRedColor = RGB(255,0,0);
	m_crGreenColor = RGB(0,255,0);
	m_crBlueColor = RGB(0,0,255);
	m_crWhiteTextColor = RGB(255,255,255);
	m_RedBrush.CreateSolidBrush(m_crRedColor);
	m_GreenBrush.CreateSolidBrush(m_crGreenColor);
	m_BlueBrush.CreateSolidBrush(m_crBlueColor);

	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DETECTION_PREVIEW);
	if (m_pDoc->m_bColorDetectionPreview)
		pCheck->SetCheck(1);
	else
		pCheck->SetCheck(0);

	// Color Detection Check Box
	CButton* pCheckColor = (CButton*)GetDlgItem(IDC_CHECK_VIDEO_DETECTION_COLOR);
	if (m_pDoc->m_VideoProcessorMode & COLOR_DETECTOR)
		pCheckColor->SetCheck(1);
	else
		pCheckColor->SetCheck(0);

	m_SpinWaitBetweenCountup.SetRange(1, 60);

	// Init Accuracy Slider
	m_SliderAccuracy.SetRange(1, 4);
	m_SliderAccuracy.SetPos(m_pDoc->m_dwColorDetectionAccuracy);
	m_SliderAccuracy.SetTicFreq(1);

	// Init Color Sliders and Progress Controls
	for (i = 0 ; i < COLDET_MAX_COLORS ; i++)
	{
		UpdateDataToSlider(i);
		GetProgress(i)->SetRange(0, RegulationTypeToMaxDetectionLevel(m_RegulationType[i]));
	}

	// Update Controls
	UpdateControls();

	// Set Page Pointer to this
	m_pDoc->m_pColorDetectionPage = this;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorDetectionPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	// Set Dialog Pointer to NULL
	m_pDoc->m_pColorDetectionPage = NULL;
}

void CColorDetectionPage::OnPickcolor0() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 0)
		{
			m_pDoc->m_ColorDetection.RemoveColor(0);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 0)
			m_pDoc->m_nDoColorPickup = 1; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor1() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 1)
		{	
			m_pDoc->m_ColorDetection.RemoveColor(1);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 1)
			m_pDoc->m_nDoColorPickup = 2; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor2() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 2)
		{
			m_pDoc->m_ColorDetection.RemoveColor(2);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 2)
			m_pDoc->m_nDoColorPickup = 3; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor3() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 3)
		{
			m_pDoc->m_ColorDetection.RemoveColor(3);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 3)
			m_pDoc->m_nDoColorPickup = 4; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor4() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 4)
		{
			m_pDoc->m_ColorDetection.RemoveColor(4);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 4)
			m_pDoc->m_nDoColorPickup = 5; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor5() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 5)
		{
			m_pDoc->m_ColorDetection.RemoveColor(5);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 5)
			m_pDoc->m_nDoColorPickup = 6; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor6() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 6)
		{
			m_pDoc->m_ColorDetection.RemoveColor(6);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 6)
			m_pDoc->m_nDoColorPickup = 7; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnPickcolor7() 
{
	if (::GetKeyState(VK_CONTROL) < 0)
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 7)
		{
			m_pDoc->m_ColorDetection.RemoveColor(7);
			UpdateControls();
		}
		m_pDoc->m_nDoColorPickup = 0;
	}
	else
	{
		if (m_pDoc->m_ColorDetection.GetColorsCount() > 7)
			m_pDoc->m_nDoColorPickup = 8; // Replace
		else
			m_pDoc->m_nDoColorPickup = -1; // Add
		m_pDoc->GetView()->SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CColorDetectionPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar)
	{
		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_ACCURACY)
				m_pDoc->m_dwColorDetectionAccuracy = m_SliderAccuracy.GetPos();
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR0)
				UpdateDataFromSlider(0, m_SliderColor0.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR1)
				UpdateDataFromSlider(1, m_SliderColor1.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR2)
				UpdateDataFromSlider(2, m_SliderColor2.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR3)
				UpdateDataFromSlider(3, m_SliderColor3.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR4)
				UpdateDataFromSlider(4, m_SliderColor4.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR5)
				UpdateDataFromSlider(5, m_SliderColor5.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR6)
				UpdateDataFromSlider(6, m_SliderColor6.GetPos());
			else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_COLOR7)
				UpdateDataFromSlider(7, m_SliderColor7.GetPos());
		}
	}
	
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CColorDetectionPage::OnCheckDetectionPreview() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DETECTION_PREVIEW);
	if (pCheck->GetCheck())
		m_pDoc->m_bColorDetectionPreview = TRUE;
	else
		m_pDoc->m_bColorDetectionPreview = FALSE;
}

void CColorDetectionPage::OnButtonResetcounter() 
{
	m_pDoc->m_ColorDetection.ResetCounter();
}

void CColorDetectionPage::OnChangeEditWaitbetweencountup() 
{
	if (::IsWindow(m_SpinWaitBetweenCountup.GetSafeHwnd()))
	{
		if (UpdateData(TRUE))
		{
			m_pDoc->m_ColorDetection.ResetCounter();
			m_pDoc->SetColorDetectionWaitTime(1000 * m_dwWaitBetweenCountup);
		}
	}
}

int CColorDetectionPage::RegulationTypeToMaxDetectionLevel(int nIndex)
{
	switch (nIndex)
	{
		case REGULATION_TYPE_THRESHOLD_1 :		return 100;		// 1 %
		case REGULATION_TYPE_THRESHOLD_5 :		return 500;		// 5 %
		case REGULATION_TYPE_THRESHOLD_10 :		return 1000;	// 10 %
		case REGULATION_TYPE_THRESHOLD_50 :		return 5000;	// 50 %
		case REGULATION_TYPE_THRESHOLD_100 :	return 10000;	// 100 %
		default : return 0;
	}
}

void CColorDetectionPage::OnRegulationTypeChanged(DWORD dwIndex, CSliderCtrl& Slider, CMacProgressCtrl& Progress)
{
	if (m_RegulationType[dwIndex] <= REGULATION_TYPE_THRESHOLD_100)
	{
		int nPos = m_DetectionThreshold[dwIndex];
		int nNewRangeMax = RegulationTypeToMaxDetectionLevel(m_RegulationType[dwIndex]);
		if (nPos > nNewRangeMax)
			nPos = nNewRangeMax;
		Progress.SetRange(0, nNewRangeMax);
		Slider.SetRange(0, nNewRangeMax, TRUE);
		Slider.SetPos(nPos);
	}
	else if (m_RegulationType[dwIndex] == REGULATION_TYPE_HUE)
	{
		Slider.SetRange(0, COLDET_MAX_HUE_RADIUS, TRUE);
		Slider.SetPos(m_HueRadius[dwIndex]);
	}
	else if (m_RegulationType[dwIndex] == REGULATION_TYPE_SATURATION)
	{
		Slider.SetRange(0, COLDET_MAX_SATURATION_RADIUS, TRUE);
		Slider.SetPos(m_SaturationRadius[dwIndex]);
	}
	else if (m_RegulationType[dwIndex] == REGULATION_TYPE_VALUE)
	{
		Slider.SetRange(0, COLDET_MAX_VALUE_RADIUS, TRUE);
		Slider.SetPos(m_ValueRadius[dwIndex]);
	}
}

void CColorDetectionPage::OnSelchangeComboRegulationType0() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(0, m_SliderColor0, m_ProgressColor0);
}

void CColorDetectionPage::OnSelchangeComboRegulationType1() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(1, m_SliderColor1, m_ProgressColor1);
}

void CColorDetectionPage::OnSelchangeComboRegulationType2() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(2, m_SliderColor2, m_ProgressColor2);
}

void CColorDetectionPage::OnSelchangeComboRegulationType3() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(3, m_SliderColor3, m_ProgressColor3);
}

void CColorDetectionPage::OnSelchangeComboRegulationType4() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(4, m_SliderColor4, m_ProgressColor4);
}

void CColorDetectionPage::OnSelchangeComboRegulationType5() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(5, m_SliderColor5, m_ProgressColor5);
}

void CColorDetectionPage::OnSelchangeComboRegulationType6() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(6, m_SliderColor6, m_ProgressColor6);
}

void CColorDetectionPage::OnSelchangeComboRegulationType7() 
{
	UpdateData(TRUE);
	OnRegulationTypeChanged(7, m_SliderColor7, m_ProgressColor7);
}

void CColorDetectionPage::OnCheckVideoDetectionColor() 
{
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_VIDEO_DETECTION_COLOR);
	if (pCheck->GetCheck())
		m_pDoc->m_VideoProcessorMode |= COLOR_DETECTOR;
	else
		m_pDoc->m_VideoProcessorMode &= ~COLOR_DETECTOR;	
}

HBRUSH CColorDetectionPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr; 

	switch (nCtlColor) 
	{ 
		// For Read/Write Edit Controls
		case CTLCOLOR_EDIT:
		case CTLCOLOR_MSGBOX:
			hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
			break;

		// Process Static text, READONLY controls, DISABLED * controls.
		//   NOTE: Disabled controls can NOT have their text color
		//         changed.
		//         Suggest you change all your DISABLED controls to
		//         READONLY.
		case CTLCOLOR_STATIC:
			switch (pWnd->GetDlgCtrlID())
			{     
				case IDC_EDIT_COUNTUP0 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x1)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP1 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x2)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP2 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x4)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP3 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x8)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP4 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x10)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP5 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x20)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP6 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x40)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_COUNTUP7 :
					if (m_pDoc->m_ColorDetection.GetMaxCountsColorIndexes() & 0x80)
					{
						pDC->SetBkColor(m_crBlueColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_BlueBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME0 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x1)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME1 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x2)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME2 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x4)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME3 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x8)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME4 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x10)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME5 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x20)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME6 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x40)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_SHORTESTTIME7 :
					if (m_pDoc->m_ColorDetection.GetShortestTimeBetweenCountsColorIndexes() & 0x80)
					{
						pDC->SetBkColor(m_crGreenColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_GreenBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME0 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x1)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME1 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x2)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME2 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x4)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME3 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x8)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME4 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x10)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME5 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x20)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME6 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x40)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

				case IDC_EDIT_LONGESTTIME7 :
					if (m_pDoc->m_ColorDetection.GetLongestTimeBetweenCountsColorIndexes() & 0x80)
					{
						pDC->SetBkColor(m_crRedColor);
						pDC->SetBkMode(TRANSPARENT);
						pDC->SetTextColor(m_crWhiteTextColor);
						hbr = (HBRUSH)m_RedBrush;
					}
					else
						hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;

		
				default:
					hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
					break;
			}
			break;

		// Otherwise, do default handling of OnCtlColor
		default:
			hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
			break;
	}

	return hbr; // return brush
}

#endif
