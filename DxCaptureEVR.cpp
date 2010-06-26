// DxCaptureEVR.cpp: implementation of the CDxCaptureEVR class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef VIDEODEVICEDOC

#include "uimager.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "DxCaptureEVR.h"
#include "crossbar.h"
#include "SampleCGB.h"
#include "getdxver.h"
#include "Dib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib, "strmiids.lib")

// Unknown Device String
#define UNKNOWN_DEVICE				_T("Unknown Device")

const GUID CLSID_EnhancedVideoRenderer_Internal = {0xfa10746c,0x9b63,0x4b6c,{0xbc,0x49,0xfc,0x30,0x0e,0xa5,0xf2,0x56}}; 
const GUID MR_VIDEO_RENDER_SERVICE_Internal = {0x1092a86c,0xab1a,0x459a,{0xa3,0x36,0x83,0x1f,0xbc,0x4d,0x11,0xff}}; 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDxCaptureEVR::CDxCaptureEVR()
{
	m_pMC = NULL;
	m_pME = NULL;
	m_pDF = NULL;
	m_lDroppedFramesBase = 0;
	m_pGraph = NULL;
	m_pCaptureGraphBuilder = NULL;
	m_pGrabberFilter = NULL;
	m_pGrabber = NULL;
	m_pEvr = NULL;
	m_pEVRGetService = NULL;
	m_pEVRVideoDisplay = NULL;
	m_pConfig = NULL;
	m_pSrcFilter = NULL;
	m_pCrossbar = NULL;
	m_hWnd = NULL;
	m_bMpeg2 = FALSE;
#ifdef _DEBUG
	m_dwRotRegister = 0;
#endif
}

CDxCaptureEVR::~CDxCaptureEVR()
{
	Close();
}

BOOL CDxCaptureEVR::ShowError(HRESULT hr)
{
    if (FAILED(hr))
    {
        TCHAR szErr[MAX_ERROR_TEXT_LEN];
        DWORD res = ::AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
        if (res == 0)
            _stprintf(szErr, _T("Unknown Error: 0x%2x"), hr);
        ::MessageBox(0, szErr, _T("Error!"), MB_OK | MB_ICONERROR);
		return TRUE;
    }
	else
		return FALSE;
}

BOOL CDxCaptureEVR::InitInterfaces()
{
    HRESULT hr;
	
	// Create the filter graph
	if (m_pGraph == NULL)
	{
		hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
								IID_IGraphBuilder, (void**)&m_pGraph);
		if (FAILED(hr))
			return FALSE;
	}
	else
		return FALSE;

	// Create the capture graph builder
	if (m_pCaptureGraphBuilder == NULL)
	{
		m_pCaptureGraphBuilder = new CCaptureGraphBuilder;
		if (!m_pCaptureGraphBuilder)
			return FALSE;
	}
	else
		return FALSE;

#ifdef _DEBUG
	hr = AddToRot(m_pGraph, &m_dwRotRegister);
	if (FAILED(hr))
		return FALSE;
#endif

	// Create the EVR
	if (m_pEvr == NULL)
	{
		hr = ::CoCreateInstance(CLSID_EnhancedVideoRenderer_Internal, NULL,
							CLSCTX_INPROC, IID_IBaseFilter, (void**)&m_pEvr);
		if (FAILED(hr))
			return FALSE;
	}
	else
		return FALSE;
	hr = m_pEvr->QueryInterface(__uuidof(IMFGetService), (void**)&m_pEVRGetService);
    if (FAILED(hr))
        return FALSE;
	hr = m_pEVRGetService->GetService(MR_VIDEO_RENDER_SERVICE_Internal, __uuidof(IMFVideoDisplayControl), (void**)&m_pEVRVideoDisplay);
	if (FAILED(hr))
        return FALSE;

    // Obtain interfaces for media control and media events
    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMC);
    if (FAILED(hr))
        return FALSE;
	
    hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void**)&m_pME);
    if (FAILED(hr))
        return FALSE;
	
    return TRUE;
}

CString CDxCaptureEVR::GetDeviceName(int nId)
{
	if (nId < 0)
		return _T("");
	
	CStringArray sDevicesName;
	CStringArray sDevicesPath;
	int nCount = EnumDevices(sDevicesName, sDevicesPath);
	if (nCount <= 0)
		return _T("");

	if (nId >= nCount)
		return _T("");
	else
		return sDevicesName[nId];
}

CString CDxCaptureEVR::GetDevicePath(int nId)
{
	if (nId < 0)
		return _T("");
	
	CStringArray sDevicesName;
	CStringArray sDevicesPath;
	int nCount = EnumDevices(sDevicesName, sDevicesPath);
	if (nCount <= 0)
		return _T("");

	if (nId >= nCount)
		return _T("");
	else
		return sDevicesPath[nId];
}

void CDxCaptureEVR::ReadDeviceNameAndPath(IPropertyBag* pBag, CString& sDeviceName, CString& sDevicePath)
{
	sDeviceName = UNKNOWN_DEVICE;
	sDevicePath = UNKNOWN_DEVICE;
	if (pBag)
	{
		VARIANT var;
		var.vt = VT_BSTR;
		HRESULT hr = pBag->Read(L"FriendlyName", &var, NULL);
		if (SUCCEEDED(hr)) 
		{
			sDeviceName = CString(var.bstrVal);
			SysFreeString(var.bstrVal);
		}
		hr = pBag->Read(L"DevicePath", &var, NULL);
		if (SUCCEEDED(hr)) 
		{
			sDevicePath = CString(var.bstrVal);
			SysFreeString(var.bstrVal);
		}
	}
}

int CDxCaptureEVR::EnumDevices(CStringArray& sDevicesName, CStringArray& sDevicesPath)
{
	// Clean-up
	sDevicesName.RemoveAll();
	sDevicesPath.RemoveAll();
	
    // Enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (FAILED(hr))
		return 0;

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (FAILED(hr))
		return 0;
	if (pEm == NULL)
		return 0;	// No Video Devices

    pEm->Reset();
    ULONG cFetched;
    IMoniker* pM;
	int index = 0;
    while (pEm->Next(1, &pM, &cFetched) == S_OK)
    {
		CString sFoundDeviceName(UNKNOWN_DEVICE);
		CString sFoundDevicePath(UNKNOWN_DEVICE);
		IPropertyBag* pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (SUCCEEDED(hr)) 
		{
			ReadDeviceNameAndPath(pBag, sFoundDeviceName, sFoundDevicePath);	
			pBag->Release();
		}
		pM->Release();
		sDevicesName.Add(sFoundDeviceName);
		sDevicesPath.Add(sFoundDevicePath);
		index++;
    }

	return index;
}

BOOL CDxCaptureEVR::HasFormats()
{
	HRESULT hr;

	if (!m_pConfig || m_bMpeg2)
		return FALSE;

	// GetNumberOfCapabilities
	int iCount = 0, iSize = 0;
	hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
		return FALSE;

	// Check the size to make sure we pass in the correct structure
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				DeleteMediaType(pmtConfig);
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CDxCaptureEVR::EnumFormatCompressions(CDWordArray& Compressions,
										   CDWordArray& Bits,
										   CStringArray& CompressionStrings)
{
	if (!m_pConfig)
		return 0;

	// Clean-up
	Compressions.RemoveAll();
	Bits.RemoveAll();
	CompressionStrings.RemoveAll();

	// GetNumberOfCapabilities
	int iCount = 0, iSize = 0;
	HRESULT hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
		return 0;

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
				if (pVih && pVih->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER))
				{
					CString sCompression = CDib::GetCompressionName((LPBITMAPINFO)(&pVih->bmiHeader));
					BOOL bAlreadyInArray = FALSE;
					for (int i = 0 ; i < Compressions.GetSize() ; i++)
					{
						if (Compressions[i] == pVih->bmiHeader.biCompression &&
							Bits[i] == pVih->bmiHeader.biBitCount)
						{
							bAlreadyInArray = TRUE;
							break;
						}
					}
					if (!bAlreadyInArray)
					{
						Compressions.Add(pVih->bmiHeader.biCompression);
						Bits.Add(pVih->bmiHeader.biBitCount);
						CompressionStrings.Add(sCompression);
					}
				}
				DeleteMediaType(pmtConfig);
			}
		}
	}

	return Compressions.GetSize();
}

