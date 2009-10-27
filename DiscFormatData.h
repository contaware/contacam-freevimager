#if !defined(AFX_DISCFORMATDATA_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DISCFORMATDATA_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_
///////////////////////////////////////////////////////////////////////
// DiscFormatData.h
//
// Wrapper for IDiscFormat2Data Interface
//
// Written by Eric Haddan
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Imapi2Small.h"
#include "DiscRecorder.h"

// Progress handling
#define WM_IMAPI_UPDATE		WM_APP + 842
typedef struct _IMAPI_STATUS {
	// IDiscFormat2DataEventArgs Interface
    LONG elapsedTime;		// Elapsed time in seconds
    LONG remainingTime;		// Remaining time in seconds
    LONG totalTime;			// total estimated time in seconds
	// IWriteEngine2EventArgs Interface
    LONG startLba;			// the starting lba of the current operation
    LONG sectorCount;		// the total sectors to write in the current operation
    LONG lastReadLba;		// the last read lba address
    LONG lastWrittenLba;	// the last written lba address
    LONG totalSystemBuffer;	// total size of the system buffer
    LONG usedSystemBuffer;	// size of used system buffer
    LONG freeSystemBuffer;	// size of the free system buffer
} IMAPI_STATUS, *PIMAPI_STATUS;
#define RETURN_CANCEL_WRITE	0
#define RETURN_CONTINUE		1

class CDiscRecorder;

class CDiscFormatData
{
private:
	IDiscFormat2Data*	m_discFormatData;
	SAFEARRAY*			m_mediaTypesArray;
	HRESULT				m_hResult;
	CString				m_errorMessage;
	HWND				m_hNotificationWnd;
	bool				m_closeMedia;


public:
	CDiscFormatData(void);
	~CDiscFormatData(void);

	inline	HRESULT GetHresult() {return m_hResult;}
	inline	CString GetErrorMessage() {return m_errorMessage;}

	inline	IDiscFormat2Data* GetInterface() {return m_discFormatData;}
	bool	Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName);

	bool	Burn(HWND hWnd, IStream* data);

	ULONG	GetTotalSupportedMediaTypes();
	int		GetSupportedMediaType(ULONG index);

	inline	void SetCloseMedia(bool closeMedia){m_closeMedia = closeMedia;}
	inline	bool IsMediaBlank() {	if (m_discFormatData)
									{
										VARIANT_BOOL value = VARIANT_FALSE;
										m_discFormatData->get_MediaHeuristicallyBlank(&value);
										return value == VARIANT_TRUE ? true : false;
									}
									else
										return false;};

protected:
	IStream*	m_streamData;
};

#endif // !defined(AFX_DISCFORMATDATA_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)