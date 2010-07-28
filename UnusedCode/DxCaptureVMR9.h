// DxCaptureVMR9.h: interface for the CDxCaptureVMR9 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DXCAPTUREVMR9_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_)
#define AFX_DXCAPTUREVMR9_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_

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
#include <d3d9.h>
#include <vmr9.h>
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

class CDxCaptureVMR9
{
public:
	CDxCaptureVMR9();									// Constructor
	virtual ~CDxCaptureVMR9();							// Destructor
	void SetDoc(CVideoDeviceDoc* pDoc) {m_pDoc = pDoc;};// The Doc Pointer
	BOOL Open(	HWND hWnd,								// window for notifications
				int nId,								// device id, if it is negative m_sDeviceName and m_sDevicePath are used
				double dFrameRate,						// if zero or negative, the default Frame-Rate is used
				int nFormatId,							// If -1, the format is chosen in the following order:
														// RGB32, RGB24, RGB16, I420, IYUV, YV12, YUY2, YUNV, VYUY, V422, YUYV, then the first format is used
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

	inline IVMRWindowlessControl9*						// Get The m_pWindowlessControlVMR9 Pointer
			GetWindowlessControlVMR9()
				const {return m_pWindowlessControlVMR9;};

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
	BOOL SetVideoPosition(RECT& rcDest);				// Set The VMR9 Window Video Position

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
	// Note: for Vista and higher do a regsvr32 "Path to SDKS\Bin\proppage.dll"
	// and use graphedt.exe from the above bin path!
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
	IBaseFilter* m_pVmr9;								// VMR9
	IBaseFilter* m_pAVIDecoder;							// AVI Decoder Filter
	IBaseFilter* m_pHauppaugeColorSpaceConverter;		// Hauppauge Color Space Converter
	IBaseFilter* m_pColorSpaceConverter;				// Color Space Converter
	IVMRWindowlessControl9* m_pWindowlessControlVMR9;	// VMR9 Windowless Control
	IBaseFilter* m_pGrabberFilter;						// Sample Grabber Filter
	ISampleGrabber* m_pGrabber;							// It's interface
	IAMStreamConfig* m_pConfig;							// Configuration interface
	IBaseFilter* m_pSrcFilter;							// Capture Device Filter
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
#endif // !defined(AFX_DXCAPTUREVMR9_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_)
