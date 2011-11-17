#if !defined(AFX_BATCHPROCDLG_H__9BFBF245_5EF0_434B_BD34_855B457EDE2A__INCLUDED_)
#define AFX_BATCHPROCDLG_H__9BFBF245_5EF0_434B_BD34_855B457EDE2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "DibStatic.h"
#include "AviPlay.h"
#include "StaticLink.h"
#include "DragDropListCtrl.h"
#include "TabCtrlSSL.h"
#include "BatchProcGeneralTab.h"
#include "BatchProcJpegTab.h"
#include "BatchProcTiffTab.h"
#include "BatchProcShrinkTab.h"
#include "TryEnterCriticalSection.h"
#include "AnimGifSave.h"
#include "AVRec.h"

#define	WM_ONEXIT							WM_USER + 1200
#define	WM_UPDATEDIBS_POSTPONE				WM_USER + 1201
#define WM_SET_DST_SIZE						WM_USER + 1202

#define NUM_DIBS							6
#define FSCHANGES_TIMEOUT					15000
#define SUCCESSIVE_FSCHANGES_TIMEOUT		500
#define THUMBLOAD_POLL_TIME					50

#ifndef HDF_SORTUP
#define HDF_SORTUP							0x0400
#endif
#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN						0x0200
#endif

#define TMP_BATCH_OUT_DIR					_T("BatchOut")

#define DEFAULT_OUTFILE_FRAMERATE			0.25		// Default Framerate for Animated GIFs and AVIs


/////////////////////////////////////////////////////////////////////////////
// CBatchProcDlg dialog

class CBatchProcDlg : public CDialog
{
public:
	// The Process Thread
	class CProcessThread : public CWorkerThread
	{
		public:
			CProcessThread(){	m_nPrevPercentDone = -5;
								m_pAnimGifSave = NULL;
								m_TiffOutFile = NULL;
								m_bFirstOutputFile = TRUE;
								m_pAVRec = NULL;
							};
			virtual ~CProcessThread(){Kill();};
			void SetDlg(CBatchProcDlg* pDlg) {m_pDlg = pDlg;};

			int m_nPrevPercentDone;

			CString m_szTempDstDirPath;
			CString m_szOrigDstDirPath;
			DWORD m_dwMaxSize;
			BOOL m_bMaxSizePercent;
			BOOL m_bShrinkPictures;
			BOOL m_bSharpen;
			BOOL m_bWorkOnAllPages;
			BOOL m_bAutoOrientate;
			int m_nJpegQuality;
			BOOL m_bForceJpegQuality;
			int m_nExifThumbOperationType;
			BOOL m_bRemoveCom;
			BOOL m_bRemoveExif;
			BOOL m_bRemoveIcc;
			BOOL m_bRemoveXmp;
			BOOL m_bRemoveIptc;
			BOOL m_bRemoveJfif;
			BOOL m_bRemoveOtherAppSections;
			BOOL m_bMergeXmp;
			BOOL m_bExifTimeOffset;
			int m_nExifTimeOffsetSign;
			COleDateTimeSpan m_ExifTimeOffset;
			BOOL m_bDeleteInputFile;
			int m_nTiffJpegQuality;
			int m_nTiffCompression;
			BOOL m_bTiffForceCompression;
			CString m_sPdfPaperSize;
			BOOL m_bIptcFromXmpPriority;
			double m_dFrameRate;
			
		protected:
			CAnimGifSave* m_pAnimGifSave;
			TIFF* m_TiffOutFile;
			CString m_sOcrText;
			CDib m_FirstDib;
			CDib m_Dib;
			CString m_sTempOutputFileName;
			BOOL m_bFirstOutputFile;
			CBatchProcDlg* m_pDlg;
			CAVRec* m_pAVRec;

