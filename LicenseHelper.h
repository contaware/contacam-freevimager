#if !defined(AFX_LICENSEHELPER_H__6F2B2BF7_7B72_4FC7_99F4_938AFC9A956F__INCLUDED_)
#define AFX_LICENSEHELPER_H__6F2B2BF7_7B72_4FC7_99F4_938AFC9A956F__INCLUDED_

#pragma once

// LicenseHelper.h : header file
//

#include <afxwin.h>
#include "WorkerThread.h"

// The Donor Email Validation Thread Class
class CDonorEmailValidateThread : public CWorkerThread
{
public:
	CDonorEmailValidateThread() { m_bNoDonation = FALSE; };
	virtual ~CDonorEmailValidateThread() { Kill(); };

	// return:	-1:	unknown answer or server busy or no answer from server
	//			0:	bad email
	//			1:	good email
	int DonorEmailValidate();

	volatile BOOL m_bNoDonation;
	CString m_sDonorEmail;

protected:
	static CString GetAppLanguage();
	int Work();
};

extern CDonorEmailValidateThread g_DonorEmailValidateThread;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEHELPER_H__6F2B2BF7_7B72_4FC7_99F4_938AFC9A956F__INCLUDED_)
