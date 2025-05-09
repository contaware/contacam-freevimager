#ifndef __ISampleGrabber_h__
#define __ISampleGrabber_h__

#pragma once

#ifndef __ISampleGrabberCB_INTERFACE_DEFINED__
#define __ISampleGrabberCB_INTERFACE_DEFINED__

/* interface ISampleGrabberCB */
/* [unique][helpstring][local][uuid][object] */ 


EXTERN_C const IID IID_ISampleGrabberCB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
    ISampleGrabberCB : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SampleCB( 
            double SampleTime,
            IMediaSample *pSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BufferCB( 
            double SampleTime,
            BYTE *pBuffer,
            long BufferLen) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleGrabberCBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleGrabberCB * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleGrabberCB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleGrabberCB * This);
        
        HRESULT ( STDMETHODCALLTYPE *SampleCB )( 
            ISampleGrabberCB * This,
            double SampleTime,
            IMediaSample *pSample);
        
        HRESULT ( STDMETHODCALLTYPE *BufferCB )( 
            ISampleGrabberCB * This,
            double SampleTime,
            BYTE *pBuffer,
            long BufferLen);
        
        END_INTERFACE
    } ISampleGrabberCBVtbl;

    interface ISampleGrabberCB
    {
        CONST_VTBL struct ISampleGrabberCBVtbl *lpVtbl;
    };

#endif 	/* C style interface */


HRESULT STDMETHODCALLTYPE ISampleGrabberCB_SampleCB_Proxy( 
    ISampleGrabberCB * This,
    double SampleTime,
    IMediaSample *pSample);


void __RPC_STUB ISampleGrabberCB_SampleCB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabberCB_BufferCB_Proxy( 
    ISampleGrabberCB * This,
    double SampleTime,
    BYTE *pBuffer,
    long BufferLen);


void __RPC_STUB ISampleGrabberCB_BufferCB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISampleGrabberCB_INTERFACE_DEFINED__ */


#ifndef __ISampleGrabber_INTERFACE_DEFINED__
#define __ISampleGrabber_INTERFACE_DEFINED__

/* interface ISampleGrabber */
/* [unique][helpstring][local][uuid][object] */ 


EXTERN_C const IID IID_ISampleGrabber;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
    ISampleGrabber : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOneShot( 
            BOOL OneShot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMediaType( 
            const AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( 
            AM_MEDIA_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( 
            BOOL BufferThem) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( 
            /* [out][in] */ long *pBufferSize,
            /* [out] */ long *pBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( 
            /* [retval][out] */ IMediaSample **ppSample) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCallback( 
            ISampleGrabberCB *pCallback,
            long WhichMethodToCallback) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleGrabberVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleGrabber * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleGrabber * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleGrabber * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetOneShot )( 
            ISampleGrabber * This,
            BOOL OneShot);
        
        HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            ISampleGrabber * This,
            const AM_MEDIA_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *GetConnectedMediaType )( 
            ISampleGrabber * This,
            AM_MEDIA_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferSamples )( 
            ISampleGrabber * This,
            BOOL BufferThem);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentBuffer )( 
            ISampleGrabber * This,
            /* [out][in] */ long *pBufferSize,
            /* [out] */ long *pBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentSample )( 
            ISampleGrabber * This,
            /* [retval][out] */ IMediaSample **ppSample);
        
        HRESULT ( STDMETHODCALLTYPE *SetCallback )( 
            ISampleGrabber * This,
            ISampleGrabberCB *pCallback,
            long WhichMethodToCallback);
        
        END_INTERFACE
    } ISampleGrabberVtbl;

    interface ISampleGrabber
    {
        CONST_VTBL struct ISampleGrabberVtbl *lpVtbl;
    };

#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISampleGrabber_SetOneShot_Proxy( 
    ISampleGrabber * This,
    BOOL OneShot);


void __RPC_STUB ISampleGrabber_SetOneShot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_SetMediaType_Proxy( 
    ISampleGrabber * This,
    const AM_MEDIA_TYPE *pType);


void __RPC_STUB ISampleGrabber_SetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_GetConnectedMediaType_Proxy( 
    ISampleGrabber * This,
    AM_MEDIA_TYPE *pType);


void __RPC_STUB ISampleGrabber_GetConnectedMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_SetBufferSamples_Proxy( 
    ISampleGrabber * This,
    BOOL BufferThem);


void __RPC_STUB ISampleGrabber_SetBufferSamples_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_GetCurrentBuffer_Proxy( 
    ISampleGrabber * This,
    /* [out][in] */ long *pBufferSize,
    /* [out] */ long *pBuffer);


void __RPC_STUB ISampleGrabber_GetCurrentBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_GetCurrentSample_Proxy( 
    ISampleGrabber * This,
    /* [retval][out] */ IMediaSample **ppSample);


void __RPC_STUB ISampleGrabber_GetCurrentSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISampleGrabber_SetCallback_Proxy( 
    ISampleGrabber * This,
    ISampleGrabberCB *pCallback,
    long WhichMethodToCallback);


void __RPC_STUB ISampleGrabber_SetCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	/* __ISampleGrabber_INTERFACE_DEFINED__ */



EXTERN_C const CLSID CLSID_SampleGrabber;

#ifdef __cplusplus

class DECLSPEC_UUID("C1F400A0-3F08-11d3-9F0B-006008039E37")
SampleGrabber;
#endif

EXTERN_C const CLSID CLSID_NullRenderer;

#ifdef __cplusplus

class DECLSPEC_UUID("C1F400A4-3F08-11d3-9F0B-006008039E37")
NullRenderer;
#endif


#endif


