// ZipInternalInfo.cpp: implementation of the CZipInternalInfo class.
//
////////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2000 Tadeusz Dracz.
//  For conditions of distribution and use, see copyright notice in ZipArchive.h
////////////////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "ZipInternalInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZipInternalInfo::CZipInternalInfo()
{
	m_iBufferSize = 16384;
}

CZipInternalInfo::~CZipInternalInfo()
{

}

void CZipInternalInfo::Init()
{
	m_pBuffer.Allocate(m_iBufferSize);
}
