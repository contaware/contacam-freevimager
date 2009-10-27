// DxTuner.cpp: implementation of the CDxTuner class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DxTuner.h"
#include <ks.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef VIDEODEVICEDOC

#define INSTANCE_DATA_OF_PROPERTY_PTR(x) ( (PKSPROPERTY((x)) ) + 1 )
#define INSTANCE_DATA_OF_PROPERTY_SIZE(x) ( sizeof((x)) - sizeof(KSPROPERTY) )

#define STATIC_PROPSETID_TUNER\
    0x6a2e0605L, 0x28e4, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56
DEFINE_GUIDSTRUCT("6a2e0605-28e4-11d0-a18c-00a0c9118956", PROPSETID_TUNER);
#define PROPSETID_TUNER DEFINE_GUIDNAMED(PROPSETID_TUNER)

typedef enum {
    KSPROPERTY_TUNER_CAPS,              // R  -overall device capabilities
    KSPROPERTY_TUNER_MODE_CAPS,         // R  -capabilities in this mode
    KSPROPERTY_TUNER_MODE,              // RW -set a mode (TV, FM, AM, DSS)
    KSPROPERTY_TUNER_STANDARD,          // R  -get TV standard (only if TV mode)
    KSPROPERTY_TUNER_FREQUENCY,         // RW -set/get frequency
    KSPROPERTY_TUNER_INPUT,             // RW -select an input
    KSPROPERTY_TUNER_STATUS,            // R  -tuning status
    KSPROPERTY_TUNER_IF_MEDIUM          // R O-Medium for IF or Transport Pin
} KSPROPERTY_TUNER;

typedef enum {
    KSPROPERTY_TUNER_MODE_TV            = 0X0001,
    KSPROPERTY_TUNER_MODE_FM_RADIO      = 0X0002,
    KSPROPERTY_TUNER_MODE_AM_RADIO      = 0X0004,
    KSPROPERTY_TUNER_MODE_DSS           = 0X0008,
    KSPROPERTY_TUNER_MODE_ATSC          = 0X0010,  // also used for DVB-T, DVB-C
} KSPROPERTY_TUNER_MODES;

// Describes how the device tunes.  Only one of these flags may be set
// in KSPROPERTY_TUNER_MODE_CAPS_S.Strategy

// Describe how the driver should attempt to tune:
// EXACT:   just go to the frequency specified (no fine tuning)
// FINE:    (slow) do an exhaustive search for the best signal
// COARSE:  (fast) use larger frequency jumps to just determine if any signal
typedef enum {
    KS_TUNER_TUNING_EXACT = 1,        // No fine tuning
    KS_TUNER_TUNING_FINE,             // Fine grained search
    KS_TUNER_TUNING_COARSE,           // Coarse search
} KS_TUNER_TUNING_FLAGS;

typedef enum {
    KS_TUNER_STRATEGY_PLL             = 0X01, // Tune by PLL offset
    KS_TUNER_STRATEGY_SIGNAL_STRENGTH = 0X02, // Tune by signal strength
    KS_TUNER_STRATEGY_DRIVER_TUNES    = 0X04, // Driver does fine tuning
}KS_TUNER_STRATEGY;

typedef struct {
    KSPROPERTY Property;
    ULONG  ModesSupported;              // KS_PROPERTY_TUNER_MODES_*
    KSPIN_MEDIUM VideoMedium;           // GUID_NULL (no pin), or GUID
    KSPIN_MEDIUM TVAudioMedium;         // GUID_NULL (no pin), or GUID
    KSPIN_MEDIUM RadioAudioMedium;      // GUID_NULL (no pin), or GUID
} KSPROPERTY_TUNER_CAPS_S, *PKSPROPERTY_TUNER_CAPS_S;

typedef struct {
    KSPROPERTY Property;
    KSPIN_MEDIUM IFMedium;              // GUID_NULL (no pin), or GUID
} KSPROPERTY_TUNER_IF_MEDIUM_S, *PKSPROPERTY_TUNER_IF_MEDIUM_S;

typedef struct {
    KSPROPERTY Property;
    ULONG  Mode;                        // IN: KSPROPERTY_TUNER_MODE
    ULONG  StandardsSupported;          // KS_AnalogVideo_* (if TV or DSS)
    ULONG  MinFrequency;                // Hz
    ULONG  MaxFrequency;                // Hz
    ULONG  TuningGranularity;           // Hz
    ULONG  NumberOfInputs;              // count of inputs
    ULONG  SettlingTime;                // milliSeconds
    ULONG  Strategy;                    // KS_TUNER_STRATEGY
} KSPROPERTY_TUNER_MODE_CAPS_S, *PKSPROPERTY_TUNER_MODE_CAPS_S;

typedef struct {
    KSPROPERTY Property;
    ULONG  Mode;                        // IN: KSPROPERTY_TUNER_MODE
} KSPROPERTY_TUNER_MODE_S, *PKSPROPERTY_TUNER_MODE_S;

