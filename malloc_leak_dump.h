#ifndef MALLOC_LEAK_DUMP_H
#define MALLOC_LEAK_DUMP_H

/*
  When _DEBUG is defined, the CRT maps all calls to heap functions such 
  as malloc or free to debug versions of those functions that operate 
  in the debug heap. MFC already initializes the reporting for us by 
  calling _CrtSetDbgFlag.
  In addition, defining _CRTDBG_MAP_ALLOC before the inclusion of 
  crtdbg.h, will show malloc leaks/corruptions WITH file path and line 
  number (MFC's new already does it). 

  We could define _CRTDBG_MAP_ALLOC in project's "Property Pages" under 
  "C/C++ - Preprocessor Definitions", but that alone is not enough, if 
  crtdbg.h is not included. All the libs like giflib, libjpeg, ... do 
  not include afx.h, so also crtdbg.h is usually not included. 
  
  In conclusion if a malloc memory leak/corruption is displayed, but 
  the exact file path and line number are not shown, we must include 
  this file (before any other include) in all source files we suspect 
  being the cause of the leak!!
  
  References:
  https://learn.microsoft.com/en-us/cpp/c-runtime-library/find-memory-leaks-using-the-crt-library
  https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-debug-heap-details
  https://learn.microsoft.com/en-us/cpp/c-runtime-library/debug-versions-of-heap-allocation-functions
*/

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#endif
