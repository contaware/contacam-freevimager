// DxCaptureEVR.h: interface for the CDxCaptureEVR class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DXCAPTUREEVR_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_)
#define AFX_DXCAPTUREEVR_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef VIDEODEVICEDOC

// some old vc6 includes don't define them!
typedef unsigned long DWORD_PTR;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
#include "streams.h"
#include <atlbase.h>
#include <dshow.h>
// Modified qedit.h to be compatible with directx > 7!!!
#include "ISampleGrabber.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE( x )  \
    if ( NULL != x )       \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }
#endif

#ifndef WM_DIRECTSHOW_GRAPHNOTIFY
#define WM_DIRECTSHOW_GRAPHNOTIFY				WM_USER + 300
#endif

// Forward Declaration
class CCrossbar;
class CCaptureGraphBuilder;
class CVideoDeviceDoc;

#ifndef __IMFGetService_INTERFACE_DEFINED__
#define __IMFGetService_INTERFACE_DEFINED__

MIDL_INTERFACE("fa993888-4383-415a-a930-dd472a8cf6f7")
IMFGetService : public IUnknown
{
	public:
		virtual HRESULT STDMETHODCALLTYPE GetService( 
             /* [in] */ REFGUID guidService,
             /* [in] */ REFIID riid,
             /* [iid_is][out] */ LPVOID *ppvObject) = 0;
};

#endif  /* __IMFGetService_INTERFACE_DEFINED__ */

#ifndef __IMFVideoDisplayControl_INTERFACE_DEFINED__
#define __IMFVideoDisplayControl_INTERFACE_DEFINED__

typedef struct MFVideoNormalizedRect {
    float left;
    float top;
    float right;
    float bottom;
} 	MFVideoNormalizedRect;

