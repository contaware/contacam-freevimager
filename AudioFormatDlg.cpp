// AudioFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AviPlay.h"
#include "AudioFormatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef SUPPORT_LIBAVCODEC	

#define MPEGAUDIO_CTRL_OFFSET	120

/////////////////////////////////////////////////////////////////////////////
// CAudioFormatDlg dialog


CAudioFormatDlg::CAudioFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioFormatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioFormatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAudioFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioFormatDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioFormatDlg)
	ON_BN_CLICKED(IDC_RADIO_ADPCM, OnRadioAdpcm)
	ON_BN_CLICKED(IDC_RADIO_FLAC, OnRadioFlac)
	ON_BN_CLICKED(IDC_RADIO_VORBIS, OnRadioVorbis)
	ON_BN_CLICKED(IDC_RADIO_MP2, OnRadioMp2)
	ON_BN_CLICKED(IDC_RADIO_MP3, OnRadioMp3)
	ON_BN_CLICKED(IDC_RADIO_PCM, OnRadioPcm)
	ON_CBN_SELCHANGE(IDC_COMBO_PCM_BITS, OnSelchangeComboPcmBits)
	ON_CBN_SELCHANGE(IDC_COMBO_PCM_CHANNELS, OnSelchangeComboPcmChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_PCM_SAMPLINGRATE, OnSelchangeComboPcmSamplingrate)
	ON_CBN_SELCHANGE(IDC_COMBO_ADPCM_CHANNELS, OnSelchangeComboAdpcmChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_ADPCM_SAMPLINGRATE, OnSelchangeComboAdpcmSamplingrate)
	ON_CBN_SELCHANGE(IDC_COMBO_FLAC_CHANNELS, OnSelchangeComboFlacChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_FLAC_SAMPLINGRATE, OnSelchangeComboFlacSamplingrate)
	ON_CBN_SELCHANGE(IDC_COMBO_VORBIS_QUALITY, OnSelchangeComboVorbisQuality)
	ON_CBN_SELCHANGE(IDC_COMBO_VORBIS_CHANNELS, OnSelchangeComboVorbisChannels)
	ON_CBN_SELCHANGE(IDC_COMBO_VORBIS_SAMPLINGRATE, OnSelchangeComboVorbisSamplingrate)
	ON_CBN_SELCHANGE(IDC_COMBO_MP2_BITRATE, OnSelchangeComboMp2Bitrate)
	ON_CBN_SELCHANGE(IDC_COMBO_MP2_CHANNELS, OnSelchangeComboMp2Channels)
	ON_CBN_SELCHANGE(IDC_COMBO_MP2_SAMPLINGRATE, OnSelchangeComboMp2Samplingrate)
	ON_CBN_SELCHANGE(IDC_COMBO_MP3_BITRATE, OnSelchangeComboMp3Bitrate)
	ON_CBN_SELCHANGE(IDC_COMBO_MP3_CHANNELS, OnSelchangeComboMp3Channels)
	ON_CBN_SELCHANGE(IDC_COMBO_MP3_SAMPLINGRATE, OnSelchangeComboMp3Samplingrate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioFormatDlg message handlers


void CAudioFormatDlg::ResetAllCtrls() 
{
	CComboBox* pCombo;

	// Reset PCM
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_BITS);
	pCombo->SetCurSel(1);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
	pCombo->SetCurSel(1);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_SAMPLINGRATE);
	pCombo->SetCurSel(2);
		
	// Reset ADPCM
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
	pCombo->SetCurSel(1);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_SAMPLINGRATE);
	pCombo->SetCurSel(2);

	// Reset FLAC
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
	pCombo->SetCurSel(1);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_SAMPLINGRATE);
	pCombo->SetCurSel(3);

	// Reset VORBIS
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);	
	pCombo->SetCurSel(2);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_CHANNELS);
	pCombo->SetCurSel(0);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_SAMPLINGRATE);
	pCombo->SetCurSel(3);

	// Reset MP2
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);	
	pCombo->SetCurSel(0);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
	pCombo->SetCurSel(1);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
	pCombo->SetCurSel(0);

	// Reset MP3
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);	
	pCombo->SetCurSel(5);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
	pCombo->SetCurSel(1);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
	pCombo->SetCurSel(4);
}

