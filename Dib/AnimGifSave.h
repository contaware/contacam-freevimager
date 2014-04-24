#ifndef _INC_ANIMGIFSAVE
#define _INC_ANIMGIFSAVE

class CAnimGifSave
{
public:
	CAnimGifSave() {ResetVars();};
	CAnimGifSave(	const CString& sFileName,
					const CString& sTempDir,
					BOOL bDitherColorConversion,
					UINT uiMaxColors,
					UINT uiPlayTimes,
					COLORREF crBackgroundColor)
	{
		ResetVars();
		Open(sFileName,
			sTempDir,
			bDitherColorConversion,
			uiMaxColors,
			uiPlayTimes,
			crBackgroundColor);
	};
	virtual ~CAnimGifSave() {Close();};
	void Open(	const CString& sFileName,
				const CString& sTempDir,
				BOOL bDitherColorConversion,
				UINT uiMaxColors,
				UINT uiPlayTimes,
				COLORREF crBackgroundColor);
	BOOL CreatePaletteFromColors(UINT uiMaxColors, RGBQUAD* pColors);
	BOOL Close();
	BOOL SaveAsAnimGIF(const CDib* pDib, int nMsDelay);
	
	

private:
	void ResetVars();
	void Free();

	CString m_sFileName;
	CString m_sTempDir;
	CString m_sConsecDiffFileName;
	CString m_sBkgDiffFileName;
	CPalette* m_pUniquePalette;
	BOOL m_bDitherColorConversion;
	UINT m_uiMaxColors;
	// m_uiPlayTimes = 0: Infinite
	// m_uiPlayTimes = 1: Show All Frames One Time
	// m_uiPlayTimes = 2: Show All Frames Two Times
	// ...
	UINT m_uiPlayTimes;
	COLORREF m_crBackgroundColor;
	int m_nFramePos;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	CDib* m_pPrevDib;
	CDib* m_pConsecDiffTranspDib;
	CDib* m_pBkgDiffTranspDib;
	CDib* m_pConsecDiffSaveDib;
	CDib* m_pBkgDiffSaveDib;
};

#endif //!_INC_ANIMGIFSAVE