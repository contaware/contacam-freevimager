#if !defined(AFX_REMOTECAMPPG_H__B61C4C25_4D02_4CBB_8662_A252A20F6DFF__INCLUDED_)
#define AFX_REMOTECAMPPG_H__B61C4C25_4D02_4CBB_8662_A252A20F6DFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// RemoteCamPpg.h : Declaration of the CRemoteCamPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CRemoteCamPropPage : See RemoteCamPpg.cpp.cpp for implementation.

class CRemoteCamPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CRemoteCamPropPage)
	DECLARE_OLECREATE_EX(CRemoteCamPropPage)

// Constructor
public:
	CRemoteCamPropPage();

// Dialog Data
	//{{AFX_DATA(CRemoteCamPropPage)
	enum { IDD = IDD_PROPPAGE_REMOTECAM };
	CString	m_sHost;
	long	m_lPort;
	long    m_lMaxFrames;
	BOOL	m_bDisableResend;
	CString m_sGetFrameUsername;
	CString m_sGetFramePassword;
	BOOL	m_bIPv6;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CRemoteCamPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTECAMPPG_H__B61C4C25_4D02_4CBB_8662_A252A20F6DFF__INCLUDED)