void CAudioFormatDlg::CtrlsToWaveFormat()
{
	CComboBox* pCombo;
	if (m_WaveFormat.wFormatTag == WAVE_FORMAT_PCM)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_BITS);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.wBitsPerSample = 8;
						m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels;
						break;
			case 1  :	m_WaveFormat.wBitsPerSample = 16;
						m_WaveFormat.nBlockAlign = 2 * m_WaveFormat.nChannels;
						break;
			default :	ASSERT(FALSE);
						break;
		}
		

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nChannels = 1;
						break;
			case 1  :	m_WaveFormat.nChannels = 2;
						break;
			default :	ASSERT(FALSE);
						break;
		}
		
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_SAMPLINGRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 11025;
						break;
			case 2	:	m_WaveFormat.nSamplesPerSec = 22050;
						break;
			case 3	:	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 4	:	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 5	:	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}
		
		if (m_WaveFormat.wBitsPerSample == 8)
			m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels;
		else
			m_WaveFormat.nBlockAlign = 2 * m_WaveFormat.nChannels;
		m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_DVI_ADPCM)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nChannels = 1;
						break;
			case 1  :	m_WaveFormat.nChannels = 2;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_SAMPLINGRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 11025;
						break;
			case 2	:	m_WaveFormat.nSamplesPerSec = 22050;
						break;
			case 3	:	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 4	:	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 5	:	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		m_WaveFormat.wBitsPerSample = 4;
		m_WaveFormat.nBlockAlign = 0;	 // Calculated by AddAudioStream()

		// Guess nAvgBytesPerSec, calculated more precisely by codec!
		m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_FLAC)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nChannels = 1;
						break;
			case 1  :	m_WaveFormat.nChannels = 2;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_SAMPLINGRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 22050;
						break;
			case 2	:	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 3	:	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 4	:	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		m_WaveFormat.wBitsPerSample = 16;
		m_WaveFormat.nBlockAlign = 0;
		
		// Guess nAvgBytesPerSec = PCM size / 2.5
		m_WaveFormat.nAvgBytesPerSec = Round(2 * m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2.5);
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_VORBIS)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nAvgBytesPerSec = 60000 / 8;	// q = 10
						break;
			case 1  :	m_WaveFormat.nAvgBytesPerSec = 90000 / 8;	// q = 15;
						break;
			case 2  :	m_WaveFormat.nAvgBytesPerSec = 120000 / 8;	// q = 22;
						break;
			case 3  :	m_WaveFormat.nAvgBytesPerSec = 160000 / 8;	// q = 35;
						break;
			default :	ASSERT(FALSE);
						break;
		}
		
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_SAMPLINGRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 11025;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 22050;
						break;
			case 2  :	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 3  :	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 4  :	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		m_WaveFormat.nChannels = 2;
		m_WaveFormat.nBlockAlign = 0;
		m_WaveFormat.wBitsPerSample = 0;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_MPEG)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
						break;
			case 1  :	m_WaveFormat.nAvgBytesPerSec = 224000 / 8;
						break;
			case 2  :	m_WaveFormat.nAvgBytesPerSec = 256000 / 8;
						break;
			case 3  :	m_WaveFormat.nAvgBytesPerSec = 320000 / 8;
						break;
			case 4  :	m_WaveFormat.nAvgBytesPerSec = 384000 / 8;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
		if (pCombo->GetCount() == 2)
		{
			switch (pCombo->GetCurSel())
			{
				case 0  :	m_WaveFormat.nChannels = 1;
							break;
				case 1  :	m_WaveFormat.nChannels = 2;
							break;
				default :	ASSERT(FALSE);
							break;
			}
		}
		else
			m_WaveFormat.nChannels = 2;

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 2  :	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}

		m_WaveFormat.nBlockAlign = 0;
		m_WaveFormat.wBitsPerSample = 0;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_MPEGLAYER3)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nAvgBytesPerSec = 32000 / 8;
						break;
			case 1  :	m_WaveFormat.nAvgBytesPerSec = 56000 / 8;
						break;
			case 2  :	m_WaveFormat.nAvgBytesPerSec = 64000 / 8;
						break;
			case 3  :	m_WaveFormat.nAvgBytesPerSec = 96000 / 8;
						break;
			case 4  :	m_WaveFormat.nAvgBytesPerSec = 112000 / 8;
						break;
			case 5  :	m_WaveFormat.nAvgBytesPerSec = 128000 / 8;
						break;
			case 6  :	m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
						break;
			default :	ASSERT(FALSE);
						break;
		}
		
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nChannels = 1;
						break;
			case 1  :	m_WaveFormat.nChannels = 2;
						break;
			default :	ASSERT(FALSE);
						break;
		}
	
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
		if (pCombo->GetCount() == 6)
		{
			switch (pCombo->GetCurSel())
			{
				case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
							break;
				case 1  :	m_WaveFormat.nSamplesPerSec = 11025;
							break;
				case 2  :	m_WaveFormat.nSamplesPerSec = 22050;
							break;
				case 3  :	m_WaveFormat.nSamplesPerSec = 32000;
							break;
				case 4  :	m_WaveFormat.nSamplesPerSec = 44100;
							break;
				case 5  :	m_WaveFormat.nSamplesPerSec = 48000;
							break;
				default :	ASSERT(FALSE);
							break;
			}
		}
		else
		{
			switch (pCombo->GetCurSel())
			{
				case 0  :	m_WaveFormat.nSamplesPerSec = 32000;
							break;
				case 1  :	m_WaveFormat.nSamplesPerSec = 44100;
							break;
				case 2  :	m_WaveFormat.nSamplesPerSec = 48000;
							break;
				default :	ASSERT(FALSE);
							break;
			}
		}

		m_WaveFormat.nBlockAlign = 0;
		m_WaveFormat.wBitsPerSample = 0;
	}
}

