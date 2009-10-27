// AudioVideoShiftDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AudioVideoShiftDlg.h"
#include "VideoAviView.h"
#include "VideoAviDoc.h"
#include "AviPlay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Values in ms
#define SHIFT_RANGE_MAX			5000
#define SHIFT_RANGE_MIN			-5000
#define FRAMERATE_RANGE_MAX		100
#define FRAMERATE_RANGE_MIN		-100

/////////////////////////////////////////////////////////////////////////////
// CAudioVideoShiftDlg dialog


CAudioVideoShiftDlg::CAudioVideoShiftDlg(CWnd* pParent)
	: CDialog(CAudioVideoShiftDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioVideoShiftDlg)
	//}}AFX_DATA_INIT

	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CAudioVideoShiftDlg::IDD, pParent);
}


void CAudioVideoShiftDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioVideoShiftDlg)
	DDX_Control(pDX, IDC_SLIDER_VIDEOFRAMERATE, m_SliderVideoFrameRate);
	DDX_Control(pDX, IDC_SLIDER_AUDIOVIDEOSHIFT, m_SliderAudioVideoShift);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioVideoShiftDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioVideoShiftDlg)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_SHIFT_SAVETOAVI, OnButtonShiftSavetoavi)
	ON_BN_CLICKED(IDC_BUTTON_UNDO_SHIFT, OnButtonUndoShift)
	ON_BN_CLICKED(IDC_BUTTON_UNDO_FRAMERATE, OnButtonUndoFramerate)
	ON_BN_CLICKED(IDC_BUTTON_ZEROSHIFT, OnButtonZeroshift)
	ON_BN_CLICKED(IDC_BUTTON_FRAMERATE_SAVETOAVI, OnButtonFramerateSavetoavi)
	ON_BN_CLICKED(IDC_BUTTON_AV_SAMELENGTH, OnButtonAvSamelength)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioVideoShiftDlg message handlers

void CAudioVideoShiftDlg::OnClose() 
{	
	DestroyWindow();
}

void CAudioVideoShiftDlg::Close()
{
	OnClose();
}

BOOL CAudioVideoShiftDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
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

BOOL CAudioVideoShiftDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == _T('s') || nVirtKey == _T('S'))
		{
			OnClose();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CAudioVideoShiftDlg::PostNcDestroy() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->m_pAudioVideoShiftDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}

