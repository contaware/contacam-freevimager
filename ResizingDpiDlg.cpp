// ResizingDpiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ResizingDpiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizingDpiDlg dialog

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define UPPER_VALUE				1048576.0
#define MAX_RESIZE_FACTOR		128.0

CResizingDpiDlg::CResizingDpiDlg(int nWidth, int nHeight,
								int nXDpi, int nYDpi,
								int nResizingMethod,
								CWnd* pParent /*=NULL*/)
	: CDialog(CResizingDpiDlg::IDD, pParent)
{
	m_bDlgInitialized = FALSE;
	if (nWidth <= 0)
		nWidth = 1;
	if (nHeight <= 0)
		nHeight = 1;
	if (nXDpi <= 0)
		nXDpi = 1;
	if (nYDpi <= 0)
		nYDpi = 1;
	//{{AFX_DATA_INIT(CResizingDpiDlg)
	m_nPercentWidth = 100;
	m_nPercentHeight = 100;
	m_nPixelsWidth = nWidth;
	m_nPixelsHeight = nHeight;
	m_bRetainAspect = TRUE;
	m_ResizingMethod = nResizingMethod;
	m_nXDpi = nXDpi;
	m_nYDpi = nYDpi;
	//}}AFX_DATA_INIT
	if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_INCH)
		m_PhysUnit = 1;	// inch
	else
		m_PhysUnit = 0;	// cm
	m_nInitResizingMethod = nResizingMethod;
	m_nInitWidth = nWidth;
	m_nInitHeight = nHeight;
	m_nInitXDpi = nXDpi;
	m_nInitYDpi = nYDpi;
	m_dRatio = (double)nWidth / (double)nHeight;
	m_bNormalValidate = FALSE;
	m_bPixAndPercentAreChanging = FALSE;
	m_bPhysOrDpiAreChanging = FALSE;
}


void CResizingDpiDlg::DoDataExchange(CDataExchange* pDX)
{
	if (!m_bNormalValidate)
	{
		CString sText;
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT_HEIGHT);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PERCENT_WIDTH);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_HEIGHT);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_WIDTH);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_YDPI);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_XDPI);
		pEdit->GetWindowText(sText);
		if (sText == _T(""))
			return;
	}

	// Update resizing method depending from
	// the new width and height
	if (pDX->m_bSaveAndValidate == FALSE)
	{
		// Calc. Ratio
		double dRatioX = 1000.0;
		if (m_nPixelsWidth > 0)
			dRatioX = (double)m_nInitWidth / (double)m_nPixelsWidth;
		double dRatioY = 1000.0;
		if (m_nPixelsHeight > 0)
			dRatioY = (double)m_nInitHeight / (double)m_nPixelsHeight;
		double dRatioMin = MIN(dRatioX, dRatioY);

		// Shrink
		if (dRatioMin > 1.0)
		{
			m_ResizingMethod = AVG;
		}
		// Enlarge
		else
		{
			if (m_ResizingMethod == AVG)
				m_ResizingMethod = m_nInitResizingMethod;
		}
	}

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResizingDpiDlg)
	DDX_Control(pDX, IDC_EDIT_PHYS_HEIGHT, m_EditPhysHeight);
	DDX_Control(pDX, IDC_EDIT_PHYS_WIDTH, m_EditPhysWidth);
	DDX_Control(pDX, IDC_SPIN_PHYS_WIDTH, m_SpinPhysWidth);
	DDX_Control(pDX, IDC_SPIN_PHYS_HEIGHT, m_SpinPhysHeight);
	DDX_Text(pDX, IDC_EDIT_PERCENT_HEIGHT, m_nPercentHeight);
	DDX_Text(pDX, IDC_EDIT_PERCENT_WIDTH, m_nPercentWidth);
	DDX_Text(pDX, IDC_EDIT_PIXELS_HEIGHT, m_nPixelsHeight);
	DDX_Text(pDX, IDC_EDIT_PIXELS_WIDTH, m_nPixelsWidth);
	DDX_Check(pDX, IDC_CHECK_RETAIN_ASPECT, m_bRetainAspect);
	DDX_CBIndex(pDX, IDC_COMBO_METHOD, m_ResizingMethod);
	DDX_CBIndex(pDX, IDC_COMBO_PHYS_UNIT, m_PhysUnit);
	DDX_Text(pDX, IDC_EDIT_XDPI, m_nXDpi);
	DDX_Text(pDX, IDC_EDIT_YDPI, m_nYDpi);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResizingDpiDlg, CDialog)
	//{{AFX_MSG_MAP(CResizingDpiDlg)
	ON_EN_CHANGE(IDC_EDIT_PERCENT_HEIGHT, OnChangeEditPercentHeight)
	ON_EN_CHANGE(IDC_EDIT_PERCENT_WIDTH, OnChangeEditPercentWidth)
	ON_EN_CHANGE(IDC_EDIT_PIXELS_HEIGHT, OnChangeEditPixelsHeight)
	ON_EN_CHANGE(IDC_EDIT_PIXELS_WIDTH, OnChangeEditPixelsWidth)
	ON_EN_CHANGE(IDC_EDIT_PHYS_HEIGHT, OnChangeEditPhysHeight)
	ON_EN_CHANGE(IDC_EDIT_PHYS_WIDTH, OnChangeEditPhysWidth)
	ON_EN_CHANGE(IDC_EDIT_XDPI, OnChangeEditXdpi)
	ON_EN_CHANGE(IDC_EDIT_YDPI, OnChangeEditYdpi)
	ON_CBN_SELCHANGE(IDC_COMBO_PHYS_UNIT, OnSelchangeComboPhysUnit)
	ON_BN_CLICKED(IDC_CHECK_RETAIN_ASPECT, OnCheckRetainAspect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizingDpiDlg message handlers

void CResizingDpiDlg::Reset()
{
	m_bPixAndPercentAreChanging = TRUE;
	m_nPercentHeight = 100;
	m_nPercentWidth = 100;
	m_nPixelsHeight = m_nInitHeight;
	m_nPixelsWidth = m_nInitWidth;
	m_nXDpi = m_nInitXDpi;
	m_nYDpi = m_nInitYDpi;
	FromPixToPhys();
	UpdateData(FALSE);
	m_bPixAndPercentAreChanging = FALSE;
}

double CResizingDpiDlg::GetMaxPhysWidth()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			if (m_nXDpi > 0)
				return m_dMaxResizeFactor * (double)m_nInitWidth * 2.54 / (double)m_nXDpi;
			else
				return 1000.0;

		// inch
		case 1:
			if (m_nXDpi > 0)
				return m_dMaxResizeFactor * (double)m_nInitWidth / (double)m_nXDpi;
			else
				return 1000.0;

		default:
			return 0.0;
	}
}

