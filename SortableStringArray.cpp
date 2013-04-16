//
// SortableStringArray.cpp
//

#include "stdafx.h"
#include "SortableStringArray.h"
#include "Helpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CSortableStringArray::Sort()
{
	CString * prgstr = GetData();
	typedef int (__cdecl *GENERICCOMPAREFN)(const void * elem1, const void * elem2);
	qsort(prgstr, GetSize(), sizeof(CString), (GENERICCOMPAREFN)::CompareNatural);
}