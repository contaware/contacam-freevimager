// AudioInSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AudioInSourceDlg.h"
#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioInSourceDlg dialog


CAudioInSourceDlg::CAudioInSourceDlg(UINT uiDeviceID)
	: CDialog(CAudioInSourceDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CAudioInSourceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_uiDeviceID = uiDeviceID;
}


void CAudioInSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioInSourceDlg)
	DDX_Control(pDX, IDC_COMBO_AUDIOIN_SOURCE, m_AudioInSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioInSourceDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioInSourceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioInSourceDlg message handlers

BOOL CAudioInSourceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	MMRESULT res;

	// Get Number of Audio Devices
	m_uiNumDev = ::waveInGetNumDevs(); 
	if (m_uiNumDev == 0)
	{
		::AfxMessageBox(ML_STRING(1354, "No Sound Input Device."));
		return TRUE;
	}

	m_Devices.SetSize(m_uiNumDev);

	// Enumerate The Devices
	WAVEINCAPS DevCaps;
	for (UINT i = 0 ; i < m_uiNumDev ; i++)
	{
		res = ::waveInGetDevCaps(i, &DevCaps, sizeof(WAVEINCAPS));
		if (res != MMSYSERR_NOERROR)
		{
			::AfxMessageBox(ML_STRING(1355, "Sound Input Cannot Determine Card Capabilities!"));
			m_Devices[i] = _T("Unknown Device");
		}
		else
		{
			m_Devices.SetAt(i, (CString)DevCaps.szPname); 
		}
		m_AudioInSource.AddString(m_Devices[i]);
	}

	if (m_uiDeviceID > (m_uiNumDev - 1))
		m_uiDeviceID = 0;
	 m_AudioInSource.SelectString(0, m_Devices[m_uiDeviceID]);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioInSourceDlg::OnOK()
{
	m_uiDeviceID = m_AudioInSource.GetCurSel();
	CDialog::OnOK();
}
