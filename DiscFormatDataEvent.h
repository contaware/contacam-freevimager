#if !defined(AFX_DISCFORMATDATAEVENT_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DISCFORMATDATAEVENT_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_
///////////////////////////////////////////////////////////////////////
// DiscFormatDataEvent.h
//
// Wrapper for DDiscFormat2DataEvents Interface Events
//
// Written by Eric Haddan
//
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Imapi2Small.h"

class CDiscFormatData;

// CDiscFormatDataEvent command target

class CDiscFormatDataEvent : public CCmdTarget
{
	DECLARE_DYNAMIC(CDiscFormatDataEvent)
private:
	LPTYPEINFO  m_ptinfo;           // ITest type information
	DWORD		m_dwCookie;
	LPUNKNOWN	m_pUnkSink;
	LPUNKNOWN	m_pUnkSrc;
	HWND		m_hNotifyWnd;

public:
	CDiscFormatDataEvent();
	virtual ~CDiscFormatDataEvent();

	static CDiscFormatDataEvent* CreateEventSink();

	bool ConnectDiscFormatData(CDiscFormatData*);
	inline void SetHwnd(HWND hWnd){m_hNotifyWnd = hWnd;}

    DECLARE_INTERFACE_MAP()

    BEGIN_INTERFACE_PART(FormatDataEvents, DDiscFormat2DataEvents)
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
		// DDiscFormat2DataEvents Methods
		//
		STDMETHOD_(HRESULT, Update)(LPDISPATCH, LPDISPATCH);
    END_INTERFACE_PART(FormatDataEvents)

protected:
	static BOOL AFXAPI ConnectionAdvise(LPUNKNOWN pUnkSrc, REFIID iid,
										LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD* pdwCookie);
	static BOOL AFXAPI ConnectionUnadvise(LPUNKNOWN pUnkSrc, REFIID iid,
										LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD dwCookie);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DISCFORMATDATAEVENT_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)