BOOL CAudioVideoShiftDlg::OnInitDialog() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CDialog::OnInitDialog();
	
	// Init Audio Video Shift Slider
	m_SliderAudioVideoShift.SetRange(SHIFT_RANGE_MIN, SHIFT_RANGE_MAX, TRUE);
	m_SliderAudioVideoShift.SetLineSize(1);
	m_SliderAudioVideoShift.SetPageSize(20);
	int nShiftValue = pDoc->m_nAudioLeadsVideoShift;
	m_SliderAudioVideoShift.SetPos(nShiftValue);
	DisplayShiftValue(nShiftValue);

	// Init Frame Rate Slider
	int nVideoFrameRateRangeMin = MIN(-(FRAMERATE_RANGE_MIN), Round(pDoc->m_PlayVideoFileThread.GetFrameRate() * 1000.0));
	m_SliderVideoFrameRate.SetRange(-nVideoFrameRateRangeMin, FRAMERATE_RANGE_MAX, TRUE);
	m_SliderVideoFrameRate.SetLineSize(1);
	m_SliderVideoFrameRate.SetPageSize(20);
	int nFrameRateDeltaValue = Round((pDoc->GetPlayFrameRate() - pDoc->m_PlayVideoFileThread.GetFrameRate()) * 1000.0);
	m_SliderVideoFrameRate.SetPos(nFrameRateDeltaValue);
	DisplayFrameRateValue(nFrameRateDeltaValue);
	
	// Update
	UpdateDisplay();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioVideoShiftDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

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
			if (m_pParentWnd && m_pParentWnd->m_hWnd)
			{
				if (pSlider->GetDlgCtrlID() == IDC_SLIDER_AUDIOVIDEOSHIFT)
				{	
					int nShiftValue = m_SliderAudioVideoShift.GetPos();
					pDoc->m_nAudioLeadsVideoShift = nShiftValue;
					DisplayShiftValue(nShiftValue);

					// Update
					UpdateDisplay();
				}
				else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_VIDEOFRAMERATE)
				{
					int nFrameRateDeltaValue = m_SliderVideoFrameRate.GetPos();
					if (nFrameRateDeltaValue == 0)
					{
						DWORD dwRate, dwScale;
						pDoc->m_PlayVideoFileThread.GetFrameRate(&dwRate, &dwScale);
						pDoc->SetPlayFrameRate(dwRate, dwScale);
					}
					else
						pDoc->SetPlayFrameRate(pDoc->m_PlayVideoFileThread.GetFrameRate() + ((double)nFrameRateDeltaValue / 1000.0));
					DisplayFrameRateValue(nFrameRateDeltaValue);

					// Update
					UpdateDisplay();
				}
			}
		}
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAudioVideoShiftDlg::DisplayShiftValue(int nShiftValue)
{
	CString sShiftDesc;
	CString sShiftValue;
	CEdit* pDesc = (CEdit*)GetDlgItem(IDC_DESC_AUDIOVIDEOSHIFT);
	CEdit* pValue = (CEdit*)GetDlgItem(IDC_VALUE_AUDIOVIDEOSHIFT);
	
	if (nShiftValue == 0)
	{
		sShiftDesc = _T("No Shift Set.");
		sShiftValue = _T("");
	}
	else if (nShiftValue > 0)
	{
		sShiftDesc = _T("Audio Plays Ahead of Video By:");
		sShiftValue.Format(_T("%i ms"), nShiftValue);
	}
	else
	{
		sShiftDesc = _T("Video Plays Ahead of Audio By:");
		sShiftValue.Format(_T("%i ms"), -nShiftValue);
	}

	pValue->SetWindowText(sShiftValue);
	pDesc->SetWindowText(sShiftDesc);
}

void CAudioVideoShiftDlg::DisplayFrameRateValue(int nFrameRateDelta)
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_VALUE_VIDEOFRAMERATE);

	if (nFrameRateDelta > 0)
		sText.Format(_T("%0.4f (%0.4f+%0.4f) fps"),
								pDoc->GetPlayFrameRate(),
								pDoc->m_PlayVideoFileThread.GetFrameRate(),
								(double)nFrameRateDelta / 1000.0);
	else if (nFrameRateDelta < 0)
		sText.Format(_T("%0.4f (%0.4f%0.4f) fps"),
								pDoc->GetPlayFrameRate(),
								pDoc->m_PlayVideoFileThread.GetFrameRate(),
								(double)nFrameRateDelta / 1000.0);
	else
		sText.Format(_T("%0.4f fps"),
								pDoc->GetPlayFrameRate());

	pEdit->SetWindowText(sText);
}

