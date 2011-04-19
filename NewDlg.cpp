// NewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "NewDlg.h"
#include "Dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewDlg dialog

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define MIN_PIX_SIZE	1
#define MAX_PIX_SIZE	SHRT_MAX
#define MIN_DPI_SIZE	1
#define MAX_DPI_SIZE	SHRT_MAX
#define MIN_PHYS_SIZE	0.01
#define MAX_PHYS_SIZE	100000.0

CNewDlg::CNewDlg(	int nWidth, int nHeight,
					int nXDpi, int nYDpi,
					int nPhysUnit,
					const CString& sPaperSize/*=_T("")*/,
					COLORREF crBackgroundColor/*=RGB(255,255,255)*/,
					CWnd* pParent /*=NULL*/)
	: CDialog(CNewDlg::IDD, pParent)
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
	//{{AFX_DATA_INIT(CNewDlg)
	m_nPixelsWidth = nWidth;
	m_nPixelsHeight = nHeight;
	m_nXDpi = nXDpi;
	m_nYDpi = nYDpi;
	//}}AFX_DATA_INIT
	m_PhysUnit = nPhysUnit;
	m_sPaperSize = sPaperSize;
	m_crBackgroundColor = crBackgroundColor;
	m_nInitWidth = nWidth;
	m_nInitHeight = nHeight;
	m_nInitXDpi = nXDpi;
	m_nInitYDpi = nYDpi;
	m_bNormalValidate = FALSE;
	m_bPixAreChanging = FALSE;
	m_bPhysOrDpiAreChanging = FALSE;
	m_bPaperSizeChanging = FALSE;
}


void CNewDlg::DoDataExchange(CDataExchange* pDX)
{
	if (!m_bNormalValidate)
	{
		CString sText;
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_HEIGHT);
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

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewDlg)
	DDX_Control(pDX, IDC_PICK_COLOR, m_BackgroundColorButton);
	DDX_Control(pDX, IDC_EDIT_PHYS_HEIGHT, m_EditPhysHeight);
	DDX_Control(pDX, IDC_EDIT_PHYS_WIDTH, m_EditPhysWidth);
	DDX_Control(pDX, IDC_SPIN_PHYS_WIDTH, m_SpinPhysWidth);
	DDX_Control(pDX, IDC_SPIN_PHYS_HEIGHT, m_SpinPhysHeight);
	DDX_Text(pDX, IDC_EDIT_PIXELS_HEIGHT, m_nPixelsHeight);
	DDX_Text(pDX, IDC_EDIT_PIXELS_WIDTH, m_nPixelsWidth);
	DDX_CBIndex(pDX, IDC_COMBO_PHYS_UNIT, m_PhysUnit);
	DDX_Text(pDX, IDC_EDIT_XDPI, m_nXDpi);
	DDX_Text(pDX, IDC_EDIT_YDPI, m_nYDpi);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewDlg, CDialog)
	//{{AFX_MSG_MAP(CNewDlg)
	ON_EN_CHANGE(IDC_EDIT_PIXELS_HEIGHT, OnChangeEditPixelsHeight)
	ON_EN_CHANGE(IDC_EDIT_PIXELS_WIDTH, OnChangeEditPixelsWidth)
	ON_EN_CHANGE(IDC_EDIT_PHYS_HEIGHT, OnChangeEditPhysHeight)
	ON_EN_CHANGE(IDC_EDIT_PHYS_WIDTH, OnChangeEditPhysWidth)
	ON_EN_CHANGE(IDC_EDIT_XDPI, OnChangeEditXdpi)
	ON_EN_CHANGE(IDC_EDIT_YDPI, OnChangeEditYdpi)
	ON_CBN_SELCHANGE(IDC_COMBO_PHYS_UNIT, OnSelchangeComboPhysUnit)
	ON_CBN_SELENDOK(IDC_COMBO_PAPER_SIZE, OnSelendokComboPaperSize)
	ON_BN_CLICKED(IDC_PICK_COLOR, OnPickColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewDlg message handlers

void CNewDlg::FromPixToPhys()
{
	FromPixToPhysX();
	FromPixToPhysY();
}

void CNewDlg::FromPixToPhysX()
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

void CNewDlg::FromPixToPhysY()
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

void CNewDlg::FromPixToDpi()
{
	FromPixToDpiX();
	FromPixToDpiY();
}

void CNewDlg::FromPixToDpiX()
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

void CNewDlg::FromPixToDpiY()
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

void CNewDlg::FromPhysAndDpiToPix()
{
	FromPhysAndDpiToPixX();
	FromPhysAndDpiToPixY();
}

void CNewDlg::FromPhysAndDpiToPixX()
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
}

