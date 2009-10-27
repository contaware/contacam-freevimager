// AviOpenStreamsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AviOpenStreamsDlg.h"
#include "VideoAviDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAviOpenStreamsDlg dialog


CAviOpenStreamsDlg::CAviOpenStreamsDlg(CVideoAviDoc* pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CAviOpenStreamsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAviOpenStreamsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = pDoc;
}


void CAviOpenStreamsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAviOpenStreamsDlg)
	DDX_Control(pDX, IDC_VIDEO_STREAMS, m_VideoStreams);
	DDX_Control(pDX, IDC_AUDIO_STREAMS, m_AudioStreams);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAviOpenStreamsDlg, CDialog)
	//{{AFX_MSG_MAP(CAviOpenStreamsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAviOpenStreamsDlg message handlers

BOOL CAviOpenStreamsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	DWORD dwStreamNum;
	
	for (dwStreamNum = 0 ; dwStreamNum < m_pDoc->m_pAVIPlay->GetVideoStreamsCount() ; dwStreamNum++)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(dwStreamNum);
		if (pVideoStream)
			m_VideoStreams.AddString(pVideoStream->GetStreamName());
	}
	for (dwStreamNum = 0 ; dwStreamNum < m_pDoc->m_pAVIPlay->GetAudioStreamsCount() ; dwStreamNum++)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(dwStreamNum);
		if (pAudioStream)
			m_AudioStreams.AddString(pAudioStream->GetStreamName());
	}
	m_VideoStreams.SetCurSel(0);
	m_AudioStreams.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAviOpenStreamsDlg::OnOK()
{
	int nIndex;
	if (LB_ERR == (nIndex = m_VideoStreams.GetCurSel()))
		m_pDoc->m_nActiveVideoStream = -1;
	else
		m_pDoc->m_nActiveVideoStream = nIndex;
	if (LB_ERR == (nIndex = m_AudioStreams.GetCurSel()))
		m_pDoc->m_nActiveAudioStream = -1;
	else
		m_pDoc->m_nActiveAudioStream = nIndex;
	CDialog::OnOK();
}
