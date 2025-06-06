#if !defined(AFX_CAPTUREGRAPHBUILDER_H__333F94AF_5CBE_4CD9_8EB3_877A4BEC43AA__INCLUDED_)
#define AFX_CAPTUREGRAPHBUILDER_H__333F94AF_5CBE_4CD9_8EB3_877A4BEC43AA__INCLUDED_

#pragma once

#ifdef VIDEODEVICEDOC

// some old vc6 includes don't define them!
typedef unsigned long DWORD_PTR;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
#include "streams.h"
#include <dshow.h>
#include "dvdmedia.h"
#include <atlbase.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>

#ifndef MEDIASUBTYPE_Mpeg2
#define STATIC_MEDIASUBTYPE_Mpeg2\
    0x7DC2C665, 0x4033, 0x4CAF, 0x92, 0x73, 0xF7, 0xD7, 0x97, 0xFB, 0x24, 0x5C
DEFINE_GUIDSTRUCT("7DC2C665-4033-4CAF-9273-F7D797FB245C", MEDIASUBTYPE_Mpeg2);
#define MEDIASUBTYPE_Mpeg2 DEFINE_GUIDNAMED(MEDIASUBTYPE_Mpeg2)
#endif

#ifndef CLSID_Dump
#define STATIC_CLSID_Dump\
    0x36a5f770, 0xfe4c, 0x11ce, 0xa8, 0xed, 0x00, 0xaa, 0x00, 0x2f, 0xea, 0xb5
DEFINE_GUIDSTRUCT("36a5f770-fe4c-11ce-a8ed-00aa002feab5", CLSID_Dump);
#define CLSID_Dump DEFINE_GUIDNAMED(CLSID_Dump)
#endif

class CCaptureGraphBuilder
{
public:
    CCaptureGraphBuilder();
	virtual ~CCaptureGraphBuilder();

    //
    //  OnFinalConstruct build the ICaptureGraphBuilder2
    //
    void ReleaseFilters( )
    {
        pMultiplexer_.Release();
        pEncoder_.Release();
        pMPEG2Demux_.Release();
    }

    STDMETHOD(AllocCapFile)( LPCOLESTR lpwstr, DWORDLONG dwlSize );

    STDMETHOD(ControlStream)( const GUID *pCategory,
                          const GUID *pType,
                          IBaseFilter *pFilter,
                          REFERENCE_TIME *pstart,
                          REFERENCE_TIME *pstop,
                          WORD wStartCookie,
                          WORD wStopCookie
                          );

    STDMETHOD(CopyCaptureFile)(  LPOLESTR lpwstrOld,
                              LPOLESTR lpwstrNew,
                              int fAllowEscAbort,
                              IAMCopyCaptureFileProgress *pCallback
                              );

    STDMETHOD(FindInterface)(const GUID *pCategory,
                          const GUID *pType,
                          IBaseFilter *pf,
                          REFIID riid,
                          void **ppint
                          );

    STDMETHOD(FindPin)( IUnknown *pSource,
                      PIN_DIRECTION pindir,
                      const GUID *pCategory,
                      const GUID *pType,
                      BOOL fUnconnected,
                      int num,
                      IPin **ppPin
                      );


    STDMETHOD(GetFiltergraph)( IGraphBuilder **ppfg );

    STDMETHOD(RenderStream)( const GUID *pCategory,
                          const GUID *pType,
                          IUnknown *pSource,
                          IBaseFilter *pIntermediate,
                          IBaseFilter *pSink
                          );

    STDMETHOD(SetFiltergraph)( IGraphBuilder *pfg );


    STDMETHOD(SetOutputFileName)(
                                const GUID *pType,
                                LPCOLESTR lpwstrFile,
                                IBaseFilter **ppf,
                                IFileSinkFilter **pSink
                                );


protected:

    HRESULT CreateVideoPin( CComPtr<IMpeg2Demultiplexer> pIMpeg2Demux );
    HRESULT CreateAudioPin( CComPtr<IMpeg2Demultiplexer> pIMpeg2Demux );


    HRESULT ConfigureMPEG2Demux( CComPtr<IBaseFilter> pFilter);

    HRESULT FindMPEG2Pin( CComPtr<IBaseFilter> pFilter, CComPtr<IPin>& pPin );
    HRESULT FindPin( 
                CComPtr<IBaseFilter> pFilter, 
                const REGPINMEDIUM& regPinMedium, 
                PIN_DIRECTION direction, 
                BOOL video,             
                CComPtr<IPin>& pPin);

