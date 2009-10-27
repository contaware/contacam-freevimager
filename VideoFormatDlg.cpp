// VideoFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoFormatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef FCC
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
				  (((DWORD)(ch4) & 0xFF00) << 8) |    \
				  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
				  (((DWORD)(ch4) & 0xFF000000) >> 24))
#endif

/////////////////////////////////////////////////////////////////////////////
// CVideoFormatDlg dialog


CVideoFormatDlg::CVideoFormatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoFormatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVideoFormatDlg)
	m_nVideoCompressorDataRate = DEFAULT_VIDEO_DATARATE / 1000;
	m_nVideoCompressorKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_bDeinterlace = FALSE;
	m_nRawCompressionIndex = 0;
	m_nQualityBitrate = 0;
	//}}AFX_DATA_INIT
	m_dwVideoCompressorFourCC = FCC('MJPG');
	m_fVideoCompressorQuality = DEFAULT_VIDEO_QUALITY;
	m_dVideoLength = 0.0;
	m_llTotalAudioBytes = 0;
	m_bShowDeinterlaceCheck = TRUE;
	m_bShowRawChoose = TRUE;
	m_nFileType = FILETYPE_AVI;
}

void CVideoFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVideoFormatDlg)
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoCompressorQuality);
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_CHOOSE, m_VideoCompressionChoose);
	DDX_Text(pDX, IDC_EDIT_DATARATE, m_nVideoCompressorDataRate);
	DDX_Text(pDX, IDC_EDIT_KEYFRAMES_RATE, m_nVideoCompressorKeyframesRate);
	DDX_Check(pDX, IDC_CHECK_DEINTERLACE, m_bDeinterlace);
	DDX_Radio(pDX, IDC_RADIO_RGB, m_nRawCompressionIndex);
	DDX_Radio(pDX, IDC_RADIO_QUALITY, m_nQualityBitrate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVideoFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CVideoFormatDlg)
	ON_CBN_SELCHANGE(IDC_VIDEO_COMPRESSION_CHOOSE, OnSelchangeVideoCompressionChoose)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_DATARATE, OnChangeEditDatarate)
	ON_BN_CLICKED(IDC_RADIO_QUALITY, OnRadioQuality)
	ON_BN_CLICKED(IDC_RADIO_BITRATE, OnRadioBitrate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoFormatDlg message handlers

void CVideoFormatDlg::ShowHideCtrls()
{
	// Keyframes Rate
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_KEYFRAMES_RATE);
	if (m_VideoCompressionKeyframesRateSupport[m_VideoCompressionChoose.GetCurSel()])
	{
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE0);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE1);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE0);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_KEYFRAMES_RATE1);
		pEdit->ShowWindow(SW_HIDE);
	}

	// Datarate
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DATARATE);
	if (m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()] &&
		(m_nQualityBitrate == 1													||
		!m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()]))
	{	
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE0);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE1);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE0);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_DATARATE1);
		pEdit->ShowWindow(SW_HIDE);
	}
	
	// Quality
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY);
	if (m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()]	&&
		(m_nQualityBitrate == 0													||
		!m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()]))
	{
		pSlider->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pSlider->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_HIDE);
	}
	
	// Total Data Length
	pEdit = (CEdit*)GetDlgItem(IDC_TOTAL_DATA);
	if (m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()]	&&
		m_nQualityBitrate == 1													&&
		m_dVideoLength > 0.0)
		pEdit->ShowWindow(SW_SHOW);
	else
		pEdit->ShowWindow(SW_HIDE);

	// Quality / Bitrate radio
	CButton* pRadio;
	if (m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()] &&
		m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()])
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_QUALITY);
		pRadio->ShowWindow(SW_SHOW);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_BITRATE);
		pRadio->ShowWindow(SW_SHOW);
	}
	else
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_QUALITY);
		pRadio->ShowWindow(SW_HIDE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_BITRATE);
		pRadio->ShowWindow(SW_HIDE);
	}

	// De-interlace check
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DEINTERLACE);
	pCheck->ShowWindow(m_bShowDeinterlaceCheck ? SW_SHOW : SW_HIDE);
	
	// Raw radio
	if (m_bShowRawChoose && m_dwVideoCompressorFourCC == BI_RGB)
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_RGB);
		pRadio->ShowWindow(SW_SHOW);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_I420);
		pRadio->ShowWindow(SW_SHOW);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_YV12);
		pRadio->ShowWindow(SW_SHOW);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_YUY2);
		pRadio->ShowWindow(SW_SHOW);
	}
	else
	{
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_RGB);
		pRadio->ShowWindow(SW_HIDE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_I420);
		pRadio->ShowWindow(SW_HIDE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_YV12);
		pRadio->ShowWindow(SW_HIDE);
		pRadio = (CButton*)GetDlgItem(IDC_RADIO_YUY2);
		pRadio->ShowWindow(SW_HIDE);
	}
}