MIDL_INTERFACE("a490b1e4-ab84-4d31-a1b2-181e03b1077a")
IMFVideoDisplayControl : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetNativeVideoSize( 
        /* [unique][out][in] */ SIZE *pszVideo,
        /* [unique][out][in] */ SIZE *pszARVideo) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetIdealVideoSize( 
        /* [unique][out][in] */ SIZE *pszMin,
        /* [unique][out][in] */ SIZE *pszMax) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetVideoPosition( 
        /* [unique][in] */ const MFVideoNormalizedRect *pnrcSource,
        /* [unique][in] */ const LPRECT prcDest) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoPosition( 
        /* [out] */ MFVideoNormalizedRect *pnrcSource,
        /* [out] */ LPRECT prcDest) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetAspectRatioMode( 
        /* [in] */ DWORD dwAspectRatioMode) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetAspectRatioMode( 
        /* [out] */ DWORD *pdwAspectRatioMode) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetVideoWindow( 
        /* [in] */ HWND hwndVideo) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetVideoWindow( 
        /* [out] */ HWND *phwndVideo) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE RepaintVideo( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetCurrentImage( 
        /* [out][in] */ BITMAPINFOHEADER *pBih,
        /* [size_is][size_is][out] */ BYTE **pDib,
        /* [out] */ DWORD *pcbDib,
        /* [unique][out][in] */ LONGLONG *pTimeStamp) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetBorderColor( 
        /* [in] */ COLORREF Clr) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetBorderColor( 
        /* [out] */ COLORREF *pClr) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetRenderingPrefs( 
        /* [in] */ DWORD dwRenderFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetRenderingPrefs( 
        /* [out] */ DWORD *pdwRenderFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetFullscreen( 
        /* [in] */ BOOL fFullscreen) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetFullscreen( 
        /* [out] */ BOOL *pfFullscreen) = 0;
    
};

#endif 	/* __IMFVideoDisplayControl_INTERFACE_DEFINED__ */


class CDxCaptureEVR
{
public:
	CDxCaptureEVR();									// Constructor
	virtual ~CDxCaptureEVR();							// Destructor
	void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};// The Doc Pointer
	BOOL Open(	HWND hWnd,								// window for notifications
				int nId,								// device id, if it is negative m_sDeviceName and m_sDevicePath are used
				double dFrameRate,						// if zero or negative, the default Frame-Rate is used
				int nFormatId,							// If -1, the format is chosen in the following order:
														// I420, IYUV, YV12, YUY2, YUNV, VYUY, V422, YUYV, RGB32, RGB16, RGB24, then the first format is used
				int nWidth,								// if Width or Height are invalid the default size is used
				int nHeight,
				BOOL bMpeg2);							// If TRUE opens the Mpeg2 device
	void Close();										// Close Capture & Clean-Up
	BOOL IsMpeg2() const {return m_bMpeg2;};			// Is the output compressed Mpeg2 video
	BOOL ShowError(HRESULT hr);							// Show Message Box with error
	int GetDeviceID();									// Get the Device ID from the m_sDevicePath string
														// (Device IDs dynamically change when a 
														// plug&play device is removed)
	static int GetDeviceID(CString sDevicePath);		// Get the Device ID from the given sDevicePath string
														// (Device IDs dynamically change when a 
														// plug&play device is removed)
	CString GetDeviceName()								// Returns Friendly Device Name
		const {return m_sDeviceName;};

	CString GetDevicePath()								// Returns Unique Device Path (It's a Unique Id)
		const {return m_sDevicePath;};

	BOOL Run();											// Do Capture
	BOOL Pause();										// Pause Capture
	BOOL Stop();										// Stop Capture

	BOOL IsRunning();
	BOOL IsPaused();
	BOOL IsStopped();

	BOOL GetEvent(long* plEventCode,					// Get Events in response of a
				  LONG_PTR* pplParam1,					// WM_DIRECTSHOW_GRAPHNOTIFY Message
				  LONG_PTR* pplParam2,
				  long msTimeout);
	BOOL FreeEvent(long lEventCode,						// Clean-Up after calling GetEvent()!!
					LONG_PTR plParam1,
					LONG_PTR plParam2);

	static CString GetDeviceName(int nId);				// From Device Id To Friendly Name
	static CString GetDevicePath(int nId);				// From Device Id To Device Path Name
	static int EnumDevices(	CStringArray& sDevicesName,
							CStringArray& sDevicesPath);// Enumerate All Capture Devices to the String Arrays
	BOOL HasFormats();									// Are formats available?
	static CString GetFormatCompressionDescription(DWORD dwFourCC);// Pixel Compression Description
	int EnumFormatCompressions(	CDWordArray& Compressions,// Enumerate All Compressions of the Current Capture Device
								CDWordArray& Bits,
								CStringArray& CompressionStrings);
	static CString FormatSizesToName(const CSize& Size);// Human Readable Size Description
	int EnumFormatSizes(CArray<CSize,CSize>& Sizes);	// Enumerate All Sizes of the Current Capture Device
	int EnumInputs(CStringArray& sInputs);				// Enumerate All Input Lines (S-Video, TV-Tuner,...)
														// of the Current Capture Device
	int GetInputsCount();								// Get the Number of Inputs,
														// WebCams do not have inputs, it return -1 for them
	BOOL SetCurrentInput(int nId);						// Select Input Line
	int SetDefaultInput();								// Sets composite input and if not available sets first input
	int GetCurrentInputID();							// Get Current Input Line Id
	CString GetCurrentInputName();						// Get Current Input Name
	
	BOOL GetCurrentFormat(AM_MEDIA_TYPE** ppmtConfig);	// Returns Current Selected Video Format
	BOOL GetFormatByID(int nId, AM_MEDIA_TYPE** ppmtConfig); // Get the Format given the Format ID
	static void DeleteMediaType(AM_MEDIA_TYPE *pmt);	// Used to clean-up Media Types created by GetCurrentFormat() and GetFormatByID()
	BOOL SetCurrentFormat(AM_MEDIA_TYPE* pmtConfig);	// Set new Video Format
	BOOL SetCurrentFormatByID(int nId);					// Set new Video Format by Format ID
	int GetCurrentFormatID();							// Returns the Current Selected Format ID
	int GetFormatID(DWORD biCompression, DWORD biBitCount);	// Returns Matching Format ID given the Compression and the Bits Count
	ULONG GetCurrentAnalogVideoStandards();				// The Current Selected Analog Video Standards
	static CString GetAnalogVideoStandards(ULONG VideoStandard);
														// Returns the Analog Video Standards String
	BOOL SetVideoPosition(RECT& rcDest);				// Set EVR Window Video Position

	BOOL ResetDroppedFrames();							// Reset Dropped Frames Count
	LONG GetDroppedFrames();							// Get Current Dropped Frames Count, -1 if not supported
	LONG GetAvgFrameSize();								// Get Average Frame Size (changing only for MJPEG or MPEG Streams), -1 if not supported
	double SetFrameRate(double dFrameRate);				// Set new Frame Rate
	double GetFrameRate();								// Get Current Frame Rate
	BOOL GetFrameRateRange(double& dMin, double& dMax);	// Get Frame Rate Range for the Current Selected Format

	BOOL HasVideoCaptureFilterDlg();					// Is Video Capture Filter Dialog Supported?
	BOOL ShowVideoCaptureFilterDlg();					// Show Video Capture Filter Dialog

	BOOL HasVideoCapturePinDlg();						// Is Video Capture Pin Dialog Supported?
	BOOL ShowVideoCapturePinDlg();						// Show Video Capture Pin Dialog, graph must be stopped!

	BOOL HasVideoTVTunerDlg();							// Is Video TV Tuner Dialog Supported?
	BOOL ShowVideoTVTunerDlg();							// Show Video TV Tuner Dialog

protected:
	static void ReadDeviceNameAndPath(IPropertyBag* pBag, CString& sDeviceName, CString& sDevicePath);
	BOOL InitInterfaces();								// Initialize DirectShow Interfaces
	BOOL BindFilter(int nId);							// Bind the capture filter indentified by it's Device ID
	BOOL BindFilter(const CString& sDeviceName,			// Bind the capture filter indentified by
					const CString& sDevicePath);		// Device Name and Device Path
	BOOL InitCrossbar();								// Init Crossbar for device input Lines 
														// (S-Video, TV-Tuner,...) handling
	static void FreeMediaType(AM_MEDIA_TYPE& mt);		// Called by DeleteMediaType()
	static BOOL IsSizeInRange(	CSize s,				// Is Size Inside The Specified Rect?
								int nMinSizeX,
								int nMinSizeY,
								int nMaxSizeX,
								int nMaxSizeY);

	// For Debug With GraphEdt
#ifdef _DEBUG
	HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);
	void RemoveFromRot(DWORD pdwRegister);
	DWORD m_dwRotRegister;
#endif

	HWND m_hWnd;										// Parent Window
	CVideoDeviceDoc* m_pDoc;							// The Doc Pointer
	CString m_sDeviceName;								// Friendly Device Name
	CString m_sDevicePath;								// Unique Device Path
	IGraphBuilder* m_pGraph;							// The Graph
	CCaptureGraphBuilder* m_pCaptureGraphBuilder;		// The Capture Graph Builder Wrapper Class
	IBaseFilter* m_pEvr;								// EVR
	IBaseFilter* m_pGrabberFilter;						// Sample Grabber Filter
	ISampleGrabber* m_pGrabber;							// It's interface
	IAMStreamConfig* m_pConfig;							// Configuration interface
	IBaseFilter* m_pSrcFilter;							// Capture Device Filter
	IMFGetService* m_pEVRGetService;
	IMFVideoDisplayControl* m_pEVRVideoDisplay;
	IMediaControl* m_pMC;								// Control Interface
	IMediaEventEx* m_pME;								// Events Interface
	IAMDroppedFrames* m_pDF;							// Dropped Frames Interface
	LONG m_lDroppedFramesBase;							// Dropped Frames Base
	BOOL m_bMpeg2;										// Be Mpeg2 Device
	// Use the crossbar class to help us sort out all the possible video inputs
    // The class needs to be given the capture filters ANALOGVIDEO input pin
	CCrossbar* m_pCrossbar;
};

#endif
#endif // !defined(AFX_DXCAPTUREEVR_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_)