			int Work();
			void OpenOutputFile(int nFilesCount);
			void CloseOutputFile(bool bException);
			void AddToOutputFile(	int nFilesCount,
									CString sSrcDirPath,
									CString sFileName);
			void AddToOutputAvi(	CString sInFileName);
			void AddToOutputTiff(	int nFilesCount,
									CString sInFileName,
									CString sOutFileName);
			void AddToOutputPdf(	int nFilesCount,
									CString sInFileName,
									CString sOutFileName);
			void AddToOutputOcr(	int nFilesCount,
									CString sInFileName,
									CString sOutFileName);
			void DoRename(	int& num,
							const CString& sSrcFileName,
							CString& sDstFileName,
							CString& sDstFileNameSameExt);
			BOOL Copy(	const CString& sSrcFileName,
						const CString& sDstFileNameSameExt);
			void JpegProcessing(BOOL bJpegHasBeenSaved,
								const CString& sDstFileName,
								CString& sTempFileName);
			void TiffProcessing(const CString& sDstFileName);
			int OnExit(BOOL bOk);
			void Progress(	int nCurrentFilePos,
							int nTotalFiles);
	};

	// List Element Class
	class CListElement
	{
		public:
			CListElement() {m_pDibHdr = NULL;
							m_pDibFull = NULL;
							m_pAlphaRenderedDib = NULL;
							m_pAVIPlay = NULL;
							m_pcsDibHdr = NULL;
							m_pcsDibFull = NULL;};
			virtual ~CListElement() {;};
			CString GetFileName() {return m_sPath + m_sShortFileName;};
			CDib* m_pDibHdr;
			CDib* m_pDibFull;
			CDib* m_pAlphaRenderedDib;
			CAVIPlay* m_pAVIPlay;
			CTryEnterCriticalSection* m_pcsDibHdr;
			CTryEnterCriticalSection* m_pcsDibFull;
			CString m_sShortFileName;
			CString m_sPath;
			CDibStatic m_DibStatic;
	};

	// The ChangeNotification Thread Class
	class CChangeNotificationThread : public CWorkerThread
	{
		public:
			CChangeNotificationThread() {m_pDlg = NULL;
										m_hFindChangeNotification = INVALID_HANDLE_VALUE;
										m_hDoUpdateEvent	= ::CreateEvent(NULL, TRUE, FALSE, NULL);
										m_hEventArray[0]	= GetKillEvent();
										m_hEventArray[1]	= m_hFindChangeNotification;
										m_hEventArray[2]	= m_hDoUpdateEvent;};
			virtual ~CChangeNotificationThread() {Kill(); ::CloseHandle(m_hDoUpdateEvent);};

			// Get / Set Functions
			void SetDlg(CBatchProcDlg* pDlg) {m_pDlg = pDlg;};
			void DoUpdate() {
								if (m_pDlg)
								{
									if (::IsExistingDir(m_pDlg->m_sDst))
										::SetEvent(m_hDoUpdateEvent);
									else
										m_pDlg->UpdateDstFolderSizes(this);
								}
							};

		protected:
			// Worker Thread Entry
			int Work();

			// Find Change Notification Handle
			HANDLE m_hFindChangeNotification;

			// Do Update Event Handle
			HANDLE m_hDoUpdateEvent;

			// Contains Shutdown Thread,
			// Find Change and do Update Events
			HANDLE m_hEventArray[3];

			// The Dlg Pointer
			CBatchProcDlg* m_pDlg;
	};

	typedef struct _DllVersionInfo
	{
			DWORD cbSize;
			DWORD dwMajorVersion;                   // Major version
			DWORD dwMinorVersion;                   // Minor version
			DWORD dwBuildNumber;                    // Build number
			DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
	} DLLVERSIONINFO;

public:
	enum INPUT_SELECTION
						{	INPUT_DIR=0,
							INPUT_LIST=1};

	enum OUTPUT_SELECTION
						{	OUTPUT_DIR=0,
							OUTPUT_FILE=1};