double CResizingDpiDlg::GetMaxPhysHeight()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			if (m_nYDpi > 0)
				return m_dMaxResizeFactor * (double)m_nInitHeight * 2.54 / (double)m_nYDpi;
			else
				return 1000.0;

		// inch
		case 1:
			if (m_nYDpi > 0)
				return m_dMaxResizeFactor * (double)m_nInitHeight / (double)m_nYDpi;
			else
				return 1000.0;

		default:
			return 0.0;
	}
}

void CResizingDpiDlg::FromPixToPhys()
{
	FromPixToPhysX();
	FromPixToPhysY();
}

void CResizingDpiDlg::FromPixToPhysX()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			if (m_nXDpi > 0)
				m_SpinPhysWidth.SetPos((double)m_nPixelsWidth * 2.54 / (double)m_nXDpi);
			break;

		// inch
		case 1:
			if (m_nXDpi > 0)
				m_SpinPhysWidth.SetPos((double)m_nPixelsWidth / (double)m_nXDpi);
			break;

		default:
			ASSERT(FALSE);
			break;
	}
}

void CResizingDpiDlg::FromPixToPhysY()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			if (m_nYDpi > 0)
				m_SpinPhysHeight.SetPos((double)m_nPixelsHeight * 2.54 / (double)m_nYDpi);
			break;

		// inch
		case 1:
			if (m_nYDpi > 0)
				m_SpinPhysHeight.SetPos((double)m_nPixelsHeight / (double)m_nYDpi);
			break;

		default:
			ASSERT(FALSE);
			break;
	}
}

void CResizingDpiDlg::FromPixToDpi()
{
	FromPixToDpiX();
	FromPixToDpiY();
}

