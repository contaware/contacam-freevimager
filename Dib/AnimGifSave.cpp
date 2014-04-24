#include "stdafx.h"
#include "dib.h"
#include "Quantizer.h"
#include "AnimGifSave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CAnimGifSave::Open(const CString& sFileName,
						const CString& sTempDir,
						BOOL bDitherColorConversion,
						UINT uiMaxColors,
						UINT uiPlayTimes,
						COLORREF crBackgroundColor)
{
	Close();
	m_nFramePos = 0;
	m_sFileName = sFileName;
	m_sTempDir = sTempDir;

	m_bDitherColorConversion = bDitherColorConversion;
	m_uiMaxColors = uiMaxColors;
	m_uiPlayTimes = uiPlayTimes;
	m_crBackgroundColor = crBackgroundColor;
}

BOOL CAnimGifSave::Close()
{
	try
	{
		Free();
		ULARGE_INTEGER size1 = ::GetFileSize64(m_sConsecDiffFileName);
		ULARGE_INTEGER size2 = ::GetFileSize64(m_sBkgDiffFileName);
		if (size1.QuadPart <= size2.QuadPart)
		{
			if (::IsExistingFile(m_sBkgDiffFileName))
				CFile::Remove(m_sBkgDiffFileName);
			if (::IsExistingFile(m_sFileName))
				CFile::Remove(m_sFileName);
			if (::IsExistingFile(m_sConsecDiffFileName))
				CFile::Rename(m_sConsecDiffFileName, m_sFileName);
		}
		else
		{
			if (::IsExistingFile(m_sConsecDiffFileName))
				CFile::Remove(m_sConsecDiffFileName);
			if (::IsExistingFile(m_sFileName))
				CFile::Remove(m_sFileName);
			if (::IsExistingFile(m_sBkgDiffFileName))
				CFile::Rename(m_sBkgDiffFileName, m_sFileName);
		}
		return TRUE;
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
		return FALSE;
	}
}

void CAnimGifSave::ResetVars()
{
	m_nFramePos = 0;
	m_bDitherColorConversion = FALSE;
	m_uiMaxColors = 255;
	m_uiPlayTimes = 0;
	m_crBackgroundColor = RGB(0,0,0);
	m_dwWidth = 0;
	m_dwHeight = 0;
	m_pUniquePalette = NULL;
	m_pPrevDib = NULL;
	m_pConsecDiffTranspDib = NULL;
	m_pBkgDiffTranspDib = NULL;
	m_pConsecDiffSaveDib = NULL;
	m_pBkgDiffSaveDib = NULL;
}

void CAnimGifSave::Free()
{	
	if (m_pPrevDib)
	{
		delete m_pPrevDib;
		m_pPrevDib = NULL;
	}
	if (m_pConsecDiffTranspDib)
	{
		delete m_pConsecDiffTranspDib;
		m_pConsecDiffTranspDib = NULL;
	}
	if (m_pBkgDiffTranspDib)
	{
		delete m_pBkgDiffTranspDib;
		m_pBkgDiffTranspDib = NULL;
	}
	if (m_pConsecDiffSaveDib)
	{
		delete m_pConsecDiffSaveDib;
		m_pConsecDiffSaveDib = NULL;
	}
	if (m_pBkgDiffSaveDib)
	{
		delete m_pBkgDiffSaveDib;
		m_pBkgDiffSaveDib = NULL;
	}
	if (m_pUniquePalette)
	{
		m_pUniquePalette->DeleteObject();
		delete m_pUniquePalette;
		m_pUniquePalette = NULL;
	}
}

BOOL CAnimGifSave::CreatePaletteFromColors(UINT uiMaxColors, RGBQUAD* pColors)
{
	m_pUniquePalette = new CPalette;
	if (m_pUniquePalette)
		return CDib::CreatePaletteFromColors(m_pUniquePalette, uiMaxColors, pColors);
	else
		return FALSE;
}

