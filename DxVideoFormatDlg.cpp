// DxVideoFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "DxVideoFormatDlg.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "PostDelayedMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CDxVideoFormatDlg dialog


CDxVideoFormatDlg::CDxVideoFormatDlg(CVideoDeviceDoc* pDoc)
	: CDialog(CDxVideoFormatDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CDxVideoFormatDlg)
	//}}AFX_DATA_INIT
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
	m_bChangingFormat = FALSE;
}


void CDxVideoFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDxVideoFormatDlg)
	DDX_Control(pDX, IDC_COMBO_VIDEOSIZE, m_VideoSize);
	DDX_Control(pDX, IDC_COMBO_VIDEOCOMPRESSION, m_VideoCompression);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDxVideoFormatDlg, CDialog)
	//{{AFX_MSG_MAP(CDxVideoFormatDlg)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DX_APPLY_VIDEOFORMAT_CHANGE, OnApplyVideoFormatChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDxVideoFormatDlg message handlers

BOOL CDxVideoFormatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Get Current Format
	AM_MEDIA_TYPE* pmtConfig = NULL;
	BOOL res = FALSE;
	if (m_pDoc->m_pDxCapture)
		res = m_pDoc->m_pDxCapture->GetCurrentFormat(&pmtConfig);

	// If Ok
	if (res)
	{
		// Compression
		int nCompressionsCount = 0;
		if (m_pDoc->m_pDxCapture)
			nCompressionsCount = m_pDoc->m_pDxCapture->EnumFormatCompressions(	m_Compressions,
																				m_Bits,	
																				m_CompressionStrings);
		for (int nCompression = 0 ; nCompression < nCompressionsCount ; nCompression++)
		{
			CString sCompressionDescription =
				CDxCapture::GetFormatCompressionDescription(m_Compressions[nCompression]);
			if (sCompressionDescription != _T(""))
				m_VideoCompression.AddString(m_CompressionStrings[nCompression] +
											_T(" , ") + sCompressionDescription);
			else
				m_VideoCompression.AddString(m_CompressionStrings[nCompression]);
		}

		// Size
		int nSizesCount = 0;
		if (m_pDoc->m_pDxCapture)
			nSizesCount = m_pDoc->m_pDxCapture->EnumFormatSizes(m_Sizes);
		for (int nSize = 0 ; nSize < nSizesCount ; nSize++)
		{
			CString sSize;
			sSize.Format(_T("%i x %i"), m_Sizes[nSize].cx, m_Sizes[nSize].cy);
			CString sFormatName = CDxCapture::FormatSizesToName(m_Sizes[nSize]);
			if (sFormatName != _T(""))
				m_VideoSize.AddString(sSize + _T(" , ") + sFormatName);
			else
				m_VideoSize.AddString(sSize);
		}

		// Set Selection
		FormatToCurSel(pmtConfig);

		// Clean-Up
		DeleteMediaType(pmtConfig);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDxVideoFormatDlg::ReStart()
{
	if (m_pDoc->m_pDxCapture->Run())
	{
		// Some devices need that...
		// Process frame must still be stopped when calling Dx Stop()!
		m_pDoc->m_pDxCapture->Stop();
		m_pDoc->m_pDxCapture->Run();

		// Restart process frame
		m_pDoc->StartProcessFrame(PROCESSFRAME_DXFORMATDIALOG);
	}
}

void CDxVideoFormatDlg::Apply()
{
	AM_MEDIA_TYPE* pmtConfig;
	AM_MEDIA_TYPE* pmtTrueConfig;

	if (m_pDoc->m_pDxCapture)
	{
		// Stop
		m_pDoc->m_pDxCapture->Stop();

		// Get Format ID corresponding to the selected compression and bpp
		int nID = -1;
		if (m_VideoCompression.GetCurSel() >= 0)
		{
			nID = m_pDoc->m_pDxCapture->GetFormatID(m_Compressions[m_VideoCompression.GetCurSel()],
													m_Bits[m_VideoCompression.GetCurSel()]);
		}
		else
		{
			ReStart();
			return;
		}

		// Set Size
		if (!m_pDoc->m_pDxCapture->GetFormatByID(nID, &pmtConfig))
		{
			ReStart();
			return;
		}
		VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
		LPBITMAPINFOHEADER lpBmiH = NULL;
		if (pVih)
		{
			lpBmiH = &pVih->bmiHeader;
			if (m_VideoSize.GetCurSel() >= 0)
			{
				lpBmiH->biWidth = m_Sizes[m_VideoSize.GetCurSel()].cx;
				lpBmiH->biHeight = m_Sizes[m_VideoSize.GetCurSel()].cy;
			}
			lpBmiH->biSizeImage = DWALIGNEDWIDTHBYTES(lpBmiH->biWidth * lpBmiH->biBitCount) * lpBmiH->biHeight;

			// Frame Rate
			if (m_pDoc->m_dFrameRate > 0.0)
				pVih->AvgTimePerFrame = (REFERENCE_TIME)(10000000.0 / m_pDoc->m_dFrameRate + 0.5);

			// Reset BitRate, driver will calculate it
			pVih->dwBitRate = 0;

			// Update Sample Size
			pmtConfig->lSampleSize = lpBmiH->biSizeImage;

			// Set Format
			m_pDoc->m_pDxCapture->SetCurrentFormat(pmtConfig);
		}

		// Update ComboBoxes if selected format is not supported!
		if (!m_pDoc->m_pDxCapture->GetCurrentFormat(&pmtTrueConfig))
		{
			DeleteMediaType(pmtConfig);
			ReStart();
			return;
		}
		VIDEOINFOHEADER* pTrueVih = (VIDEOINFOHEADER*)pmtTrueConfig->pbFormat;
		if (lpBmiH && pTrueVih)
		{
			LPBITMAPINFOHEADER lpTrueBmiH = &pTrueVih->bmiHeader;
			if ((lpTrueBmiH->biCompression != lpBmiH->biCompression)	||
				(lpTrueBmiH->biWidth != lpBmiH->biWidth)				||
				(lpTrueBmiH->biHeight != lpBmiH->biHeight)				||
				(lpTrueBmiH->biBitCount != lpBmiH->biBitCount))
			{
				FormatToCurSel(pmtTrueConfig);
			}
		}

		// Update
		m_pDoc->m_bSizeToDoc = TRUE;
		m_pDoc->OnChangeVideoFormat();
		m_pDoc->m_nDeviceFormatId = nID;
		m_pDoc->m_nDeviceFormatWidth = m_Sizes[m_VideoSize.GetCurSel()].cx;
		m_pDoc->m_nDeviceFormatHeight = m_Sizes[m_VideoSize.GetCurSel()].cy;

		// Restart
		ReStart();
		
		// Clean-Up
		DeleteMediaType(pmtConfig);
		DeleteMediaType(pmtTrueConfig);
	}
}

void CDxVideoFormatDlg::FormatToCurSel(AM_MEDIA_TYPE* pmtConfig)
{
	int nVideoCompressionIndex;
	int nVideoSizeIndex;
	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
	if (pVih)
	{
		LPBITMAPINFOHEADER lpBmiH = &pVih->bmiHeader;
		for (nVideoCompressionIndex = 0 ; nVideoCompressionIndex < m_Compressions.GetSize() ; nVideoCompressionIndex++)
		{
			if ((m_Compressions[nVideoCompressionIndex] == lpBmiH->biCompression) && 
				(m_Bits[nVideoCompressionIndex] == lpBmiH->biBitCount))
				break;
		}
		for (nVideoSizeIndex = 0 ; nVideoSizeIndex < m_Sizes.GetSize() ; nVideoSizeIndex++)
		{
			if ((m_Sizes[nVideoSizeIndex].cx == lpBmiH->biWidth) && 
				(m_Sizes[nVideoSizeIndex].cy == lpBmiH->biHeight))
				break;
		}
		if (nVideoCompressionIndex < m_VideoCompression.GetCount())
			m_VideoCompression.SetCurSel(nVideoCompressionIndex);
		else
			m_VideoCompression.SetCurSel(-1);
		if (nVideoSizeIndex < m_VideoSize.GetCount())
			m_VideoSize.SetCurSel(nVideoSizeIndex);
		else
			m_VideoSize.SetCurSel(-1);
	}
	else
		m_VideoSize.SetCurSel(-1);
}

void CDxVideoFormatDlg::OnOK()
{
	// Do not call 2 or more times!
	if (!m_bChangingFormat)
	{
		m_bChangingFormat = TRUE;
		m_pDoc->StopProcessFrame(PROCESSFRAME_DXFORMATDIALOG);
		double dFrameRate = m_pDoc->m_dEffectiveFrameRate;
		int delay;
		if (dFrameRate >= 1.0)
			delay = Round(1000.0 / dFrameRate); // In ms
		else
			delay = 1000;
		CPostDelayedMessageThread::PostDelayedMessage(	GetSafeHwnd(),
														WM_DX_APPLY_VIDEOFORMAT_CHANGE,
														delay,
														(WPARAM)TRUE,	// Call OnOK() when done
														delay);
	}
}

void CDxVideoFormatDlg::OnApply() 
{
	// Do not call 2 or more times!
	if (!m_bChangingFormat)
	{
		m_bChangingFormat = TRUE;
		m_pDoc->StopProcessFrame(PROCESSFRAME_DXFORMATDIALOG);
		double dFrameRate = m_pDoc->m_dEffectiveFrameRate;
		int delay;
		if (dFrameRate >= 1.0)
			delay = Round(1000.0 / dFrameRate); // In ms
		else
			delay = 1000;
		CPostDelayedMessageThread::PostDelayedMessage(	GetSafeHwnd(),
														WM_DX_APPLY_VIDEOFORMAT_CHANGE,
														delay,
														(WPARAM)FALSE,	// Do not call OnOK() when done
														delay);
	}
}

LONG CDxVideoFormatDlg::OnApplyVideoFormatChange(WPARAM wparam, LPARAM lparam)
{
	if (m_pDoc->m_bClosing)
	{
		CDialog::OnCancel();
		return 1;
	}
	else
	{
		int nPrevTotalDelay = (int)lparam;
		if (nPrevTotalDelay > PROCESSFRAME_MAX_RETRY_TIME || m_pDoc->IsProcessFrameStopped(PROCESSFRAME_DXFORMATDIALOG))
		{
			BOOL bCallOnOK = (BOOL)wparam;
			Apply();
			m_bChangingFormat = FALSE;
			if (bCallOnOK)
				CDialog::OnOK();
			return 1;
		}
		else
		{
			double dFrameRate = m_pDoc->m_dEffectiveFrameRate;
			int delay;
			if (dFrameRate >= 1.0)
				delay = Round(1000.0 / dFrameRate); // In ms
			else
				delay = 1000;
			CPostDelayedMessageThread::PostDelayedMessage(	GetSafeHwnd(),
															WM_DX_APPLY_VIDEOFORMAT_CHANGE,
															delay,
															wparam,
															nPrevTotalDelay + delay);
			return 0;
		}
	}
}

#endif
