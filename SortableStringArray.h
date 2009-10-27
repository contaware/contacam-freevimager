//
// SortableStringArray.h
//

#if !defined(AFX_SORTABLESTRINGARRAY_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_SORTABLESTRINGARRAY_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef int (__cdecl *GENERICCOMPAREFN)(const void * elem1, const void * elem2);
typedef int (__cdecl *STRINGCOMPAREFN)(CString * elem1, CString * elem2);

class CSortableStringArray : public CStringArray
{
public:
	void Sort(STRINGCOMPAREFN pfnCompare = Compare);

protected:
	static int __cdecl Compare(CString * pstr1, CString * pstr2);
};


#endif // !defined(AFX_SORTABLESTRINGARRAY_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