typedef struct {
    KSPROPERTY Property;
    ULONG  Frequency;                   // Hz
    ULONG  LastFrequency;               // Hz (last known good)
    ULONG  TuningFlags;                 // KS_TUNER_TUNING_FLAGS
    ULONG  VideoSubChannel;             // DSS
    ULONG  AudioSubChannel;             // DSS
    ULONG  Channel;                     // VBI decoders
    ULONG  Country;                     // VBI decoders
} KSPROPERTY_TUNER_FREQUENCY_S, *PKSPROPERTY_TUNER_FREQUENCY_S;

typedef struct {
    KSPROPERTY Property;
    ULONG  Standard;                    // KS_AnalogVideo_*
} KSPROPERTY_TUNER_STANDARD_S, *PKSPROPERTY_TUNER_STANDARD_S;

typedef struct {
    KSPROPERTY Property;
    ULONG  InputIndex;                  // 0 to (n-1) inputs
} KSPROPERTY_TUNER_INPUT_S, *PKSPROPERTY_TUNER_INPUT_S;

typedef struct {
    KSPROPERTY Property;
    ULONG  CurrentFrequency;            // Hz
    ULONG  PLLOffset;                   // if Strategy.KS_TUNER_STRATEGY_PLL
    ULONG  SignalStrength;              // if Stretegy.KS_TUNER_STRATEGY_SIGNAL_STRENGTH
    ULONG  Busy;                        // TRUE if in the process of tuning
} KSPROPERTY_TUNER_STATUS_S, *PKSPROPERTY_TUNER_STATUS_S;

#define STATIC_EVENTSETID_TUNER\
    0x6a2e0606L, 0x28e4, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56
DEFINE_GUIDSTRUCT("6a2e0606-28e4-11d0-a18c-00a0c9118956", EVENTSETID_TUNER);
#define EVENTSETID_TUNER DEFINE_GUIDNAMED(EVENTSETID_TUNER)

