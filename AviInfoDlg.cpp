// AviInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AviInfoDlg.h"
#include "VideoAviView.h"
#include "VideoAviDoc.h"
#include "AviPlay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAviInfoDlg dialog


CAviInfoDlg::CAviInfoDlg(CWnd* pParent)
	: CDialog(CAviInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAviInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CAviInfoDlg::IDD, pParent);
}

void CAviInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAviInfoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAviInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CAviInfoDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAviInfoDlg message handlers

BOOL CAviInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateDisplay();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAviInfoDlg::UpdateDisplay()
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	DWORD dwStreamNum;

	if (pDoc->m_pAVIPlay)
	{
		CString s, t;

		// File Info

		// File Name
		t.Format(ML_STRING(1307, "File: %s\r\n"), ::GetShortFileName(pDoc->m_sFileName));
		s+=t;

		// Path
		t.Format(ML_STRING(1309, "Path: %s\r\n"), ::GetDriveAndDirName(pDoc->m_sFileName));
		s+=t;

		// Get File Time
		CFileStatus FileStatus;
		BOOL bFileTimesOk = ::GetFileStatus(pDoc->m_sFileName, FileStatus);
		if (bFileTimesOk)
		{
			t =	ML_STRING(1310, "Date: ") +
				::MakeDateLocalFormat(FileStatus.m_mtime) +
				_T(" ") +
				::MakeTimeLocalFormat(FileStatus.m_mtime, TRUE) +
				_T("\r\n");
		}
		else
		{
			t = ML_STRING(1311, "Error Reading Timestamp\r\n");
		}
		s+=t;

		// File Size
		ULARGE_INTEGER FileSize = pDoc->m_pAVIPlay->GetFileSize();
		if (FileSize.QuadPart >= (1024*1024*1024))
			t.Format(ML_STRING(1824, "File Size: ") + CString(_T("%0.2f ") + ML_STRING(1826, "GB")), (double)(LONGLONG)FileSize.QuadPart / 1073741824.0);
		else if (FileSize.QuadPart >= (1024*1024))
			t.Format(ML_STRING(1824, "File Size: ") + CString(_T("%0.1f ") + ML_STRING(1825, "MB")), (double)(LONGLONG)FileSize.QuadPart / 1048576.0);
		else if (FileSize.QuadPart >= 1024)
			t.Format(ML_STRING(1824, "File Size: ") + CString(_T("%d ") + ML_STRING(1243, "KB")), (int)FileSize.QuadPart >> 10);
		else
			t.Format(ML_STRING(1824, "File Size: ") + CString(_T("%d ") + ML_STRING(1244, "Bytes")), (int)FileSize.QuadPart);
		s+=t;
		
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FILEINFO);
		pEdit->SetWindowText(s);


		// Video

		s = _T("");
		for (dwStreamNum = 0 ; dwStreamNum < pDoc->m_pAVIPlay->GetVideoStreamsCount() ; dwStreamNum++)
		{
			CAVIPlay::CAVIVideoStream* pVideoStream =
						pDoc->m_pAVIPlay->GetVideoStream(dwStreamNum);
			if (!pVideoStream)
				return;

			// Time Lengths
			double dLength		= pVideoStream->GetTotalTime();							// Total Length in Seconds
			int nLengthHour		= (int)(dLength / 3600.0);								// Hours Part
			int nLengthMin		= (int)((dLength - nLengthHour * 3600.0) / 60.0);		// Minutes Part
			double dLengthSec	= dLength - nLengthHour * 3600.0 - nLengthMin * 60.0;	// Seconds Part

			// Spaces
			if (dwStreamNum > 0)
				s += _T("\r\n\r\n");

			// Stream Number
			if (dwStreamNum == (DWORD)pDoc->m_nActiveVideoStream)
				t.Format(_T("Video Stream %d (Active):\r\n"), dwStreamNum);
			else
				t.Format(_T("Video Stream %d:\r\n"), dwStreamNum);
			s += t;

			// Name & FourCC
			if (pVideoStream->IsRgb() ||	// Uncompressed RGB
				pVideoStream->IsYuv())		// YUV Color Space Encoded
			{
				t.Format(_T("    Format: %s\r\n"),
					CDib::GetCompressionName(pVideoStream->GetFormat(true)));
			}
			else
			{
#ifdef SUPPORT_LIBAVCODEC
				if (pVideoStream->IsUsingAVCodec())
				{
					CString sFormat(_T(""));
					switch (pVideoStream->GetAVCodecCtx()->pix_fmt)
					{
						case PIX_FMT_YUVJ420P :
							sFormat = _T("Planar YUV 4:2:0, 12bpp, full scale (jpeg)");
							break;
						case PIX_FMT_YUVJ422P :
							sFormat = _T("Planar YUV 4:2:2, 16bpp, full scale (jpeg)");
							break;
						case PIX_FMT_YUVJ444P :
							sFormat = _T("Planar YUV 4:4:4, 24bpp, full scale (jpeg)");
							break;
						case PIX_FMT_YUV420P :
							sFormat = _T("Planar YUV 4:2:0, 12bpp");
							break;
						case PIX_FMT_YUYV422 : //	Note: PIX_FMT_YUV422 = PIX_FMT_YUYV422
							sFormat = _T("Packed YUV 4:2:2, 16bpp");
							break;
						case PIX_FMT_YUV422P :
							sFormat = _T("Planar YUV 4:2:2, 16bpp");
							break;
						case PIX_FMT_YUV410P :
							sFormat = _T("Planar YUV 4:1:0, 9bpp, (1 Cr & Cb sample per 4x4 Y samples)");
							break;
						case PIX_FMT_YUV411P :
							sFormat = _T("Planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)");
							break;
						case PIX_FMT_PAL8 :
							sFormat = _T("RGB8 with Palette");
							break;
						case PIX_FMT_RGB565 :
							sFormat = _T("RGB 5:6:5, 16bpp");
							break;
						case PIX_FMT_BGR565 :
							sFormat = _T("BGR 5:6:5, 16bpp");
							break;
						case PIX_FMT_RGB555 :
							sFormat = _T("RGB 5:5:5, 16bpp");
							break;
						case PIX_FMT_BGR555 :
							sFormat = _T("BGR 5:5:5, 16bpp");
							break;
						case PIX_FMT_RGB32 :
							sFormat = _T("RGB32");
							break;
						case PIX_FMT_RGB24 :
						case PIX_FMT_BGR24 :
							sFormat = _T("RGB24");
							break;
						default :
							break;
					}
					if (sFormat != _T(""))
					{
						t.Format(_T("    Decompressor: Internal to %s\r\n    FourCC: %s\r\n"),
										sFormat,
										CDib::GetCompressionName(pVideoStream->GetFormat(true)));
					}
					else
					{
						t.Format(_T("    Decompressor: Internal\r\n    FourCC: %s\r\n"),
									CDib::GetCompressionName(pVideoStream->GetFormat(true)));
					}
				}
				else
#endif
				if (pVideoStream->IsUsingVCM())
				{
					t.Format(_T("    Decompressor: %s\r\n    FourCC: %s\r\n"),
								pVideoStream->GetDecompressorDescription(),
								CDib::GetCompressionName(pVideoStream->GetFormat(true)));
				}
				else
				{	
					t.Format(_T("    Decompressor: None\r\n    FourCC: %s\r\n"),
								CDib::GetCompressionName(pVideoStream->GetFormat(true)));
				}
			}
			s += t;

			// Size
			t.Format(_T("    Size: %dx%d\r\n"),	pVideoStream->GetWidth(),
												pVideoStream->GetHeight());
			s += t;

			// Frame Rate
			t.Format(_T("    Frame Rate: %.3f fps (%u / %u)\r\n"),
												pVideoStream->GetFrameRate(),
												pVideoStream->GetRate(),
												pVideoStream->GetScale());
			s += t;
			
			// Data Rate
			t.Format(_T("    Data Rate: %d kbps\r\n"),
				Round(((double)pVideoStream->GetTotalBytes() / dLength * 8.0) / 1000.0));
			s += t;

			// Keyframes Rate
			double dKeyframesRate = 0.0;
			if (pVideoStream->GetTotalKeyFrames() > 0)
				dKeyframesRate = (double)pVideoStream->GetTotalFrames() / (double)pVideoStream->GetTotalKeyFrames();
			if (dKeyframesRate > 1.0)
			{
				t.Format(_T("    Keyframes Rate: %0.1f\r\n"), dKeyframesRate);
				s += t;
			}

			// Totals
			t.Format(_T("    Total: %d frame%s , %I64d byte%s , %d keyframe%s\r\n"), 
												pVideoStream->GetTotalFrames(),
												pVideoStream->GetTotalFrames() == 1 ? _T("") : _T("s"),
												pVideoStream->GetTotalBytes(),
												pVideoStream->GetTotalBytes() == 1 ? _T("") : _T("s"),
												pVideoStream->GetTotalKeyFrames(),
												pVideoStream->GetTotalKeyFrames() == 1 ? _T("") : _T("s"));
			s += t;

			// Length
			if ((nLengthHour == 0) && (nLengthMin != 0))
				t.Format(_T("    Length: %02d min %.3f sec"),		nLengthMin,
																	dLengthSec);
			else if ((nLengthHour == 0) && (nLengthMin == 0))	
				t.Format(_T("    Length: %.3f sec"),				dLengthSec);
			else
				t.Format(_T("    Length: %02d hr %02d min %.3f sec"),nLengthHour,
																	nLengthMin,
																	dLengthSec);
			s += t;
		}
		if (pDoc->m_pAVIPlay->GetVideoStreamsCount() == 0)
			s = _T("No Video Stream");
		pEdit = (CEdit*)GetDlgItem(IDC_VIDEOINFO);
		pEdit->SetWindowText(s);


		// Audio

		s = _T("");
		for (dwStreamNum = 0 ; dwStreamNum < pDoc->m_pAVIPlay->GetAudioStreamsCount() ; dwStreamNum++)
		{
			CAVIPlay::CAVIAudioStream* pAudioStream = 
							pDoc->m_pAVIPlay->GetAudioStream(dwStreamNum);
			if (!pAudioStream)
				return;

			// Time Lengths
			double dLength		= pAudioStream->GetTotalTime();							// Total Length in Seconds
			int nLengthHour		= (int)(dLength / 3600.0);								// Hours Part
			int nLengthMin		= (int)((dLength - nLengthHour * 3600.0) / 60.0);		// Minutes Part
			double dLengthSec	= dLength - nLengthHour * 3600.0 - nLengthMin * 60.0;	// Seconds Part

			// Spaces
			if (dwStreamNum > 0)
				s += _T("\r\n\r\n");

			// Stream Number
			if (dwStreamNum == (DWORD)pDoc->m_nActiveAudioStream)
				t.Format(_T("Audio Stream %d (Active):\r\n"), dwStreamNum);
			else
				t.Format(_T("Audio Stream %d:\r\n"), dwStreamNum);
			s += t;

			// Decompressor
			if (pAudioStream->GetFormatTag(true) != WAVE_FORMAT_PCM)
			{
#ifdef SUPPORT_LIBAVCODEC
				if (pAudioStream->IsUsingAVCodec())
				{
					t.Format(_T("    Decompressor: Internal\r\n"));
				}
				else
#endif
				if (pAudioStream->IsUsingACM())
					t.Format(_T("    Decompressor: %s\r\n"), pAudioStream->GetACMDecompressorLongName());
				else
					t.Format(_T("    Decompressor: None\r\n"));
				s += t;
			}

			// Wave Format Tag
			if (CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)) != _T(""))
			{
				if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)
				{
					if (pAudioStream->IsVBR())
					{
						t.Format(_T("    Format: Mp3 VBR (%i samples/chunk)\r\n"),
														pAudioStream->GetVBRSamplesPerChunk());
					}
					else
						t.Format(_T("    Format: Mp3 CBR\r\n"));
				}
				else if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_MPEG)
				{
					if (pAudioStream->IsVBR())
					{
						if (pAudioStream->GetMpegAudioLayer() == 2)
						{
							t.Format(_T("    Format: Mp2 VBR (%i samples/chunk)\r\n"),
														pAudioStream->GetVBRSamplesPerChunk());
						}
						else if (pAudioStream->GetMpegAudioLayer() == 1)
						{
							t.Format(_T("    Format: Mp1 VBR (%i samples/chunk)\r\n"),
														pAudioStream->GetVBRSamplesPerChunk());
						}
						else
						{
							t.Format(_T("    Format: %s\r\n"),
								CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));
						}
					}
					else
					{
						if (pAudioStream->GetMpegAudioLayer() == 2)
							t.Format(_T("    Format: Mp2 CBR\r\n"));
						else if (pAudioStream->GetMpegAudioLayer() == 1)
							t.Format(_T("    Format: Mp1 CBR\r\n"));
						else
						{
							t.Format(_T("    Format: %s\r\n"),
								CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));
						}
					}
				}
				else
				{
					if (pAudioStream->IsVBR())
					{
						t.Format(_T("    Format: %s VBR (%i samples/chunk)\r\n"),
							CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)),
							pAudioStream->GetVBRSamplesPerChunk());
					}
					else
					{
						t.Format(_T("    Format: %s CBR\r\n"),
							CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));
					}
				}
			}
			else
				t.Format(_T("    Format: 0x%04x\r\n"), pAudioStream->GetFormatTag(true));
			s += t;
	
			// Bits
			if (pAudioStream->GetBits(true) != 0)
			{
				if (pAudioStream->GetChannels(true) == 1) // Mono
					t.Format(_T("    Quality: %dHz , %dbits , mono\r\n"),
															pAudioStream->GetSampleRate(true),
															pAudioStream->GetBits(true));
				else if (pAudioStream->GetChannels(true) == 2) // Stereo
					t.Format(_T("    Quality: %dHz , %dbits , stereo\r\n"),
															pAudioStream->GetSampleRate(true),
															pAudioStream->GetBits(true));
				else // Multi-Channel
					t.Format(_T("    Quality: %dHz , %dbits , %d channels\r\n"),
															pAudioStream->GetSampleRate(true),
															pAudioStream->GetBits(true),
															pAudioStream->GetChannels(true));
			}
			else
			{
				if (pAudioStream->GetChannels(true) == 1) // Mono
					t.Format(_T("    Quality: %dHz , mono\r\n"),
															pAudioStream->GetSampleRate(true));
				else if (pAudioStream->GetChannels(true) == 2) // Stereo
					t.Format(_T("    Quality: %dHz , stereo\r\n"),
															pAudioStream->GetSampleRate(true));
				else // Multi-Channel
					t.Format(_T("    Quality: %dHz , %d channels\r\n"),
															pAudioStream->GetSampleRate(true),
															pAudioStream->GetChannels(true));
			}
			s += t;

			// Sample Size in Bytes
			if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_PCM)
			{
				t.Format(_T("    Sample Size: %d byte%s\r\n"),
															pAudioStream->GetSampleSize(true),
															pAudioStream->GetSampleSize(true) == 1 ? _T("") : _T("s"));
				s += t;
			}

			// Data Rate
			DWORD dwVBRKBitsPerSecond = 0;
			if (pAudioStream->IsVBR())
				dwVBRKBitsPerSecond = pAudioStream->CalcVBRBytesPerSeconds() * 8 / 1000;
			DWORD dwKBitsPerSecond = pAudioStream->GetBytesPerSeconds(true) * 8 / 1000;
			if (dwKBitsPerSecond > 0 && dwVBRKBitsPerSecond == 0)
			{
				t.Format(_T("    Data Rate: %d kbps\r\n"),	dwKBitsPerSecond);
				s += t;
			}
			else if (dwKBitsPerSecond == 0 && dwVBRKBitsPerSecond > 0)
			{
				t.Format(_T("    Data Rate: %d kbps\r\n"), dwVBRKBitsPerSecond);
				s += t;
			}
			else if (dwKBitsPerSecond > 0 && dwVBRKBitsPerSecond > 0)
			{
				if (dwKBitsPerSecond != dwVBRKBitsPerSecond)
					t.Format(_T("    Data Rate: %d kbps (effective: %d kbps)\r\n"), dwKBitsPerSecond, dwVBRKBitsPerSecond);
				else
					t.Format(_T("    Data Rate: %d kbps\r\n"),	dwKBitsPerSecond);
				s += t;
			}

			// Totals
			t.Format(_T("    Total: %I64d sample%s , %I64d byte%s , %d chunk%s\r\n"),	
														pAudioStream->GetTotalSamples(),
														pAudioStream->GetTotalSamples() == 1 ? _T("") : _T("s"),
														pAudioStream->GetTotalBytes(),
														pAudioStream->GetTotalBytes() == 1 ? _T("") : _T("s"),
														pAudioStream->GetTotalChunks(),
														pAudioStream->GetTotalChunks() == 1 ? _T("") : _T("s"));
			s += t;

			// Length
			if ((nLengthHour == 0) && (nLengthMin != 0))
				t.Format(_T("    Length: %02d min %.3f sec"),		nLengthMin,
																	dLengthSec);
			else if ((nLengthHour == 0) && (nLengthMin == 0))	
				t.Format(_T("    Length: %.3f sec"),				dLengthSec);
			else
				t.Format(_T("    Length: %02d hr %02d min %.3f sec"),nLengthHour,
																	nLengthMin,
																	dLengthSec);
			s += t;
		}
		if (pDoc->m_pAVIPlay->GetAudioStreamsCount() == 0)
			s = _T("No Audio Stream");
		pEdit = (CEdit*)GetDlgItem(IDC_AUDIOINFO);
		pEdit->SetWindowText(s);
	}

	// Just to remove focus from the text box
	CWnd* pWnd = (CWnd*)GetDlgItem(IDC_STATIC);
	if (pWnd)
		pWnd->SetFocus();
}

void CAviInfoDlg::OnClose() 
{	
	DestroyWindow();
}

void CAviInfoDlg::Close()
{
	OnClose();
}

BOOL CAviInfoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				OnClose();
				return TRUE;
			case IDCANCEL:
				OnClose();
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CAviInfoDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == _T('i') || nVirtKey == _T('I'))
		{
			OnClose();
			return TRUE;
		}
	}	
	return CDialog::PreTranslateMessage(pMsg);
}

void CAviInfoDlg::PostNcDestroy() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	pDoc->m_pAviInfoDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}
