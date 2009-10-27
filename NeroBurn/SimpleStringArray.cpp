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
|* PROGRAM: SimpleStringArray.cpp
|*
|* PURPOSE: Simple string vector class implementation
******************************************************************************/


#include "stdafx.h"
#include "SimpleStringArray.h"


// CSimpleStringArray destructor

CSimpleStringArray::~CSimpleStringArray ()
{
	if (m_bOwnData)
	{
		// Iterate through the vector and kill the strings.
		//
		for (int i = 0; i < (int)(vect.size()); i ++)
		{
			LPSTR psString = vect[i];

			delete [] psString;
		}
	}
}


// Add a string to the CSimpleStringArray vector

void CSimpleStringArray::Add (LPSTR psString)
{
	vect.insert (vect.end (), psString);
}

int CSimpleStringArray::GetSize() const
{
	return vect.size();
}
