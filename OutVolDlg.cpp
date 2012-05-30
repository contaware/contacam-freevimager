// OutVolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "OutVolDlg.h"
#include "VideoAviView.h"
#include "VideoAviDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutVolDlg dialog


COutVolDlg::COutVolDlg(CWnd* pParent)
	: CDialog(COutVolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COutVolDlg)
	//}}AFX_DATA_INIT
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(COutVolDlg::IDD, pParent);
}


void COutVolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutVolDlg)
	DDX_Control(pDX, IDC_SLIDER_WAVEVOL_RIGHT, m_SliderWaveVolRight);
	DDX_Control(pDX, IDC_SLIDER_WAVEVOL_LEFT, m_SliderWaveVolLeft);
	DDX_Control(pDX, IDC_SLIDER_MASTERVOL_RIGHT, m_SliderMasterVolRight);
	DDX_Control(pDX, IDC_SLIDER_MASTERVOL_LEFT, m_SliderMasterVolLeft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COutVolDlg, CDialog)
	//{{AFX_MSG_MAP(COutVolDlg)
	ON_WM_CLOSE()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_CHECK_MASTERVOL, OnCheckMastervol)
	ON_BN_CLICKED(IDC_CHECK_WAVEVOL, OnCheckWavevol)
	ON_BN_CLICKED(IDC_AUDIO_DESTINATION, OnAudioDestination)
	//}}AFX_MSG_MAP
	ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerCtrlChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutVolDlg message handlers

void COutVolDlg::OnClose()
{	
	DestroyWindow();
}

void COutVolDlg::Close()
{
	OnClose();
}

// Catch the ENTER and the ESC Button Press!
BOOL COutVolDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
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

BOOL COutVolDlg::PreTranslateMessage(MSG* pMsg) 
{	
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == _T('v') || nVirtKey == _T('V'))
		{
			OnClose();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void COutVolDlg::PostNcDestroy() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	pDoc->m_PlayAudioFileThread.m_Mixer.Close();
	pDoc->m_pOutVolDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}

void COutVolDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

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
			if (m_pParentWnd && m_pParentWnd->m_hWnd)
			{
				if (pSlider->GetDlgCtrlID() == IDC_SLIDER_MASTERVOL_LEFT)
				{	
					DWORD dwVolRight, dwVolLeft;
					CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_MASTERVOL_LOCK);
					if (pCheck->GetCheck())
					{
						int nDelta = m_dwMasterVolLeftLastPos - m_SliderMasterVolLeft.GetPos();
						int nNewPos = m_dwMasterVolRightLastPos - nDelta;
						if (nNewPos < 0)
							nNewPos = 0;
						else if (nNewPos > (int)pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax())
							nNewPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax();
						m_SliderMasterVolRight.SetPos(nNewPos);
					}
					m_dwMasterVolLeftLastPos = m_SliderMasterVolLeft.GetPos();
					m_dwMasterVolRightLastPos = m_SliderMasterVolRight.GetPos();
					dwVolLeft = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - m_dwMasterVolLeftLastPos;
					dwVolRight = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - m_dwMasterVolRightLastPos;
					pDoc->m_PlayAudioFileThread.m_Mixer.SetMasterVolume(dwVolLeft, dwVolRight);
				}
				else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_MASTERVOL_RIGHT)
				{
					DWORD dwVolRight, dwVolLeft;
					CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_MASTERVOL_LOCK);
					if (pCheck->GetCheck())
					{
						int nDelta = m_dwMasterVolRightLastPos - m_SliderMasterVolRight.GetPos();
						int nNewPos = m_dwMasterVolLeftLastPos - nDelta;
						if (nNewPos < 0)
							nNewPos = 0;
						else if (nNewPos > (int)pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax())
							nNewPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax();
						m_SliderMasterVolLeft.SetPos(nNewPos);
					}
					m_dwMasterVolLeftLastPos = m_SliderMasterVolLeft.GetPos();
					m_dwMasterVolRightLastPos = m_SliderMasterVolRight.GetPos();
					dwVolLeft = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - m_dwMasterVolLeftLastPos;
					dwVolRight = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - m_dwMasterVolRightLastPos;
					pDoc->m_PlayAudioFileThread.m_Mixer.SetMasterVolume(dwVolLeft, dwVolRight);
				}
				else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_WAVEVOL_LEFT)
				{	
					DWORD dwVolRight, dwVolLeft;
					CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAVEVOL_LOCK);
					if (pCheck->GetCheck())
					{
						int nDelta = m_dwWaveVolLeftLastPos - m_SliderWaveVolLeft.GetPos();
						int nNewPos = m_dwWaveVolRightLastPos - nDelta;
						if (nNewPos < 0)
							nNewPos = 0;
						else if (nNewPos > (int)pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax())
							nNewPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax();
						m_SliderWaveVolRight.SetPos(nNewPos);
					}
					m_dwWaveVolLeftLastPos = m_SliderWaveVolLeft.GetPos();
					m_dwWaveVolRightLastPos = m_SliderWaveVolRight.GetPos();
					dwVolLeft = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - m_dwWaveVolLeftLastPos;
					dwVolRight = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - m_dwWaveVolRightLastPos;
					pDoc->m_PlayAudioFileThread.m_Mixer.SetWaveVolume(dwVolLeft, dwVolRight);
				}
				else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_WAVEVOL_RIGHT)
				{
					DWORD dwVolRight, dwVolLeft;
					CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAVEVOL_LOCK);
					if (pCheck->GetCheck())
					{
						int nDelta = m_dwWaveVolRightLastPos - m_SliderWaveVolRight.GetPos();
						int nNewPos = m_dwWaveVolLeftLastPos - nDelta;
						if (nNewPos < 0)
							nNewPos = 0;
						else if (nNewPos > (int)pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax())
							nNewPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax();
						m_SliderWaveVolLeft.SetPos(nNewPos);
					}
					m_dwWaveVolLeftLastPos = m_SliderWaveVolLeft.GetPos();
					m_dwWaveVolRightLastPos = m_SliderWaveVolRight.GetPos();
					dwVolLeft = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - m_dwWaveVolLeftLastPos;
					dwVolRight = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - m_dwWaveVolRightLastPos;
					pDoc->m_PlayAudioFileThread.m_Mixer.SetWaveVolume(dwVolLeft, dwVolRight);
				}
			}
		}
	}
	CDialog::OnVScroll(nSBCode, nPos, (CScrollBar*)pScrollBar);
}