void CAudioFormatDlg::WaveFormatToCtrls()
{
	CComboBox* pCombo;
	if (m_WaveFormat.wFormatTag == WAVE_FORMAT_DVI_ADPCM)
	{
		CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_ADPCM);
		pRadio->SetCheck(1);

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
		if (m_WaveFormat.nChannels == 1)
			pCombo->SetCurSel(0);
		else
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nChannels = 2;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_SAMPLINGRATE);
		if (m_WaveFormat.nSamplesPerSec <= 8000)
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nSamplesPerSec = 8000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 11025)
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nSamplesPerSec = 11025;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 22050)
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nSamplesPerSec = 22050;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 32000)
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nSamplesPerSec = 32000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 44100)
		{
			pCombo->SetCurSel(4);
			m_WaveFormat.nSamplesPerSec = 44100;
		}
		else
		{
			pCombo->SetCurSel(5);
			m_WaveFormat.nSamplesPerSec = 48000;
		}

		m_WaveFormat.wBitsPerSample = 4;
		m_WaveFormat.nBlockAlign = 0;	 // Calculated by AddAudioStream()

		// Guess nAvgBytesPerSec, calculated more precisely by codec!
		m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_FLAC)
	{
		CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_FLAC);
		pRadio->SetCheck(1);

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
		if (m_WaveFormat.nChannels == 1)
			pCombo->SetCurSel(0);
		else
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nChannels = 2;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_SAMPLINGRATE);
		if (m_WaveFormat.nSamplesPerSec <= 8000)
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nSamplesPerSec = 8000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 22050)
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nSamplesPerSec = 22050;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 32000)
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nSamplesPerSec = 32000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 44100)
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nSamplesPerSec = 44100;
		}
		else
		{
			pCombo->SetCurSel(4);
			m_WaveFormat.nSamplesPerSec = 48000;
		}

		m_WaveFormat.wBitsPerSample = 16;
		m_WaveFormat.nBlockAlign = 0;
		
		// Guess nAvgBytesPerSec = PCM size / 2.5
		m_WaveFormat.nAvgBytesPerSec = Round(2 * m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2.5);
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_VORBIS)
	{
		CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_VORBIS);
		pRadio->SetCheck(1);

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);
		if (m_WaveFormat.nAvgBytesPerSec <= (60000 / 8))
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nAvgBytesPerSec = 60000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (90000 / 8))
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nAvgBytesPerSec = 90000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (120000 / 8))
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nAvgBytesPerSec = 120000 / 8;
		}
		else
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nAvgBytesPerSec = 160000 / 8;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_CHANNELS);
		pCombo->SetCurSel(0);

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_SAMPLINGRATE);
		if (m_WaveFormat.nSamplesPerSec <= 11025)
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nSamplesPerSec = 11025;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 22050)
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nSamplesPerSec = 22050;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 32000)
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nSamplesPerSec = 32000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 44100)
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nSamplesPerSec = 44100;
		}
		else
		{
			pCombo->SetCurSel(4);
			m_WaveFormat.nSamplesPerSec = 48000;
		}

		m_WaveFormat.nChannels = 2;
		m_WaveFormat.nBlockAlign = 0;
		m_WaveFormat.wBitsPerSample = 0;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_MPEG && ((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
	{
		CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_MP2);
		pRadio->SetCheck(1);

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
		if (m_WaveFormat.nAvgBytesPerSec > (192000 / 8))
		{
			if (pCombo->GetCount() == 2)
				pCombo->DeleteString(0);
		}
		else
		{
			if (pCombo->GetCount() == 1)
				pCombo->InsertString(0, _T("Mono"));
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
		if (m_WaveFormat.nChannels == 1)
		{
			if (pCombo->GetCount() == 5)
			{
				pCombo->DeleteString(pCombo->GetCount() - 1);
				pCombo->DeleteString(pCombo->GetCount() - 1);
				pCombo->DeleteString(pCombo->GetCount() - 1);
				pCombo->DeleteString(pCombo->GetCount() - 1);
			}
		}
		else
		{
			if (pCombo->GetCount() == 1)
			{
				pCombo->AddString(_T("224 kBit/s"));
				pCombo->AddString(_T("256 kBit/s"));
				pCombo->AddString(_T("320 kBit/s"));
				pCombo->AddString(_T("384 kBit/s"));
			}
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
		if (m_WaveFormat.nAvgBytesPerSec <= (192000 / 8))
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (224000 / 8))
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nAvgBytesPerSec = 224000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (256000 / 8))
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nAvgBytesPerSec = 256000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (320000 / 8))
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nAvgBytesPerSec = 320000 / 8;
		}
		else
		{
			pCombo->SetCurSel(4);
			m_WaveFormat.nAvgBytesPerSec = 384000 / 8;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
		if (pCombo->GetCount() == 2)
		{
			if (m_WaveFormat.nChannels == 1)
				pCombo->SetCurSel(0);
			else
			{
				pCombo->SetCurSel(1);
				m_WaveFormat.nChannels = 2;
			}
		}
		else
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nChannels = 2;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
		if (m_WaveFormat.nSamplesPerSec <= 32000)
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nSamplesPerSec = 32000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 44100)
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nSamplesPerSec = 44100;
		}
		else
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nSamplesPerSec = 48000;
		}

		m_WaveFormat.nBlockAlign = 0;
		m_WaveFormat.wBitsPerSample = 0;
	}
	else if (m_WaveFormat.wFormatTag == WAVE_FORMAT_MPEGLAYER3 && ((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
	{
		CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_MP3);
		pRadio->SetCheck(1);

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
		if (m_WaveFormat.nAvgBytesPerSec >= (192000 / 8))
		{
			if (pCombo->GetCount() == 6)
			{
				pCombo->DeleteString(0);
				pCombo->DeleteString(0);
				pCombo->DeleteString(0);
			}
		}
		else
		{
			if (pCombo->GetCount() == 3)
			{
				pCombo->InsertString(0, _T("22.050 KHz"));
				pCombo->InsertString(0, _T("11.025 KHz"));
				pCombo->InsertString(0, _T("8 KHz"));
			}
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
		if (m_WaveFormat.nSamplesPerSec <= 22050)
		{
			if (pCombo->GetCount() == 7)
				pCombo->DeleteString(pCombo->GetCount() - 1); // 192 kbits not supported
		}
		else
		{
			if (pCombo->GetCount() == 6)
				pCombo->AddString(_T("192 kBit/s")); // Add 192 kbits
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
		if (m_WaveFormat.nAvgBytesPerSec <= (32000 / 8))
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nAvgBytesPerSec = 32000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (56000 / 8))
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nAvgBytesPerSec = 56000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (64000 / 8))
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nAvgBytesPerSec = 64000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (96000 / 8))
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nAvgBytesPerSec = 96000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec <= (112000 / 8))
		{
			pCombo->SetCurSel(4);
			m_WaveFormat.nAvgBytesPerSec = 112000 / 8;
		}
		else if (m_WaveFormat.nAvgBytesPerSec < (192000 / 8))
		{
			pCombo->SetCurSel(5);
			m_WaveFormat.nAvgBytesPerSec = 128000 / 8;
		}
		else
		{
			pCombo->SetCurSel(6);
			m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
		if (m_WaveFormat.nChannels == 1)
			pCombo->SetCurSel(0);
		else
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nChannels = 2;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
		if (pCombo->GetCount() == 6)
		{
			if (m_WaveFormat.nSamplesPerSec <= 8000)
			{
				pCombo->SetCurSel(0);
				m_WaveFormat.nSamplesPerSec = 8000;
			}
			else if (m_WaveFormat.nSamplesPerSec <= 11025)
			{
				pCombo->SetCurSel(1);
				m_WaveFormat.nSamplesPerSec = 11025;
			}
			else if (m_WaveFormat.nSamplesPerSec <= 22050)
			{
				pCombo->SetCurSel(2);
				m_WaveFormat.nSamplesPerSec = 22050;
			}
			else if (m_WaveFormat.nSamplesPerSec <= 32000)
			{
				pCombo->SetCurSel(3);
				m_WaveFormat.nSamplesPerSec = 32000;
			}
			else if (m_WaveFormat.nSamplesPerSec <= 44100)
			{
				pCombo->SetCurSel(4);
				m_WaveFormat.nSamplesPerSec = 44100;
			}
			else
			{
				pCombo->SetCurSel(5);
				m_WaveFormat.nSamplesPerSec = 48000;
			}
		}
		else
		{
			if (m_WaveFormat.nSamplesPerSec <= 32000)
			{
				pCombo->SetCurSel(0);
				m_WaveFormat.nSamplesPerSec = 32000;
			}
			else if (m_WaveFormat.nSamplesPerSec <= 44100)
			{
				pCombo->SetCurSel(1);
				m_WaveFormat.nSamplesPerSec = 44100;
			}
			else
			{
				pCombo->SetCurSel(2);
				m_WaveFormat.nSamplesPerSec = 48000;
			}
		}

		m_WaveFormat.nBlockAlign = 0;
		m_WaveFormat.wBitsPerSample = 0;
	}
	// Default to: WAVE_FORMAT_PCM
	else
	{
		CButton* pRadio = (CButton*)GetDlgItem(IDC_RADIO_PCM);
		pRadio->SetCheck(1);
		m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_BITS);
		if (m_WaveFormat.wBitsPerSample == 8)
			pCombo->SetCurSel(0);
		else
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.wBitsPerSample = 16;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
		if (m_WaveFormat.nChannels == 1)
			pCombo->SetCurSel(0);
		else
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nChannels = 2;
		}

		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_SAMPLINGRATE);
		if (m_WaveFormat.nSamplesPerSec <= 8000)
		{
			pCombo->SetCurSel(0);
			m_WaveFormat.nSamplesPerSec = 8000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 11025)
		{
			pCombo->SetCurSel(1);
			m_WaveFormat.nSamplesPerSec = 11025;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 22050)
		{
			pCombo->SetCurSel(2);
			m_WaveFormat.nSamplesPerSec = 22050;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 32000)
		{
			pCombo->SetCurSel(3);
			m_WaveFormat.nSamplesPerSec = 32000;
		}
		else if (m_WaveFormat.nSamplesPerSec <= 44100)
		{
			pCombo->SetCurSel(4);
			m_WaveFormat.nSamplesPerSec = 44100;
		}
		else
		{
			pCombo->SetCurSel(5);
			m_WaveFormat.nSamplesPerSec = 48000;
		}

		if (m_WaveFormat.wBitsPerSample == 8)
			m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels;
		else
			m_WaveFormat.nBlockAlign = 2 * m_WaveFormat.nChannels;
		m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;
	}
}