BOOL CAnimGifSave::SaveAsAnimGIF(const CDib* pDib, int nMsDelay)
{
	// Check
	if (!pDib)
		return FALSE;

	// Init Vars
	BOOL res1 = FALSE;
	BOOL res2 = FALSE;
	CDib Dib = *pDib;

	// Allocate Dibs
	if (m_nFramePos == 0)
	{
		ASSERT(m_sTempDir != _T(""));
		m_sConsecDiffFileName = ::MakeTempFileName(m_sTempDir, m_sFileName);
		m_sBkgDiffFileName = ::MakeTempFileName(m_sTempDir, m_sFileName);
		m_pPrevDib = new CDib;
		m_pConsecDiffTranspDib = new CDib;
		m_pBkgDiffTranspDib = new CDib;
		m_pConsecDiffSaveDib = new CDib;
		m_pBkgDiffSaveDib = new CDib;
		if (!m_pPrevDib || !m_pConsecDiffTranspDib ||
			!m_pBkgDiffTranspDib || !m_pConsecDiffSaveDib || !m_pBkgDiffSaveDib)
			return FALSE;
		m_dwWidth = Dib.GetWidth();
		m_dwHeight = Dib.GetHeight();
	}
	else
	{
		if (!Dib.StretchBitsMaintainAspectRatio(m_dwWidth,
												m_dwHeight,
												m_crBackgroundColor))
			return FALSE;
	}

	// Convert to 8 bpp with m_uiMaxColors colors
	if (m_pUniquePalette)
	{
		if (m_bDitherColorConversion)
			Dib.ConvertTo8bitsErrDiff(m_pUniquePalette);
		else
			Dib.ConvertTo8bits(m_pUniquePalette);
	}
	else
	{
		RGBQUAD* pGIFColors = (RGBQUAD*)new RGBQUAD[m_uiMaxColors];
		CQuantizer Quantizer(m_uiMaxColors, 8); // max is 255 colors = 256 (8 bits colors) - 1 (transparency index)
		Quantizer.ProcessImage(&Dib);
		Quantizer.SetColorTable(pGIFColors);
		Dib.CreatePaletteFromColors(m_uiMaxColors, pGIFColors);
		if (m_bDitherColorConversion)
			Dib.ConvertTo8bitsErrDiff(Dib.GetPalette());
		else
			Dib.ConvertTo8bits(Dib.GetPalette());
		delete [] pGIFColors;
	}

	// Save Frames
	if (m_nFramePos == 0)
	{
		// Init Dib(s)
		*m_pPrevDib = *m_pConsecDiffSaveDib = Dib;

		// Set up GIF Vars
		m_pConsecDiffSaveDib->GetGif()->SetConfig(	TRUE,				// Screen Color Table
													!m_pUniquePalette,	// Image Color Table?
													TRUE,				// Graphic Extension (for Delay)
													(m_uiPlayTimes == 1) ?	FALSE :	// No Loop Count -> Show all frames 1x
													TRUE);				// See GIFSetLoopCount()
		m_pConsecDiffSaveDib->GetGif()->SetRect(	0,
													0,
													m_pConsecDiffSaveDib->GetWidth(),
													m_pConsecDiffSaveDib->GetHeight());
		m_pConsecDiffSaveDib->GetGif()->SetTransparency(TRUE);
		m_pConsecDiffSaveDib->GetGif()->SetTransparencyColorIndex(m_uiMaxColors);
		m_pConsecDiffSaveDib->GetGif()->SetDispose(GIF_DISPOSE_NONE);
		m_pConsecDiffSaveDib->GetGif()->SetDelay(nMsDelay);
		m_pConsecDiffSaveDib->GetGif()->SetBackgroundColorIndex(m_pConsecDiffSaveDib->GetPalette()->
																GetNearestPaletteIndex(m_crBackgroundColor));
		// 0 means loop infinite
		// 1 means loop one time -> Show all frames 2x
		// 2 means loop two times -> Show all frames 3x
		if (m_uiPlayTimes != 1)
			m_pConsecDiffSaveDib->GetGif()->SetLoopCount((m_uiPlayTimes == 0) ? 0 : m_uiPlayTimes - 1);
		*m_pBkgDiffSaveDib = *m_pConsecDiffSaveDib; 
		res1 = m_pConsecDiffSaveDib->SaveFirstGIF(m_sConsecDiffFileName);
		res2 = m_pBkgDiffSaveDib->SaveFirstGIF(m_sBkgDiffFileName);
		m_pBkgDiffSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
	}
	else
	{
		// Calc. Dib with Transparency Index
		*m_pConsecDiffTranspDib = *m_pBkgDiffTranspDib = Dib;
		m_pConsecDiffTranspDib->DiffTransp8(m_pPrevDib, m_uiMaxColors);
		m_pBkgDiffTranspDib->DiffTransp8(m_pBkgDiffSaveDib, m_uiMaxColors);

		// Save Next Dib
		res1 = m_pConsecDiffSaveDib->SaveNextGIF(m_pConsecDiffTranspDib);
		res2 = m_pBkgDiffSaveDib->SaveNextGIF(m_pBkgDiffTranspDib);

		// Update Previous Dib with current one
		*m_pPrevDib = Dib;
	}

	// Ok?
	if (res1 && res2)
	{
		// Inc. Frame Pos
		m_nFramePos++;
		return TRUE;
	}
	else
		return FALSE;
}