CString CDxCaptureEVR::GetFormatCompressionDescription(DWORD dwFourCC)
{
	if (dwFourCC == FCC('YV12') ||
		dwFourCC == FCC('I420') ||
		dwFourCC == FCC('IYUV'))
		return _T("Planar 4:2:0 (12 Bpp)");
	else if (dwFourCC == FCC('YVU9') || dwFourCC == FCC('YUV9'))
		return _T("Planar 4:1:0 (9 Bpp)");
	else if (dwFourCC == FCC('cyuv'))
		return _T("Packed 4:1:1 (6 Bpp)");
	else if (	dwFourCC == FCC('YUY2') ||
				dwFourCC == FCC('YUNV') ||
				dwFourCC == FCC('YUYV') ||
				dwFourCC == FCC('UYVY') ||
				dwFourCC == FCC('Y422') ||
				dwFourCC == FCC('UYNV') ||
				dwFourCC == FCC('YVYU'))
		return _T("Packed 4:2:2 (16 Bpp)");
	else if (dwFourCC == FCC('Y41P'))
		return _T("Packed 4:1:1 (12 Bpp)");
	else if (	dwFourCC == FCC('YV16')	||
				dwFourCC == FCC('Y42B'))
		return _T("Planar 4:2:2 (16 Bpp)");
	else if (	dwFourCC == FCC('Y800') ||
				dwFourCC == FCC('Y8  ') ||
				dwFourCC == FCC('GREY'))
		return _T("Grayscale (8 Bpp)");
	else if (	dwFourCC == FCC('MJPG')	||
				dwFourCC == FCC('dmb1')	||
				dwFourCC == FCC('SP54'))
		return _T("Motion JPEG");
	else
		return _T("");
}