BOOL CAudioFormatDlg::OnInitDialog() 
{
	CComboBox* pComboBox;
	CButton* pButton;
	CButton* pRadio;
	CRect rc;
	
	// Init Combo Boxes
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1541, "Low"));
		pComboBox->AddString(ML_STRING(1542, "Medium"));
		pComboBox->AddString(ML_STRING(1543, "Good"));
		pComboBox->AddString(ML_STRING(1544, "Best"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1545, "Mono"));
		pComboBox->AddString(ML_STRING(1546, "Stereo"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1545, "Mono"));
		pComboBox->AddString(ML_STRING(1546, "Stereo"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1545, "Mono"));
		pComboBox->AddString(ML_STRING(1546, "Stereo"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_CHANNELS);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1546, "Stereo"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1545, "Mono"));
		pComboBox->AddString(ML_STRING(1546, "Stereo"));
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1545, "Mono"));
		pComboBox->AddString(ML_STRING(1546, "Stereo"));
	}
	
	// Hide unwanted controls
	if (!((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_MP2);
		if (pRadio)
			pRadio->ShowWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
		if (pComboBox)
			pComboBox->ShowWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
		if (pComboBox)
			pComboBox->ShowWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
		if (pComboBox)
			pComboBox->ShowWindow(FALSE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_MP3);
		if (pRadio)
			pRadio->ShowWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
		if (pComboBox)
			pComboBox->ShowWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
		if (pComboBox)
			pComboBox->ShowWindow(FALSE);
		pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
		if (pComboBox)
			pComboBox->ShowWindow(FALSE);
		pButton = (CButton*)GetDlgItem(IDCANCEL);
		if (pButton)
		{
			pButton->GetWindowRect(&rc);
			ScreenToClient(&rc);
			pButton->SetWindowPos(NULL, rc.left, rc.top - MPEGAUDIO_CTRL_OFFSET, 0, 0, SWP_NOSIZE);
		}
		pButton = (CButton*)GetDlgItem(IDOK);
		if (pButton)
		{
			pButton->GetWindowRect(&rc);
			ScreenToClient(&rc);
			pButton->SetWindowPos(NULL, rc.left, rc.top - MPEGAUDIO_CTRL_OFFSET, 0, 0, SWP_NOSIZE);
		}
		GetWindowRect(&rc);
		SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height() - MPEGAUDIO_CTRL_OFFSET, SWP_NOMOVE);
	}

	CDialog::OnInitDialog();

	m_WaveFormat.cbSize = 0;
	ResetAllCtrls();
	WaveFormatToCtrls();
	EnableDisableCtrls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioFormatDlg::OnRadioPcm() 
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	CtrlsToWaveFormat();
	EnableDisableCtrls();
}

