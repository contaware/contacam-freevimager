#if !defined(AFX_DISCMASTER_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DISCMASTER_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_
///////////////////////////////////////////////////////////////////////
// DiscMaster.h
//
// Wrapper for IDiscMaster2 Interface
//
// Written by Eric Haddan
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Imapi2Small.h"

class CDiscMaster
{
private:
    IDiscMaster2*	m_discMaster;
	HRESULT			m_hResult;
	CString			m_errorMessage;

public:
	CDiscMaster(void);
	~CDiscMaster(void);

	inline HRESULT GetHresult() {return m_hResult;}
	inline CString GetErrorMessage() {return m_errorMessage;}

	bool Initialize();
	long GetTotalDevices();
	CString GetDeviceUniqueID(long index);
};

#endif // !defined(AFX_DISCMASTER_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