void CVideoFormatDlg::UpdateLength()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TOTAL_DATA);
	if (m_VideoCompressionDataRateSupport[m_VideoCompressionChoose.GetCurSel()]	&& 
		m_dVideoLength > 0.0)
	{
		if (m_llTotalAudioBytes > 0)
		{
			CString sTotalData;
			double dVideoSize = m_dVideoLength * (double)m_nVideoCompressorDataRate * 1000.0 / 8.0; // kbps -> bytes
			if (dVideoSize >= 1048576.0)
			{
				sTotalData.Format(_T("%0.1fMB(video) + %0.1fMB(audio) = %0.1fMB(+1-5%%)"),
																		dVideoSize / 1048576.0,
																		(double)m_llTotalAudioBytes / 1048576.0,
																		dVideoSize / 1048576.0 + (double)m_llTotalAudioBytes / 1048576.0);
			}
			else if (dVideoSize >= 1024.0)
			{
				if (m_llTotalAudioBytes >= 1048576)
				{
					sTotalData.Format(_T("%dKB(video) + %0.1fMB(audio) = %0.1fMB(+1-5%%)"),
																		Round(dVideoSize / 1024.0),
																		(double)m_llTotalAudioBytes / 1048576.0,
																		dVideoSize / 1048576.0 + (double)m_llTotalAudioBytes / 1048576.0);
				}
				else
				{
					sTotalData.Format(_T("%dKB(video) + %dKB(audio) = %dKB(+1-5%%)"),
																		Round(dVideoSize / 1024.0),
																		(int)(m_llTotalAudioBytes >> 10),
																		Round(dVideoSize / 1024.0) + (int)(m_llTotalAudioBytes >> 10));
				}
			}
			else
			{
				if (m_llTotalAudioBytes >= 1048576)
				{
					sTotalData.Format(_T("%dB(video) + %0.1fMB(audio) = %0.1fMB(+1-5%%)"),
																		Round(dVideoSize),
																		(double)m_llTotalAudioBytes / 1048576.0,
																		dVideoSize / 1048576.0 + (double)m_llTotalAudioBytes / 1048576.0);
				}
				else if (m_llTotalAudioBytes >= 1024)
				{
					sTotalData.Format(_T("%dB(video) + %dKB(audio) = %dKB(+1-5%%)"),
																		Round(dVideoSize),
																		(int)(m_llTotalAudioBytes >> 10),
																		Round(dVideoSize / 1024.0) + (int)(m_llTotalAudioBytes >> 10));
				}
				else
				{
					sTotalData.Format(_T("%dB(video) + %dB(audio) = %dB(+1-5%%)"),
																		Round(dVideoSize),
																		(int)m_llTotalAudioBytes,
																		Round(dVideoSize) + (int)m_llTotalAudioBytes);
				}
			}
			pEdit->SetWindowText(sTotalData);

		}
		else
		{
			CString sTotalData;
			double dVideoSize = m_dVideoLength * (double)m_nVideoCompressorDataRate * 1000.0 / 8.0; // kbps -> bytes
			if (dVideoSize >= 1048576.0)
				sTotalData.Format(_T("%0.1fMB(+1-5%%)"), dVideoSize / 1048576.0);
			else if (dVideoSize >= 1024.0)
				sTotalData.Format(_T("%dKB(+1-5%%)"), Round(dVideoSize / 1024.0));
			else
				sTotalData.Format(_T("%dB(+1-5%%)"), Round(dVideoSize));
			pEdit->SetWindowText(sTotalData);
		}
	}
}

