#if !defined(AFX_BIGFILE_H__79E0E6BD_25D6_4B82_85C5_AB397D9EC368__INCLUDED_)
#define AFX_BIGFILE_H__79E0E6BD_25D6_4B82_85C5_AB397D9EC368__INCLUDED_

#include <afxwin.h>         // MFC core and standard components

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if (_MSC_VER <= 1200)
class CBigFile : public CFile  
{
	DECLARE_DYNAMIC(CBigFile)
public:
	CBigFile() : CFile() {;};
	CBigFile(int hFile) : CFile(hFile) {;};
	CBigFile(LPCTSTR lpszFileName, UINT nOpenFlags ) :
			CFile(lpszFileName, nOpenFlags) {;};
	virtual ~CBigFile() {;};

	ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	ULONGLONG GetPosition64() const;
	ULONGLONG GetLength64() const;
};
#endif

#endif // !defined(AFX_BIGFILE_H__79E0E6BD_25D6_4B82_85C5_AB397D9EC368__INCLUDED_)