void CNewDlg::FromPhysAndDpiToPixY()
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
}

BOOL CNewDlg::OnInitDialog() 
{
	// Init Combo Boxes
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PHYS_UNIT);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1496, "cm"));
		pComboBox->AddString(ML_STRING(1497, "inch"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1564, "Custom"));
		pComboBox->AddString(_T("Executive"));
		pComboBox->AddString(_T("Letter"));
		pComboBox->AddString(_T("Legal"));
		pComboBox->AddString(_T("A4"));
		pComboBox->AddString(_T("A3"));
		pComboBox->AddString(_T("Ledger"));
		pComboBox->AddString(_T("Tabloid"));
		pComboBox->AddString(_T("Statement"));
		pComboBox->AddString(_T("Folio"));
		pComboBox->AddString(_T("Quarto"));
		pComboBox->AddString(_T("A3Extra"));
		pComboBox->AddString(_T("A4Extra"));
		pComboBox->AddString(_T("A"));
		pComboBox->AddString(_T("B"));
		pComboBox->AddString(_T("C"));
		pComboBox->AddString(_T("D"));
		pComboBox->AddString(_T("E"));
		pComboBox->AddString(_T("F"));
		pComboBox->AddString(_T("A10"));
		pComboBox->AddString(_T("A9"));
		pComboBox->AddString(_T("A8"));
		pComboBox->AddString(_T("A7"));
		pComboBox->AddString(_T("A6"));
		pComboBox->AddString(_T("A5"));
		pComboBox->AddString(_T("A2"));
		pComboBox->AddString(_T("A1"));
		pComboBox->AddString(_T("A0"));
		pComboBox->AddString(_T("B10"));
		pComboBox->AddString(_T("B9"));
		pComboBox->AddString(_T("B8"));
		pComboBox->AddString(_T("B7"));
		pComboBox->AddString(_T("B6"));
		pComboBox->AddString(_T("B5"));
		pComboBox->AddString(_T("B4"));
		pComboBox->AddString(_T("B3"));
		pComboBox->AddString(_T("B2"));
		pComboBox->AddString(_T("B1"));
		pComboBox->AddString(_T("B0"));
		pComboBox->AddString(_T("JISB10"));
		pComboBox->AddString(_T("JISB9"));
		pComboBox->AddString(_T("JISB8"));
		pComboBox->AddString(_T("JISB7"));
		pComboBox->AddString(_T("JISB6"));
		pComboBox->AddString(_T("JISB5"));
		pComboBox->AddString(_T("JISB4"));
		pComboBox->AddString(_T("JISB3"));
		pComboBox->AddString(_T("JISB2"));
		pComboBox->AddString(_T("JISB1"));
		pComboBox->AddString(_T("JISB0"));
		pComboBox->AddString(_T("C10"));
		pComboBox->AddString(_T("C9"));
		pComboBox->AddString(_T("C8"));
		pComboBox->AddString(_T("C7"));
		pComboBox->AddString(_T("C6"));
		pComboBox->AddString(_T("C5"));
		pComboBox->AddString(_T("C4"));
		pComboBox->AddString(_T("C3"));
		pComboBox->AddString(_T("C2"));
		pComboBox->AddString(_T("C1"));
		pComboBox->AddString(_T("C0"));
		pComboBox->AddString(_T("RA2"));
		pComboBox->AddString(_T("RA1"));
		pComboBox->AddString(_T("RA0"));
		pComboBox->AddString(_T("SRA4"));
		pComboBox->AddString(_T("SRA3"));
		pComboBox->AddString(_T("SRA2"));
		pComboBox->AddString(_T("SRA1"));
		pComboBox->AddString(_T("SRA0"));
	}

	CDialog::OnInitDialog();

	// Init Pix & Percent Spin Controls
	CSpinButtonCtrl* pSpin;
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS_WIDTH);
	pSpin->SetRange32(MIN_PIX_SIZE, MAX_PIX_SIZE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PIXELS_HEIGHT);
	pSpin->SetRange32(MIN_PIX_SIZE, MAX_PIX_SIZE);

	// Init DPI Spin Controls
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_XDPI);
	pSpin->SetRange32(MIN_DPI_SIZE, MAX_DPI_SIZE);
	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_YDPI);
	pSpin->SetRange32(MIN_DPI_SIZE, MAX_DPI_SIZE);

	// Init Physical Edit and Spin
	m_SpinPhysWidth.SetBuddy(&m_EditPhysWidth);
	m_SpinPhysWidth.SetRange(MIN_PHYS_SIZE, MAX_PHYS_SIZE);
	m_SpinPhysWidth.SetDelta(0.1);
	m_EditPhysWidth.SetMinNumberOfNumberAfterPoint(2);
	m_EditPhysWidth.SetMaxNumberOfNumberAfterPoint(2);
	m_SpinPhysHeight.SetBuddy(&m_EditPhysHeight);
	m_SpinPhysHeight.SetRange(MIN_PHYS_SIZE, MAX_PHYS_SIZE);
	m_SpinPhysHeight.SetDelta(0.1);
	m_EditPhysHeight.SetMinNumberOfNumberAfterPoint(2);
	m_EditPhysHeight.SetMaxNumberOfNumberAfterPoint(2);

	// Select Paper Size and set Physical Spins
	if (pComboBox)
	{
		if (m_sPaperSize == _T(""))
		{
			pComboBox->SetCurSel(0);
			FromPixToPhys();
		}
		else
		{
			int nIndex = pComboBox->FindStringExact(-1, m_sPaperSize);
			if (nIndex == CB_ERR)
				pComboBox->SetCurSel(0);
			else
				pComboBox->SetCurSel(nIndex);
			PaperNameToPhysSize();
		}
	}

	// Set Button Color
	m_BackgroundColorButton.SetColor(	CDib::HighlightColor(m_crBackgroundColor),
										m_crBackgroundColor);

	// OnInitDialog() has been called
	m_bDlgInitialized = TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewDlg::OnChangeEditPixelsWidth() 
{
	if (m_bDlgInitialized && !m_bPhysOrDpiAreChanging)
	{
		m_bPixAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			if (m_nPixelsHeight == 0) m_nPixelsHeight = 1;
			FromPixToPhys();
			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_WIDTH);
			CString sValue;
			sValue.Format(_T("%i"), m_nInitWidth);
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
		m_bPixAreChanging = FALSE;

		if (!m_bPaperSizeChanging)
		{
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
			if (pComboBox)
			{
				pComboBox->SetCurSel(0);
				m_sPaperSize = _T("");
			}
		}
	}
}