BOOL CVideoFormatDlg::OnInitDialog() 
{
	// Init Raw Type radio buttons
	if (m_bShowRawChoose)
	{
		if (m_dwVideoCompressorFourCC == BI_RGB)
		{
			m_nRawCompressionIndex = 0;
		}
		else if (m_dwVideoCompressorFourCC == FCC('I420'))
		{
			m_dwVideoCompressorFourCC = BI_RGB;
			m_nRawCompressionIndex = 1;
		}
		else if (m_dwVideoCompressorFourCC == FCC('YV12'))
		{
			m_dwVideoCompressorFourCC = BI_RGB;
			m_nRawCompressionIndex = 2;
		}
		else if (m_dwVideoCompressorFourCC == FCC('YUY2'))
		{
			m_dwVideoCompressorFourCC = BI_RGB;
			m_nRawCompressionIndex = 3;
		}
	}
	
	// Init Codec's Supports
	if (m_nFileType == FILETYPE_AVI)
	{
		m_VideoCompressionFcc.Add((DWORD)BI_RGB);
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionDataRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)0);
		
		m_VideoCompressionFcc.Add((DWORD)FCC('HFYU')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionDataRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)0);
		
		m_VideoCompressionFcc.Add((DWORD)FCC('FFVH')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionDataRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)0);
		
		m_VideoCompressionFcc.Add((DWORD)FCC('FFV1')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionDataRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)0);

		m_VideoCompressionFcc.Add((DWORD)FCC('MJPG')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionDataRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)1);

		if (((CUImagerApp*)::AfxGetApp())->m_bFFMpeg4VideoEnc)
		{
			m_VideoCompressionFcc.Add((DWORD)FCC('DIVX')); 
			m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
			m_VideoCompressionDataRateSupport.Add((DWORD)1);
			m_VideoCompressionQualitySupport.Add((DWORD)1);
		}
		
		if (((CUImagerApp*)::AfxGetApp())->m_bFFTheoraVideoEnc)
		{
			m_VideoCompressionFcc.Add((DWORD)FCC('theo')); 
			m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
			m_VideoCompressionDataRateSupport.Add((DWORD)1);
			m_VideoCompressionQualitySupport.Add((DWORD)1);
		}

		if (((CUImagerApp*)::AfxGetApp())->m_bFFSnowVideoEnc)
		{
			m_VideoCompressionFcc.Add((DWORD)FCC('SNOW')); 
			m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
			m_VideoCompressionDataRateSupport.Add((DWORD)1);
			m_VideoCompressionQualitySupport.Add((DWORD)1);
		}
	}
	else if (m_nFileType == FILETYPE_SWF)
	{
		m_VideoCompressionFcc.Add((DWORD)FCC('FLV1')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
		m_VideoCompressionDataRateSupport.Add((DWORD)1);
		m_VideoCompressionQualitySupport.Add((DWORD)1);
	}

	// Update Current Selected Codec
	int nSelection = -1;
	for (int i = 0 ; i < m_VideoCompressionFcc.GetSize() ; i++)
	{
		if (m_VideoCompressionFcc[i] == m_dwVideoCompressorFourCC)
		{
			nSelection = i;
			break;
		}
	}
	if (nSelection == -1)
	{
		if (m_nFileType == FILETYPE_AVI)
		{
			m_dwVideoCompressorFourCC = FCC('MJPG');
			for (int i = 0 ; i < m_VideoCompressionFcc.GetSize() ; i++)
			{
				if (m_VideoCompressionFcc[i] == m_dwVideoCompressorFourCC)
				{
					nSelection = i;
					break;
				}
			}
		}
		else if (m_nFileType == FILETYPE_SWF)
		{
			nSelection = 0;
			m_dwVideoCompressorFourCC = FCC('FLV1');
		}
	}

	// Update Quality / Bitrate selection radios
	if (m_VideoCompressionDataRateSupport[nSelection] &&
		!m_VideoCompressionQualitySupport[nSelection])
		m_nQualityBitrate = 1;
	else if (!m_VideoCompressionDataRateSupport[nSelection] &&
			m_VideoCompressionQualitySupport[nSelection])
		m_nQualityBitrate = 0;

	// This calls UpdateData(FALSE)
	CDialog::OnInitDialog();

	// Video Compressor Quality
	m_VideoCompressorQuality.SetRange(2, 31);
	m_VideoCompressorQuality.SetPageSize(5);
	m_VideoCompressorQuality.SetLineSize(1);
	m_VideoCompressorQuality.SetPos(33 - (int)(m_fVideoCompressorQuality)); // m_fVideoCompressorQuality has a range from 31.0f to 2.0f
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
	CString sQuality;
	sQuality.Format(_T("%i"), (int)((m_VideoCompressorQuality.GetPos() - 2) * 3.45)); // 0 .. 100
	pEdit->SetWindowText(sQuality);

	// Add Codec strings to ComboBox
	if (m_nFileType == FILETYPE_AVI)
	{
		m_VideoCompressionChoose.AddString(_T("Raw"));
		m_VideoCompressionChoose.AddString(_T("Huffman YUV 16 bits/pix"));
		m_VideoCompressionChoose.AddString(_T("Huffman YUV 12 bits/pix"));
		m_VideoCompressionChoose.AddString(_T("FFV1 Lossless YUV 12 bits/pix"));
		m_VideoCompressionChoose.AddString(_T("Motion JPEG"));
		if (((CUImagerApp*)::AfxGetApp())->m_bFFMpeg4VideoEnc)
			m_VideoCompressionChoose.AddString(_T("MPEG-4"));
		if (((CUImagerApp*)::AfxGetApp())->m_bFFTheoraVideoEnc)
			m_VideoCompressionChoose.AddString(_T("Theora"));
		if (((CUImagerApp*)::AfxGetApp())->m_bFFSnowVideoEnc)
			m_VideoCompressionChoose.AddString(_T("SNOW"));
	}
	else if (m_nFileType == FILETYPE_SWF)
		m_VideoCompressionChoose.AddString(_T("FLV1"));

	// Set Current Selection
	m_VideoCompressionChoose.SetCurSel(nSelection);

	// Update Length Display
	UpdateLength();

	// Show Hide Ctrl
	ShowHideCtrls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoFormatDlg::OnSelchangeVideoCompressionChoose() 
{
	UpdateData(TRUE);
	m_dwVideoCompressorFourCC = m_VideoCompressionFcc[m_VideoCompressionChoose.GetCurSel()];
	UpdateLength();
	ShowHideCtrls();
}

void CVideoFormatDlg::OnRadioQuality() 
{
	UpdateData(TRUE);
	ShowHideCtrls();
}

void CVideoFormatDlg::OnRadioBitrate() 
{
	UpdateData(TRUE);
	ShowHideCtrls();
}

void CVideoFormatDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;

		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pSlider->GetDlgCtrlID() == IDC_VIDEO_COMPRESSION_QUALITY)
			{
				m_fVideoCompressorQuality = (float)(33 - m_VideoCompressorQuality.GetPos());
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
				CString sQuality;
				sQuality.Format(_T("%i"), (int)((m_VideoCompressorQuality.GetPos() - 2) * 3.45)); // 0 .. 100
				pEdit->SetWindowText(sQuality);
			}
		}
	}
		
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVideoFormatDlg::OnChangeEditDatarate() 
{
	UpdateData(TRUE);
	UpdateLength();
}

void CVideoFormatDlg::OnOK() 
{
	CDialog::OnOK();
	if (m_dwVideoCompressorFourCC == BI_RGB && m_bShowRawChoose)
	{
		switch (m_nRawCompressionIndex)
		{
			case 0 : m_dwVideoCompressorFourCC = BI_RGB;
					break;
			case 1 : m_dwVideoCompressorFourCC = FCC('I420');
					break;
			case 2 : m_dwVideoCompressorFourCC = FCC('YV12');
					break;
			case 3 : m_dwVideoCompressorFourCC = FCC('YUY2');
					break;
			default : break;
		}
	}
}
