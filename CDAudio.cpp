// CDAudio.cpp: implementation of the CCDAudio class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CDAudio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//=============================================================================
// Constructor
//=============================================================================
CCDAudio::CCDAudio(LPCTSTR lpszFileName)
{
	CString sFileName(lpszFileName);
	CString sTrackNum(::GetShortFileNameNoExt(sFileName));
	sTrackNum.Delete(0, 5); // Remove "Track"
	int nTrackNum = _ttoi(sTrackNum);
	m_sDrive = ::GetDriveName(sFileName);
	m_sDrive.TrimRight('\\');
	m_sDrive += _T('\\');
	m_sDrive.MakeLower();
	
	CString s;
	m_sDeviceName = ::GetUuidString(); // Get a unique name
	s.Format(_T("open %s type cdaudio alias %s"), m_sDrive, m_sDeviceName);
	
	m_nErrorCode = mciSendString(s, NULL, 0, NULL );
	if (m_nErrorCode == 0)
		Play(GetTrackBeginTime(nTrackNum));
}

//=============================================================================
// Destructor
//=============================================================================
CCDAudio::~CCDAudio()
{	
	if (IsReady())
	{
		if (IsPaused() || IsPlaying())
			mciSendString(_T("stop ") + m_sDeviceName, NULL, 0, NULL );				
		mciSendString(_T("close ") + m_sDeviceName, NULL, 0, NULL ); 
	}
}

//=============================================================================
// Check whether CD media is inserted
//=============================================================================
bool CCDAudio::IsMediaInsert()
{
	const int nBuffSize = 16;
	TCHAR szBuff[nBuffSize];

	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" media present"), szBuff, nBuffSize, NULL);    
	if (m_nErrorCode != 0)
		return false;

	if (lstrcmp(szBuff, _T("true")) == 0) 
		return true;
    return false; 
}

//=============================================================================
// Check whether is paused mode
//=============================================================================
bool CCDAudio::IsPaused()
{
	const int nBuffSize = 64;
	TCHAR szBuff[nBuffSize];

	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" mode wait"), szBuff, nBuffSize, NULL);    
	if (m_nErrorCode != 0)
		return false;

	if (lstrcmp(szBuff, _T("paused")) == 0) 
		return true;
    return false; 
}

//=============================================================================
// Check whether is stopped mode
//=============================================================================
bool CCDAudio::IsStopped()
{
	const int nBuffSize = 64;
	TCHAR szBuff[nBuffSize];

	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" mode wait"), szBuff, nBuffSize, NULL);    
	if (m_nErrorCode != 0)
		return false;

	if (lstrcmp(szBuff, _T("stopped")) == 0) 
		return true;
    return false; 
}

//=============================================================================
// Check whether MCI is ready
//=============================================================================
bool CCDAudio::IsReady()
{
	const int nBuffSize = 64;
	TCHAR szBuff[nBuffSize];

	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" mode wait"), szBuff, nBuffSize, NULL);    
	if (m_nErrorCode != 0)
		return false;

	if (lstrcmp(szBuff, _T("not ready")) == 0) 
		return false;
    return true; 
}

//=============================================================================
// Check whether is play mode
//=============================================================================
bool CCDAudio::IsPlaying()
{
	const int nBuffSize = 64;
	TCHAR szBuff[nBuffSize];

	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" mode wait"), szBuff, nBuffSize, NULL);    
	if (m_nErrorCode != 0)
		return false;

	if (lstrcmp(szBuff, _T("playing")) == 0) 
		return true;
    return false; 
}

//=============================================================================
// Return the length of the given track
//=============================================================================
int CCDAudio::GetTrackLength(const int nTrack)
{
	if (IsMediaInsert())
	{
		const int nBuffSize = 64;
		TCHAR szBuff[nBuffSize];
		CString sTrack;
		sTrack.Format(_T("%d"), nTrack);

		m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" length track ") + sTrack, szBuff, nBuffSize, NULL);    
		if (m_nErrorCode != 0)
			return 0;

		TCHAR szMin[3], szSec[3];
		_tcsncpy(szMin, szBuff, 3);
		_tcsncpy(szSec, (szBuff + 3), 3);
		szMin[2] = _T('\0');
		szSec[2] = _T('\0');

		return ((ATOI(szMin) * 60) + ATOI(szSec));		
	}

	return 0;
}