void CAudioFormatDlg::OnSelchangeComboPcmBits()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_BITS);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.wBitsPerSample = 8;
					m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels;
					break;
		case 1  :	m_WaveFormat.wBitsPerSample = 16;
					m_WaveFormat.nBlockAlign = 2 * m_WaveFormat.nChannels;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;
}

void CAudioFormatDlg::OnSelchangeComboPcmChannels() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nChannels = 1;
					break;
		case 1  :	m_WaveFormat.nChannels = 2;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	if (m_WaveFormat.wBitsPerSample == 8)
		m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels;
	else
		m_WaveFormat.nBlockAlign = 2 * m_WaveFormat.nChannels;
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;
}

void CAudioFormatDlg::OnSelchangeComboPcmSamplingrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_SAMPLINGRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
					break;
		case 1  :	m_WaveFormat.nSamplesPerSec = 11025;
					break;
		case 2	:	m_WaveFormat.nSamplesPerSec = 22050;
					break;
		case 3	:	m_WaveFormat.nSamplesPerSec = 32000;
					break;
		case 4	:	m_WaveFormat.nSamplesPerSec = 44100;
					break;
		case 5	:	m_WaveFormat.nSamplesPerSec = 48000;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;
}

void CAudioFormatDlg::OnRadioAdpcm() 
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_DVI_ADPCM;
	CtrlsToWaveFormat();
	EnableDisableCtrls();
}


