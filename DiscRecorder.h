#if !defined(AFX_DISCRECORDER_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DISCRECORDER_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_
///////////////////////////////////////////////////////////////////////
// DiscRecorder.h
//
// Wrapper for IDiscRecorder2 Interface
//
// Written by Eric Haddan
//
#pragma once

#include "Imapi2Small.h"

class CDiscRecorder
{
private:
	IDiscRecorder2* m_discRecorder;
	SAFEARRAY*		m_volumePathNames;
	HRESULT			m_hResult;
	CString			m_recorderUniqueId;


public:
	CDiscRecorder(void);
	~CDiscRecorder(void);

	inline HRESULT GetHresult() {return m_hResult;}
	inline IDiscRecorder2* GetInterface() {return m_discRecorder;}
	inline CString GetUniqueId() {return m_recorderUniqueId;}

	bool Initialize(const CString& recorderUniqueId);

	bool AcquireExclusiveAccess(bool force, const CString& clientName);	
	bool ReleaseExclusiveAccess();
	CString ExclusiveAccessOwner();

	bool EjectMedia();
	bool CloseTray();

	bool EnableMcn();
	bool DisableMcn();

	LONG	GetLegacyDeviceNumber();
	CString GetProductID();
	CString GetProductRevision();
	CString GetVendorId();
	CString GetVolumeName();

	ULONG GetTotalVolumePaths();
	CString GetVolumePath(ULONG volumePathIndex);

};

#endif // !defined(AFX_DISCRECORDER_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
