#if !defined(AFX_DISCFORMATERASE_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DISCFORMATERASE_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_
///////////////////////////////////////////////////////////////////////
// DiscFormatErase.h
//
// Wrapper for IDiscFormat2Erase Interface
//
// Written by Eric Haddan
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Imapi2Small.h"
#include "DiscRecorder.h"

class CDiscRecorder;

class CDiscFormatErase
{
private:
	IDiscFormat2Erase*	m_discFormatErase;
	SAFEARRAY*			m_mediaTypesArray;
	HRESULT				m_hResult;
	CString				m_errorMessage;
	HWND				m_hNotificationWnd;
	bool				m_fullErase;

public:
	CDiscFormatErase(void);
	~CDiscFormatErase(void);

	inline	HRESULT GetHresult() {return m_hResult;}
	inline	CString GetErrorMessage() {return m_errorMessage;}

	inline	void SetFullErase(bool fullErase) {m_fullErase = fullErase;};
	inline	bool IsMediaBlank() {	if (m_discFormatErase)
									{
										VARIANT_BOOL value = VARIANT_FALSE;
										m_discFormatErase->get_MediaHeuristicallyBlank(&value);
										return value == VARIANT_TRUE ? true : false;
									}
									else
										return false;};

	inline	IDiscFormat2Erase* GetInterface() {return m_discFormatErase;}
	bool	Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName);

	ULONG	GetTotalSupportedMediaTypes();
	int		GetSupportedMediaType(ULONG index);

	bool	Erase(HWND hWnd);
};

#endif // !defined(AFX_DISCFORMATERASE_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)