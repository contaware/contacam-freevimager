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

// Multiple character literals like 'abcd' are of type int with the
// first character 'a' put into the most significant byte position
// and 'd' in the least significant position. The FCC macro reverts
// the order so that 'a' is the least significant byte and 'd' the
// most significant one (like in a string)
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
	m_nVideoCompressorKeyframesRate = DEFAULT_KEYFRAMESRATE;
	//}}AFX_DATA_INIT
	m_dwVideoCompressorFourCC = DEFAULT_VIDEO_FOURCC;
	m_fVideoCompressorQuality = DEFAULT_VIDEO_QUALITY;
	m_nFileType = FILETYPE_AVI;
}

void CVideoFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVideoFormatDlg)
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_QUALITY, m_VideoCompressorQuality);
	DDX_Control(pDX, IDC_VIDEO_COMPRESSION_CHOOSE, m_VideoCompressionChoose);
	DDX_Text(pDX, IDC_EDIT_KEYFRAMES_RATE, m_nVideoCompressorKeyframesRate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVideoFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CVideoFormatDlg)
	ON_CBN_SELCHANGE(IDC_VIDEO_COMPRESSION_CHOOSE, OnSelchangeVideoCompressionChoose)
	ON_WM_HSCROLL()
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
	
	// Quality
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY);
	if (m_VideoCompressionQualitySupport[m_VideoCompressionChoose.GetCurSel()])
	{
		pSlider->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_QUALITY);
		pEdit->ShowWindow(SW_SHOW);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_SHOW);
	}
	else
	{
		pSlider->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_QUALITY);
		pEdit->ShowWindow(SW_HIDE);
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEO_COMPRESSION_QUALITY_NUM);
		pEdit->ShowWindow(SW_HIDE);
	}
}

BOOL CVideoFormatDlg::OnInitDialog() 
{
	// Init Codec's Supports
	if (m_nFileType == FILETYPE_AVI)
	{
		m_VideoCompressionFcc.Add((DWORD)FCC('FFVH')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)0);

		m_VideoCompressionFcc.Add((DWORD)FCC('MJPG')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)0);
		m_VideoCompressionQualitySupport.Add((DWORD)1);

		m_VideoCompressionFcc.Add((DWORD)FCC('DIVX')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
		m_VideoCompressionQualitySupport.Add((DWORD)1);
	}
	else if (m_nFileType == FILETYPE_SWF)
	{
		m_VideoCompressionFcc.Add((DWORD)FCC('FLV1')); 
		m_VideoCompressionKeyframesRateSupport.Add((DWORD)1);
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
			m_dwVideoCompressorFourCC = DEFAULT_VIDEO_FOURCC;
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
		m_VideoCompressionChoose.AddString(_T("Huffman YUV 12 bits/pix"));
		m_VideoCompressionChoose.AddString(_T("Motion JPEG"));
		m_VideoCompressionChoose.AddString(_T("MPEG-4"));
	}
	else if (m_nFileType == FILETYPE_SWF)
		m_VideoCompressionChoose.AddString(_T("FLV1"));

	// Set Current Selection
	m_VideoCompressionChoose.SetCurSel(nSelection);

	// Show Hide Ctrl
	ShowHideCtrls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoFormatDlg::OnSelchangeVideoCompressionChoose() 
{
	UpdateData(TRUE);
	m_dwVideoCompressorFourCC = m_VideoCompressionFcc[m_VideoCompressionChoose.GetCurSel()];
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
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Below Slider
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