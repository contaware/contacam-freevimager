// AudioInSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AudioInSourceDlg.h"
#include "VideoDeviceDoc.h"
#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

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

	// Get number of devices
	UINT uiNumDev = ::waveInGetNumDevs(); 
	if (uiNumDev == 0)
	{
		::AfxMessageBox(ML_STRING(1354, "No Sound Input Device."));
		CDialog::OnCancel();
		return TRUE;
	}

	// Enumerate the devices
	for (UINT i = 0 ; i < uiNumDev ; i++)
		m_AudioInSource.AddString(CVideoDeviceDoc::CaptureAudioDeviceIDToName(i));

	// Init selection
	if (m_uiDeviceID > (uiNumDev - 1))
		m_AudioInSource.SetCurSel(-1); // no selection if m_uiDeviceID is not existing (maybe a USB device was removed)
	else
		m_AudioInSource.SetCurSel(m_uiDeviceID);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioInSourceDlg::OnOK()
{
	int nSel = m_AudioInSource.GetCurSel();
	if (nSel != CB_ERR) // nSel is CB_ERR if nothing selected
		m_uiDeviceID = nSel;
	CDialog::OnOK();
}

#endif