void CResizingDpiDlg::FromPixToDpiX()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			if (m_SpinPhysWidth.GetPos() > 0.0)
				m_nXDpi = Round((double)m_nPixelsWidth * 2.54 / m_SpinPhysWidth.GetPos());
			break;

		// inch
		case 1:
			if (m_SpinPhysWidth.GetPos() > 0.0)
				m_nXDpi = Round((double)m_nPixelsWidth / m_SpinPhysWidth.GetPos());
			break;

		default:
			ASSERT(FALSE);
			break;
	}
	if (m_nXDpi <= 0)
		m_nXDpi = 1;
}

void CResizingDpiDlg::FromPixToDpiY()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			if (m_SpinPhysHeight.GetPos() > 0.0)
				m_nYDpi = Round((double)m_nPixelsHeight * 2.54 / m_SpinPhysHeight.GetPos());
			break;

		// inch
		case 1:
			if (m_SpinPhysHeight.GetPos() > 0.0)
				m_nYDpi = Round((double)m_nPixelsHeight / m_SpinPhysHeight.GetPos());
			break;

		default:
			ASSERT(FALSE);
			break;
	}
	if (m_nYDpi <= 0)
		m_nYDpi = 1;
}

void CResizingDpiDlg::FromPhysAndDpiToPixAndPercent()
{
	FromPhysAndDpiToPixAndPercentX();
	FromPhysAndDpiToPixAndPercentY();
}

void CResizingDpiDlg::FromPhysAndDpiToPixAndPercentX()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			m_nPixelsWidth = Round((double)m_SpinPhysWidth.GetPos() * (double)m_nXDpi / 2.54);
			break;

		// inch
		case 1:
			m_nPixelsWidth = Round((double)m_SpinPhysWidth.GetPos() * (double)m_nXDpi);
			break;

		default:
			ASSERT(FALSE);
			break;
	}
	if (m_nPixelsWidth <= 0)
		m_nPixelsWidth = 1;
	m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nInitWidth);
	if (m_nPercentWidth <= 0)
		m_nPercentWidth = 1;
}

void CResizingDpiDlg::FromPhysAndDpiToPixAndPercentY()
{
	switch (m_PhysUnit)
	{
		// cm
		case 0:
			m_nPixelsHeight = Round((double)m_SpinPhysHeight.GetPos() * (double)m_nYDpi / 2.54);
			break;

		// inch
		case 1:
			m_nPixelsHeight = Round((double)m_SpinPhysHeight.GetPos() * (double)m_nYDpi);
			break;

		default:
			ASSERT(FALSE);
			break;
	}
	if (m_nPixelsHeight <= 0)
		m_nPixelsHeight = 1;
	m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nInitHeight);
	if (m_nPercentHeight <= 0)
		m_nPercentHeight = 1;
}

