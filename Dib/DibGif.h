#ifndef _INC_DIBGIF
#define _INC_DIBGIF

// Defines
#define GIF_TRANSPARENT		0x01
#define GIF_USER_INPUT		0x02
#define GIF_DISPOSE_MASK	0x07
#define GIF_DISPOSE_SHIFT	2
#define GIF_COLORINDEX_NOT_DEFINED	-1
#define GIF_DISPOSE_NONE	0		// No disposal specified. The decoder is
									// not required to take any action.
#define GIF_DISPOSE_LEAVE	1		// Do not dispose. The graphic is to be left
									// in place.
#define GIF_DISPOSE_BACKGND	2		// Restore to background color. The area used by the
									// graphic must be restored to the background color.
#define GIF_DISPOSE_RESTORE	3		// Restore to previous. The decoder is required to
									// restore the area overwritten by the graphic with
									// what was there prior to rendering the graphic.
#define GIF_NSEXT_LOOP      0x01    // Loop Count field code

class CGif
{
public:
	CGif::CGif();
	CGif(const CGif& gif); // Copy Constructor
	CGif& operator=(const CGif& gif); // Copy Assignment
	CGif::~CGif();
	void Free();
	BOOL Close();
	BOOL LoadClose();
	BOOL SaveClose();
	
	void SetConfig(	BOOL bHasScreenColorTable,
					BOOL bHasImageColorTable,
					BOOL bHasGraphicExtension,
					BOOL bHasLoopExtension)
					{
						m_bHasScreenColorTable = bHasScreenColorTable;
						m_bHasImageColorTable = bHasImageColorTable;
						m_bHasGraphicExtension = bHasGraphicExtension;
						m_bHasLoopExtension = bHasLoopExtension;
					};

	void SetRect(	int nLeft,
					int nTop,
					int nWidth,
					int nHeight)
					{
						m_nLeft = nLeft;
						m_nTop = nTop;
						m_nWidth = nWidth;
						m_nHeight = nHeight;
					};

	int GetDelay(){return (m_nDelay * 10);}; // Returns Delay in MilliSeconds
	void SetDelay(int nMilliSeconds) {m_nDelay = nMilliSeconds / 10;};

	// 0 means loop infinite
	// 1 means loop one time -> Show all frames 2x
	// 2 means loop two times -> show all frames 3x
	// ..
	// If m_bHasLoopExtension is not set Loop zero times -> Show all frames 1x
	void SetLoopCount(WORD wLoopCount) {m_wLoopCount = wLoopCount;};
	WORD GetLoopCount() const {return m_wLoopCount;};

	// Play times helper function:
	// 0 means play infinite times
	// 1 means play 1x time
	// 2 means play 2x times
	// ..
	int GetPlayTimes() const {	return (m_bHasLoopExtension == FALSE) ?
								1 : ((m_wLoopCount == 0) ? 0 : (int)m_wLoopCount + 1);};

	void SetComment(LPCTSTR szComment);
	char* GetComment() const {return m_lpszComment;};

	/*	GIF_DISPOSE_NONE	0	
		No disposal specified. The decoder is
		not required to take any action.
		
		GIF_DISPOSE_LEAVE	1
		Do not dispose. The graphic is to be left in place.
		
		GIF_DISPOSE_BACKGND	2
		Restore to background color.
		The area used by the
		graphic must be restored to the background color.
		
		GIF_DISPOSE_RESTORE	3
		Restore to previous. The decoder is required to
		restore the area overwritten by the graphic with
		what was there prior to rendering the graphic.
	*/
	void SetDispose(BYTE Dispose) {	m_nFlags &= ~(GIF_DISPOSE_MASK << GIF_DISPOSE_SHIFT);
									m_nFlags |= ((Dispose & GIF_DISPOSE_MASK) << GIF_DISPOSE_SHIFT);};
	BYTE GetDispose() const {return (m_nFlags >> GIF_DISPOSE_SHIFT) & GIF_DISPOSE_MASK;};

	void SetBackgroundColorIndex(int nBackgndColorIndex) {m_nBackgndColorIndex = nBackgndColorIndex;};
	int GetBackgroundColorIndex() const {return m_nBackgndColorIndex;};
	