// PAL:               25 fps
// NTSC: 30000/1001 = 29.97003 fps
int CDxCaptureEVR::EnumFormatSizes(CArray<CSize,CSize>& Sizes)
{
	if (!m_pConfig)
		return 0;

	// Clean-up
	Sizes.RemoveAll();

	// GetNumberOfCapabilities
	int iCount = 0, iSize = 0;
	HRESULT hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
		return 0;

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		int iFormat;
		
		// Check if we are dealing with Unique Size Formats
		// (This are often WebCams)
		BOOL bUniqueSizeFormats = TRUE;
		for (iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				DeleteMediaType(pmtConfig);
				if ((scc.MinOutputSize.cx != scc.MaxOutputSize.cx) ||
					(scc.MinOutputSize.cy != scc.MaxOutputSize.cy))
				{
					bUniqueSizeFormats = FALSE;
					break;
				}
			}
		}

		if (bUniqueSizeFormats)
		{
			for (iFormat = 0 ; iFormat < iCount ; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE* pmtConfig;
				hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					BOOL bAlreadyInArray = FALSE;
					for (int i = 0 ; i < Sizes.GetSize() ; i++)
					{
						if (Sizes[i].cx == scc.MinOutputSize.cx &&
							Sizes[i].cy == scc.MinOutputSize.cy)
						{
							bAlreadyInArray = TRUE;
							break;
						}
					}
					if (!bAlreadyInArray)
						Sizes.Add(CSize(scc.MinOutputSize.cx, scc.MinOutputSize.cy)); 
					DeleteMediaType(pmtConfig);
				}
			}
		}
		else
		{
			int nMinSizeX = INT_MAX;
			int nMinSizeY = INT_MAX;
			int nMaxSizeX = 0;
			int nMaxSizeY = 0;
			for (iFormat = 0 ; iFormat < iCount ; iFormat++)
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE* pmtConfig;
				hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					if (scc.MinOutputSize.cx < nMinSizeX)
						nMinSizeX = scc.MinOutputSize.cx;
					if (scc.MinOutputSize.cy < nMinSizeY)
						nMinSizeY = scc.MinOutputSize.cy;
					if (scc.MaxOutputSize.cx > nMaxSizeX)
						nMaxSizeX = scc.MaxOutputSize.cx;
					if (scc.MaxOutputSize.cy > nMaxSizeY)
						nMaxSizeY = scc.MaxOutputSize.cy;					
					DeleteMediaType(pmtConfig);
				}
			}

			//AM_MEDIA_TYPE* pmtConfig;
			//BOOL res = GetCurrentFormat(&pmtConfig);

			// NTSC or PAL M
			/*
			if (pmtConfig->majortype == MEDIATYPE_AnalogVideo ||
				pmtConfig->majortype == MEDIATYPE_Video)
			{
				if (VideoStandard & AnalogVideo_NTSC_M ||
					VideoStandard & AnalogVideo_NTSC_M_J ||
					VideoStandard & AnalogVideo_NTSC_433)
				{*/
					// To small to work with motion detection:
					//if (IsSizeInRange(CSize(80,60), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// QQQVGA
					//	Sizes.Add(CSize(80,60));
					if (IsSizeInRange(CSize(160,120), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// QQVGA
						Sizes.Add(CSize(160,120));
					if (IsSizeInRange(CSize(176,120), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// NTSC QCIF
						Sizes.Add(CSize(176,120)); 
					if (IsSizeInRange(CSize(320,240), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// QVGA
						Sizes.Add(CSize(320,240));
					if (IsSizeInRange(CSize(352,240), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// NTSC CIF for VCD
						Sizes.Add(CSize(352,240));
					if (IsSizeInRange(CSize(480,480), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// SVCD
						Sizes.Add(CSize(480,480));
					if (IsSizeInRange(CSize(640,240), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))
						Sizes.Add(CSize(640,240));
					if (IsSizeInRange(CSize(640,480), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// VGA
						Sizes.Add(CSize(640,480));
					if (IsSizeInRange(CSize(704,480), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// 4CIF
						Sizes.Add(CSize(704,480));
					if (IsSizeInRange(CSize(720,240), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))
						Sizes.Add(CSize(720,240));
					if (IsSizeInRange(CSize(720,480), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// D1 NTSC for DV and DVD
						Sizes.Add(CSize(720,480));
					if (IsSizeInRange(CSize(752,480), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// D2 NTSC
						Sizes.Add(CSize(752,480));/*
					
				}
				// PAL or SECAM
				else 
				{*/
					// To small not necessary:
					//if (IsSizeInRange(CSize(88,72), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// QQCIF
					//	Sizes.Add(CSize(88,72));
					//if (IsSizeInRange(CSize(128,96), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// SQCIF
					//	Sizes.Add(CSize(128,96));
					if (IsSizeInRange(CSize(176,144), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// PAL QCIF
						Sizes.Add(CSize(176,144)); 
					if (IsSizeInRange(CSize(352,288), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// PAL CIF for VCD
						Sizes.Add(CSize(352,288));
					if (IsSizeInRange(CSize(480,576), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// SVCD
						Sizes.Add(CSize(480,576));
					if (IsSizeInRange(CSize(640,288), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))
						Sizes.Add(CSize(640,288));
					if (IsSizeInRange(CSize(704,576), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// 4CIF
						Sizes.Add(CSize(704,576));
					if (IsSizeInRange(CSize(720,288), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))
						Sizes.Add(CSize(720,288));
					if (IsSizeInRange(CSize(720,576), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// PAL for DV and DVD
						Sizes.Add(CSize(720,576));
					if (IsSizeInRange(CSize(768,576), nMinSizeX, nMinSizeY, nMaxSizeX, nMaxSizeY))	// Square Pixels PAL
						Sizes.Add(CSize(768,576));/*
				}
			}
			*/

			//DeleteMediaType(pmtConfig);
		}
	}

	return Sizes.GetSize();
}

CString CDxCaptureEVR::FormatSizesToName(const CSize& Size)
{
	if (Size == CSize(80,60))
		return _T("QQQVGA");
	else if (Size == CSize(160,120))
		return _T("QQVGA");
	else if (Size == CSize(176,120))
		return _T("NTSC QCIF");
	else if (Size == CSize(320,240))
		return _T("QVGA");
	else if (Size == CSize(352,240))
		return _T("NTSC CIF for VCD");
	else if (Size == CSize(352,480))
		return _T("NTSC 2CIF for DVD");
	else if (Size == CSize(360,480))
		return _T("NTSC D1 Half for DVD");
	else if (Size == CSize(480,480))
		return _T("NTSC SVCD");
	else if (Size == CSize(640,480))
		return _T("VGA");
	else if (Size == CSize(704,480))
		return _T("NTSC 4CIF");
	else if (Size == CSize(720,480))
		return _T("NTSC D1 for DV and DVD");
	else if (Size == CSize(752,480))
		return _T("NTSC D2");
	else if (Size == CSize(88,72))
		return _T("PAL QQCIF");
	else if (Size == CSize(128,96))
		return _T("PAL SQCIF");
	else if (Size == CSize(176,144))
		return _T("PAL QCIF");
	else if (Size == CSize(352,288))
		return _T("PAL CIF for VCD");
	else if (Size == CSize(352,576))
		return _T("PAL 2CIF for DVD");
	else if (Size == CSize(360,576))
		return _T("PAL Half for DVD");
	else if (Size == CSize(480,576))
		return _T("PAL SVCD");
	else if (Size == CSize(704,576))
		return _T("PAL 4CIF");
	else if (Size == CSize(720,576))
		return _T("PAL for DV and DVD");
	else if (Size == CSize(768,576))
		return _T("PAL Square Pixels");
	else
		return _T("");
}

BOOL CDxCaptureEVR::IsSizeInRange(CSize s, int nMinSizeX, int nMinSizeY, int nMaxSizeX, int nMaxSizeY)
{
	if (nMinSizeX <= s.cx && nMinSizeY <= s.cy &&
		nMaxSizeX >= s.cx && nMaxSizeY >= s.cy)
		return TRUE;
	else
		return FALSE;
}

CString CDxCaptureEVR::GetAnalogVideoStandards(ULONG VideoStandard)
{
	CString s(_T(""));

	if (VideoStandard == AnalogVideo_None)
		return _T("");
	if (VideoStandard & AnalogVideo_NTSC_M)
		s += _T("NTSC_M "); 
	if (VideoStandard & AnalogVideo_NTSC_M_J)
		s += _T("NTSC M_J "); 
	if (VideoStandard & AnalogVideo_NTSC_433)
		s += _T("NTSC_433 ");
	if (VideoStandard & AnalogVideo_PAL_B)
		s += _T("PAL_B ");
	if (VideoStandard & AnalogVideo_PAL_D)
		s += _T("PAL_D ");
	if (VideoStandard & AnalogVideo_PAL_H)
		s += _T("PAL_H ");
	if (VideoStandard & AnalogVideo_PAL_I)
		s += _T("PAL_I ");
	if (VideoStandard & AnalogVideo_PAL_M)
		s += _T("PAL_M ");
	if (VideoStandard & AnalogVideo_PAL_N)
		s += _T("PAL_N ");
	if (VideoStandard & AnalogVideo_PAL_60)
		s += _T("PAL_60 ");
	if (VideoStandard & AnalogVideo_SECAM_B)
		s += _T("SECAM_B ");
	if (VideoStandard & AnalogVideo_SECAM_D)
		s += _T("SECAM_D ");
	if (VideoStandard & AnalogVideo_SECAM_G)
		s += _T("SECAM_G ");
	if (VideoStandard & AnalogVideo_SECAM_H)
		s += _T("SECAM_H ");
	if (VideoStandard & AnalogVideo_SECAM_K)
		s += _T("SECAM_K ");
	if (VideoStandard & AnalogVideo_SECAM_K1)
		s += _T("SECAM_K1 ");
	if (VideoStandard & AnalogVideo_SECAM_L)
		s += _T("SECAM_L ");
	if (VideoStandard & AnalogVideo_SECAM_L1)
		s += _T("SECAM_L1 ");
	if (VideoStandard & AnalogVideo_PAL_N_COMBO)
		s += _T("PAL_N_COMBO ");

	return s;
}

BOOL CDxCaptureEVR::ResetDroppedFrames()
{
	if (m_pDF)
	{
		LONG lDropped = 0;
		m_pDF->GetNumDropped(&lDropped);
		m_lDroppedFramesBase = lDropped;
		return TRUE;
	}
	else
		return FALSE;
}

LONG CDxCaptureEVR::GetDroppedFrames()
{
	if (m_pDF)
	{
		LONG lDropped = 0;
		HRESULT res = m_pDF->GetNumDropped(&lDropped);
		ASSERT(res == S_OK);
		return (lDropped - m_lDroppedFramesBase);
	}
	else
		return -1;
}

LONG CDxCaptureEVR::GetAvgFrameSize()
{
	if (m_pDF)
	{
		LONG lAvgFrameSize = 0;
		m_pDF->GetAverageFrameSize(&lAvgFrameSize);
		return lAvgFrameSize;
	}
	else
		return -1;
}

double CDxCaptureEVR::SetFrameRate(double dFrameRate)
{
	if (m_bMpeg2)
		return 0.0;

	// Set Exact NTSC Frame Rate
	if (Round(dFrameRate) == 30)
		dFrameRate = 30000.0 / 1001.0;

	AM_MEDIA_TYPE* pmtConfig = NULL;
	if (!GetCurrentFormat(&pmtConfig))
		return 0.0;
	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
	if (pVih)
	{
		pVih->AvgTimePerFrame = (REFERENCE_TIME)(10000000.0 / dFrameRate + 0.5);
		if (SetCurrentFormat(pmtConfig))
		{
			DeleteMediaType(pmtConfig);
			return GetFrameRate();
		}
	}
	DeleteMediaType(pmtConfig);
	return 0.0;
}

double CDxCaptureEVR::GetFrameRate()
{
	if (m_bMpeg2)
		return 0.0;
	else
	{
		AM_MEDIA_TYPE* pmtConfig = NULL;
		if (!GetCurrentFormat(&pmtConfig))
			return 0.0;

		double dFrameRate = 0.0;
		VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
		if (pVih)
			dFrameRate = 10000000.0 / (double)pVih->AvgTimePerFrame;

		DeleteMediaType(pmtConfig);

		return dFrameRate;
	}
}

BOOL CDxCaptureEVR::GetFrameRateRange(double& dMin, double& dMax)
{
	if (!m_pConfig || m_bMpeg2)
		return FALSE;

	AM_MEDIA_TYPE* pmtConfig = NULL;
	VIDEO_STREAM_CONFIG_CAPS scc;	
	HRESULT hr = m_pConfig->GetStreamCaps(GetCurrentFormatID(), &pmtConfig, (BYTE*)&scc);
	if (FAILED(hr))
		return FALSE;

	dMin = 10000000.0 / (double)scc.MaxFrameInterval;
	dMax = 10000000.0 / (double)scc.MinFrameInterval;
	if (dMin < 1.0)
		dMin = 1.0;
	if (dMax < 1.0)
		dMax = 1.0;

	DeleteMediaType(pmtConfig);

	return TRUE;
}

BOOL CDxCaptureEVR::GetCurrentFormat(AM_MEDIA_TYPE** ppmtConfig)
{
	HRESULT hr;

	if (m_pConfig)
	{
		hr = m_pConfig->GetFormat(ppmtConfig);
		if (SUCCEEDED(hr))
			return TRUE;
	}	
	
	return FALSE;
}

BOOL CDxCaptureEVR::SetCurrentFormat(AM_MEDIA_TYPE* pmtConfig)
{
	if (!m_pConfig)
		return FALSE;
	
	HRESULT hr = m_pConfig->SetFormat(pmtConfig);
	return SUCCEEDED(hr);
}

BOOL CDxCaptureEVR::GetFormatByID(int nId, AM_MEDIA_TYPE** ppmtConfig)
{
	HRESULT hr;

	if (nId < 0 || !m_pConfig)
		return FALSE;

	int iCount = 0, iSize = 0;
	hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
		return FALSE;
	if (nId >= iCount)
		return FALSE;

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		VIDEO_STREAM_CONFIG_CAPS scc;
		hr = m_pConfig->GetStreamCaps(nId, ppmtConfig, (BYTE*)&scc);
 		if (SUCCEEDED(hr))	
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

int CDxCaptureEVR::GetCurrentFormatID()
{
	if (!m_pConfig)
		return -1;
	
	AM_MEDIA_TYPE* pmtCurrentConfig = NULL;
	HRESULT hr = m_pConfig->GetFormat(&pmtCurrentConfig);
	if (FAILED(hr))
		return -1;
	if (pmtCurrentConfig == NULL)
		return -1;

	int iCount = 0, iSize = 0;
	hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
	{
		DeleteMediaType(pmtCurrentConfig);
		return -1;
	}

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
 			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
				VIDEOINFOHEADER* pCurrentVih = (VIDEOINFOHEADER*)pmtCurrentConfig->pbFormat;
				if (pVih && pCurrentVih)
				{
					LPBITMAPINFOHEADER lpBmiH = &pVih->bmiHeader;
					LPBITMAPINFOHEADER lpCurrentBmiH = &pCurrentVih->bmiHeader;
					if (pmtConfig->majortype == pmtCurrentConfig->majortype &&
						pmtConfig->subtype == pmtCurrentConfig->subtype &&
						pmtConfig->bFixedSizeSamples == pmtCurrentConfig->bFixedSizeSamples &&
						pmtConfig->bTemporalCompression == pmtCurrentConfig->bTemporalCompression &&
						pmtConfig->lSampleSize == pmtCurrentConfig->lSampleSize &&
						pmtConfig->formattype == pmtCurrentConfig->formattype &&
						lpBmiH->biCompression == lpCurrentBmiH->biCompression &&
						lpBmiH->biBitCount == lpCurrentBmiH->biBitCount)
					{
						DeleteMediaType(pmtConfig);
						DeleteMediaType(pmtCurrentConfig);
						return iFormat;
					}
				}
				DeleteMediaType(pmtConfig);
			}
		}
	}
	
	DeleteMediaType(pmtCurrentConfig);
	return -1;
}

ULONG CDxCaptureEVR::GetCurrentAnalogVideoStandards()
{
	if (!m_pConfig)
		return 0;
	
	AM_MEDIA_TYPE* pmtCurrentConfig = NULL;
	HRESULT hr = m_pConfig->GetFormat(&pmtCurrentConfig);
	if (FAILED(hr))
		return 0;
	if (pmtCurrentConfig == NULL)
		return 0;

	int iCount = 0, iSize = 0;
	hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
	{
		DeleteMediaType(pmtCurrentConfig);
		return 0;
	}

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
 			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
				VIDEOINFOHEADER* pCurrentVih = (VIDEOINFOHEADER*)pmtCurrentConfig->pbFormat;
				if (pVih && pCurrentVih)
				{
					LPBITMAPINFOHEADER lpBmiH = &pVih->bmiHeader;
					LPBITMAPINFOHEADER lpCurrentBmiH = &pCurrentVih->bmiHeader;
					if (pmtConfig->majortype == pmtCurrentConfig->majortype &&
						pmtConfig->subtype == pmtCurrentConfig->subtype &&
						pmtConfig->bFixedSizeSamples == pmtCurrentConfig->bFixedSizeSamples &&
						pmtConfig->bTemporalCompression == pmtCurrentConfig->bTemporalCompression &&
						pmtConfig->lSampleSize == pmtCurrentConfig->lSampleSize &&
						pmtConfig->formattype == pmtCurrentConfig->formattype &&
						lpBmiH->biCompression == lpCurrentBmiH->biCompression &&
						lpBmiH->biBitCount == lpCurrentBmiH->biBitCount)
					{
						DeleteMediaType(pmtConfig);
						DeleteMediaType(pmtCurrentConfig);
						return scc.VideoStandard;
					}
				}
				DeleteMediaType(pmtConfig);
			}
		}
	}
	
	DeleteMediaType(pmtCurrentConfig);
	return 0;
}

int CDxCaptureEVR::GetFormatID(DWORD biCompression, DWORD biBitCount)
{
	if (!m_pConfig)
		return -1;
	
	AM_MEDIA_TYPE* pmtCurrentConfig = NULL;
	HRESULT hr = m_pConfig->GetFormat(&pmtCurrentConfig);
	if (FAILED(hr))
		return -1;
	if (pmtCurrentConfig == NULL)
		return -1;

	int iCount = 0, iSize = 0;
	hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
	{
		DeleteMediaType(pmtCurrentConfig);
		return -1;
	}

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
 			if (SUCCEEDED(hr))
			{
				VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
				VIDEOINFOHEADER* pCurrentVih = (VIDEOINFOHEADER*)pmtCurrentConfig->pbFormat;
				if (pVih && pCurrentVih)
				{
					LPBITMAPINFOHEADER lpBmiH = pVih != NULL ? &pVih->bmiHeader : NULL;
					LPBITMAPINFOHEADER lpCurrentBmiH = pCurrentVih != NULL ? &pCurrentVih->bmiHeader : NULL;
					if (lpBmiH									&&
						lpBmiH->biCompression == biCompression	&&
						lpBmiH->biBitCount == biBitCount)
					{
						DeleteMediaType(pmtConfig);
						DeleteMediaType(pmtCurrentConfig);
						return iFormat;
					}
				}
				DeleteMediaType(pmtConfig);
			}
		}
	}
	
	DeleteMediaType(pmtCurrentConfig);
	return -1;
}

BOOL CDxCaptureEVR::SetCurrentFormatByID(int nId)
{
	AM_MEDIA_TYPE* pmtConfig = NULL;
	VIDEOINFOHEADER* pVih = NULL;
	REFERENCE_TIME AvgTimePerFrame = 0;

	if (nId < 0 || !m_pConfig)
		return FALSE;

	int iCount = 0, iSize = 0;
	HRESULT hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
		return FALSE;

	if (nId < 0 || nId >= iCount)
		return FALSE;

	// Store Frame-Rate
	if (!GetCurrentFormat(&pmtConfig))
		return FALSE;
	pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
	if (pVih)
		AvgTimePerFrame = pVih->AvgTimePerFrame;
	DeleteMediaType(pmtConfig);

	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0 ; iFormat < iCount ; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				if (nId == iFormat)
				{
					// Restore Frame-Rate
					pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					if (pVih									&&
						AvgTimePerFrame >= scc.MinFrameInterval &&
						AvgTimePerFrame <= scc.MaxFrameInterval)
						pVih->AvgTimePerFrame = AvgTimePerFrame;
					hr = m_pConfig->SetFormat(pmtConfig);
					DeleteMediaType(pmtConfig);
					return SUCCEEDED(hr);
				}
				DeleteMediaType(pmtConfig);
			}
		}
	}

	return FALSE;
}

int CDxCaptureEVR::GetDeviceID()
{
	return GetDeviceID(m_sDevicePath);
}

int CDxCaptureEVR::GetDeviceID(CString sDevicePath)
{
	if (sDevicePath == _T(""))
		return -1;

	// Enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr))
		return -1;

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (FAILED(hr))
		return -1;
	if (pEm == NULL)
		return -1;	// No Video Devices

    pEm->Reset();
    ULONG cFetched;
    IMoniker* pM;
	int index = 0;
    while (pEm->Next(1, &pM, &cFetched) == S_OK)
    {
		CString sFoundDeviceName(UNKNOWN_DEVICE);
		CString sFoundDevicePath(UNKNOWN_DEVICE);
		IPropertyBag* pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (SUCCEEDED(hr)) 
		{
			ReadDeviceNameAndPath(pBag, sFoundDeviceName, sFoundDevicePath);
			pBag->Release();
		}
		pM->Release();
		if (sFoundDevicePath == sDevicePath)
			return index;
		index++;
    }

	return -1;
}

// DevicePath for usb devices
/*
\\?\usb#vid_v(4)&PID_d(4)&MI_z(2)
where v(4) is the 4-digit vendor code that the USB committee assigns to the vendor.
d(4) is the 4-digit product code that the vendor assigns to the device.
z(2) is the interface number, extracted from the bInterfaceNumber field of the usb interface association descriptor.
*/
BOOL CDxCaptureEVR::BindFilter(int nId)
{
	BOOL bBindToObjectOk = FALSE;

	if (nId < 0)
		return FALSE;

	if (m_pSrcFilter)
		return FALSE;
	
    // Enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr))
		return FALSE;

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (FAILED(hr))
		return FALSE;
	if (pEm == NULL)
		return FALSE;	// No Video Devices

    pEm->Reset();
    ULONG cFetched;
    IMoniker* pM;
	int index = 0;
    while (pEm->Next(1, &pM, &cFetched) == S_OK)
    {
		if (index++ == nId)
		{
			CString sFoundDeviceName(UNKNOWN_DEVICE);
			CString sFoundDevicePath(UNKNOWN_DEVICE);
			IPropertyBag* pBag;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
			if (SUCCEEDED(hr)) 
			{
				ReadDeviceNameAndPath(pBag, sFoundDeviceName, sFoundDevicePath);
				pBag->Release();
			}
			hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pSrcFilter);
			if (SUCCEEDED(hr))
			{
				bBindToObjectOk = TRUE;
				m_sDeviceName = sFoundDeviceName;
				m_sDevicePath = sFoundDevicePath;
			}
			pM->Release();
			break;
		}
		else
			pM->Release();
    }

	return bBindToObjectOk;
}

BOOL CDxCaptureEVR::BindFilter(const CString& sDeviceName, const CString& sDevicePath)
{
	BOOL bBindToObjectOk = FALSE;

	if (m_pSrcFilter)
		return FALSE;
	
    // Enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
    HRESULT hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
									IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (FAILED(hr))
		return FALSE;

    CComPtr<IEnumMoniker> pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
    if (FAILED(hr))
		return FALSE;
	if (pEm == NULL)
		return FALSE;	// No Video Devices

    pEm->Reset();
    ULONG cFetched;
    IMoniker* pM;
    while (!bBindToObjectOk && pEm->Next(1, &pM, &cFetched) == S_OK)
    {
		CString sFoundDeviceName(UNKNOWN_DEVICE);
		CString sFoundDevicePath(UNKNOWN_DEVICE);
		IPropertyBag* pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (SUCCEEDED(hr)) 
		{
			ReadDeviceNameAndPath(pBag, sFoundDeviceName, sFoundDevicePath);
			pBag->Release();
		}	
		if (sDeviceName == sFoundDeviceName	&&
			sDevicePath == sFoundDevicePath)
		{
			hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pSrcFilter);
			if (SUCCEEDED(hr))
			{
				bBindToObjectOk = TRUE;
				m_sDeviceName = sFoundDeviceName;
				m_sDevicePath = sFoundDevicePath;
			}
		}
		pM->Release();
    }

	return bBindToObjectOk;
}

// Open device, after that call Run()
// - hWnd window for notifications
// - nId is the device id, if it is negative m_sDeviceName and m_sDevicePath are used
// - If dFrameRate is zero or negative, the default Frame-Rate is used
// - If nFormatId is -1, the format is chosen in the following order:
//   I420, IYUV, YV12, YUY2, YUNV, VYUY, V422, YUYV, RGB32, RGB16, RGB24, then the first format is used
// - If Width or Height are <= 0 the sizes are tried in the following order: 640x480, 352x288, 352x240, 320x240
// - If bMpeg2 is TRUE it tries to open as a Mpeg2 device
BOOL CDxCaptureEVR::Open(	HWND hWnd,
							int nId,
							double dFrameRate,
							int nFormatId,
							int nWidth,
							int nHeight,
							BOOL bMpeg2)
{
	HRESULT hr;

	// First Close (m_sDeviceName and m_sDevicePath are not cleared)
	Close();

	// Store Handle to Window
	m_hWnd = hWnd;

    // Init DirectShow interfaces
    if (!InitInterfaces())
    {
		TRACE(_T("Failed to init video interfaces,\nmake sure you are on Vista or higher!\n"));
        return FALSE;
    }

    // Attach the filter graph to the capture graph
    hr = m_pCaptureGraphBuilder->SetFiltergraph(m_pGraph);
    if (FAILED(hr))
    {
		TRACE(_T("Failed to set capture filter graph\n"));
        return FALSE;
    }

	// Find and bind capture filter
	if (nId < 0)
	{
		// Re-open
		if (!BindFilter(m_sDeviceName, m_sDevicePath))
			return FALSE;
	}
	else
	{
		// Find by Id
		if (!BindFilter(nId))
		{
			TRACE(_T("Failed to Bind the source filter\n"));
			return FALSE;
		}
	}

    // Add capture filter to our graph
    hr = m_pGraph->AddFilter(m_pSrcFilter, L"Video Capture");
    if (FAILED(hr))
    {
		TRACE(	_T("If you have a working video capture device, please make sure\n")
				_T("that it is connected and is not being used by another application\n"));
        return FALSE;
    }

	// Add EVR filter to our graph
	hr = m_pGraph->AddFilter(m_pEvr, L"Enhanced Video Renderer");
	if (FAILED(hr))
	{
		TRACE(_T("Failed to add Enhanced Video Renderer to the filter graph\n"));
        return FALSE;
	}

	// Set the EVR window and size
	hr = m_pEVRVideoDisplay->SetVideoWindow(hWnd);
	MFVideoNormalizedRect src;
	src.left = 0.0f;
	src.top = 0.0f;
	src.right = 1.0f;
	src.bottom = 1.0f;
	RECT rcDest;
	//::GetClientRect(hWnd, &rcDest);
	rcDest.left = 0;
	rcDest.right = 0;
	rcDest.top = 0;
	rcDest.bottom = 0;
	hr = m_pEVRVideoDisplay->SetVideoPosition(&src, &rcDest);

	// Get Dropped Frames and Configuration Interfaces
	if (bMpeg2)
	{
		m_pCaptureGraphBuilder->FindInterface(	&PIN_CATEGORY_CAPTURE,
												&MEDIATYPE_Stream,
												m_pSrcFilter,
												IID_IAMDroppedFrames, (void**)&m_pDF);
		
		// Note: some mpeg2 devices return the config interface,
		// but with no information on it!
		hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
												  &MEDIATYPE_Stream,
												  m_pSrcFilter,
												  IID_IAMStreamConfig, (void**)&m_pConfig);
	}
	else
	{
		hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
												  &MEDIATYPE_Interleaved,
												  m_pSrcFilter,
												  IID_IAMDroppedFrames, (void**)&m_pDF);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
												  &MEDIATYPE_Video,
												  m_pSrcFilter,
												  IID_IAMDroppedFrames, (void**)&m_pDF);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
													  &MEDIATYPE_Interleaved,
													  m_pSrcFilter,
													  IID_IAMDroppedFrames, (void**)&m_pDF);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
												  &MEDIATYPE_Video,
												  m_pSrcFilter,
												  IID_IAMDroppedFrames, (void**)&m_pDF);

		hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
												  &MEDIATYPE_Interleaved,
												  m_pSrcFilter,
												  IID_IAMStreamConfig, (void**)&m_pConfig);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
													  &MEDIATYPE_Video,
													  m_pSrcFilter,
													  IID_IAMStreamConfig, (void**)&m_pConfig);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
													  &MEDIATYPE_Interleaved,
													  m_pSrcFilter,
													  IID_IAMStreamConfig, (void**)&m_pConfig);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
													  &MEDIATYPE_Video,
													  m_pSrcFilter,
													  IID_IAMStreamConfig, (void**)&m_pConfig);
	}
	if (m_pDF)
		m_pDF->GetNumDropped(&m_lDroppedFramesBase);
	else
		m_lDroppedFramesBase = 0;
	if (FAILED(hr))
	{
		TRACE(_T("Failed while getting the configuration interface\n"));
		return FALSE;
	}

	// Set Format and Frame Rate
	if (!bMpeg2)
	{
		// Auto-Set Format?
		if (nFormatId == -1)
		{
			// Try Formats: from first choice down to last one

			// I420
			if ((nFormatId = GetFormatID(mmioFOURCC('I','4','2','0'), 12)) == -1)
			
			// I420 Equivalent	
			if ((nFormatId = GetFormatID(mmioFOURCC('I','Y','U','V'), 12)) == -1)
			
			// YV12
			if ((nFormatId = GetFormatID(mmioFOURCC('Y','V','1','2'), 12)) == -1)
			
			// YUY2	
			if ((nFormatId = GetFormatID(mmioFOURCC('Y','U','Y','2'), 16)) == -1)

			// YUY2 Equivalents
			if ((nFormatId = GetFormatID(mmioFOURCC('Y','U','N','V'), 16)) == -1)
			if ((nFormatId = GetFormatID(mmioFOURCC('V','Y','U','Y'), 16)) == -1)
			if ((nFormatId = GetFormatID(mmioFOURCC('V','4','2','2'), 16)) == -1)
			if ((nFormatId = GetFormatID(mmioFOURCC('Y','U','Y','V'), 16)) == -1)

			// RGB Formats
			if ((nFormatId = GetFormatID(BI_RGB, 32)) == -1)
			if ((nFormatId = GetFormatID(BI_RGB, 16)) == -1)
			if ((nFormatId = GetFormatID(BI_RGB, 24)) == -1)
				nFormatId = 0;	// If not successful set first format
		}
		
		// Set it
		AM_MEDIA_TYPE* pmtConfig = NULL;
		if (GetFormatByID(nFormatId, &pmtConfig))
		{
			VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
			if (pVih)
			{
				// Width & Height
				LPBITMAPINFOHEADER lpBmiH = &pVih->bmiHeader;
				if (nWidth > 0 && nHeight > 0)
				{
					lpBmiH->biWidth = nWidth;
					lpBmiH->biHeight = nHeight;
				}
				else
				{
					BOOL bHas640x480 = FALSE;
					BOOL bHas352x288 = FALSE;
					BOOL bHas352x240 = FALSE;
					BOOL bHas320x240 = FALSE;
					CArray<CSize,CSize> Sizes;
					int nSizesCount = EnumFormatSizes(Sizes);
					for (int nSize = 0 ; nSize < nSizesCount ; nSize++)
					{
						if (Sizes[nSize].cx == 640 && Sizes[nSize].cy == 480)
							bHas640x480 = TRUE;
						else if (Sizes[nSize].cx == 352 && Sizes[nSize].cy == 288)
							bHas352x288 = TRUE;
						else if (Sizes[nSize].cx == 352 && Sizes[nSize].cy == 240)
							bHas352x240 = TRUE;
						else if (Sizes[nSize].cx == 320 && Sizes[nSize].cy == 240)
							bHas320x240 = TRUE;
					}
					if (bHas640x480)
					{
						lpBmiH->biWidth = 640;
						lpBmiH->biHeight = 480;
					}
					else if (bHas352x288)
					{
						lpBmiH->biWidth = 352;
						lpBmiH->biHeight = 288;
					}
					else if (bHas352x240)
					{
						lpBmiH->biWidth = 352;
						lpBmiH->biHeight = 240;
					}
					else if (bHas320x240)
					{
						lpBmiH->biWidth = 320;
						lpBmiH->biHeight = 240;
					}
					// else leave it as it is
				}
				lpBmiH->biSizeImage = DWALIGNEDWIDTHBYTES(lpBmiH->biWidth * lpBmiH->biBitCount) * lpBmiH->biHeight;
				pmtConfig->lSampleSize = lpBmiH->biSizeImage;
				pVih->dwBitRate = 0; // Reset BitRate, driver will calculate it

				// Set Frame Rate
				if (Round(dFrameRate) == 30)
					dFrameRate = 30000.0 / 1001.0; // Set Exact NTSC Frame Rate
				pVih->AvgTimePerFrame = (REFERENCE_TIME)(10000000.0 / dFrameRate + 0.5);

				// Set Format
				SetCurrentFormat(pmtConfig);
			}
			DeleteMediaType(pmtConfig);
		}
	}

	// Render Mpeg2
	if (bMpeg2)
	{
		hr = m_pCaptureGraphBuilder->RenderStream(	NULL,
													&MEDIATYPE_Stream, // Setting MEDIATYPE_Stream forces the use of the mpeg2 demuxer
													m_pSrcFilter, NULL, m_pEvr);
	}
	// Render Raw Video
	else
	{
		hr = m_pCaptureGraphBuilder->RenderStream(	&PIN_CATEGORY_CAPTURE,
													&MEDIATYPE_Interleaved,
													m_pSrcFilter, NULL, m_pEvr);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->RenderStream(	&PIN_CATEGORY_CAPTURE,
														&MEDIATYPE_Video,
														m_pSrcFilter, NULL, m_pEvr);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->RenderStream(	&PIN_CATEGORY_PREVIEW,
														&MEDIATYPE_Interleaved,
														m_pSrcFilter, NULL, m_pEvr);
		if (FAILED(hr))
			hr = m_pCaptureGraphBuilder->RenderStream(	&PIN_CATEGORY_PREVIEW,
														&MEDIATYPE_Video,
														m_pSrcFilter, NULL, m_pEvr);
	}
	if (FAILED(hr))
		return FALSE;

	// Init Crossbar, input lines (S-Video, TV-Tuner,...) management
	InitCrossbar();

	// Set Flag
	m_bMpeg2 = bMpeg2;

	// Enable Events and set the window handle used to process graph events
	hr = m_pME->SetNotifyFlags(0); // Enable Events Notify
	if (FAILED(hr))
    {
		TRACE(_T("Failed to enable graph notify\n"));
        return FALSE;
    }
    hr = m_pME->SetNotifyWindow((OAHWND)hWnd, WM_DIRECTSHOW_GRAPHNOTIFY, 0);
	if (FAILED(hr))
    {
		TRACE(_T("Failed to set graph notify window\n"));
        return FALSE;
    }

    return TRUE;
}

void CDxCaptureEVR::Close()
{
	// NOTE:
	// Some crappy devices like my TRUST 380 SpaceCam when unplugged while
	// running do not notify that the device has been unplugged.
	// In this case calling Stop() infinitely blocks, also safe releasing
	// m_pCaptureGraphBuilder will infinitely block.
	// Getting the state with m_pMC->GetState returns running...

	// Stop Graph Filter
	Stop();

	// Stop receiving events
	if (m_pME)
	{
		m_pME->SetNotifyFlags(AM_MEDIAEVENT_NONOTIFY);
		m_pME->SetNotifyWindow(NULL, WM_DIRECTSHOW_GRAPHNOTIFY, 0);
	}

#ifdef _DEBUG
	RemoveFromRot(m_dwRotRegister);
#endif
	
	// Release DirectShow interfaces
	SAFE_RELEASE(m_pMC);
	SAFE_RELEASE(m_pME);
	SAFE_RELEASE(m_pDF);
	SAFE_RELEASE(m_pSrcFilter);
	SAFE_RELEASE(m_pConfig);
	SAFE_RELEASE(m_pEvr);
	SAFE_RELEASE(m_pEVRGetService);
	SAFE_RELEASE(m_pEVRVideoDisplay);
	SAFE_RELEASE(m_pGraph);
	if (m_pCaptureGraphBuilder)
		m_pCaptureGraphBuilder->ReleaseFilters();

	if (m_pCrossbar)
    {
        delete m_pCrossbar;
        m_pCrossbar = NULL;
    }
	if (m_pCaptureGraphBuilder)
	{
		delete m_pCaptureGraphBuilder;
		m_pCaptureGraphBuilder = NULL;
	}
}

void CDxCaptureEVR::FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        ::CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // Unecessary because pUnk should not be used, but safest.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

void CDxCaptureEVR::DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL)
    {
        FreeMediaType(*pmt);
        ::CoTaskMemFree(pmt);
    }
}

BOOL CDxCaptureEVR::InitCrossbar()
{
    IPin        *pP = 0;
    IEnumPins   *pins=0;
    ULONG        n;
    PIN_INFO     pinInfo;
    BOOL         bFound = FALSE;
    IKsPropertySet *pKs=0;
    GUID guid;
    DWORD dw;
    BOOL bMatch = FALSE;

	if (m_pCrossbar)
	{
		delete m_pCrossbar;
		m_pCrossbar = NULL;
	}

    if (SUCCEEDED(m_pSrcFilter->EnumPins(&pins)))
    {            
        while (!bFound && (S_OK == pins->Next(1, &pP, &n)))
        {
            if (S_OK == pP->QueryPinInfo(&pinInfo))
            {
                if (pinInfo.dir == PINDIR_INPUT)
                {
					// Note:
					// PIN_CATEGORY_ANALOGAUDIOIN = "{04394D28-52A7-4C46-AED0-65D62CC64DA6}"

                    // Is this pin an ANALOGVIDEOIN input pin?
                    if (pP->QueryInterface(	IID_IKsPropertySet,
											(void **)&pKs) == S_OK)
                    {
                        if (pKs->Get(AMPROPSETID_Pin,
                            AMPROPERTY_PIN_CATEGORY, NULL, 0,
                            &guid, sizeof(GUID), &dw) == S_OK)
                        {
                            if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
								bMatch = TRUE;
                        }
                        pKs->Release();
                    }

                    if (bMatch)
                    {
                        HRESULT hrCreate = S_OK;
                        m_pCrossbar = new CCrossbar(pP, &hrCreate);
                        if (!m_pCrossbar || FAILED(hrCreate))
						{
							pinInfo.pFilter->Release();
							pP->Release();
                            break;
						}
						else
							bFound = TRUE;
                    }
                }
                pinInfo.pFilter->Release();
            }
            pP->Release();
        }
        pins->Release();
    }

	return bFound;
}

int CDxCaptureEVR::GetInputsCount()
{
	LONG lCount;
	if (m_pCrossbar)
	{
        if (m_pCrossbar->GetInputCount(&lCount) == S_OK)
			return (int)lCount;
		else
			return -1;
	}
	else
		return -1;
}

int CDxCaptureEVR::GetCurrentInputID()
{
	LONG lIndex;
	if (m_pCrossbar)
	{
        if (m_pCrossbar->GetInputIndex(&lIndex) == S_OK)
			return (int)lIndex;
		else
			return -1;
	}
	else
		return -1;
}

CString CDxCaptureEVR::GetCurrentInputName()
{
	if (m_pCrossbar)
	{
		TCHAR Name[MAX_PATH];
        if (m_pCrossbar->GetInputName (	GetCurrentInputID(), 
										Name, 
										MAX_PATH) == S_OK)
			return CString(Name);
		else
			return _T("");
	}
	else
		return _T("");
}

BOOL CDxCaptureEVR::SetCurrentInput(int nId)
{
	if (m_pCrossbar && nId >= 0)
        return (m_pCrossbar->SetInputIndex(nId) == S_OK);
	else
		return FALSE;
}

int CDxCaptureEVR::SetDefaultInput()
{
	if (m_pCrossbar)
    {
		int nFirstCompositeIndex = -1;
        for (int j = 0 ; j < GetInputsCount() ; j++)
        {
			LONG lPhysicalType;
            if (m_pCrossbar->GetInputType(j, &lPhysicalType) == S_OK &&
				lPhysicalType == PhysConn_Video_Composite)
			{
				if (nFirstCompositeIndex == -1)
					nFirstCompositeIndex = j;
				else
					return SetCurrentInput(j) ? j : -1; // Second composite input is the preferred choice!
			}
        }
		if (nFirstCompositeIndex >= 0)
			return SetCurrentInput(nFirstCompositeIndex) ? nFirstCompositeIndex : -1;
		else
			return SetCurrentInput(0) ? 0 : -1;
    }
	else
		return -1;
}

int CDxCaptureEVR::EnumInputs(CStringArray &sInputs)
{
	if (m_pCrossbar && GetInputsCount() > 0)
    {
        TCHAR buf[MAX_PATH];

		// Clean-up
		sInputs.RemoveAll();

        for (int j = 0 ; j < GetInputsCount() ; j++)
        {
            if (m_pCrossbar->GetInputName(j, buf, MAX_PATH) != S_OK)
				return -1;
			sInputs.Add(CString(buf));
        }

		return GetInputsCount();
    }
	else
		return -1;
}

BOOL CDxCaptureEVR::Run()
{
	if (!m_pMC)
		return FALSE;
	HRESULT hr = m_pMC->Run();
	if (SUCCEEDED(hr))
		return TRUE;
	else
	{
		// Stop parts that ran
        m_pMC->Stop();
		return FALSE;
	}
}

BOOL CDxCaptureEVR::Pause()
{
	if (!m_pMC)
		return FALSE;
	HRESULT hr = m_pMC->Pause();
	if (SUCCEEDED(hr))
		return TRUE;
	else
	{
		// Stop parts that ran
        m_pMC->Stop();
		return FALSE;
	}
}

BOOL CDxCaptureEVR::Stop()
{
	if (!m_pMC)
		return FALSE;
	HRESULT hr = m_pMC->Stop();
	if (SUCCEEDED(hr))
		return TRUE;
	else
		return FALSE;
}

BOOL CDxCaptureEVR::IsRunning()
{
	if (!m_pMC)
		return FALSE;
	OAFilterState fs;
	HRESULT hr = m_pMC->GetState(200, &fs); // 200 ms timeout
	if (SUCCEEDED(hr))
	{
		if (fs == State_Running)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CDxCaptureEVR::IsPaused()
{
	if (!m_pMC)
		return FALSE;
	OAFilterState fs;
	HRESULT hr = m_pMC->GetState(200, &fs); // 200 ms timeout
	if (SUCCEEDED(hr))
	{
		if (fs == State_Paused)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CDxCaptureEVR::IsStopped()
{
	if (!m_pMC)
		return FALSE;
	OAFilterState fs;
	HRESULT hr = m_pMC->GetState(200, &fs); // 200 ms timeout
	if (SUCCEEDED(hr))
	{
		if (fs == State_Stopped)
			return TRUE;
		else
			return FALSE;
    }
	else
		return FALSE;
}

BOOL CDxCaptureEVR::GetEvent(long* plEventCode,
						  LONG_PTR* pplParam1,
						  LONG_PTR* pplParam2,
						  long msTimeout)
{
	if (!m_pME)
		return FALSE;
	return (m_pME->GetEvent(	plEventCode,
								pplParam1,
								pplParam2,
								msTimeout) == S_OK);
}

BOOL CDxCaptureEVR::FreeEvent(long lEventCode, LONG_PTR plParam1, LONG_PTR plParam2)
{
	if (!m_pME)
		return FALSE;
	return (m_pME->FreeEventParams(lEventCode, plParam1, plParam2) == S_OK);
}

BOOL CDxCaptureEVR::HasVideoCaptureFilterDlg()
{
	BOOL bHasVideoCaptureFilterDlg = FALSE;
	HRESULT hr;

	if (!m_pSrcFilter)
		return FALSE;

	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
	hr = m_pSrcFilter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pSpec);
    if (SUCCEEDED(hr))
    {
        hr = pSpec->GetPages(&cauuid);
        if (SUCCEEDED(hr) && cauuid.cElems > 0)
        {
            ::CoTaskMemFree(cauuid.pElems);
			bHasVideoCaptureFilterDlg = TRUE;
        }
        pSpec->Release();
    }

	return bHasVideoCaptureFilterDlg;
}

BOOL CDxCaptureEVR::ShowVideoCaptureFilterDlg()
{
	BOOL bOk = FALSE;
	HRESULT hr;

	if (!m_pSrcFilter)
		return FALSE;

	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    hr = m_pSrcFilter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pSpec);
    if (SUCCEEDED(hr))
    {
        hr = pSpec->GetPages(&cauuid);
		if (SUCCEEDED(hr) && cauuid.cElems > 0)
		{
			hr = ::OleCreatePropertyFrame(m_hWnd, 30, 30, NULL, 1,
										(IUnknown**)&m_pSrcFilter, cauuid.cElems,
										(GUID*)cauuid.pElems, 0, 0, NULL);
			if (SUCCEEDED(hr))
				bOk = TRUE;
			::CoTaskMemFree(cauuid.pElems);
		}
        pSpec->Release();
    }

	return bOk;
}

BOOL CDxCaptureEVR::HasVideoCapturePinDlg()
{
	BOOL bHasVideoCapturePinDlg = FALSE;
	HRESULT hr;

	if (!m_pConfig)
		return FALSE;

	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    hr = m_pConfig->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pSpec);
    if (SUCCEEDED(hr))
    {
        hr = pSpec->GetPages(&cauuid);
        if (SUCCEEDED(hr) && cauuid.cElems > 0)
        {
            ::CoTaskMemFree(cauuid.pElems);
			bHasVideoCapturePinDlg = TRUE;
        }
        pSpec->Release();
    }
    
	return bHasVideoCapturePinDlg;
}

BOOL CDxCaptureEVR::ShowVideoCapturePinDlg()
{
	BOOL bOk = FALSE;
	HRESULT hr;

	if (!m_pConfig)
		return FALSE;

	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    hr = m_pConfig->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pSpec);
    if (SUCCEEDED(hr))
    {
        hr = pSpec->GetPages(&cauuid);
		if (SUCCEEDED(hr) && cauuid.cElems > 0)
		{
			hr = ::OleCreatePropertyFrame(m_hWnd, 30, 30, NULL, 1,
										(IUnknown**)&m_pConfig, cauuid.cElems,
										(GUID*)cauuid.pElems, 0, 0, NULL);
			if (SUCCEEDED(hr))
				bOk = TRUE;
			::CoTaskMemFree(cauuid.pElems);
		}
		pSpec->Release();
	}

	return bOk;
}

BOOL CDxCaptureEVR::HasVideoTVTunerDlg()
{
	BOOL bHasVideoTVTunerDlg = FALSE;
	HRESULT hr;

	if (!m_pSrcFilter)
		return FALSE;

	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
	IAMTVTuner *pTV;
 
	hr = m_pCaptureGraphBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,
											  NULL, m_pSrcFilter,
											  IID_IAMTVTuner, (void**)&pTV);
    if (SUCCEEDED(hr))
    {
        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pSpec);
        if (SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if (SUCCEEDED(hr) && cauuid.cElems > 0)
            {
                ::CoTaskMemFree(cauuid.pElems);
				bHasVideoTVTunerDlg = TRUE;
            }
            pSpec->Release();
        }
        pTV->Release();
    }

	return bHasVideoTVTunerDlg;
}