//=============================================================================
// Eject the CDROM
//=============================================================================
void CCDAudio::EjectCDROM()
{
    if (IsMediaInsert())
        m_nErrorCode = mciSendString(_T("Set CDAudio Door Open"), NULL, 0, NULL);
	else
        m_nErrorCode = mciSendString(_T("Set CDAudio Door Closed"), NULL, 0, NULL); 
}

//=============================================================================
// handle MCI errors
//=============================================================================
void CCDAudio::MCIError(MCIERROR m_nErrorCode)
{
	TCHAR szBuff[128];
	memset(szBuff, 0, sizeof(szBuff));
	if (!mciGetErrorString(m_nErrorCode, szBuff, sizeof(szBuff) / sizeof(TCHAR)))
        lstrcpy(szBuff, _T("Unknown error"));
	::AfxMessageBox(szBuff, MB_OK | MB_ICONERROR);
	m_nErrorCode = 0;
}

//=============================================================================
// Start playing CD Audio
//=============================================================================
MCIERROR CCDAudio::Play()
{	
	m_nErrorCode = mciSendString(_T("play ") + m_sDeviceName, NULL, 0, NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}
	return m_nErrorCode;
}

//=============================================================================
// Start playing CD Audio on given position
//=============================================================================
MCIERROR CCDAudio::Play(const int nPos)
{
	CString s;
	s.Format(_T(" from %02d:%02d:00"), (nPos / 60), (nPos % 60));
	m_nErrorCode = mciSendString(_T("play ") + m_sDeviceName + s, NULL, 0, NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}
	return m_nErrorCode;
}

//=============================================================================
// Stop playing CD Audio
//=============================================================================
MCIERROR CCDAudio::Stop()
{
    m_nErrorCode = mciSendString(_T("stop ") + m_sDeviceName, NULL, 0, NULL);
	// handle error
	if (m_nErrorCode != 0)
	{
		MCIError( m_nErrorCode );
		return m_nErrorCode;
	}

	m_nErrorCode = mciSendString(_T("seek ") + m_sDeviceName + _T(" to start"), NULL, 0, NULL);
	if (m_nErrorCode != 0 )
	{ 
		MCIError( m_nErrorCode ); 
		return m_nErrorCode;
	}

	return m_nErrorCode;
}

//=============================================================================
// Pause playing CD Audio
//=============================================================================
MCIERROR CCDAudio::Pause()
{  
    m_nErrorCode = mciSendString(_T("pause ") + m_sDeviceName, NULL, 0, NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}
	return m_nErrorCode;
}

//=============================================================================
// Move to the next track
//=============================================================================
MCIERROR CCDAudio::Forward()
{   		
    TCHAR szBuff2[128], szBuff3[32];
	
    int nCurrTrack = GetCurrentTrack();
	const int nLastTrack = GetTracksCount();
	
	// now the current track is the last track
	if ((nCurrTrack == nLastTrack) || (nLastTrack == 0)) 
		return 0;

	// increment track position	
    nCurrTrack++;

	// form the command string
	_stprintf(szBuff2, _T("status ") + m_sDeviceName + _T(" position track %d"), nCurrTrack); 

	// send the command string
    m_nErrorCode = mciSendString(szBuff2, szBuff3, sizeof(szBuff3) / sizeof(TCHAR), NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}

	memset(szBuff2, 0, sizeof(szBuff2));
	
	// form the command string
    _stprintf(szBuff2, _T("play ") + m_sDeviceName + _T(" from %s"), szBuff3);

	// send the command string
	m_nErrorCode = mciSendString(szBuff2, NULL, 0, NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}

	return m_nErrorCode;
}