	/*
		Indicates whether or not user input is
		expected before continuing. If the flag is set, processing will
		continue when user input is entered. The nature of the User input
		is determined by the application (Carriage Return, Mouse Button
		Click, etc.).
		When the flag is set, indicating that user
		input is expected, the decoder may sound the bell (0x07) to alert
		the user that input is being expected. In the absence of a
		specified Delay Time, the decoder should wait for user input
		indefinitely. It is recommended that the encoder does not set the User
		Input Flag without a Delay Time specified.
	*/
	void SetUserInput(BOOL bUserInput) {bUserInput ? (m_nFlags |= GIF_USER_INPUT) : (m_nFlags &= ~GIF_USER_INPUT);};
	BOOL HasUserInput() const {return (m_nFlags & GIF_USER_INPUT) ? TRUE : FALSE;};

	void SetTransparency(BOOL bTransparency) {bTransparency ? (m_nFlags |= GIF_TRANSPARENT) : (m_nFlags &= ~GIF_TRANSPARENT);};
	BOOL HasTransparency() const {return (m_nFlags & GIF_TRANSPARENT) ? TRUE : FALSE;};
	void SetTransparencyColorIndex(int nTransparencyColorIndex) {m_nTransparencyColorIndex = nTransparencyColorIndex;}; 
	int GetTransparencyColorIndex() const {return m_nTransparencyColorIndex;};

	// Netscape 2.0 looping extension block
	static const GifByteType m_szNetscape20ext[];

	//
	//  Appendix E. Interlaced Images.
	//
	//  The rows of an Interlaced images are arranged in the following order:
	//  
	//        Group 1 : Every 8th. row, starting with row 0.              (Pass 1)
	//        Group 2 : Every 8th. row, starting with row 4.              (Pass 2)
	//        Group 3 : Every 4th. row, starting with row 2.              (Pass 3)
	//        Group 4 : Every 2nd. row, starting with row 1.              (Pass 4)
	//  
	static const int m_InterlacedOffset[]; /* The way Interlaced image should. */
	static const int m_InterlacedJumps[];  /* be read - offsets and jumps... */

	BOOL SavePrepare(LPCTSTR FileName,
					ColorMapObject *OutputColorMap,
					int ExpColorMapSize,
					int Width, int Height,
					const char* Version = "87a"); // "87a" or "89a"
	BOOL Save(	GifByteType *OutputBuffer,
				int nBufLineLen,
				ColorMapObject *OutputColorMap,
				int ExpColorMapSize,
				int Left, int Top,
				int Width, int Height,
				CWnd* pProgressWnd = NULL,
				BOOL bProgressSend = TRUE,
				CWorkerThread* pThread = NULL);
	static void GetColorMap(ColorMapObject* pColorMap, LPBITMAPINFO pBMI);
	static void SetColorMap(ColorMapObject* pColorMap, LPBITMAPINFO pBMI);
	void CopyLine(	int nBitCount,
					BOOL bAlpha,
					LPBYTE pDst,
					LPBYTE pSrc,
					int width,
					const int transparent,
					BOOL& bAlphaUsed,
					RGBQUAD* pColors = NULL);
	void FillLine(	int nBitCount,
					LPBYTE pDst,
					const int index,
					int width,
					RGBQUAD* pColors = NULL);

	int m_nBackgndColorIndex; // The background color is the color which should be used around the picture
	int m_nLoadImageCount;
	int m_nSaveImageCount;
	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
	WORD m_wLoopCount;	// 0 means loop infinite
						// 1 means loop one time -> Show all frames 2x
						// 2 means loop two times -> Show all frames 3x
						// ..
						// 0xFFFF means loop 0xFFFF times -> Show all frames 0x10000x
	BOOL m_bHasLoopExtension;// If not set Loop zero times -> Show all frames 1x
	/*	This field specifies the number of
		hundredths (1/100) of a second to wait before continuing with the
		processing of the Data Stream. The clock starts ticking immediately
		after the graphic is rendered. This field may be used in
		conjunction with the User Input Flag field. */
	int m_nDelay;
	int m_nTransparencyColorIndex;
	int m_nFlags;
	char* m_lpszComment;
	BOOL m_bHasGraphicExtension;	// Graphic Extensions Are the Delay, Dispose and Transparent Vars
	BOOL m_bHasScreenColorTable;
	BOOL m_bHasImageColorTable;
	BOOL m_bAlphaUsed;	// This is set if the alpha channel while loading to a 32 bpp RGBA has been set
						// in at least one pixel to a value which is different from 255 (fully opaque)
	GifFileType* m_pLoadFile;
	GifFileType* m_pSaveFile; 
	LPBITMAPINFO m_pScreenBMI;		// Only used when loading!
	LPBITMAPINFO m_pCurrentImageBMI;// Only used when loading!
};

#endif //!_INC_DIBGIF