void CNewDlg::OnChangeEditPixelsHeight() 
{
	if (m_bDlgInitialized && !m_bPhysOrDpiAreChanging)
	{
		m_bPixAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			if (m_nPixelsWidth == 0) m_nPixelsWidth = 1;
			FromPixToPhys();
			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PIXELS_HEIGHT);
			CString sValue;
			sValue.Format(_T("%i"), m_nInitHeight);
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
		m_bPixAreChanging = FALSE;

		if (!m_bPaperSizeChanging)
		{
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
			if (pComboBox)
			{
				pComboBox->SetCurSel(0);
				m_sPaperSize = _T("");
			}
		}
	}
}

void CNewDlg::OnChangeEditPhysWidth() 
{
	if (m_bDlgInitialized && !m_bPixAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		
		UpdateData(TRUE);

		// Calc. Pix Width
		FromPhysAndDpiToPixX();

		UpdateData(FALSE);
		
		m_bPhysOrDpiAreChanging = FALSE;

		if (!m_bPaperSizeChanging)
		{
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
			if (pComboBox)
			{
				pComboBox->SetCurSel(0);
				m_sPaperSize = _T("");
			}
		}
	}
}

void CNewDlg::OnChangeEditPhysHeight() 
{
	if (m_bDlgInitialized && !m_bPixAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		
		UpdateData(TRUE);

		// Calc. Pix Height
		FromPhysAndDpiToPixY();
		
		UpdateData(FALSE);

		m_bPhysOrDpiAreChanging = FALSE;

		if (!m_bPaperSizeChanging)
		{
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
			if (pComboBox)
			{
				pComboBox->SetCurSel(0);
				m_sPaperSize = _T("");
			}
		}
	}
}

