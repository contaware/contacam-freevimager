/******************************************************************************
|* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
|* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
|* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
|* PARTICULAR PURPOSE.
|* 
|* Copyright 1995-2005 Nero AG. All Rights Reserved.
|*-----------------------------------------------------------------------------
|* NeroSDK / NeroCmd
|*
|* PROGRAM: SimpleStringArray.h
|*
|* PURPOSE: Declaration of a simple string vector class
******************************************************************************/


#ifndef _SIMPLE_STRING_ARRAY_H_
#define _SIMPLE_STRING_ARRAY_H_

#include "stdafx.h"

// Simple vector of strings

class CSimpleStringArray {
public:
	int GetSize() const;
	std::vector<LPSTR> vect;
	bool m_bOwnData;
	
	CSimpleStringArray () {m_bOwnData = true;}
	~CSimpleStringArray ();

	void Add (LPSTR psString);
};

#endif