//=============================================================================
// Move to the previous track
//=============================================================================
MCIERROR CCDAudio::Backward()
{			
    TCHAR szBuff2[128], szBuff3[32];

	int nCurrTrack = GetCurrentTrack();
	const int nLastTrack = GetTracksCount();

	if ((nCurrTrack == 0) || (nLastTrack == 0) || (nCurrTrack == 1)) 
		return 0;
	
    nCurrTrack--;

	_stprintf(szBuff2, _T("status ") + m_sDeviceName + _T(" position track %d"), nCurrTrack); 

    m_nErrorCode = mciSendString(szBuff2, szBuff3, sizeof(szBuff3) / sizeof(TCHAR), NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}

	memset(szBuff2, 0, sizeof(szBuff2));
	
    _stprintf(szBuff2, _T("play ") + m_sDeviceName + _T(" from %s"), szBuff3);

	m_nErrorCode = mciSendString(szBuff2, NULL, 0, NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return m_nErrorCode;
	}

	return m_nErrorCode;
}

//=============================================================================
// Return the current track position in seconds
//=============================================================================
int CCDAudio::GetCurrentPos()
{
	if (IsMediaInsert())
	{
		TCHAR szBuff[16];
		TCHAR min[3], sec[3];
		m_nErrorCode = 0;

		m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" position"), szBuff, sizeof(szBuff) / sizeof(TCHAR), NULL);
		if (m_nErrorCode != 0)
		{ 
			MCIError(m_nErrorCode); 
			return 0;
		}

		_tcsncpy(min, szBuff, 3);
		_tcsncpy(sec, (szBuff + 3), 3);
		min[2] = _T('\0');
		sec[2] = _T('\0');

		return ((ATOI(min) * 60) + ATOI(sec));
	}

	return 0;
}

//=============================================================================
// Return the current track number
//=============================================================================
int CCDAudio::GetCurrentTrack()
{
	TCHAR szBuff[16];
   
	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" current track"), szBuff, sizeof(szBuff) / sizeof(TCHAR), NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return 0;
	}

	return ATOI(szBuff);
}

//=============================================================================
// Return the total tracks length
//=============================================================================
int CCDAudio::GetLenghtAllTracks()
{
	TCHAR szBuff[16];
	TCHAR min[3], sec[3];
    m_nErrorCode = 0;

	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" length"), szBuff, sizeof(szBuff) / sizeof(TCHAR), NULL);
	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return 0;
	}

	_tcsncpy(min, szBuff, 3);
    _tcsncpy(sec, (szBuff + 3), 3);
	min[2] = _T('\0');
	sec[2] = _T('\0');

	return ((ATOI(min) * 60) + ATOI(sec));
}

//=============================================================================
// Return the count of tracks
//=============================================================================
int CCDAudio::GetTracksCount()
{
	TCHAR szBuff[16];
	
	m_nErrorCode = mciSendString(_T("status ") + m_sDeviceName + _T(" number of tracks"), szBuff, sizeof(szBuff) / sizeof(TCHAR), NULL);

	if (m_nErrorCode != 0)
	{ 
		MCIError(m_nErrorCode); 
		return 0;
	}

    return ATOI(szBuff); 
}

//=============================================================================
// Return the start time of the given track
//=============================================================================
int CCDAudio::GetTrackBeginTime(const int nTrack)
{
	TCHAR szBuff[64], szReq[64];
	_stprintf(szReq, _T("status ") + m_sDeviceName + _T(" position track %d"), nTrack);

	m_nErrorCode = mciSendString(szReq, szBuff, sizeof(szBuff) / sizeof(TCHAR), NULL);
	if (m_nErrorCode != 0) 
		return 0;

	TCHAR szMin[3], szSec[3];

	STRNCPY(szMin, szBuff, 2);
	szMin[2] = _T('\0');

	STRNCPY(szSec, (szBuff + 3), 2);
	szSec[2] = _T('\0');

	return ((ATOI(szMin) * 60) + ATOI(szSec));		
}