    HRESULT GetMedium( CComPtr<IPin> pPin, REGPINMEDIUM& regPinMedium );
    HRESULT AddMPEG2Demux( );

    HRESULT FindEncoder( CComPtr<IEnumMoniker> pEncoders, REGPINMEDIUM pinMedium, 
                         CComPtr<IBaseFilter>& pEncoder );

    BOOL IsMPEG2Pin( CComPtr<IPin> pPin );
    BOOL IsVideoPin( CComPtr<IPin> pPin );
    BOOL IsAudioPin( CComPtr<IPin> pPin );
    
    BOOL HasMediaType( CComPtr<IPin> pPin, REFGUID majorType );

    HRESULT FindAudioPin( CComPtr<IBaseFilter> pFilter, CComPtr<IPin>& pPin  );
    HRESULT FindVideoPin( CComPtr<IBaseFilter> pFilter, CComPtr<IPin>& pPin  );

    //
    //  tries to build MPEG2 segment for pFilter capture filter
    //
    HRESULT BuildMPEG2Segment( CComPtr<IBaseFilter> pFilter );
    //
    //  renders pPin to a MPEG2 demux
    //
    HRESULT RenderToMPEG2Demux( CComPtr<IPin> pPin );
    //
    //  renders pin pPin with pinMedium to an encoder
    //
    HRESULT RenderToMPEG2Demux( CComPtr<IPin> pPin, const REGPINMEDIUM& pinMedium,  
                                CComPtr<IEnumMoniker> pEncoders );
    //
    //  renders pPin to a MPEG2 demux; there is no special medium, the encoder will be 
    //  serched in the encoder category
    //
    HRESULT RenderToMPEG2Demux( CComPtr<IPin> pPin, CComPtr<IEnumMoniker> pEncoders  );
    //
    //  renders the encoder to a MPEG2 demux
    //
    HRESULT ConnectEncoderToMPEG2Demux( CComPtr< IBaseFilter > pEncoder, 
                                        const REGPINMEDIUM& pinMedium );
    //
    //  renders the demux to a MPEG2 demux
    //
    HRESULT ConnectMultiplexerToMPEG2Demux( CComPtr<IBaseFilter> pEncoder, 
                                            CComPtr< IEnumMoniker > pMultiplexers );
    //
    //  helper methods; connects a pin to a filter and a filter to another one
    //
    HRESULT ConnectPin( CComPtr<IPin>, CComPtr< IBaseFilter > );
    HRESULT ConnectFilters(CComPtr<IBaseFilter> pUpFilter, 
                           CComPtr<IBaseFilter> pDownFilter);

    //
    //  for audio pin; the multiplexer has already beem chosen
    //  if there is no encoder, the method fails
    //
    HRESULT ConnectAudioPinToMultiplexer( CComPtr<IPin> pPin, 
                                          CComPtr<IBaseFilter> pMultiplexer);


    //
    //  helper methods - get the encoders using SystemDeviceEnum or IFilterMapper2
    //
    HRESULT GetEncodersByCategory( CComPtr<IEnumMoniker>& pEncoders );
    HRESULT GetEncodersByEnumerating( CComPtr<IPin> pPin, const REGPINMEDIUM& pinMedium, 
                                      CComPtr<IEnumMoniker>& pEncoders );

    //
    //  helper methods - get the multiplexers using SystemDeviceEnum or IFilterMapper2
    //
    HRESULT GetMultiplexersByCategory( CComPtr<IEnumMoniker>& pMultiplexers);
    HRESULT GetMultiplexersByFilterMapper( CComPtr<IEnumMoniker>& pMultiplexers, 
                                           const REGPINMEDIUM& pinMedium );

    
    CComPtr<IBaseFilter> pMultiplexer_;
    CComPtr<IBaseFilter> pEncoder_;
    CComPtr<IBaseFilter> pMPEG2Demux_;

protected:

    CComPtr<ICaptureGraphBuilder2> graphBuilder2_;
    CComPtr<IGraphBuilder> graph_;

	MPEG2VIDEOINFO* m_pMpeg2ProgramVideo;

    ULONG   VidPID_, 
            AudPID_;
};

#endif

#endif // !defined(AFX_CAPTUREGRAPHBUILDER_H__333F94AF_5CBE_4CD9_8EB3_877A4BEC43AA__INCLUDED_)