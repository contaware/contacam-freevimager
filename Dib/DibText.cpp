#include "stdafx.h"
#include "dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DIBSECTION_DEFAULT_BPP		24

// Font Creation Example:
/*
	CFont Font;
	LOGFONT* pLF;
	pLF = (LOGFONT*)calloc(1, sizeof(LOGFONT));
	_tcscpy(pLF->lfFaceName, _T("Times New Roman"));
	pLF->lfHeight=64;
	pLF->lfWeight=FW_MEDIUM;;
	pLF->lfItalic=1;
	pLF->lfUnderline=0;
	Font.CreateFontIndirect(pLF);

	...

	Font.DeleteObject();
	free(pLF);
*/
BOOL CDib::AddSingleLineText(LPCTSTR szText,
							CRect Pos,		// Positive!
							CFont* pFont,	// If NULL, ANSI_VAR_FONT is selected
							UINT Align,		// Align Combination of: DT_LEFT, DT_TOP, DT_BOTTOM, DT_RIGHT, DT_CENTER, DT_VCENTER
							COLORREF crTextColor,
							int nBkgMode,	// TRANSPARENT or OPAQUE
							COLORREF crBkgColor)
{
	// Check
	if (!IsAddSingleLineTextSupported(m_pBMI))
		return FALSE;

	// Bits
	if (m_pBits)
	{
		// Get Display dc
		HDC hDC = ::GetDC(NULL);
		ASSERT(hDC);
		
		// Create mem dc 
		HDC hTmpDC = ::CreateCompatibleDC(hDC);
		ASSERT(hTmpDC);

		// Select the font in the memory dc
		COLORREF crOldTextColor = ::SetTextColor(hTmpDC, crTextColor);
		COLORREF crOldBkgColor = ::SetBkColor(hTmpDC, crBkgColor);
		int nOldBkgMode = ::SetBkMode(hTmpDC, nBkgMode);
		HFONT hFont = NULL;
		HFONT hOldFont = NULL;
		if (pFont)
			hOldFont = (HFONT)::SelectObject(hTmpDC, (HFONT)(pFont->GetSafeHandle()));
		else
		{
			hFont = (HFONT)::GetStockObject(ANSI_VAR_FONT);
			hOldFont = (HFONT)::SelectObject(hTmpDC, hFont);
		}

		// Calc Text Size
		CRect rcOrigText(0,0,0,0);
		int res = ::DrawText(hTmpDC, szText, -1, rcOrigText, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
		
		// 4 pixel alignment for YVU9 and YUV9 formats
		if (GetCompression() == FCC('YVU9') ||
			GetCompression() == FCC('YUV9'))
		{
			int nModHeight = rcOrigText.Height() % 4;
			if (nModHeight > 0)
				rcOrigText.bottom += 4 - nModHeight;
			int nModWidth = rcOrigText.Width() % 4;
			if (nModWidth > 0)
				rcOrigText.right += 4 - nModWidth;
		}
		// Even Size (necessary for I420, YUY2, ... formats)
		// Do it also for RGB formats so that the displayed
		// text is not changed while switching between RGB and I420, YUY2
		else
		{
			if (rcOrigText.Height() % 2)
				rcOrigText.bottom++;
			if (rcOrigText.Width() % 2)
				rcOrigText.right++;
		}

		// - Create a new dibsection and select it in to the memory dc
		// - Calc. Destination Scan Line Size
		LPBITMAPINFO pBMI;
		int nTextWidth = rcOrigText.Width();
		int nTextHeight = rcOrigText.Height();
		DWORD uiDIBSrcScanLineSize, uiDIBDstScanLineSize;
		if (m_pBMI->bmiHeader.biCompression == BI_RGB ||
			m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			pBMI = (LPBITMAPINFO)new BYTE[GetBMISize()];
			memcpy(pBMI, m_pBMI, GetBMISize());
			uiDIBSrcScanLineSize = DWALIGNEDWIDTHBYTES(nTextWidth * GetBitCount());
			uiDIBDstScanLineSize = DWALIGNEDWIDTHBYTES(GetWidth() * GetBitCount());
		}
		else
		{
			pBMI = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
			memset(pBMI, 0, sizeof(BITMAPINFOHEADER));
			pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pBMI->bmiHeader.biCompression = BI_RGB;
			pBMI->bmiHeader.biPlanes = 1;
			pBMI->bmiHeader.biBitCount = DIBSECTION_DEFAULT_BPP;
			uiDIBSrcScanLineSize = ::CalcYUVStride(m_pBMI->bmiHeader.biCompression, nTextWidth);
			uiDIBDstScanLineSize = ::CalcYUVStride(m_pBMI->bmiHeader.biCompression, GetWidth());
		}
		pBMI->bmiHeader.biWidth = nTextWidth;
		pBMI->bmiHeader.biHeight = nTextHeight;
		pBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biBitCount) * pBMI->bmiHeader.biHeight;
		LPBYTE pSrcBits = NULL;
		HBITMAP hTmpDib = ::CreateDIBSection(	hDC,
												(const BITMAPINFO*)pBMI,
												DIB_RGB_COLORS,
												(void**)&pSrcBits,
												NULL, 0);
		if (!hTmpDib)
		{
			delete [] pBMI;
			::SelectObject(hTmpDC, hOldFont);
			::SetBkMode(hTmpDC, nOldBkgMode);
			::SetBkColor(hTmpDC, crOldBkgColor);
			::SetTextColor(hTmpDC, crOldTextColor);
			::DeleteDC(hTmpDC);
			::ReleaseDC(NULL, hDC);
			return FALSE;
		}
		HGDIOBJ hOldBitmap = ::SelectObject(hTmpDC, hTmpDib);
		
		/* WinUser.h
		#define DT_TOP                      0x00000000
		#define DT_LEFT                     0x00000000
		#define DT_CENTER                   0x00000001
		#define DT_RIGHT                    0x00000002
		#define DT_VCENTER                  0x00000004
		#define DT_BOTTOM                   0x00000008
		*/
		CRect rcText(rcOrigText);
		// Calc Text Rectangle (Position) With Respect to this Dib
		if ((Align & 0xF) == (DT_LEFT | DT_TOP)) 
		{	
			rcText.left = Pos.left;
			rcText.top = Pos.top;
			rcText.right = Pos.left + nTextWidth;
			rcText.bottom = Pos.top + nTextHeight;
		}
		else if ((Align & 0xF) == (DT_LEFT | DT_BOTTOM))
		{	
			rcText.left = Pos.left;
			rcText.top = Pos.bottom - nTextHeight;
			rcText.right = Pos.left + nTextWidth;
			rcText.bottom = Pos.bottom;
		}
		else if ((Align & 0xF) == (DT_RIGHT | DT_BOTTOM))
		{
			rcText.left = Pos.right - nTextWidth;
			rcText.top = Pos.bottom - nTextHeight;
			rcText.right = Pos.right;
			rcText.bottom = Pos.bottom;
		}
		else if ((Align & 0xF) == (DT_RIGHT | DT_TOP))
		{
			rcText.left = Pos.right - nTextWidth;
			rcText.top = Pos.top;
			rcText.right = Pos.right;
			rcText.bottom = Pos.top + nTextHeight;
		}
		else if ((Align & 0xF) == (DT_LEFT | DT_VCENTER))
		{	
			rcText.left = Pos.left;
			rcText.top = Pos.top + (Pos.Height()/2) - (nTextHeight/2);
			rcText.right = Pos.left + nTextWidth;
			rcText.bottom = Pos.top + (Pos.Height()/2) + (nTextHeight/2);
		}
		else if ((Align & 0xF) == (DT_RIGHT | DT_VCENTER))
		{	
			rcText.left = Pos.right - nTextWidth;
			rcText.top = Pos.top + (Pos.Height()/2) - (nTextHeight/2);
			rcText.right = Pos.right;
			rcText.bottom = Pos.top + (Pos.Height()/2) + (nTextHeight/2);
		}
		else if ((Align & 0xF) == (DT_CENTER | DT_TOP)) 
		{	
			rcText.left = Pos.left + (Pos.Width()/2) - (nTextWidth/2);
			rcText.top = Pos.top;
			rcText.right = Pos.left + (Pos.Width()/2) + (nTextWidth/2);
			rcText.bottom = Pos.top + nTextHeight;
		}
		else if ((Align & 0xF) == (DT_CENTER | DT_BOTTOM)) 
		{	
			rcText.left = Pos.left + (Pos.Width()/2) - (nTextWidth/2);
			rcText.top = Pos.bottom - nTextHeight;
			rcText.right = Pos.left + (Pos.Width()/2) + (nTextWidth/2);
			rcText.bottom = Pos.bottom;
		}
		else if ((Align & 0xF) == (DT_CENTER | DT_VCENTER)) 
		{	
			rcText.left = Pos.left + (Pos.Width()/2) - (nTextWidth/2);
			rcText.top = Pos.top + (Pos.Height()/2) - (nTextHeight/2);
			rcText.right = Pos.left + (Pos.Width()/2) + (nTextWidth/2);
			rcText.bottom = Pos.top + (Pos.Height()/2) + (nTextHeight/2);
		}

		// Clip
		if (rcText.left < Pos.left)
			rcText.left = Pos.left;
		if (rcText.right > Pos.right)
			rcText.right = Pos.right;
		if (rcText.top < Pos.top)
			rcText.top = Pos.top;
		if (rcText.bottom > Pos.bottom)
			rcText.bottom = Pos.bottom;

		// Copy Background if in Transparent Mode
		if (nBkgMode == TRANSPARENT)
		{
			if (m_pBMI->bmiHeader.biCompression == BI_RGB ||
				m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
			{
				VERIFY(CopyBits(m_pBMI->bmiHeader.biCompression,// FourCC
									GetBitCount(),					// Bit Count
									0,								// Dst X Offset
									0,								// Dst Y Offset
									rcText.left,					// Src X Offset
									GetHeight() - rcText.bottom,	// Src Y Offset
									rcText.Width(),					// Width to Copy
									rcText.Height(),				// Height to Copy
									rcText.Height(),				// Dst Height
									GetHeight(),					// Src Height
									pSrcBits,						// Dst Bits
									m_pBits,						// Src Bits
									uiDIBSrcScanLineSize,			// Dst Scan Line Size in Bytes
									uiDIBDstScanLineSize));			// Src Scan Line Size in Bytes
			}
			else 
			{
				CDib Dib;
				Dib.SetBMI(pBMI);
				Dib.SetBits(pSrcBits);
				if (Dib.Compress(m_pBMI->bmiHeader.biCompression, uiDIBSrcScanLineSize))
				{
					if (CopyBits(	m_pBMI->bmiHeader.biCompression,// FourCC
									GetBitCount(),					// Bit Count
									0,								// Dst X Offset
									0,								// Dst Y Offset
									rcText.left,					// Src X Offset
									rcText.top,						// Src Y Offset
									rcText.Width(),					// Width to Copy
									rcText.Height(),				// Height to Copy
									rcText.Height(),				// Dst Height
									GetHeight(),					// Src Height
									Dib.GetBits(),					// Dst Bits
									m_pBits,						// Src Bits
									uiDIBSrcScanLineSize,			// Dst Scan Line Size in Bytes
									uiDIBDstScanLineSize))			// Src Scan Line Size in Bytes
					{
						if (Dib.Decompress(DIBSECTION_DEFAULT_BPP))
						{
							LPBYTE ps = Dib.GetBits();
							LPBYTE pd = pSrcBits;
							DWORD uiDIBScanLineSize = DWALIGNEDWIDTHBYTES(pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biBitCount);
							for (int line = 0 ; line < pBMI->bmiHeader.biHeight ; line++)
							{
								memcpy(pd, ps, uiDIBScanLineSize);
								ps += uiDIBScanLineSize;
								pd += uiDIBScanLineSize;
							}
						}
					}
				}
			}
		}
		else
		{
			// Necessary because text rect may be bigger than the
			// calculated one do to the even text size!
			HBRUSH hBrush = ::CreateSolidBrush(crBkgColor);
			ASSERT(hBrush);
			VERIFY(::FillRect(	hTmpDC,
								&rcOrigText,
								hBrush));
			::DeleteObject(hBrush);
		}

		// Draw Text
		res = ::DrawText(hTmpDC, szText, -1, rcOrigText, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP);

		// Copy Text To Bitmap Bits
		if (m_pBMI->bmiHeader.biCompression == BI_RGB ||
			m_pBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			VERIFY(CopyBits(m_pBMI->bmiHeader.biCompression,	// FourCC
							GetBitCount(),						// Bit Count
							rcText.left,						// Dst X Offset
							GetHeight() - rcText.bottom,		// Dst Y Offset
							0,									// Src X Offset
							0,									// Src Y Offset
							rcText.Width(),						// Width to Copy
							rcText.Height(),					// Height to Copy
							GetHeight(),						// Dst Height
							rcText.Height(),					// Src Height
							m_pBits,							// Dst Bits
							pSrcBits,							// Src Bits
							uiDIBDstScanLineSize,				// Dst Scan Line Size in Bytes
							uiDIBSrcScanLineSize));				// Src Scan Line Size in Bytes
		}
		else 
		{
			CDib Dib;
			Dib.SetBMI(pBMI);
			Dib.SetBits(pSrcBits);
			if (Dib.Compress(m_pBMI->bmiHeader.biCompression, uiDIBSrcScanLineSize))
			{
				VERIFY(CopyBits(m_pBMI->bmiHeader.biCompression,	// FourCC
								GetBitCount(),						// Bit Count
								rcText.left,						// Dst X Offset
								rcText.top,							// Dst Y Offset
								0,									// Src X Offset
								0,									// Src Y Offset
								rcText.Width(),						// Width to Copy
								rcText.Height(),					// Height to Copy
								GetHeight(),						// Dst Height
								rcText.Height(),					// Src Height
								m_pBits,							// Dst Bits
								Dib.GetBits(),						// Src Bits
								uiDIBDstScanLineSize,				// Dst Scan Line Size in Bytes
								uiDIBSrcScanLineSize));				// Src Scan Line Size in Bytes
			}
		}

		// Cleanup 
		::SelectObject(hTmpDC, hOldFont);
		::SetBkMode(hTmpDC, nOldBkgMode);
		::SetBkColor(hTmpDC, crOldBkgColor);
		::SetTextColor(hTmpDC, crOldTextColor);
		::SelectObject(hTmpDC, hOldBitmap);
		::DeleteObject(hTmpDib);
		::DeleteDC(hTmpDC);
		::ReleaseDC(NULL, hDC);
		delete [] pBMI;
		return TRUE;
	}
	// Dib Section
	else
	{
		if (!m_hDibSection)
			return FALSE;

		// Create memory dc
		HDC hDC = ::GetDC(NULL);
		HDC hTmpDC = ::CreateCompatibleDC(hDC);

		// Select dibsection into the temp memory dc
		HGDIOBJ hOldBitmap = ::SelectObject(hTmpDC, m_hDibSection);

		// Select the font in to the temp memory dc
		HFONT hFont = NULL;
		HFONT hOldFont = NULL;
		if (pFont)
			hOldFont = (HFONT)::SelectObject(hTmpDC, (HFONT)(pFont->GetSafeHandle()));
		else
		{
			hFont = (HFONT)::GetStockObject(ANSI_VAR_FONT);
			hOldFont = (HFONT)::SelectObject(hTmpDC, hFont);
		}
		// Draw the text to the temp memory dc
		COLORREF crOldTextColor = ::SetTextColor(hTmpDC, crTextColor);
		COLORREF crOldBkgColor = ::SetBkColor(hTmpDC, crBkgColor);
		int nOldBkgMode = ::SetBkMode(hTmpDC, nBkgMode);
		int nHeightText = ::DrawText(hTmpDC, szText, -1, Pos, Align | DT_SINGLELINE | DT_NOCLIP);

		// Cleanup 
		::SelectObject(hTmpDC, hOldFont);
		::SetBkMode(hTmpDC, nOldBkgMode);
		::SetBkColor(hTmpDC, crOldBkgColor);
		::SetTextColor(hTmpDC, crOldTextColor);
		::SelectObject(hTmpDC, hOldBitmap);
		::DeleteDC(hTmpDC);
		::ReleaseDC(NULL, hDC);

		return TRUE;
	}
}