	enum OPTIMIZATION_SELECTION
						{	AUTO_OPT=0,
							ADV_OPT=1};
	enum LISTCOL
						{	LIST_FILENAME=0,
							LIST_EXIFDATE=1,
							LIST_CREATEDDATE=2,
							LIST_MODIFIEDDATE=3,
							LIST_FILESIZE=4,
							LIST_IMAGESIZE=5,
							LIST_WIDTH=6,
							LIST_HEIGHT=7,
							LIST_PATH=8};
	enum SORTTYPE
						{	FILENAME_ASC=0,
							FILENAME_DES=1,
							EXIFDATE_ASC=2,
							EXIFDATE_DES=3,
							CREATEDDATE_ASC=4,
							CREATEDDATE_DES=5,
							MODIFIEDDATE_ASC=6,
							MODIFIEDDATE_DES=7,
							FILESIZE_ASC=8,
							FILESIZE_DES=9,
							IMAGESIZE_ASC=10,
							IMAGESIZE_DES=11,
							WIDTH_ASC=12,
							WIDTH_DES=13,
							HEIGHT_ASC=14,
							HEIGHT_DES=15,
							PATH_ASC=16,
							PATH_DES=17};

	// Friend
	friend CProcessThread;
	friend CChangeNotificationThread;

// Construction
public:
	CBatchProcDlg(CWnd* pParent);
	virtual ~CBatchProcDlg() {::DeleteCriticalSection(&m_csOutDir);};

	// Update Controls
	void UpdateControls();

	// Enable / Disable All Controls
	void EnableAllControls(BOOL bEnable, BOOL bIncludeProcessButton);
	
// Dialog Data
	//{{AFX_DATA(CBatchProcDlg)
	enum { IDD = IDD_BATCH_PROC };
	CTabCtrlSSL	m_TabAdvSettings;
	CProgressCtrl	m_Progress;
	CDragDropListCtrl	m_List;
	int		m_nOptimizationSelection;
	BOOL	m_bRecursive;
	CString	m_sSrc;
	CString	m_sEditDst;
	CString	m_sOutputFileName;
	int		m_nOutputSelection;
	int		m_nInputSelection;
	BOOL	m_bRename;
	CString	m_sRename;
	BOOL	m_bConversion;
	BOOL	m_bMusicPreview;
	//}}AFX_DATA

	CString m_sDst;				// Thread Safety: m_sEditDst is copied to m_sDst
								// when change notification thread is not running!

	CDibStatic m_Dibs[NUM_DIBS];
	CStaticLink m_InputDirLabel;
	CStaticLink m_OutputDirLabel;
	CStaticLink m_OutputFileNameLabel;
	enum MiLANGUAGES m_lOcrLangId;

protected:
	// Settings
	void LoadSettings();
	void SaveSettings();

	// Start Load Dibs
	void StartLoadDibs();

	// List Load / Save Form / To Text File
	BOOL ListLoad(CString sFileName);
	BOOL ListSave(CString sFileName);

	// Check whether the given file is already
	// in the list ctrl
	BOOL IsAlreadyInList(CString sFileName, int nStart = -1);

	// Set Exif Date Column
	void SetExifDateColumn(int nItem);

	// Format Size Number
	CString FormatNumber(CString& sNumber);

	// Set Created Date Column
	void SetCreatedDateColumn(int nItem);

	// Set Modified Date Column
	void SetModifiedDateColumn(int nItem);

	// Set File Size Column
	void SetFileSizeColumn(int nItem);

	// Set Image Size Column
	void SetImageSizeColumn(int nItem);

	// Set Width Column
	void SetWidthColumn(int nItem);

	// Set Height Column
	void SetHeightColumn(int nItem);

	// Add File Name to specified position
	BOOL ListAdd(int nItem, CString sFileName);

	// Return the Item Index given a DibStatic Pointer
	int DibStaticToItem(CDibStatic* pDibStatic);

	// Start the Next nNumOfLoads Dib Header Load Threads
	BOOL StartNextLoadDibsHdr(int nNumOfLoads/*=1*/);

	// Start the Next nNumOfLoads Dib Full Load Threads
	BOOL StartNextLoadDibsFull(int nNumOfLoads = 1);

	// Delete Entire List
	void ListDeleteAll();

	// Delete List Element
	void DeleteListElement(CListElement* pListElement);

	// Get File Created Time
	static CTime GetCreatedFileTime(CString sFileName);

	// Get File Modified Time
	static CTime GetModifiedFileTime(CString sFileName);

	// Compare function for sorting
	static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// Update Dst Folder Sizes
	void UpdateDstFolderSizes(CWorkerThread* pThread = NULL);