BOOL CDxCaptureEVR::ShowVideoTVTunerDlg()
{
	BOOL bOk = FALSE;
	HRESULT hr;

	if (!m_pSrcFilter)
		return FALSE;

	ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
	IAMTVTuner *pTV;
 
	hr = m_pCaptureGraphBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,
											  NULL, m_pSrcFilter,
											  IID_IAMTVTuner, (void**)&pTV);
    if (SUCCEEDED(hr))
    {
        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pSpec);
        if (SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
			if (SUCCEEDED(hr) && cauuid.cElems > 0)
            {
				hr = ::OleCreatePropertyFrame(m_hWnd, 30, 30, NULL, 1,
											(IUnknown**)&pTV,
											cauuid.cElems,
											(GUID*)cauuid.pElems,
											0, 0, NULL);
				if (SUCCEEDED(hr))
					bOk = TRUE;
				::CoTaskMemFree(cauuid.pElems);
			}
            pSpec->Release();
        }
        pTV->Release();
	}

	return bOk;
}

BOOL CDxCaptureEVR::SetVideoPosition(RECT& rcDest)
{
	if (m_pEVRVideoDisplay)
	{
		if (SUCCEEDED(m_pEVRVideoDisplay->SetVideoPosition(NULL, &rcDest)))
			return TRUE;
	}
	
	return FALSE;
}

#ifdef _DEBUG

HRESULT CDxCaptureEVR::AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return E_FAIL;
    }
    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) {
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
            pMoniker, pdwRegister);
        pMoniker->Release();
    }
    pROT->Release();
    return hr;
}

void CDxCaptureEVR::RemoveFromRot(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;
    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

#endif
#endif

