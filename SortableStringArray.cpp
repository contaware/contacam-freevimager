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

int CSortableStringArray::Compare(CString * pstr1, CString * pstr2)
{
	ASSERT(pstr1);
	ASSERT(pstr2);
	return pstr1->CompareNoCase(*pstr2);	
}

void CSortableStringArray::Sort(STRINGCOMPAREFN pfnCompare /*=CSortedStringArray::Compare*/)
{
	CString * prgstr = GetData();
	qsort(prgstr,GetSize(), sizeof(CString), (GENERICCOMPAREFN)pfnCompare);
}