	// Update Dst File Size
	void UpdateDstFileSize();

	// Nero Burn API Available
	BOOL CanBurnWithNero();

	// Starting with version 4.71, the Shell and common
	// controls DLLs, among others, began exporting DllGetVersion
	int GetComCtl32MajorVersion();

	// ComCtl32.dll Major Version
	int m_nComCtl32MajorVersion;

	// Process Thread
	CProcessThread m_ProcessThread;

	// Change Notification Thread
	CChangeNotificationThread m_ChangeNotificationThread;

	// Output Dir Content Size
	volatile ULARGE_INTEGER m_OutDirContentSize;

	// Output Dir Files Count
	volatile int m_nOutDirFilesCount;

	// Output Dir Content Size and Files Count Critical Section
	CRITICAL_SECTION m_csOutDir;

	// Destination Sizes
	CString m_sDstFileSize;
	CString m_sDstDirSize;

	// Exit Flags
	BOOL m_bDoCloseDlg;		// Close Dialog When Finished
	BOOL m_bThreadExited;	// Thread Exited Flag

	// Enable Next LoadDibs Flag,
	// cleared when exiting
	BOOL m_bEnableNextLoadDibs;

	// Total Number of stored list files
	int m_nStoredListFilesCount;

	// Dib Static Height / Width
	int m_nDibStaticWidth;
	int m_nDibStaticHeight;

	// Dibs Load Counts
	unsigned int m_uiDibsHdrLoadTotCount;
	unsigned int m_uiDibsHdrLoadStartCount;
	unsigned int m_uiDibsHdrLoadDoneCount;

	// Dib Static Functions
	void ClearDibs(BOOL bPaint = TRUE);
	__forceinline void UpdateDibs();

	// If set UpdateDibs() will update the Dibs
	BOOL m_bEnableUpdateDibs;

	// List Header Image List
	CImageList m_HdrImageList;

	// The Sort Type
	int m_nSortType;

	// Avi Compression Params
	DWORD m_dwVideoCompressorFourCC;
	int m_nVideoCompressorDataRate;
	int m_nVideoCompressorKeyframesRate;
	float m_fVideoCompressorQuality;
	int m_nQualityBitrate;

	// Tabs
	int m_nInitTab;
	CBatchProcGeneralTab m_GeneralTab;
	CBatchProcShrinkTab m_ShrinkTab;
	CBatchProcJpegTab m_JpegTab;
	CBatchProcTiffTab m_TiffTab;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchProcDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBatchProcDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioOptimizeAdvanced();
	afx_msg void OnRadioOptimizeAuto();
	afx_msg void OnButtonSrcDir();
	afx_msg void OnButtonDstDir();
	afx_msg void OnButtonDstFile();
	afx_msg void OnRadioOutput();
	afx_msg void OnRadioOutputFile();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRadioInput();
	afx_msg void OnRadioInputList();
	afx_msg void OnCheckRename();
	afx_msg void OnButtonListAdd();
	afx_msg void OnButtonListDelete();
	afx_msg void OnButtonListDown();
	afx_msg void OnButtonListUp();
	afx_msg void OnButtonListSelectall();
	afx_msg void OnItemchangedListInput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemListInput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnInsertitemListInput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteallitemsListInput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownListInput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonListLoad();
	afx_msg void OnButtonListSave();
	afx_msg void OnCheckConversion();
	afx_msg void OnButtonAddSlideshowExe();
	afx_msg void OnButtonViewSlideshow();
	afx_msg void OnButtonBurnSlideshow();
	afx_msg void OnChangeEditDstdir();
	afx_msg void OnChangeEditOutputFileName();
	afx_msg void OnChangeEditSrcdir();
	afx_msg void OnCheckMusicPreview();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnItemclickListInput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LONG OnExitHandler(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnUpdateDibsPostpone(WPARAM wparam, LPARAM lparam);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg LONG OnLoadDone(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnSetDstSize(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPROCDLG_H__9BFBF245_5EF0_434B_BD34_855B457EDE2A__INCLUDED_)