BOOL COutVolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	// Open Mixer
	pDoc->m_PlayAudioFileThread.m_Mixer.Open(pDoc->m_PlayAudioFileThread.GetWaveHandle(), GetSafeHwnd());
	
	// Vars
	DWORD dwVolLeft, dwVolRight;
	BOOL bCheck;

	// Master Volume / Mute Init
	m_SliderMasterVolLeft.SetRange(	pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMin(),
									pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax());
	m_SliderMasterVolRight.SetRange(pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMin(),
									pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax());
	if (pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolume(dwVolLeft, dwVolRight))
	{
		m_SliderMasterVolLeft.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolLeft);
		m_SliderMasterVolRight.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolRight);
		m_dwMasterVolLeftLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolLeft;
		m_dwMasterVolRightLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolRight;
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_MASTERVOL_LOCK);
		if (dwVolLeft == dwVolRight)
			pCheck->SetCheck(1);
		else
			pCheck->SetCheck(0);
	}
	if (pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterMute(bCheck))
	{
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_MASTERVOL);
		pCheck->SetCheck(bCheck);
	}

	// Wave Volume / Mute Init
	m_SliderWaveVolLeft.SetRange(	pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMin(),
									pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax());
	m_SliderWaveVolRight.SetRange(	pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMin(),
									pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax());
	if (pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolume(dwVolLeft, dwVolRight))
	{
		m_SliderWaveVolLeft.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolLeft);
		m_SliderWaveVolRight.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolRight);
		m_dwWaveVolLeftLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolLeft;
		m_dwWaveVolRightLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolRight;
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAVEVOL_LOCK);
		if (dwVolLeft == dwVolRight)
			pCheck->SetCheck(1);
		else
			pCheck->SetCheck(0);
	}
	if (pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveMute(bCheck))
	{
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAVEVOL);
		pCheck->SetCheck(bCheck);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT COutVolDlg::OnMixerCtrlChange(WPARAM wParam, LPARAM lParam)
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	if ((HMIXER)wParam == pDoc->m_PlayAudioFileThread.m_Mixer.GetHandle())
	{
		if (pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlID() == (DWORD)lParam)
		{
			DWORD dwVolLeft, dwVolRight;
			if (pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolume(dwVolLeft, dwVolRight))
			{
				m_SliderMasterVolLeft.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolLeft);
				m_SliderMasterVolRight.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolRight);
				m_dwMasterVolLeftLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolLeft;
				m_dwMasterVolRightLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterVolumeControlMax() - dwVolRight;
			}
		}
		else if (pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterMuteControlID() == (DWORD)lParam)
		{
			BOOL bCheck;
			if (pDoc->m_PlayAudioFileThread.m_Mixer.GetMasterMute(bCheck))
			{
				CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_MASTERVOL);
				pCheck->SetCheck(bCheck);
			}
		}
		else if (pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlID() == (DWORD)lParam)
		{
			DWORD dwVolLeft, dwVolRight;
			if (pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolume(dwVolLeft, dwVolRight))
			{
				m_SliderWaveVolLeft.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolLeft);
				m_SliderWaveVolRight.SetPos(pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolRight);
				m_dwWaveVolLeftLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolLeft;
				m_dwWaveVolRightLastPos = pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveVolumeControlMax() - dwVolRight;
			}
		}
		else if (pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveMuteControlID() == (DWORD)lParam)
		{
			BOOL bCheck;
			if (pDoc->m_PlayAudioFileThread.m_Mixer.GetWaveMute(bCheck))
			{
				CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAVEVOL);
				pCheck->SetCheck(bCheck);
			}
		}
	}

	return 0;
}

void COutVolDlg::OnCheckMastervol() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_MASTERVOL);
	pDoc->m_PlayAudioFileThread.m_Mixer.SetMasterMute(pCheck->GetCheck());
}

void COutVolDlg::OnCheckWavevol() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_WAVEVOL);
	pDoc->m_PlayAudioFileThread.m_Mixer.SetWaveMute(pCheck->GetCheck());
}

void COutVolDlg::OnAudioDestination() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();

	pDoc->m_PlayAudioFileThread.AudioOutDestinationDialog();	
}
