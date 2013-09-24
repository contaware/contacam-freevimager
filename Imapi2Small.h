#if !defined(AFX_IMAPI2SMALL_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_IMAPI2SMALL_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_

#pragma once

// Defines
#ifndef IMAPI_E_FSI_INTERNAL_ERROR
#define IMAPI_E_FSI_INTERNAL_ERROR       ((HRESULT)0xC0AAB100L)
#endif
#ifndef IMAPI_E_IMAGE_SIZE_LIMIT
#define IMAPI_E_IMAGE_SIZE_LIMIT         ((HRESULT)0xC0AAB120L)
#endif
#ifndef E_IMAPI_ERASE_MEDIA_IS_NOT_SUPPORTED
#define E_IMAPI_ERASE_MEDIA_IS_NOT_SUPPORTED ((HRESULT)0xC0AA0909L)
#endif
#ifndef E_IMAPI_RECORDER_MEDIA_NO_MEDIA
#define E_IMAPI_RECORDER_MEDIA_NO_MEDIA ((HRESULT)0xC0AA0202L)
#endif
const IID LIBID_IMAPI2 = {0x2735412F,0x7F64,0x5B0F,{0x8F,0x00,0x5D,0x77,0xAF,0xBE,0x26,0x1E}};
#ifndef IMAPILib2_MajorVersion
#define IMAPILib2_MajorVersion 1
#endif
#ifndef IMAPILib2_MinorVersion
#define IMAPILib2_MinorVersion 0
#endif
#ifndef LIBID_IMAPILib2
#define LIBID_IMAPILib2 LIBID_IMAPI2
#endif
#ifndef IID_PPV_ARGS
extern "C++"
{
    template<typename T> void** IID_PPV_ARGS_Helper(T** pp) 
    {
        // make sure everyone derives from IUnknown
        static_cast<IUnknown*>(*pp);
        
        return reinterpret_cast<void**>(pp);
    }
}
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif

// Enums
typedef /* [helpstring][public][v1_enum] */ 
enum EmulationType
    {	EmulationNone	= 0,
	Emulation12MFloppy	= 1,
	Emulation144MFloppy	= 2,
	Emulation288MFloppy	= 3,
	EmulationHardDisk	= 4
    } 	EmulationType;


typedef /* [helpstring][public][v1_enum] */ 
enum PlatformId
    {	PlatformX86	= 0,
	PlatformPowerPC	= 1,
	PlatformMac	= 2
    } 	PlatformId;


typedef /* [helpstring][public][v1_enum] */ 
enum FsiItemType
    {	FsiItemNotFound	= 0,
	FsiItemDirectory	= 1,
	FsiItemFile	= 2
    } 	FsiItemType;


typedef /* [helpstring][public][v1_enum] */ 
enum FsiFileSystems
    {	FsiFileSystemNone	= 0,
	FsiFileSystemISO9660	= 1,
	FsiFileSystemJoliet	= 2,
	FsiFileSystemUDF	= 4,
	FsiFileSystemUnknown	= 0x40000000
    } 	FsiFileSystems;


typedef /* [public][helpstring][v1_enum] */ 
enum _IMAPI_FORMAT2_DATA_MEDIA_STATE
    {	IMAPI_FORMAT2_DATA_MEDIA_STATE_UNKNOWN	= 0,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_INFORMATIONAL_MASK	= 0xf,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MASK	= 0xfc00,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY	= 0x1,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_RANDOMLY_WRITABLE	= 0x1,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_BLANK	= 0x2,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_APPENDABLE	= 0x4,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_FINAL_SESSION	= 0x8,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_DAMAGED	= 0x400,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_ERASE_REQUIRED	= 0x800,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_NON_EMPTY_SESSION	= 0x1000,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_WRITE_PROTECTED	= 0x2000,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_FINALIZED	= 0x4000,
	IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MEDIA	= 0x8000
    } 	IMAPI_FORMAT2_DATA_MEDIA_STATE;