void CAudioFormatDlg::OnSelchangeComboAdpcmChannels() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nChannels = 1;
					break;
		case 1  :	m_WaveFormat.nChannels = 2;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	// Guess nAvgBytesPerSec, calculated more precisely by codec!
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2;
}

void CAudioFormatDlg::OnSelchangeComboAdpcmSamplingrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_SAMPLINGRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
					break;
		case 1  :	m_WaveFormat.nSamplesPerSec = 11025;
					break;
		case 2	:	m_WaveFormat.nSamplesPerSec = 22050;
					break;
		case 3	:	m_WaveFormat.nSamplesPerSec = 32000;
					break;
		case 4	:	m_WaveFormat.nSamplesPerSec = 44100;
					break;
		case 5	:	m_WaveFormat.nSamplesPerSec = 48000;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	// Guess nAvgBytesPerSec, calculated more precisely by codec!
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2;
}

void CAudioFormatDlg::OnRadioFlac() 
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_FLAC;
	CtrlsToWaveFormat();
	EnableDisableCtrls();
}

void CAudioFormatDlg::OnSelchangeComboFlacChannels() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nChannels = 1;
					break;
		case 1  :	m_WaveFormat.nChannels = 2;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	// Guess nAvgBytesPerSec = PCM size / 2.5
	m_WaveFormat.nAvgBytesPerSec = Round(2 * m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2.5);
}

void CAudioFormatDlg::OnSelchangeComboFlacSamplingrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_SAMPLINGRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
					break;
		case 1  :	m_WaveFormat.nSamplesPerSec = 22050;
					break;
		case 2	:	m_WaveFormat.nSamplesPerSec = 32000;
					break;
		case 3	:	m_WaveFormat.nSamplesPerSec = 44100;
					break;
		case 4	:	m_WaveFormat.nSamplesPerSec = 48000;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	// Guess nAvgBytesPerSec = PCM size / 2.5
	m_WaveFormat.nAvgBytesPerSec = Round(2 * m_WaveFormat.nSamplesPerSec * m_WaveFormat.nChannels / 2.5);
}

void CAudioFormatDlg::OnRadioVorbis() 
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_VORBIS;
	CtrlsToWaveFormat();
	EnableDisableCtrls();
}

void CAudioFormatDlg::OnSelchangeComboVorbisQuality() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nAvgBytesPerSec = 60000 / 8;	// q = 10
					break;
		case 1  :	m_WaveFormat.nAvgBytesPerSec = 90000 / 8;	// q = 15;
					break;
		case 2  :	m_WaveFormat.nAvgBytesPerSec = 120000 / 8;	// q = 22;
					break;
		case 3  :	m_WaveFormat.nAvgBytesPerSec = 160000 / 8;	// q = 35;
					break;
		default :	ASSERT(FALSE);
					break;
	}
}

void CAudioFormatDlg::OnSelchangeComboVorbisChannels() 
{
	m_WaveFormat.nChannels = 2;
}

void CAudioFormatDlg::OnSelchangeComboVorbisSamplingrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_SAMPLINGRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nSamplesPerSec = 11025;
					break;
		case 1  :	m_WaveFormat.nSamplesPerSec = 22050;
					break;
		case 2  :	m_WaveFormat.nSamplesPerSec = 32000;
					break;
		case 3  :	m_WaveFormat.nSamplesPerSec = 44100;
					break;
		case 4  :	m_WaveFormat.nSamplesPerSec = 48000;
					break;
		default :	ASSERT(FALSE);
					break;
	}
}