void CAudioVideoShiftDlg::OnButtonShiftSavetoavi() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return;
	CAVIPlay::CAVIAudioStream* pAudioStream = pDoc->m_pAVIPlay->GetAudioStream(pDoc->m_nActiveAudioStream);
	if (!pAudioStream)
		return;

	// Change the Start Offsets
	if (pDoc->m_nAudioLeadsVideoShift > 0)
	{
		DWORD dwStart = 0;
		if (pVideoStream->GetScale() > 0)
			dwStart = (DWORD)::MulDiv(	pDoc->m_nAudioLeadsVideoShift,
										pVideoStream->GetRate(),
										pVideoStream->GetScale()) / 1000;

		CAVIPlay::AviChangeVideoStartOffset(pDoc->m_sFileName,
											pDoc->m_nActiveVideoStream,
											dwStart,
											true);
		CAVIPlay::AviChangeAudioStartOffset(pDoc->m_sFileName,
											pDoc->m_nActiveAudioStream,
											0,
											true);
	}
	else if (pDoc->m_nAudioLeadsVideoShift < 0)
	{
		DWORD dwStart = 0;
		if (pAudioStream->GetScale() > 0)
			dwStart = (DWORD)::MulDiv(	-pDoc->m_nAudioLeadsVideoShift,
										pAudioStream->GetRate(),
										pAudioStream->GetScale()) / 1000;

		CAVIPlay::AviChangeAudioStartOffset(pDoc->m_sFileName,
											pDoc->m_nActiveAudioStream,
											dwStart,
											true);
		CAVIPlay::AviChangeVideoStartOffset(pDoc->m_sFileName,
											pDoc->m_nActiveVideoStream,
											0,
											true);
	}
	else
	{
		CAVIPlay::AviChangeVideoStartOffset(pDoc->m_sFileName,
											pDoc->m_nActiveVideoStream,
											0,
											true);
		CAVIPlay::AviChangeAudioStartOffset(pDoc->m_sFileName,
											pDoc->m_nActiveAudioStream,
											0,
											true);
	}

	// Set the new Init Value
	pDoc->m_nAudioLeadsVideoShiftInitValue = pDoc->m_nAudioLeadsVideoShift;

	// Update
	UpdateDisplay();
}

void CAudioVideoShiftDlg::OnButtonFramerateSavetoavi() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return;
	
	// Write to Avi File
	CAVIPlay::AviChangeVideoFrameRate(	pDoc->m_sFileName,
										pDoc->m_nActiveVideoStream,
										pDoc->m_dwPlayRate,
										pDoc->m_dwPlayScale,
										true);

	// Update inside the CAVIPlay Object, so that
	// we do not have to re-open the file to read the new values
	// from the above updated file.
	pVideoStream->SetFrameRate(	pDoc->m_dwPlayRate,
								pDoc->m_dwPlayScale);

	// Update
	UpdateDisplay();

	// Update Avi Info Dialog if it is open
	pDoc->UpdateAviInfoDlg();
}

void CAudioVideoShiftDlg::OnButtonUndoShift() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Restore From Init Value
	pDoc->m_nAudioLeadsVideoShift = pDoc->m_nAudioLeadsVideoShiftInitValue;
	int nShiftValue = pDoc->m_nAudioLeadsVideoShift;
	m_SliderAudioVideoShift.SetPos(nShiftValue);
	DisplayShiftValue(nShiftValue);

	// Update
	UpdateDisplay();
}

void CAudioVideoShiftDlg::OnButtonUndoFramerate() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return;

	// Restore From Init Value
	pDoc->m_dwPlayRate = pVideoStream->GetRate();
	pDoc->m_dwPlayScale = pVideoStream->GetScale();

	m_SliderVideoFrameRate.SetPos(0);
	DisplayFrameRateValue(0);

	// Update
	UpdateDisplay();
}

void CAudioVideoShiftDlg::OnButtonZeroshift() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Set to 0
	pDoc->m_nAudioLeadsVideoShift = 0;
	m_SliderAudioVideoShift.SetPos(0);
	DisplayShiftValue(0);

	// Update
	UpdateDisplay();
}

void CAudioVideoShiftDlg::OnButtonAvSamelength() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return;
	CAVIPlay::CAVIAudioStream* pAudioStream = pDoc->m_pAVIPlay->GetAudioStream(pDoc->m_nActiveAudioStream);
	if (!pAudioStream)
		return;

	// Audio Length
	double dAudioLength	= (double)pAudioStream->GetTotalTime(); // Total Length in Seconds	

	// New Play Frame-Rate
	pDoc->SetPlayFrameRate((double)pVideoStream->GetTotalFrames() / dAudioLength);
	
	// Set Slider
	int nFrameRateDeltaValue = (int)((pDoc->GetPlayFrameRate() - pDoc->m_PlayVideoFileThread.GetFrameRate()) * 1000.0);
	m_SliderVideoFrameRate.SetPos(nFrameRateDeltaValue);
	DisplayFrameRateValue(nFrameRateDeltaValue);

	// Update
	UpdateDisplay();
}