BOOL CResizingDpiDlg::OnInitDialog() 
{
	// Init Combo Boxes
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PHYS_UNIT);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1496, "cm"));
		pComboBox->AddString(ML_STRING(1497, "inch"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_METHOD);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1498, "Avg (shrink, thumb)"));
		pComboBox->AddString(ML_STRING(1499, "Nearest Neighbor"));
		pComboBox->AddString(ML_STRING(1500, "Bilinear"));
		pComboBox->AddString(ML_STRING(1501, "Bicubic"));
	}

	// Init m_dMaxResizeFactor
	double dMaxResizeFactorX = UPPER_VALUE / (double)m_nInitWidth;
	double dMaxResizeFactorY = UPPER_VALUE / (double)m_nInitHeight;
	double dMaxResizeFactorXY = MIN(dMaxResizeFactorX, dMaxResizeFactorY);
	m_dMaxResizeFactor = MIN(MAX_RESIZE_FACTOR, dMaxResizeFactorXY);

	CDialog::OnInitDialog();

	// Init Pix & Percent Spin Controls
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS_WIDTH);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * (double)m_nInitWidth));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS_HEIGHT);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * (double)m_nInitHeight));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT_WIDTH);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * 100.0));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PERCENT_HEIGHT);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * 100.0));

	// Init DPI Spin Controls
	// Note: the -1 is to have some margin,
	// otherwise also the Pix And Percent Checks will Popup!
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_XDPI);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * (double)m_nInitXDpi - 1.0));
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_YDPI);
	pSpin->SetRange32(1, (int)(m_dMaxResizeFactor * (double)m_nInitYDpi - 1.0));

	// Init Physical Edit and Spin
	// Note: the -0.1 is to have some margin,
	// otherwise also the Pix And Percent Checks will Popup!
	m_SpinPhysWidth.SetBuddy(&m_EditPhysWidth);
	m_SpinPhysWidth.SetRange(0.1, MAX(0.1, GetMaxPhysWidth() - 0.1));
	m_SpinPhysWidth.SetDelta(0.1);
	m_EditPhysWidth.SetMinNumberOfNumberAfterPoint(2);
	m_EditPhysWidth.SetMaxNumberOfNumberAfterPoint(2);
	m_SpinPhysHeight.SetBuddy(&m_EditPhysHeight);
	m_SpinPhysHeight.SetRange(0.1, MAX(0.1, GetMaxPhysHeight() - 0.1));
	m_SpinPhysHeight.SetDelta(0.1);
	m_EditPhysHeight.SetMinNumberOfNumberAfterPoint(2);
	m_EditPhysHeight.SetMaxNumberOfNumberAfterPoint(2);
	FromPixToPhys();

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResizingDpiDlg::OnChangeEditPercentWidth() 
{
	if (m_bDlgInitialized && !m_bPhysOrDpiAreChanging)
	{
		m_bPixAndPercentAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPercentHeight = m_nPercentWidth;
				m_nPixelsWidth = Round(m_nInitWidth * m_nPercentWidth / 100.0);
				m_nPixelsHeight = Round(m_nInitHeight * m_nPercentHeight / 100.0);
			}
			else
				m_nPixelsWidth = Round(m_nInitWidth * m_nPercentWidth / 100.0);

			if (m_nPercentHeight == 0) m_nPercentHeight = 1;
			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;
			FromPixToPhys();
			UpdateData(FALSE);
		}
		m_bPixAndPercentAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditPercentHeight() 
{
	if (m_bDlgInitialized && !m_bPhysOrDpiAreChanging)
	{
		m_bPixAndPercentAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPercentWidth = m_nPercentHeight;
				m_nPixelsWidth = Round(m_nInitWidth * m_nPercentWidth / 100.0);
				m_nPixelsHeight = Round(m_nInitHeight * m_nPercentHeight / 100.0);
			}
			else
				m_nPixelsHeight = Round(m_nInitHeight * m_nPercentHeight / 100.0);

			if (m_nPercentWidth == 0) m_nPercentWidth = 1;
			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;
			FromPixToPhys();
			UpdateData(FALSE);
		}
		m_bPixAndPercentAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditPixelsWidth() 
{
	if (m_bDlgInitialized && !m_bPhysOrDpiAreChanging)
	{
		m_bPixAndPercentAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPixelsHeight = Round(m_nPixelsWidth / m_dRatio);
				m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nInitWidth);
				m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nInitHeight);
			}
			else
				m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nInitWidth);

			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;
			if (m_nPercentWidth == 0) m_nPercentWidth = 1;
			if (m_nPercentHeight == 0) m_nPercentHeight = 1;
			FromPixToPhys();
			UpdateData(FALSE);
		}
		m_bPixAndPercentAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditPixelsHeight() 
{
	if (m_bDlgInitialized && !m_bPhysOrDpiAreChanging)
	{
		m_bPixAndPercentAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			if (m_bRetainAspect)
			{
				m_nPixelsWidth = Round(m_nPixelsHeight * m_dRatio);
				m_nPercentWidth = Round(m_nPixelsWidth * 100.0 / m_nInitWidth);
				m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nInitHeight);
			}
			else
				m_nPercentHeight = Round(m_nPixelsHeight * 100.0 / m_nInitHeight);

			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			if (m_nPercentWidth == 0) m_nPercentWidth = 1;
			if (m_nPercentHeight == 0) m_nPercentHeight = 1;
			FromPixToPhys();
			UpdateData(FALSE);
		}
		m_bPixAndPercentAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditPhysWidth() 
{
	if (m_bDlgInitialized && !m_bPixAndPercentAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		
		UpdateData(TRUE);

		// Calc. Pix And Percent Width
		FromPhysAndDpiToPixAndPercentX();

		if (m_bRetainAspect)
		{
			// Update Pix And Percent Height
			m_nPixelsHeight = Round(m_nPixelsWidth / m_dRatio);
			m_nPercentHeight = m_nPercentWidth;

			// Calc. Phys Y
			m_bPixAndPercentAreChanging = TRUE;
			FromPixToPhysY();
			m_bPixAndPercentAreChanging = FALSE;
		}

		UpdateData(FALSE);
		
		m_bPhysOrDpiAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditPhysHeight() 
{
	if (m_bDlgInitialized && !m_bPixAndPercentAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		
		UpdateData(TRUE);

		// Calc. Pix And Percent Height
		FromPhysAndDpiToPixAndPercentY();

		if (m_bRetainAspect)
		{
			// Update Pix And Percent Width
			m_nPixelsWidth = Round(m_nPixelsHeight * m_dRatio);
			m_nPercentWidth = m_nPercentHeight;

			// Calc. Phys X
			m_bPixAndPercentAreChanging = TRUE;
			FromPixToPhysX();
			m_bPixAndPercentAreChanging = FALSE;
		}
		
		UpdateData(FALSE);
	
		m_bPhysOrDpiAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditXdpi() 
{
	if (m_bDlgInitialized && !m_bPixAndPercentAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			// Calc. Pix And Percent Width
			FromPhysAndDpiToPixAndPercentX();
			
			if (m_bRetainAspect)
			{
				// Update Pix And Percent Height
				m_nPixelsHeight = Round(m_nPixelsWidth / m_dRatio);
				m_nPercentHeight = m_nPercentWidth;

				// Calc. Y Dpi
				m_bPixAndPercentAreChanging = TRUE;
				FromPixToDpiY();
				m_bPixAndPercentAreChanging = FALSE;
			}

			UpdateData(FALSE);
		}
		m_bPhysOrDpiAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnChangeEditYdpi() 
{
	if (m_bDlgInitialized && !m_bPixAndPercentAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			// Calc. Pix And Percent Height
			FromPhysAndDpiToPixAndPercentY();

			if (m_bRetainAspect)
			{
				// Update Pix And Percent Width
				m_nPixelsWidth = Round(m_nPixelsHeight * m_dRatio);
				m_nPercentWidth = m_nPercentHeight;

				// Calc. X Dpi
				m_bPixAndPercentAreChanging = TRUE;
				FromPixToDpiX();
				m_bPixAndPercentAreChanging = FALSE;
			}

			UpdateData(FALSE);
		}
		m_bPhysOrDpiAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnSelchangeComboPhysUnit() 
{
	if (m_bDlgInitialized)
	{
		m_bPixAndPercentAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			// Note: the -0.1 is to have some margin,
			// otherwise also the Pix And Percent Checks will Popup!
			m_SpinPhysWidth.SetRange(0.1, MAX(0.1, GetMaxPhysWidth() - 0.1));
			m_SpinPhysHeight.SetRange(0.1, MAX(0.1, GetMaxPhysHeight() - 0.1));
			FromPixToPhys();
			UpdateData(FALSE);
		}
		m_bPixAndPercentAreChanging = FALSE;
	}
}

void CResizingDpiDlg::OnOK() 
{
	m_bNormalValidate = TRUE;
	CDialog::OnOK();
	m_bNormalValidate = FALSE;
	if (m_nPixelsWidth < 1)
		m_nPixelsWidth = 1;
	else if (m_nPixelsWidth > (int)UPPER_VALUE)
		m_nPixelsWidth = (int)UPPER_VALUE;
	if (m_nPixelsHeight < 1)
		m_nPixelsHeight = 1;
	else if (m_nPixelsHeight > (int)UPPER_VALUE)
		m_nPixelsHeight = (int)UPPER_VALUE;
	if (m_nXDpi < 1)
		m_nXDpi = 1;
	else if (m_nXDpi > (int)UPPER_VALUE)
		m_nXDpi = (int)UPPER_VALUE;
	if (m_nYDpi < 1)
		m_nYDpi = 1;
	else if (m_nYDpi > (int)UPPER_VALUE)
		m_nYDpi = (int)UPPER_VALUE;
	if (m_nPercentWidth < 1)
		m_nPercentWidth = 1;
	else if (m_nPercentWidth > (int)UPPER_VALUE)
		m_nPercentWidth = (int)UPPER_VALUE;
	if (m_nPercentHeight < 1)
		m_nPercentHeight = 1;
	else if (m_nPercentHeight > (int)UPPER_VALUE)
		m_nPercentHeight = (int)UPPER_VALUE;
}

void CResizingDpiDlg::OnCheckRetainAspect() 
{
	UpdateData(TRUE);
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_RETAIN_ASPECT);
	if (pCheck->GetCheck())
		Reset();
}