void CAudioFormatDlg::OnRadioMp2() 
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_MPEG;
	CtrlsToWaveFormat();
	EnableDisableCtrls();
}

void CAudioFormatDlg::OnSelchangeComboMp2Bitrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
					break;
		case 1  :	m_WaveFormat.nAvgBytesPerSec = 224000 / 8;
					break;
		case 2  :	m_WaveFormat.nAvgBytesPerSec = 256000 / 8;
					break;
		case 3  :	m_WaveFormat.nAvgBytesPerSec = 320000 / 8;
					break;
		case 4  :	m_WaveFormat.nAvgBytesPerSec = 384000 / 8;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
	if (m_WaveFormat.nAvgBytesPerSec > (192000 / 8))
	{
		if (pCombo->GetCount() == 2)
		{
			pCombo->DeleteString(0);
			pCombo->SetCurSel(0);
			m_WaveFormat.nChannels = 2;
		}
	}
	else
	{
		if (pCombo->GetCount() == 1)
		{
			pCombo->InsertString(0, _T("Mono"));
			pCombo->SetCurSel(1);
		}
	}
}

void CAudioFormatDlg::OnSelchangeComboMp2Channels() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
	if (pCombo->GetCount() == 2)
	{
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nChannels = 1;
						break;
			case 1  :	m_WaveFormat.nChannels = 2;
						break;
			default :	ASSERT(FALSE);
						break;
		}
	}
	else
		m_WaveFormat.nChannels = 2;

	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
	if (m_WaveFormat.nChannels == 1)
	{
		if (pCombo->GetCount() == 5)
		{
			pCombo->DeleteString(pCombo->GetCount() - 1);
			pCombo->DeleteString(pCombo->GetCount() - 1);
			pCombo->DeleteString(pCombo->GetCount() - 1);
			pCombo->DeleteString(pCombo->GetCount() - 1);
			pCombo->SetCurSel(0);
			m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
		}
	}
	else
	{
		if (pCombo->GetCount() == 1)
		{
			pCombo->AddString(_T("224 kBit/s"));
			pCombo->AddString(_T("256 kBit/s"));
			pCombo->AddString(_T("320 kBit/s"));
			pCombo->AddString(_T("384 kBit/s"));
			pCombo->SetCurSel(0);
		}
	}
}

void CAudioFormatDlg::OnSelchangeComboMp2Samplingrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nSamplesPerSec = 32000;
					break;
		case 1  :	m_WaveFormat.nSamplesPerSec = 44100;
					break;
		case 2  :	m_WaveFormat.nSamplesPerSec = 48000;
					break;
		default :	ASSERT(FALSE);
					break;
	}
}

void CAudioFormatDlg::OnRadioMp3() 
{
	m_WaveFormat.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
	CtrlsToWaveFormat();
	EnableDisableCtrls();
}

void CAudioFormatDlg::OnSelchangeComboMp3Bitrate()
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nAvgBytesPerSec = 32000 / 8;
					break;
		case 1  :	m_WaveFormat.nAvgBytesPerSec = 56000 / 8;
					break;
		case 2  :	m_WaveFormat.nAvgBytesPerSec = 64000 / 8;
					break;
		case 3  :	m_WaveFormat.nAvgBytesPerSec = 96000 / 8;
					break;
		case 4  :	m_WaveFormat.nAvgBytesPerSec = 112000 / 8;
					break;
		case 5  :	m_WaveFormat.nAvgBytesPerSec = 128000 / 8;
					break;
		case 6  :	m_WaveFormat.nAvgBytesPerSec = 192000 / 8;
					break;
		default :	ASSERT(FALSE);
					break;
	}
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
	if (m_WaveFormat.nAvgBytesPerSec == (192000 / 8))
	{
		if (pCombo->GetCount() == 6)
		{
			int nSel = pCombo->GetCurSel();
			pCombo->DeleteString(0);
			pCombo->DeleteString(0);
			pCombo->DeleteString(0);
			nSel -= 3;
			if (nSel < 0)
			{
				nSel = 0;
				m_WaveFormat.nSamplesPerSec = 32000;
			}
			pCombo->SetCurSel(nSel);
		}
	}
	else
	{
		if (pCombo->GetCount() == 3)
		{
			int nSel = pCombo->GetCurSel();
			pCombo->InsertString(0, _T("22.050 KHz"));
			pCombo->InsertString(0, _T("11.025 KHz"));
			pCombo->InsertString(0, _T("8 KHz"));
			nSel += 3;
			pCombo->SetCurSel(nSel);
		}
	}
}

