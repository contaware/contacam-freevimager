// AviSaveAsStreamsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AviSaveAsStreamsDlg.h"
#include "AVIPlay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAviSaveAsStreamsDlg dialog


CAviSaveAsStreamsDlg::CAviSaveAsStreamsDlg(	CAVIPlay* pAVIPlay,
											int nDlgID,
											CDWordArray* pVideoStreamsSave,
											CDWordArray* pVideoStreamsChange,
											CDWordArray* pAudioStreamsSave,
											CDWordArray* pAudioStreamsChange,
											CWnd* pParent /*=NULL*/)
	: CDialog(nDlgID, pParent)
{
	//{{AFX_DATA_INIT(CAviSaveAsStreamsDlg)
	//}}AFX_DATA_INIT
	m_pAVIPlay = pAVIPlay;
	m_bUseVfwCodecs = FALSE;
	m_pVideoStreamsSave = pVideoStreamsSave;
	m_pVideoStreamsChange = pVideoStreamsChange;
	m_pAudioStreamsSave = pAudioStreamsSave;
	m_pAudioStreamsChange = pAudioStreamsChange;
}

void CAviSaveAsStreamsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAviSaveAsStreamsDlg)
	DDX_Control(pDX, IDC_STREAM_SAVE, m_StreamSave);
	DDX_Control(pDX, IDC_STREAM_CHANGE, m_StreamChange);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAviSaveAsStreamsDlg, CDialog)
	//{{AFX_MSG_MAP(CAviSaveAsStreamsDlg)
	ON_LBN_SELCHANGE(IDC_STREAM_SAVE, OnSelchangeStreamSave)
	ON_LBN_SELCHANGE(IDC_STREAM_CHANGE, OnSelchangeStreamChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAviSaveAsStreamsDlg message handlers

BOOL CAviSaveAsStreamsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int nIndex;
	DWORD dwStreamNum;
	
	for (dwStreamNum = 0 ; dwStreamNum < m_pAVIPlay->GetVideoStreamsCount() ; dwStreamNum++)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(dwStreamNum);
		if (pVideoStream)
		{
			nIndex = m_StreamSave.AddString(pVideoStream->GetStreamName());
			m_StreamSave.SetSel(nIndex, TRUE);
			m_StreamChange.AddString(pVideoStream->GetStreamName());
		}
	}
	for (dwStreamNum = 0 ; dwStreamNum < m_pAVIPlay->GetAudioStreamsCount() ; dwStreamNum++)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pAVIPlay->GetAudioStream(dwStreamNum);
		if (pAudioStream)
		{
			nIndex = m_StreamSave.AddString(pAudioStream->GetStreamName());
			m_StreamSave.SetSel(nIndex, TRUE);
			m_StreamChange.AddString(pAudioStream->GetStreamName());
		}
	}

	CButton* pRadioCodecInternal = (CButton*)GetDlgItem(IDC_RADIO_CODEC_INTERNAL);
	CButton* pRadioCodecVFW = (CButton*)GetDlgItem(IDC_RADIO_CODEC_VFW);
	if (pRadioCodecInternal && pRadioCodecVFW)
	{
		pRadioCodecInternal->SetCheck(m_bUseVfwCodecs ? 0 : 1);
		pRadioCodecVFW->SetCheck(m_bUseVfwCodecs ? 1 : 0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAviSaveAsStreamsDlg::OnOK() 
{
	DWORD dwIndexVideo;
	DWORD dwIndexAudio;
	for (dwIndexVideo = 0 ; dwIndexVideo < m_pAVIPlay->GetVideoStreamsCount() ; dwIndexVideo++)
	{
		if (m_StreamSave.GetSel(dwIndexVideo))
		{
			m_pVideoStreamsSave->Add(1);
			if (m_StreamChange.GetSel(dwIndexVideo) > 0)
				m_pVideoStreamsChange->Add(1);
			else
				m_pVideoStreamsChange->Add(0);
		}
		else
		{
			m_pVideoStreamsSave->Add(0);
			m_pVideoStreamsChange->Add(0);
		}
	}
	for (dwIndexAudio = dwIndexVideo ; dwIndexAudio < (m_pAVIPlay->GetAudioStreamsCount() + dwIndexVideo) ; dwIndexAudio++)
	{
		if (m_StreamSave.GetSel(dwIndexAudio))
		{
			m_pAudioStreamsSave->Add(1);
			if (m_StreamChange.GetSel(dwIndexAudio) > 0)
				m_pAudioStreamsChange->Add(1);
			else
				m_pAudioStreamsChange->Add(0);
		}
		else
		{
			m_pAudioStreamsSave->Add(0);
			m_pAudioStreamsChange->Add(0);
		}
	}

	CButton* pRadioCodecVFW = (CButton*)GetDlgItem(IDC_RADIO_CODEC_VFW);
	if (pRadioCodecVFW)
		m_bUseVfwCodecs = pRadioCodecVFW->GetCheck() == 1 ? TRUE : FALSE;

	CDialog::OnOK();
}

void CAviSaveAsStreamsDlg::OnSelchangeStreamSave() 
{
	DWORD dwIndexVideo;
	DWORD dwIndexAudio;
	for (dwIndexVideo = 0 ; dwIndexVideo < m_pAVIPlay->GetVideoStreamsCount() ; dwIndexVideo++)
	{
		if (!m_StreamSave.GetSel(dwIndexVideo))
			m_StreamChange.SetSel(dwIndexVideo, FALSE);
	}
	for (dwIndexAudio = dwIndexVideo ; dwIndexAudio < (m_pAVIPlay->GetAudioStreamsCount() + dwIndexVideo) ; dwIndexAudio++)
	{
		if (!m_StreamSave.GetSel(dwIndexAudio))
			m_StreamChange.SetSel(dwIndexAudio, FALSE);
	}
}

void CAviSaveAsStreamsDlg::OnSelchangeStreamChange() 
{
	DWORD dwIndexVideo;
	DWORD dwIndexAudio;
	for (dwIndexVideo = 0 ; dwIndexVideo < m_pAVIPlay->GetVideoStreamsCount() ; dwIndexVideo++)
	{
		if (m_StreamChange.GetSel(dwIndexVideo) && !m_StreamSave.GetSel(dwIndexVideo))
			m_StreamChange.SetSel(dwIndexVideo, FALSE);
	}
	for (dwIndexAudio = dwIndexVideo ; dwIndexAudio < (m_pAVIPlay->GetAudioStreamsCount() + dwIndexVideo) ; dwIndexAudio++)
	{
		if (m_StreamChange.GetSel(dwIndexAudio) && !m_StreamSave.GetSel(dwIndexAudio))
			m_StreamChange.SetSel(dwIndexAudio, FALSE);
	}	
}
