/*
Module : PJNLoadLibraryFromSystem32.H
Purpose: Simple header definition of a PJNLoadLibraryFromSystem32 function. This function is designed to 
         avoid calling LoadLibrary with no path information which can result in DLL Planting security
         issues. For more information about this security issue please see 
         http://msdn.microsoft.com/en-us/library/windows/desktop/ff919712(v=vs.85).aspx. Please note that
         PJNLoadLibraryFromSystem32 should only be called with a "lpFileName" parameter with no path 
         information.
Created: PJN / 30-09-2012

Copyright (c) 2012 - 2013 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////// Defines ///////////////////////////////////////

#pragma once

#ifndef __PJNLOADLIBRARYFROMSYSTEM32_H__
#define __PJNLOADLIBRARYFROMSYSTEM32_H__


/////////////////////////////// Function calls ////////////////////////////////

inline HMODULE PJNLoadLibraryFromSystem32(LPCTSTR lpFileName)
{
  //Get the Windows System32 directory
  TCHAR szFullPath[_MAX_PATH];
  szFullPath[0] = _T('\0');
  if (GetSystemDirectory(szFullPath, _countof(szFullPath)) == 0)
    return NULL;

  //Setup the full path and delegate to LoadLibrary    
  _tcscat_s(szFullPath, _countof(szFullPath), _T("\\"));
  _tcscat_s(szFullPath, _countof(szFullPath), lpFileName);
  return LoadLibrary(szFullPath);
}

#endif //__PJNLOADLIBRARYFROMSYSTEM32_H__
