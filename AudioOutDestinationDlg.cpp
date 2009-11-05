// AudioOutDestinationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AudioOutDestinationDlg.h"
#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioOutDestinationDlg dialog


CAudioOutDestinationDlg::CAudioOutDestinationDlg(CWnd* pParent, UINT uiDeviceID)
	: CDialog(CAudioOutDestinationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioOutDestinationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_uiDeviceID = uiDeviceID;
}


void CAudioOutDestinationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioOutDestinationDlg)
	DDX_Control(pDX, IDC_COMBO_AUDIOOUT_DESTINATION, m_AudioOutDestination);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioOutDestinationDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioOutDestinationDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioOutDestinationDlg message handlers

BOOL CAudioOutDestinationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	MMRESULT res;

	// Get Number of Audio Devices
	m_uiNumDev = ::waveOutGetNumDevs(); 
	if (m_uiNumDev == 0)
	{
		::AfxMessageBox(ML_STRING(1426, "No Sound Output Device."));
		CDialog::OnCancel();
		return TRUE;
	}

	m_Devices.SetSize(m_uiNumDev);

	// Enumerate The Devices
	WAVEOUTCAPS DevCaps;
	for (UINT i = 0 ; i < m_uiNumDev ; i++)
	{
		res = ::waveOutGetDevCaps(i, &DevCaps, sizeof(WAVEOUTCAPS));
		if (res != MMSYSERR_NOERROR)
		{
			::AfxMessageBox(ML_STRING(1356, "Sound Output Cannot Determine Card Capabilities!"));
			m_Devices[i] = _T("Unknown Device");
		}
		else
		{
			m_Devices.SetAt(i, (CString)DevCaps.szPname); 
		}
		m_AudioOutDestination.AddString(m_Devices[i]);
	}

	if (m_uiDeviceID > (m_uiNumDev - 1))
		m_uiDeviceID = 0;
	 m_AudioOutDestination.SelectString(0, m_Devices[m_uiDeviceID]);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioOutDestinationDlg::OnOK()
{
	m_uiDeviceID = m_AudioOutDestination.GetCurSel();
	CDialog::OnOK();
}