void CAudioFormatDlg::OnSelchangeComboMp3Channels() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
	switch (pCombo->GetCurSel())
	{
		case 0  :	m_WaveFormat.nChannels = 1;
					break;
		case 1  :	m_WaveFormat.nChannels = 2;
					break;
		default :	ASSERT(FALSE);
					break;
	}
}

void CAudioFormatDlg::OnSelchangeComboMp3Samplingrate() 
{
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
	if (pCombo->GetCount() == 6)
	{
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 8000;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 11025;
						break;
			case 2  :	m_WaveFormat.nSamplesPerSec = 22050;
						break;
			case 3  :	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 4  :	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 5  :	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}
	}
	else
	{
		switch (pCombo->GetCurSel())
		{
			case 0  :	m_WaveFormat.nSamplesPerSec = 32000;
						break;
			case 1  :	m_WaveFormat.nSamplesPerSec = 44100;
						break;
			case 2  :	m_WaveFormat.nSamplesPerSec = 48000;
						break;
			default :	ASSERT(FALSE);
						break;
		}
	}
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
	if (m_WaveFormat.nSamplesPerSec <= 22050)
	{		
		if (pCombo->GetCount() == 7)
		{
			int nSel = pCombo->GetCurSel();
			pCombo->DeleteString(pCombo->GetCount() - 1); // 192 kbits not supported
			if (nSel > pCombo->GetCount() - 1)
			{
				nSel--;
				m_WaveFormat.nAvgBytesPerSec = 128000 / 8;
			}
			pCombo->SetCurSel(nSel);
		}
	}
	else
	{
		if (pCombo->GetCount() == 6)
		{
			int nSel = pCombo->GetCurSel();
			pCombo->AddString(_T("192 kBit/s")); // Add 192 kbits
			pCombo->SetCurSel(nSel);
		}
	}
}

void CAudioFormatDlg::EnableDisableCtrls()
{
	CComboBox* pCombo;
	CButton* pRadio;
	CEdit* pEdit;
	
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_PCM);
	if (pRadio->GetCheck() == 1)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_BITS);
		pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
		pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_SAMPLINGRATE);
		pCombo->EnableWindow(TRUE);
	}
	else
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_BITS);
		pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_CHANNELS);
		pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_PCM_SAMPLINGRATE);
		pCombo->EnableWindow(FALSE);
	}

	pRadio = (CButton*)GetDlgItem(IDC_RADIO_ADPCM);
	if (pRadio->GetCheck() == 1)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
		pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_SAMPLINGRATE);
		pCombo->EnableWindow(TRUE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_4BITS);
		pEdit->EnableWindow(TRUE);
	}
	else
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_CHANNELS);
		pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_ADPCM_SAMPLINGRATE);
		pCombo->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_4BITS);
		pEdit->EnableWindow(FALSE);
	}

	pRadio = (CButton*)GetDlgItem(IDC_RADIO_FLAC);
	if (pRadio->GetCheck() == 1)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
		pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_SAMPLINGRATE);
		pCombo->EnableWindow(TRUE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_16BITS);
		pEdit->EnableWindow(TRUE);
	}
	else
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_CHANNELS);
		pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_FLAC_SAMPLINGRATE);
		pCombo->EnableWindow(FALSE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_16BITS);
		pEdit->EnableWindow(FALSE);
	}

	pRadio = (CButton*)GetDlgItem(IDC_RADIO_VORBIS);
	if (pRadio->GetCheck() == 1)
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);
		pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_CHANNELS);
		pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_SAMPLINGRATE);
		pCombo->EnableWindow(TRUE);
	}
	else
	{
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_QUALITY);
		pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_CHANNELS);
		pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_VORBIS_SAMPLINGRATE);
		pCombo->EnableWindow(FALSE);
	}

	if (((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_MP2);
		if (pRadio->GetCheck() == 1)
		{
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
			pCombo->EnableWindow(TRUE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
			pCombo->EnableWindow(TRUE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
			pCombo->EnableWindow(TRUE);
		}
		else
		{
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_BITRATE);
			pCombo->EnableWindow(FALSE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_CHANNELS);
			pCombo->EnableWindow(FALSE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP2_SAMPLINGRATE);
			pCombo->EnableWindow(FALSE);
		}

		pRadio = (CButton*)GetDlgItem(IDC_RADIO_MP3);
		if (pRadio->GetCheck() == 1)
		{
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
			pCombo->EnableWindow(TRUE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
			pCombo->EnableWindow(TRUE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
			pCombo->EnableWindow(TRUE);
		}
		else
		{
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_BITRATE);
			pCombo->EnableWindow(FALSE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_CHANNELS);
			pCombo->EnableWindow(FALSE);
			pCombo = (CComboBox*)GetDlgItem(IDC_COMBO_MP3_SAMPLINGRATE);
			pCombo->EnableWindow(FALSE);
		}
	}
}

#endif