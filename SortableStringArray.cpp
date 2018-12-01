//
// SortableStringArray.cpp
//

#include "stdafx.h"
#include "SortableStringArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int __cdecl CompareNatural(const CString* pstr1, const CString* pstr2)
{
	if (pstr1 == NULL || pstr2 == NULL)
		return 0;
	LPCWSTR pwstr1 = (LPCWSTR)(*pstr1);
	LPCWSTR pwstr2 = (LPCWSTR)(*pstr2);
	return StrCmpLogicalW(pwstr1, pwstr2);
}

int __cdecl CompareNaturalReverse(const CString* pstr1, const CString* pstr2)
{
	if (pstr1 == NULL || pstr2 == NULL)
		return 0;
	LPCWSTR pwstr1 = (LPCWSTR)(*pstr1);
	LPCWSTR pwstr2 = (LPCWSTR)(*pstr2);
	return StrCmpLogicalW(pwstr2, pwstr1);
}

BOOL CSortableStringArray::InStringArray(const CString& s)
{
	for (int i = 0; i < GetSize(); i++)
	{
		if (s.Compare(GetAt(i)) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CSortableStringArray::InStringArrayNoCase(const CString& s)
{
	for (int i = 0; i < GetSize(); i++)
	{
		if (s.CompareNoCase(GetAt(i)) == 0)
			return TRUE;
	}
	return FALSE;
}

void CSortableStringArray::Sort(BOOL bAscending/*=TRUE*/)
{
	CString * prgstr = GetData();
	typedef int(__cdecl *GENERICCOMPAREFN)(const void * elem1, const void * elem2);
	if (bAscending)
		qsort(prgstr, GetSize(), sizeof(CString), (GENERICCOMPAREFN)::CompareNatural);
	else
		qsort(prgstr, GetSize(), sizeof(CString), (GENERICCOMPAREFN)::CompareNaturalReverse);
}