BOOL CAudioVideoShiftDlg::AreAVSameLength() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return FALSE;
	CAVIPlay::CAVIAudioStream* pAudioStream = pDoc->m_pAVIPlay->GetAudioStream(pDoc->m_nActiveAudioStream);
	if (!pAudioStream)
		return FALSE;

	// Audio Length
	double dAudioLength	= (double)pAudioStream->GetTotalTime();

	// Video Length
	double dVideoLength = (double)pVideoStream->GetTotalFrames() /
											pDoc->GetPlayFrameRate();

	// Round Check To The 10 ms
	dVideoLength *= 100.0;
	dAudioLength *= 100.0;
	return (((int)dVideoLength) == ((int)dAudioLength));
}

void CAudioVideoShiftDlg::UpdateDisplay()
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);
	CButton* pButton;

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return;

	// If Audio & Video available
	if (pDoc->m_pAVIPlay->HasVideo() && pDoc->m_nActiveVideoStream >= 0 &&
		pDoc->m_pAVIPlay->HasAudio() && pDoc->m_nActiveAudioStream >= 0)
	{
		// Shift Save & Shift Undo Buttons
		if (pDoc->m_nAudioLeadsVideoShift != pDoc->m_nAudioLeadsVideoShiftInitValue)
		{
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_SHIFT_SAVETOAVI);
			pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_UNDO_SHIFT);
			pButton->EnableWindow(TRUE);
		}
		else
		{
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_SHIFT_SAVETOAVI);
			pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_UNDO_SHIFT);
			pButton->EnableWindow(FALSE);
		}

		// Zero Shift Button
		if (pDoc->m_nAudioLeadsVideoShift != 0)
		{
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_ZEROSHIFT);
			pButton->EnableWindow(TRUE);
		}
		else
		{
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_ZEROSHIFT);
			pButton->EnableWindow(FALSE);
		}
	}
	else
	{
		CEdit* pDesc = (CEdit*)GetDlgItem(IDC_DESC_AUDIOVIDEOSHIFT);
		pDesc->EnableWindow(FALSE);
		CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_AUDIOVIDEOSHIFT);
		pSlider->EnableWindow(FALSE);
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_SHIFT_SAVETOAVI);
		pButton->EnableWindow(FALSE);
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_UNDO_SHIFT);
		pButton->EnableWindow(FALSE);
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_ZEROSHIFT);
		pButton->EnableWindow(FALSE);
	}
	
	// Frame-Rate Save & Frame-Rate Undo Buttons
	if ((pDoc->m_dwPlayRate != pVideoStream->GetRate()) ||
		(pDoc->m_dwPlayScale != pVideoStream->GetScale()))
	{
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_FRAMERATE_SAVETOAVI);
		pButton->EnableWindow(TRUE);
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_UNDO_FRAMERATE);
		pButton->EnableWindow(TRUE);
	}
	else
	{
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_FRAMERATE_SAVETOAVI);
		pButton->EnableWindow(FALSE);
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_UNDO_FRAMERATE);
		pButton->EnableWindow(FALSE);
	}

	// If Audio & Video available
	if (pDoc->m_pAVIPlay->HasVideo() && pDoc->m_nActiveVideoStream >= 0 &&
		pDoc->m_pAVIPlay->HasAudio() && pDoc->m_nActiveAudioStream >= 0)
	{
		// AV Same Length Button
		if (AreAVSameLength())
		{
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_AV_SAMELENGTH);
			pButton->EnableWindow(FALSE);
		}
		else
		{
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_AV_SAMELENGTH);
			pButton->EnableWindow(TRUE);
		}
	}
	else
	{
		pButton = (CButton*)GetDlgItem(IDC_BUTTON_AV_SAMELENGTH);
		pButton->EnableWindow(FALSE);
	}

	// Update Title
	pDoc->SetDocumentTitle();
}
