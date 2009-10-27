#if !defined(AFX_TRANSALPHA_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_TRANSALPHA_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

bool TransparentBltU(
     HDC dcDest,         // handle to Dest DC
     int nXOriginDest,   // x-coord of destination upper-left corner
     int nYOriginDest,   // y-coord of destination upper-left corner
     int nWidthDest,     // width of destination rectangle
     int nHeightDest,    // height of destination rectangle
     HDC dcSrc,          // handle to source DC
     int nXOriginSrc,    // x-coord of source upper-left corner
     int nYOriginSrc,    // y-coord of source upper-left corner
     int nWidthSrc,      // width of source rectangle
     int nHeightSrc,     // height of source rectangle
     UINT crTransparent  // color to make transparent
  );

bool AlphaBlendU(HDC dcDest, int x, int y, int cx, int cy,
                 HDC dcSrc, int sx, int sy, int scx, int scy,
                 int alpha);

bool AlphaBlendCK(HDC dcDest, int x, int y, int cx, int cy,
                  HDC dcSrc, int sx, int sy, int scx, int scy,
                  int alpha, COLORREF rgbMask);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSALPHA_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)