void CNewDlg::OnChangeEditXdpi() 
{
	if (m_bDlgInitialized && !m_bPixAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			// Calc. Pix Width
			FromPhysAndDpiToPixX();

			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_XDPI);
			CString sValue;
			sValue.Format(_T("%i"), m_nInitXDpi);
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
		m_bPhysOrDpiAreChanging = FALSE;
	}
}

void CNewDlg::OnChangeEditYdpi() 
{
	if (m_bDlgInitialized && !m_bPixAreChanging)
	{
		m_bPhysOrDpiAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			// Calc. Pix Height
			FromPhysAndDpiToPixY();

			UpdateData(FALSE);
		}
		else
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_YDPI);
			CString sValue;
			sValue.Format(_T("%i"), m_nInitYDpi);
			pEdit->SetWindowText(sValue);
			pEdit->SetFocus();
			pEdit->SetSel(0xFFFF0000);
		}
		m_bPhysOrDpiAreChanging = FALSE;
	}
}

void CNewDlg::OnSelchangeComboPhysUnit() 
{
	if (m_bDlgInitialized)
	{
		m_bPixAreChanging = TRUE;
		if (UpdateData(TRUE))
		{
			FromPixToPhys();
			UpdateData(FALSE);
		}
		m_bPixAreChanging = FALSE;
	}
}

void CNewDlg::OnOK() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pComboBox)
	{
		if (pComboBox->GetCurSel() <= 0)
			m_sPaperSize = _T("");
		else
			pComboBox->GetLBText(pComboBox->GetCurSel(), m_sPaperSize);
	}
	m_bNormalValidate = TRUE;
	CDialog::OnOK();
	m_bNormalValidate = FALSE;
	m_nPixelsWidth = MIN(MAX(MIN_PIX_SIZE,m_nPixelsWidth),MAX_PIX_SIZE);
	m_nPixelsHeight = MIN(MAX(MIN_PIX_SIZE,m_nPixelsHeight),MAX_PIX_SIZE);
	m_nXDpi = MIN(MAX(MIN_DPI_SIZE,m_nXDpi),MAX_DPI_SIZE);
	m_nYDpi = MIN(MAX(MIN_DPI_SIZE,m_nYDpi),MAX_DPI_SIZE);
}