template<typename DataType>
BOOL GetKSData(CComPtr<IKsPropertySet> pKSProp, KSPROPERTY_TUNER Property, DataType &Data)
{
	if (pKSProp)
	{
		DWORD dwSupported = 0;
		HRESULT hr = pKSProp->QuerySupported(PROPSETID_TUNER,Property,&dwSupported);
		if (SUCCEEDED(hr) && dwSupported&KSPROPERTY_SUPPORT_GET)
		{
			DWORD cbBytes=0;
			hr=pKSProp->Get(PROPSETID_TUNER,
				Property,
				INSTANCE_DATA_OF_PROPERTY_PTR(&Data),
				INSTANCE_DATA_OF_PROPERTY_SIZE(Data),
				&Data,
				sizeof(Data),
				&cbBytes);
			if (SUCCEEDED(hr) && cbBytes>=sizeof(DataType))
				return TRUE;
			else
				return FALSE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

CDxTuner::CDxTuner(CComPtr<IAMTVTuner> pTVTuner)
{
	pTVTuner.QueryInterface(&m_pKSProp);
}

CDxTuner::~CDxTuner()
{

}

AMTunerModeType CDxTuner::GetTunerMode()
{
	KSPROPERTY_TUNER_MODE_S TunerMode;
	memset(&TunerMode,0,sizeof(KSPROPERTY_TUNER_MODE_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE,TunerMode);
	return (AMTunerModeType)TunerMode.Mode;
}

BOOL CDxTuner::SetTunerMode(AMTunerModeType Mode)
{
	if (m_pKSProp && (Mode & GetAvailableModes()))
	{
		KSPROPERTY_TUNER_MODE_S TunerMode;
		memset(&TunerMode,0,sizeof(KSPROPERTY_TUNER_MODE_S));
		TunerMode.Mode = Mode;
		HRESULT hr = m_pKSProp->Set(PROPSETID_TUNER,
			KSPROPERTY_TUNER_MODE,
			INSTANCE_DATA_OF_PROPERTY_PTR(&TunerMode),
			INSTANCE_DATA_OF_PROPERTY_SIZE(TunerMode),
			&TunerMode,
			sizeof(TunerMode));
		if (FAILED(hr))
			return FALSE;
		else
			return TRUE;
	}
	else
		return FALSE;
}

long CDxTuner::GetAvailableModes()
{
	KSPROPERTY_TUNER_CAPS_S TunerCaps;
	memset(&TunerCaps,0,sizeof(KSPROPERTY_TUNER_CAPS_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_CAPS,TunerCaps);
	return TunerCaps.ModesSupported;
}

long CDxTuner::GetFrequency()
{
	KSPROPERTY_TUNER_STATUS_S TunerStatus;
	memset(&TunerStatus,0,sizeof(KSPROPERTY_TUNER_STATUS_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_STATUS,TunerStatus);
	return TunerStatus.CurrentFrequency;
}

// Usual frequency steps: XXX'000'000 then XXX'250'000 then XXX'500'000 then XXX'750'000 ...
BOOL CDxTuner::SetFrequency(long Freq, AMTunerModeType Mode, AnalogVideoStandard Format)
{
	SetTunerMode(Mode);
	if ((GetAvailableTVFormats() | Format) && (Format != AnalogVideo_None))
		SetTVFormat(Format);
	
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	KSPROPERTY_TUNER_FREQUENCY_S Frequency;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	memset(&Frequency,0,sizeof(KSPROPERTY_TUNER_FREQUENCY_S));
	ModeCaps.Mode = Mode;
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	
	Frequency.Frequency = Freq;
	if (ModeCaps.Strategy == KS_TUNER_STRATEGY_DRIVER_TUNES)
		Frequency.TuningFlags = KS_TUNER_TUNING_FINE;
	else
		Frequency.TuningFlags = KS_TUNER_TUNING_EXACT;
	if (m_pKSProp && Freq >= (long)ModeCaps.MinFrequency && Freq <= (long)ModeCaps.MaxFrequency)
	{
		HRESULT hr = m_pKSProp->Set(PROPSETID_TUNER,
			KSPROPERTY_TUNER_FREQUENCY,
			INSTANCE_DATA_OF_PROPERTY_PTR(&Frequency),
			INSTANCE_DATA_OF_PROPERTY_SIZE(Frequency),
			&Frequency,
			sizeof(Frequency));
		if (FAILED(hr))
			return FALSE;
		else
			return TRUE;
	}
	else
		return FALSE;
}

void CDxTuner::GetMinMaxFrequency(long &min, long &max, AMTunerModeType Mode)
{
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	ModeCaps.Mode = Mode;
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	min=ModeCaps.MinFrequency;
	max=ModeCaps.MaxFrequency;
}

long CDxTuner::GetSignalStrength(CDxTuner::eSignalType& type)
{
	AMTunerModeType Mode = GetTunerMode();
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	KSPROPERTY_TUNER_STATUS_S TunerStatus;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	memset(&TunerStatus,0,sizeof(KSPROPERTY_TUNER_STATUS_S));
	ModeCaps.Mode = Mode;
	if (!GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps))
	{
		type = SIGNALTYPE_NONE;
		return 0;
	}

	if (ModeCaps.Strategy == KS_TUNER_STRATEGY_PLL)
	{
		type = SIGNALTYPE_PLL;
		GetKSData(m_pKSProp,KSPROPERTY_TUNER_STATUS,TunerStatus);
		return TunerStatus.PLLOffset;
	}
	else if (ModeCaps.Strategy == KS_TUNER_STRATEGY_SIGNAL_STRENGTH)
	{
		type = SIGNALTYPE_SIGNALSTRENGTH;
		GetKSData(m_pKSProp,KSPROPERTY_TUNER_STATUS,TunerStatus);
		return TunerStatus.SignalStrength;
	}
	else
	{
		type = SIGNALTYPE_NONE;
		return 0;
	}
}

long CDxTuner::GetInput()
{
	KSPROPERTY_TUNER_INPUT_S TunerInput;
	memset(&TunerInput,0,sizeof(KSPROPERTY_TUNER_INPUT_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_INPUT,TunerInput);
	return TunerInput.InputIndex;
}

BOOL CDxTuner::SetInput(long Input)
{
	if (m_pKSProp)
	{
		KSPROPERTY_TUNER_INPUT_S TunerInput;
		memset(&TunerInput,0,sizeof(KSPROPERTY_TUNER_INPUT_S));
		HRESULT hr = m_pKSProp->Set(PROPSETID_TUNER,
			KSPROPERTY_TUNER_INPUT,
			INSTANCE_DATA_OF_PROPERTY_PTR(&TunerInput),
			INSTANCE_DATA_OF_PROPERTY_SIZE(TunerInput),
			&TunerInput,
			sizeof(TunerInput));
		if (FAILED(hr))
			return FALSE;
		else
			return TRUE;
	}
	else
		return FALSE;
}

long CDxTuner::GetNumInputs()
{
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	ModeCaps.Mode = GetTunerMode();
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	return ModeCaps.NumberOfInputs;
}

AnalogVideoStandard CDxTuner::GetTVFormat()
{
	KSPROPERTY_TUNER_STANDARD_S Standard;
	memset(&Standard,0,sizeof(KSPROPERTY_TUNER_STANDARD_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_STANDARD,Standard);
	return (AnalogVideoStandard)Standard.Standard;
}

BOOL CDxTuner::SetTVFormat(AnalogVideoStandard Format)
{
	if (m_pKSProp)
	{
		KSPROPERTY_TUNER_STANDARD_S Standard;
		memset(&Standard,0,sizeof(KSPROPERTY_TUNER_STANDARD_S));
		Standard.Standard=Format;
		HRESULT hr = m_pKSProp->Set(PROPSETID_TUNER,
			KSPROPERTY_TUNER_STANDARD,
			INSTANCE_DATA_OF_PROPERTY_PTR(&Standard),
			INSTANCE_DATA_OF_PROPERTY_SIZE(Standard),
			&Standard,
			sizeof(Standard));
		if (FAILED(hr))
			return FALSE;
		else
			return TRUE;
	}
	else
		return FALSE;
}

long CDxTuner::GetAvailableTVFormats()
{
	AMTunerModeType Mode=GetTunerMode();
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	ModeCaps.Mode = Mode;
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	return ModeCaps.StandardsSupported;
}

#endif

