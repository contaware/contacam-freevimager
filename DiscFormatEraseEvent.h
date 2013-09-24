#if !defined(AFX_DISCFORMATERASEEVENT_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DISCFORMATERASEEVENT_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_
///////////////////////////////////////////////////////////////////////
// DiscFormatEraseEvent.h
//
// Wrapper for DDiscFormat2EraseEvents Interface Events
//
// Written by Eric Haddan
//
#pragma once

#include "Imapi2Small.h"

#define IMAPI_FORMAT2_ERASE		0xFFFF

class CDiscFormatErase;

// CDiscFormatEraseEvent command target

class CDiscFormatEraseEvent : public CCmdTarget
{
	DECLARE_DYNAMIC(CDiscFormatEraseEvent)
private:
	LPTYPEINFO  m_ptinfo;           // ITest type information
	DWORD		m_dwCookie;
	LPUNKNOWN	m_pUnkSink;
	LPUNKNOWN	m_pUnkSrc;

	HWND		m_hNotifyWnd;

public:
	CDiscFormatEraseEvent();
	virtual ~CDiscFormatEraseEvent();

	static CDiscFormatEraseEvent* CreateEventSink();

	bool ConnectDiscFormatErase(CDiscFormatErase*);
	inline void SetHwnd(HWND hWnd){m_hNotifyWnd = hWnd;}

    DECLARE_INTERFACE_MAP()

    BEGIN_INTERFACE_PART(FormatEraseEvents, DDiscFormat2EraseEvents)
		//
		// IDispatch Methods
		//
        STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
        STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames,
			UINT cNames, LCID lcid, DISPID FAR* rgdispid);
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid,
			WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
			EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);
		//
		// DDiscFormat2EraseEvents Methods
		//
		STDMETHOD_(HRESULT, Update)(LPDISPATCH, LONG , LONG);
    END_INTERFACE_PART(FormatEraseEvents)

protected:
	static BOOL AFXAPI ConnectionAdvise(LPUNKNOWN pUnkSrc, REFIID iid,
										LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD* pdwCookie);
	static BOOL AFXAPI ConnectionUnadvise(LPUNKNOWN pUnkSrc, REFIID iid,
										LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD dwCookie);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DISCFORMATERASEEVENT_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)