/*
Executive
Letter
Legal
A4
A3
Ledger
Tabloid
Statement
Folio
Quarto
A3Extra
A4Extra
A
B
C
D
E
F
G
H
J
K
A10
A9
A8
A7
A6
A5
A2
A1
A0
2A0
4A0
2A
4A
B10
B9
B8
B7
B6
B5
B4
B3
B2
B1
B0
JISB10
JISB9
JISB8
JISB7
JISB6
JISB5
JISB4
JISB3
JISB2
JISB1
JISB0
C10
C9
C8
C7
C6
C5
C4
C3
C2
C1
C0
RA2
RA1
RA0
SRA4
SRA3
SRA2
SRA1
SRA0
*/
BOOL CNewDlg::PaperNameToPhysSize()
{
	const TCHAR* sizes[]={
		_T("EXECUTIVE"), _T("LETTER"), _T("LEGAL"), _T("LEDGER"), _T("TABLOID"), 
		_T("A"), _T("B"), _T("C"), _T("D"), _T("E"), _T("F"), _T("G"), _T("H"), _T("J"), _T("K"), 
		_T("A10"), _T("A9"), _T("A8"), _T("A7"), _T("A6"), _T("A5"), _T("A4"), _T("A3"), _T("A2"), _T("A1"), _T("A0"), 
		_T("2A0"), _T("4A0"), _T("2A"), _T("4A"), 
		_T("B10"), _T("B9"), _T("B8"), _T("B7"), _T("B6"), _T("B5"), _T("B4"), _T("B3"), _T("B2"), _T("B1"), _T("B0"), 
		_T("JISB10"), _T("JISB9"), _T("JISB8"), _T("JISB7"), _T("JISB6"), _T("JISB5"), _T("JISB4"), 
		_T("JISB3"), _T("JISB2"), _T("JISB1"), _T("JISB0"), 
		_T("C10"), _T("C9"), _T("C8"), _T("C7"), _T("C6"), _T("C5"), _T("C4"), _T("C3"), _T("C2"), _T("C1"), _T("C0"), 
		_T("RA2"), _T("RA1"), _T("RA0"), _T("SRA4"), _T("SRA3"), _T("SRA2"), _T("SRA1"), _T("SRA0"), 
		_T("A3EXTRA"), _T("A4EXTRA"), 
		_T("STATEMENT"), _T("FOLIO"), _T("QUARTO"), 
		NULL
	};
	const int inches[]={
		1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 
		1, 1, 1, 
		0
	};
	const int widths[]={
		522,612,612,792,792,
		612,792,1224,1584,2448,2016,792,2016,2448,2880,
		74,105,147,210,298,420,595,842,1191,1684,2384,
		3370,4768,3370,4768,
		88,125,176,249,354,499,709,1001,1417,2004,2835,
		91,128,181,258,363,516,729,
		1032,1460,2064,2920,
		79,113,162,230,323,459,649,918,1298,1298,2599,
		1219,1729,2438,638,907,1276,1814,2551,
		914,667,
		396, 612, 609, 
		0
	};
	const int heights[]={
		756,792,1008,1224,1224,
		792,1224,1584,2448,3168,2880,6480,10296,12672,10296,
		105,147,210,298,420,595,842,1191,1684,2384,3370,
		4768,6741,4768,6741,
		125,176,249,354,499,709,1001,1417,2004,2835,4008,
		128,181,258,363,516,729,1032,
		1460,2064,2920,4127,
		113,162,230,323,459,649,918,1298,1837,1837,3677,
		1729,2438,3458,907,1276,1814,2551,3628,
		1262,914,
		612, 936, 780, 
		0
	};
	ASSERT(sizeof(sizes) == sizeof(inches));
	ASSERT(sizeof(inches) == sizeof(widths));
	ASSERT(sizeof(widths) == sizeof(heights));

	for (int i = 0 ; sizes[i] != NULL ; i++)
	{
		if (_tcsicmp((const TCHAR*)m_sPaperSize, sizes[i]) == 0)
		{
			m_PhysUnit = inches[i];
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PHYS_UNIT);
			if (pComboBox)
				pComboBox->SetCurSel(m_PhysUnit);
			switch (m_PhysUnit)
			{
				// cm
				case 0:
				{
					m_bPaperSizeChanging = TRUE;
					// Round to millimeter precision
					double width = (double)Round((double)widths[i] * 25.4 / 72.0);
					double height = (double)Round((double)heights[i] * 25.4 / 72.0);
					m_SpinPhysWidth.SetPos(width / 10.0);
					m_SpinPhysHeight.SetPos(height / 10.0);
					m_bPaperSizeChanging = FALSE;
					return TRUE;
				}
				// inch
				case 1:
				{
					m_bPaperSizeChanging = TRUE;
					m_SpinPhysWidth.SetPos((double)widths[i] / 72.0);
					m_SpinPhysHeight.SetPos((double)heights[i] / 72.0);
					m_bPaperSizeChanging = FALSE;
					return TRUE;
				}
				default:
					return FALSE;
			}
		}
	}

	return FALSE;
}

void CNewDlg::OnSelendokComboPaperSize() 
{
	if (m_bDlgInitialized)
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PAPER_SIZE);
		if (pComboBox)
		{
			if (pComboBox->GetCurSel() <= 0)
				m_sPaperSize = _T("");
			else
			{
				pComboBox->GetLBText(pComboBox->GetCurSel(), m_sPaperSize);
				PaperNameToPhysSize();
			}
		}
	}
}

void CNewDlg::OnPickColor() 
{
	if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(m_crBackgroundColor, this))
	{
		m_BackgroundColorButton.SetColor(CDib::HighlightColor(m_crBackgroundColor),
										m_crBackgroundColor);
	}	
}
