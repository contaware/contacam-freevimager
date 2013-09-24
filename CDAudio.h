// CDAudio.h: interface for the CCDAudio class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CDAUDIO_H__FDE356E6_CB80_41FE_82BE_9F051620A4A0__INCLUDED_)
#define AFX_CDAUDIO_H__FDE356E6_CB80_41FE_82BE_9F051620A4A0__INCLUDED_

#pragma once

#include "mmsystem.h"

#ifdef _UNICODE
	#define	ATOI	_wtoi
	#define	ITOA	_itow	
	#define	STRNCPY	wcsncpy
#else
	#define ATOI	atoi
	#define	ITOA	_itoa
	#define	STRNCPY	strncpy
#endif


//=============================================================================
// A class wrapper of MCI API functions
//=============================================================================
class CCDAudio  
{
public:
	// constructor
	CCDAudio(LPCTSTR lpszDrive);
	// destructor
	virtual ~CCDAudio();

	// Start playing CD Audio
	MCIERROR Play();
	// Start playing CD Audio on given position
	MCIERROR Play(const int nPos);
	// Stop playing CD Audio
	MCIERROR Stop();
	// Pause playing CD Audio
	MCIERROR Pause();
	// Move to the next track
	MCIERROR Forward();
	// Move to the previous track
	MCIERROR Backward();
	// Eject the CDROM
	void EjectCDROM();

	//
	// Note:
	//
	// Tracks are numbered from 1 .. GetTracksCount()
	//

	// Return the current position in seconds
	int GetCurrentPos();
	// Return the current track number
	int GetCurrentTrack();
	// Return length of all track in seconds
	int GetLenghtAllTracks();
	// Return total tracks count
	int GetTracksCount();
	// Return length of given track
	int GetTrackLength(const int nTrack);
	// Return begin time of given track in seconds
	int GetTrackBeginTime( const int nTrack );
	// Return current drive letter with ending backslash
	CString GetCurrentDrive() const {return m_sDrive;};
	// Return the error code, 0 means no error
	int GetErrorCode() const {return m_nErrorCode;};

	// check wheter CD media is inserted
	bool IsMediaInsert();
	// is paused mode
	bool IsPaused();
	// is stopped mode
	bool IsStopped();
	// the device is ready
	bool IsReady();
	// is playing mode
	bool IsPlaying();

protected:
	// MCI error code
	MCIERROR m_nErrorCode;
	CString m_sDrive;
	CString m_sDeviceName;

protected:
	// handle MCI errors
	inline void MCIError(MCIERROR MCIError);
};

#endif // !defined(AFX_CDAUDIO_H__FDE356E6_CB80_41FE_82BE_9F051620A4A0__INCLUDED_)