typedef /* [public][helpstring][v1_enum] */ 
enum _IMAPI_MEDIA_WRITE_PROTECT_STATE
    {	IMAPI_WRITEPROTECTED_UNTIL_POWERDOWN	= 0x1,
	IMAPI_WRITEPROTECTED_BY_CARTRIDGE	= 0x2,
	IMAPI_WRITEPROTECTED_BY_MEDIA_SPECIFIC_REASON	= 0x4,
	IMAPI_WRITEPROTECTED_BY_SOFTWARE_WRITE_PROTECT	= 0x8,
	IMAPI_WRITEPROTECTED_BY_DISC_CONTROL_BLOCK	= 0x10,
	IMAPI_WRITEPROTECTED_READ_ONLY_MEDIA	= 0x4000
    } 	IMAPI_MEDIA_WRITE_PROTECT_STATE;


typedef /* [public][helpstring][v1_enum] */ 
enum _IMAPI_MEDIA_PHYSICAL_TYPE
    {	IMAPI_MEDIA_TYPE_UNKNOWN	= 0,
	IMAPI_MEDIA_TYPE_CDROM	= 0x1,
	IMAPI_MEDIA_TYPE_CDR	= 0x2,
	IMAPI_MEDIA_TYPE_CDRW	= 0x3,
	IMAPI_MEDIA_TYPE_DVDROM	= 0x4,
	IMAPI_MEDIA_TYPE_DVDRAM	= 0x5,
	IMAPI_MEDIA_TYPE_DVDPLUSR	= 0x6,
	IMAPI_MEDIA_TYPE_DVDPLUSRW	= 0x7,
	IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER	= 0x8,
	IMAPI_MEDIA_TYPE_DVDDASHR	= 0x9,
	IMAPI_MEDIA_TYPE_DVDDASHRW	= 0xa,
	IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER	= 0xb,
	IMAPI_MEDIA_TYPE_DISK	= 0xc,
	IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER	= 0xd,
	IMAPI_MEDIA_TYPE_HDDVDROM	= 0xe,
	IMAPI_MEDIA_TYPE_HDDVDR	= 0xf,
	IMAPI_MEDIA_TYPE_HDDVDRAM	= 0x10,
	IMAPI_MEDIA_TYPE_BDROM	= 0x11,
	IMAPI_MEDIA_TYPE_BDR	= 0x12,
	IMAPI_MEDIA_TYPE_BDRE	= 0x13,
	IMAPI_MEDIA_TYPE_MAX	= 0x13
    } 	IMAPI_MEDIA_PHYSICAL_TYPE;


typedef /* [public][helpstring][v1_enum] */ 
enum _IMAPI_FORMAT2_DATA_WRITE_ACTION
    {	IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA	= 0,
	IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA	= 0x1,
	IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE	= 0x2,
	IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER	= 0x3,
	IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA	= 0x4,
	IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION	= 0x5,
	IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED	= 0x6
    } 	IMAPI_FORMAT2_DATA_WRITE_ACTION;



#ifndef __IDiscMaster2_INTERFACE_DEFINED__
#define __IDiscMaster2_INTERFACE_DEFINED__

/* interface IDiscMaster2 */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 
    
MIDL_INTERFACE("27354130-7F64-5B0F-8F00-5D77AFBE261E")
IDiscMaster2 : public IDispatch
{
public:
    virtual /* [helpstring][restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
        /* [retval][ref][out] */ IEnumVARIANT **ppunk) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
        /* [in] */ LONG index,
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsSupportedEnvironment( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
};

#endif 	/* __IDiscMaster2_INTERFACE_DEFINED__ */

#ifndef __MsftDiscMaster2_FWD_DEFINED__
#define __MsftDiscMaster2_FWD_DEFINED__
class DECLSPEC_UUID("2735412E-7F64-5B0F-8F00-5D77AFBE261E")
MsftDiscMaster2;
typedef class MsftDiscMaster2 MsftDiscMaster2;
#endif 	/* __MsftDiscMaster2_FWD_DEFINED__ */



#ifndef __IDiscRecorder2_INTERFACE_DEFINED__
#define __IDiscRecorder2_INTERFACE_DEFINED__

/* interface IDiscRecorder2 */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 
    
