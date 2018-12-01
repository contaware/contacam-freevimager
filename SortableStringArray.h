//
// SortableStringArray.h
//

#if !defined(AFX_SORTABLESTRINGARRAY_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_SORTABLESTRINGARRAY_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

// Natural order string comparision functions
extern int __cdecl CompareNatural(const CString* pstr1, const CString* pstr2);
extern int __cdecl CompareNaturalReverse(const CString* pstr1, const CString* pstr2);

class CSortableStringArray : public CStringArray
{
public:
	BOOL InStringArray(const CString& s);
	BOOL InStringArrayNoCase(const CString& s);
	void Sort(BOOL bAscending = TRUE);
};

#endif // !defined(AFX_SORTABLESTRINGARRAY_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