MIDL_INTERFACE("27354133-7F64-5B0F-8F00-5D77AFBE261E")
IDiscRecorder2 : public IDispatch
{
public:
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EjectMedia( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CloseTray( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AcquireExclusiveAccess( 
        /* [in] */ VARIANT_BOOL force,
        /* [in] */ BSTR __MIDL__IDiscRecorder20000) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReleaseExclusiveAccess( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DisableMcn( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnableMcn( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InitializeDiscRecorder( 
        /* [in] */ BSTR recorderUniqueId) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ActiveDiscRecorder( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VendorId( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProductId( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProductRevision( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VolumeName( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VolumePathNames( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DeviceCanLoadMedia( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LegacyDeviceNumber( 
        /* [retval][ref][out] */ LONG *legacyDeviceNumber) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedFeaturePages( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentFeaturePages( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedProfiles( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentProfiles( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedModePages( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ExclusiveAccessOwner( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
};
#endif 	/* __IDiscRecorder2_INTERFACE_DEFINED__ */

#ifndef __MsftDiscRecorder2_FWD_DEFINED__
#define __MsftDiscRecorder2_FWD_DEFINED__
class DECLSPEC_UUID("2735412D-7F64-5B0F-8F00-5D77AFBE261E")
MsftDiscRecorder2;
typedef class MsftDiscRecorder2 MsftDiscRecorder2;
#endif 	/* __MsftDiscRecorder2_FWD_DEFINED__ */



#ifndef __IDiscFormat2_INTERFACE_DEFINED__
#define __IDiscFormat2_INTERFACE_DEFINED__

/* interface IDiscFormat2 */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 
    
MIDL_INTERFACE("27354152-8F64-5B0F-8F00-5D77AFBE261E")
IDiscFormat2 : public IDispatch
{
public:
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRecorderSupported( 
        /* [in] */ IDiscRecorder2 *recorder,
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsCurrentMediaSupported( 
        /* [in] */ IDiscRecorder2 *recorder,
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MediaPhysicallyBlank( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MediaHeuristicallyBlank( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedMediaTypes( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
};

#endif 	/* __IDiscFormat2_INTERFACE_DEFINED__ */



#ifndef __IDiscFormat2Data_INTERFACE_DEFINED__
#define __IDiscFormat2Data_INTERFACE_DEFINED__

/* interface IDiscFormat2Data */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 
    
MIDL_INTERFACE("27354153-9F64-5B0F-8F00-5D77AFBE261E")
IDiscFormat2Data : public IDiscFormat2
{
public:
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Recorder( 
        /* [in] */ IDiscRecorder2 *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Recorder( 
        /* [retval][ref][out] */ IDiscRecorder2 **value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BufferUnderrunFreeDisabled( 
        /* [in] */ VARIANT_BOOL value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BufferUnderrunFreeDisabled( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PostgapAlreadyInImage( 
        /* [in] */ VARIANT_BOOL value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PostgapAlreadyInImage( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentMediaStatus( 
        /* [retval][ref][out] */ IMAPI_FORMAT2_DATA_MEDIA_STATE *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WriteProtectStatus( 
        /* [retval][ref][out] */ IMAPI_MEDIA_WRITE_PROTECT_STATE *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TotalSectorsOnMedia( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FreeSectorsOnMedia( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NextWritableAddress( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartAddressOfPreviousSession( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastWrittenAddressOfPreviousSession( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ForceMediaToBeClosed( 
        /* [in] */ VARIANT_BOOL value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ForceMediaToBeClosed( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DisableConsumerDvdCompatibilityMode( 
        /* [in] */ VARIANT_BOOL value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DisableConsumerDvdCompatibilityMode( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentPhysicalMediaType( 
        /* [retval][ref][out] */ IMAPI_MEDIA_PHYSICAL_TYPE *value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ClientName( 
        /* [in] */ BSTR value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClientName( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RequestedWriteSpeed( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RequestedRotationTypeIsPureCAV( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentWriteSpeed( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentRotationTypeIsPureCAV( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedWriteSpeeds( 
        /* [retval][ref][out] */ SAFEARRAY * *supportedSpeeds) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SupportedWriteSpeedDescriptors( 
        /* [retval][ref][out] */ SAFEARRAY * *supportedSpeedDescriptors) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ForceOverwrite( 
        /* [in] */ VARIANT_BOOL value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ForceOverwrite( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MultisessionInterfaces( 
        /* [retval][ref][out] */ SAFEARRAY * *value) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Write( 
        /* [in] */ IStream *data) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CancelWrite( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetWriteSpeed( 
        /* [in] */ LONG RequestedSectorsPerSecond,
        /* [in] */ VARIANT_BOOL RotationTypeIsPureCAV) = 0;
    
};
   
#endif 	/* __IDiscFormat2Data_INTERFACE_DEFINED__ */

#ifndef __MsftDiscFormat2Data_FWD_DEFINED__
#define __MsftDiscFormat2Data_FWD_DEFINED__
class DECLSPEC_UUID("2735412A-7F64-5B0F-8F00-5D77AFBE261E")
MsftDiscFormat2Data;
typedef class MsftDiscFormat2Data MsftDiscFormat2Data;
#endif 	/* __MsftDiscFormat2Data_FWD_DEFINED__ */



#ifndef __DDiscFormat2DataEvents_INTERFACE_DEFINED__
#define __DDiscFormat2DataEvents_INTERFACE_DEFINED__

/* interface DDiscFormat2DataEvents */
/* [helpstring][unique][uuid][oleautomation][nonextensible][object] */ 

const IID IID_DDiscFormat2DataEvents = {0x2735413C,0x7F64,0x5B0F,{0x8F,0x00,0x5D,0x77,0xAF,0xBE,0x26,0x1E}};

MIDL_INTERFACE("2735413C-7F64-5B0F-8F00-5D77AFBE261E")
DDiscFormat2DataEvents : public IDispatch
{
public:
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Update( 
        /* [in] */ IDispatch *object,
        /* [in] */ IDispatch *progress) = 0;
    
};

#endif 	/* __DDiscFormat2DataEvents_INTERFACE_DEFINED__ */



#ifndef __IWriteEngine2EventArgs_INTERFACE_DEFINED__
#define __IWriteEngine2EventArgs_INTERFACE_DEFINED__

/* interface IWriteEngine2EventArgs */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 

MIDL_INTERFACE("27354136-7F64-5B0F-8F00-5D77AFBE261E")
IWriteEngine2EventArgs : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartLba( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SectorCount( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastReadLba( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastWrittenLba( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TotalSystemBuffer( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UsedSystemBuffer( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FreeSystemBuffer( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
};

#endif 	/* __IWriteEngine2EventArgs_INTERFACE_DEFINED__ */



#ifndef __IDiscFormat2DataEventArgs_INTERFACE_DEFINED__
#define __IDiscFormat2DataEventArgs_INTERFACE_DEFINED__

/* interface IDiscFormat2DataEventArgs */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 
    
MIDL_INTERFACE("2735413D-7F64-5B0F-8F00-5D77AFBE261E")
IDiscFormat2DataEventArgs : public IWriteEngine2EventArgs
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ElapsedTime( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_RemainingTime( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TotalTime( 
        /* [retval][ref][out] */ LONG *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentAction( 
        /* [retval][ref][out] */ IMAPI_FORMAT2_DATA_WRITE_ACTION *value) = 0;
    
};

#endif 	/* __IDiscFormat2DataEventArgs_INTERFACE_DEFINED__ */



#ifndef __DDiscFormat2EraseEvents_INTERFACE_DEFINED__
#define __DDiscFormat2EraseEvents_INTERFACE_DEFINED__

/* interface DDiscFormat2EraseEvents */
/* [helpstring][unique][uuid][oleautomation][nonextensible][object] */ 
    
const IID IID_DDiscFormat2EraseEvents = {0x2735413A,0x7F64,0x5B0F,{0x8F,0x00,0x5D,0x77,0xAF,0xBE,0x26,0x1E}};

MIDL_INTERFACE("2735413A-7F64-5B0F-8F00-5D77AFBE261E")
DDiscFormat2EraseEvents : public IDispatch
{
public:
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Update( 
        /* [in] */ IDispatch *object,
        /* [in] */ LONG elapsedSeconds,
        /* [in] */ LONG estimatedTotalSeconds) = 0;
    
};

#endif 	/* __DDiscFormat2EraseEvents_INTERFACE_DEFINED__ */



#ifndef __IDiscFormat2Erase_INTERFACE_DEFINED__
#define __IDiscFormat2Erase_INTERFACE_DEFINED__

/* interface IDiscFormat2Erase */
/* [helpstring][unique][uuid][dual][nonextensible][object] */ 
    
MIDL_INTERFACE("27354156-8F64-5B0F-8F00-5D77AFBE261E")
IDiscFormat2Erase : public IDiscFormat2
{
public:
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Recorder( 
        /* [in] */ IDiscRecorder2 *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Recorder( 
        /* [retval][ref][out] */ IDiscRecorder2 **value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FullErase( 
        /* [in] */ VARIANT_BOOL value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FullErase( 
        /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentPhysicalMediaType( 
        /* [retval][ref][out] */ IMAPI_MEDIA_PHYSICAL_TYPE *value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ClientName( 
        /* [in] */ BSTR value) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClientName( 
        /* [retval][ref][out] */ BSTR *value) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EraseMedia( void) = 0;
    
};

#endif 	/* __IDiscFormat2Erase_INTERFACE_DEFINED__ */

#ifndef __MsftDiscFormat2Erase_FWD_DEFINED__
#define __MsftDiscFormat2Erase_FWD_DEFINED__
class DECLSPEC_UUID("2735412B-7F64-5B0F-8F00-5D77AFBE261E")
MsftDiscFormat2Erase;
typedef class MsftDiscFormat2Erase MsftDiscFormat2Erase;
#endif 	/* __MsftDiscFormat2Erase_FWD_DEFINED__ */



#ifndef __IFsiItem_INTERFACE_DEFINED__
#define __IFsiItem_INTERFACE_DEFINED__

/* interface IFsiItem */
/* [helpstring][uuid][nonextensible][oleautomation][dual][unique][object] */ 
    
MIDL_INTERFACE("2C941FD9-975B-59BE-A960-9A2A262853A5")
IFsiItem : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
        /* [retval][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FullPath( 
        /* [retval][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CreationTime( 
        /* [retval][out] */ DATE *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_CreationTime( 
        /* [in] */ DATE newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastAccessedTime( 
        /* [retval][out] */ DATE *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LastAccessedTime( 
        /* [in] */ DATE newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastModifiedTime( 
        /* [retval][out] */ DATE *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_LastModifiedTime( 
        /* [in] */ DATE newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_IsHidden( 
        /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_IsHidden( 
        /* [in] */ VARIANT_BOOL newVal) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FileSystemName( 
        /* [in] */ FsiFileSystems fileSystem,
        /* [retval][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FileSystemPath( 
        /* [in] */ FsiFileSystems fileSystem,
        /* [retval][out] */ BSTR *pVal) = 0;
    
};

#endif 	/* __IFsiItem_INTERFACE_DEFINED__ */



#ifndef __IEnumFsiItems_INTERFACE_DEFINED__
#define __IEnumFsiItems_INTERFACE_DEFINED__

/* interface IEnumFsiItems */
/* [helpstring][uuid][unique][object] */ 
    
MIDL_INTERFACE("2C941FDA-975B-59BE-A960-9A2A262853A5")
IEnumFsiItems : public IUnknown
{
public:
    virtual /* [helpstring][local] */ HRESULT STDMETHODCALLTYPE Next( 
        /* [in] */ ULONG celt,
        /* [length_is][size_is][out] */ IFsiItem **rgelt,
        /* [out] */ ULONG *pceltFetched) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
        /* [in] */ ULONG celt) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
        /* [out] */ IEnumFsiItems **ppEnum) = 0;
    
};

/* [helpstring][call_as] */ HRESULT STDMETHODCALLTYPE IEnumFsiItems_RemoteNext_Proxy( 
    IEnumFsiItems * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IFsiItem **rgelt,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumFsiItems_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	/* __IEnumFsiItems_INTERFACE_DEFINED__ */



#ifndef __IFsiFileItem_INTERFACE_DEFINED__
#define __IFsiFileItem_INTERFACE_DEFINED__

/* interface IFsiFileItem */
/* [helpstring][uuid][nonextensible][oleautomation][dual][unique][object] */ 
    
MIDL_INTERFACE("2C941FDB-975B-59BE-A960-9A2A262853A5")
IFsiFileItem : public IFsiItem
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DataSize( 
        /* [retval][out] */ LONGLONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DataSize32BitLow( 
        /* [retval][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DataSize32BitHigh( 
        /* [retval][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Data( 
        /* [retval][out] */ IStream **pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Data( 
        /* [in] */ IStream *newVal) = 0;
    
};

#endif 	/* __IFsiFileItem_INTERFACE_DEFINED__ */



#ifndef __IFsiDirectoryItem_INTERFACE_DEFINED__
#define __IFsiDirectoryItem_INTERFACE_DEFINED__

/* interface IFsiDirectoryItem */
/* [helpstring][uuid][nonextensible][oleautomation][dual][unique][object] */ 
    
MIDL_INTERFACE("2C941FDC-975B-59BE-A960-9A2A262853A5")
IFsiDirectoryItem : public IFsiItem
{
public:
    virtual /* [helpstring][restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
        /* [retval][out] */ IEnumVARIANT **NewEnum) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
        /* [in] */ BSTR path,
        /* [retval][out] */ IFsiItem **item) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
        /* [retval][out] */ LONG *Count) = 0;
    
    virtual /* [helpstring][restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnumFsiItems( 
        /* [retval][out] */ IEnumFsiItems **NewEnum) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddDirectory( 
        /* [in] */ BSTR path) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddFile( 
        /* [in] */ BSTR path,
        /* [in] */ IStream *fileData) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddTree( 
        /* [in] */ BSTR sourceDirectory,
        /* [in] */ VARIANT_BOOL includeBaseDirectory) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
        /* [in] */ IFsiItem *item) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
        /* [in] */ BSTR path) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RemoveTree( 
        /* [in] */ BSTR path) = 0;
    
};

#endif 	/* __IFsiDirectoryItem_INTERFACE_DEFINED__ */



#ifndef __IProgressItem_INTERFACE_DEFINED__
#define __IProgressItem_INTERFACE_DEFINED__

/* interface IProgressItem */
/* [helpstring][uuid][oleautomation][nonextensible][dual][unique][object] */ 

MIDL_INTERFACE("2C941FD5-975B-59BE-A960-9A2A262853A5")
IProgressItem : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Description( 
        /* [retval][out] */ BSTR *desc) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FirstBlock( 
        /* [retval][out] */ ULONG *block) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_LastBlock( 
        /* [retval][out] */ ULONG *block) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlockCount( 
        /* [retval][out] */ ULONG *blocks) = 0;
    
};

#endif 	/* __IProgressItem_INTERFACE_DEFINED__ */



#ifndef __IEnumProgressItems_INTERFACE_DEFINED__
#define __IEnumProgressItems_INTERFACE_DEFINED__

/* interface IEnumProgressItems */
/* [helpstring][uuid][unique][object] */ 
    
MIDL_INTERFACE("2C941FD6-975B-59BE-A960-9A2A262853A5")
IEnumProgressItems : public IUnknown
{
public:
    virtual /* [helpstring][local] */ HRESULT STDMETHODCALLTYPE Next( 
        /* [in] */ ULONG celt,
        /* [length_is][size_is][out] */ IProgressItem **rgelt,
        /* [out] */ ULONG *pceltFetched) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
        /* [in] */ ULONG celt) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
        /* [out] */ IEnumProgressItems **ppEnum) = 0;
    
};

/* [helpstring][call_as] */ HRESULT STDMETHODCALLTYPE IEnumProgressItems_RemoteNext_Proxy( 
    IEnumProgressItems * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IProgressItem **rgelt,
    /* [out] */ ULONG *pceltFetched);

void __RPC_STUB IEnumProgressItems_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	/* __IEnumProgressItems_INTERFACE_DEFINED__ */



#ifndef __IProgressItems_INTERFACE_DEFINED__
#define __IProgressItems_INTERFACE_DEFINED__

/* interface IProgressItems */
/* [helpstring][uuid][oleautomation][nonextensible][dual][unique][object] */ 

MIDL_INTERFACE("2C941FD7-975B-59BE-A960-9A2A262853A5")
IProgressItems : public IDispatch
{
public:
    virtual /* [helpstring][restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
        /* [retval][out] */ IEnumVARIANT **NewEnum) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
        /* [in] */ long Index,
        /* [retval][out] */ IProgressItem **item) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
        /* [retval][out] */ long *Count) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ProgressItemFromBlock( 
        /* [in] */ ULONG block,
        /* [retval][out] */ IProgressItem **item) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ProgressItemFromDescription( 
        /* [in] */ BSTR description,
        /* [retval][out] */ IProgressItem **item) = 0;
    
    virtual /* [helpstring][restricted][hidden][id][propget] */ HRESULT STDMETHODCALLTYPE get_EnumProgressItems( 
        /* [retval][out] */ IEnumProgressItems **NewEnum) = 0;
    
};

#endif 	/* __IProgressItems_INTERFACE_DEFINED__ */



#ifndef __IFileSystemImageResult_INTERFACE_DEFINED__
#define __IFileSystemImageResult_INTERFACE_DEFINED__

/* interface IFileSystemImageResult */
/* [helpstring][uuid][nonextensible][oleautomation][dual][unique][object] */ 
    
MIDL_INTERFACE("2C941FD8-975B-59BE-A960-9A2A262853A5")
IFileSystemImageResult : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageStream( 
        /* [retval][out] */ IStream **pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ProgressItems( 
        /* [retval][out] */ IProgressItems **pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TotalBlocks( 
        /* [retval][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BlockSize( 
        /* [retval][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DiscId( 
        /* [retval][out] */ BSTR *pVal) = 0;
    
};

#endif 	/* __IFileSystemImageResult_INTERFACE_DEFINED__ */



#ifndef __IBootOptions_INTERFACE_DEFINED__
#define __IBootOptions_INTERFACE_DEFINED__

/* interface IBootOptions */
/* [helpstring][uuid][oleautomation][nonextensible][dual][unique][object] */ 
    
MIDL_INTERFACE("2C941FD4-975B-59BE-A960-9A2A262853A5")
IBootOptions : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BootImage( 
        /* [retval][out] */ IStream **pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Manufacturer( 
        /* [retval][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Manufacturer( 
        /* [in] */ BSTR newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PlatformId( 
        /* [retval][out] */ PlatformId *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PlatformId( 
        /* [in] */ PlatformId newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Emulation( 
        /* [retval][out] */ EmulationType *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Emulation( 
        /* [in] */ EmulationType newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageSize( 
        /* [retval][out] */ ULONG *pVal) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AssignBootImage( 
        /* [in] */ IStream *newVal) = 0;
    
};

#endif 	/* __IBootOptions_INTERFACE_DEFINED__ */



#ifndef __IFileSystemImage_INTERFACE_DEFINED__
#define __IFileSystemImage_INTERFACE_DEFINED__

/* interface IFileSystemImage */
/* [helpstring][uuid][nonextensible][oleautomation][dual][unique][object] */ 
    
MIDL_INTERFACE("2C941FE1-975B-59BE-A960-9A2A262853A5")
IFileSystemImage : public IDispatch
{
public:
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Root( 
        /* [retval][ref][out] */ IFsiDirectoryItem **pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SessionStartBlock( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SessionStartBlock( 
        /* [in] */ LONG newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FreeMediaBlocks( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FreeMediaBlocks( 
        /* [in] */ LONG newVal) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetMaxMediaBlocksFromDevice( 
        /* [in] */ IDiscRecorder2 *discRecorder) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UsedBlocks( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VolumeName( 
        /* [retval][ref][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VolumeName( 
        /* [in] */ BSTR newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImportedVolumeName( 
        /* [retval][ref][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BootImageOptions( 
        /* [retval][ref][out] */ IBootOptions **pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BootImageOptions( 
        /* [in] */ IBootOptions *newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileCount( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DirectoryCount( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WorkingDirectory( 
        /* [retval][ref][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_WorkingDirectory( 
        /* [in] */ BSTR newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ChangePoint( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StrictFileSystemCompliance( 
        /* [retval][ref][out] */ VARIANT_BOOL *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StrictFileSystemCompliance( 
        /* [in] */ VARIANT_BOOL newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UseRestrictedCharacterSet( 
        /* [retval][ref][out] */ VARIANT_BOOL *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UseRestrictedCharacterSet( 
        /* [in] */ VARIANT_BOOL newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileSystemsToCreate( 
        /* [retval][ref][out] */ FsiFileSystems *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FileSystemsToCreate( 
        /* [in] */ FsiFileSystems newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileSystemsSupported( 
        /* [retval][ref][out] */ FsiFileSystems *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_UDFRevision( 
        /* [in] */ LONG newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UDFRevision( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_UDFRevisionsSupported( 
        /* [retval][ref][out] */ SAFEARRAY * *pVal) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ChooseImageDefaults( 
        /* [in] */ IDiscRecorder2 *discRecorder) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ChooseImageDefaultsForMediaType( 
        /* [in] */ IMAPI_MEDIA_PHYSICAL_TYPE value) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ISO9660InterchangeLevel( 
        /* [in] */ LONG newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ISO9660InterchangeLevel( 
        /* [retval][ref][out] */ LONG *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ISO9660InterchangeLevelsSupported( 
        /* [retval][ref][out] */ SAFEARRAY * *pVal) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateResultImage( 
        /* [retval][ref][out] */ IFileSystemImageResult **resultStream) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Exists( 
        /* [in] */ BSTR fullPath,
        /* [retval][ref][out] */ FsiItemType *itemType) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CalculateDiscIdentifier( 
        /* [retval][ref][out] */ BSTR *discIdentifier) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IdentifyFileSystemsOnDisc( 
        /* [in] */ IDiscRecorder2 *discRecorder,
        /* [retval][ref][out] */ FsiFileSystems *fileSystems) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDefaultFileSystemForImport( 
        /* [in] */ FsiFileSystems fileSystems,
        /* [retval][ref][out] */ FsiFileSystems *importDefault) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ImportFileSystem( 
        /* [retval][ref][out] */ FsiFileSystems *importedFileSystem) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ImportSpecificFileSystem( 
        /* [in] */ FsiFileSystems fileSystemToUse) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE RollbackToChangePoint( 
        /* [in] */ LONG changePoint) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE LockInChangePoint( void) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateDirectoryItem( 
        /* [in] */ BSTR name,
        /* [retval][ref][out] */ IFsiDirectoryItem **newItem) = 0;
    
    virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateFileItem( 
        /* [in] */ BSTR name,
        /* [retval][ref][out] */ IFsiFileItem **newItem) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VolumeNameUDF( 
        /* [retval][ref][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VolumeNameJoliet( 
        /* [retval][ref][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VolumeNameISO9660( 
        /* [retval][ref][out] */ BSTR *pVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StageFiles( 
        /* [retval][ref][out] */ VARIANT_BOOL *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StageFiles( 
        /* [in] */ VARIANT_BOOL newVal) = 0;
    
    virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_MultisessionInterfaces( 
        /* [retval][ref][out] */ SAFEARRAY * *pVal) = 0;
    
    virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_MultisessionInterfaces( 
        /* [in] */ SAFEARRAY * newVal) = 0;
    
};

#endif 	/* __IFileSystemImage_INTERFACE_DEFINED__ */

const CLSID CLSID_MsftFileSystemImage = {0x2C941FC5,0x975B,0x59BE,{0xA9,0x60,0x9A,0x2A,0x26,0x28,0x53,0xA5}};

#ifndef __MsftFileSystemImage_FWD_DEFINED__
#define __MsftFileSystemImage_FWD_DEFINED__
class DECLSPEC_UUID("2C941FC5-975B-59BE-A960-9A2A262853A5")
MsftFileSystemImage;
typedef class MsftFileSystemImage MsftFileSystemImage;
#endif 	/* __MsftFileSystemImage_FWD_DEFINED__ */


#endif // !defined(AFX_IMAPI2SMALL_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
