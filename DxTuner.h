// DxTuner.h: interface for the CDxTuner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DXTUNER_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_)
#define AFX_DXTUNER_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_

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

class CDxTuner
{
	public:
		// enum used by GetSignalStrength
		enum eSignalType
		{
			SIGNALTYPE_NONE,
			SIGNALTYPE_PLL,
			SIGNALTYPE_SIGNALSTRENGTH,
		};
		
		CDxTuner(CComPtr<IAMTVTuner> pTVTuner);
		virtual ~CDxTuner();
		
		// return current tuner mode
		AMTunerModeType GetTunerMode();

		// change tuner mode (tv, fm/am radio)
		BOOL SetTunerMode(AMTunerModeType Mode);

		// return a combination of AMTunerModeType
		long GetAvailableModes();
		
		// return current frequency in hz
		long GetFrequency();

		/**
		 * Sets tuner frequency.
		 * param Freq frequency
		 * param Mode Tuner mode to use (tv, fm/am radio)
		 * param Format Video format to use
		 */
		BOOL SetFrequency(long Freq, AMTunerModeType Mode, AnalogVideoStandard Format);

		/**
		 * Retrieves minumum and maximum allowed frequency for specified tuner mode.
		 * param min returns minimum allowed frequency
		 * param max returns maximum allowed frequency
		 * param Mode Tuner mode
		 */
		void GetMinMaxFrequency(long &min, long &max, AMTunerModeType Mode = AMTUNER_MODE_TV);
		
		/**
		 * Gets current signal strength.
		 * The type of signal depends on the tuning strategy that the device driver uses.
		 * If strategy is KS_TUNER_STRATEGY_DRIVER_TUNES then there is no way of 
		 * geting the signal status in that case IAMAnalogVideoDecoder::get_HorizontalLocked
		 * could be used instead.
		 *
		 * param type of signal
		 * return current signal, meaning of this value depends on the type parameter
		 */
		long GetSignalStrength(eSignalType& type);
		
		// return currently selected tuner input
		long GetInput();

		// Input new tuner input to use
		BOOL SetInput(long Input);

		// return number of tuner inputs
		long GetNumInputs();
		
		// return current tvformat
		AnalogVideoStandard GetTVFormat();

		// format new tvformat
		BOOL SetTVFormat(AnalogVideoStandard Format);

		// return a combination of AnalogVideoStandard
		long GetAvailableTVFormats();

	private:
		CComPtr<IKsPropertySet> m_pKSProp;
};

#endif

#endif // !defined(AFX_DXTUNER_H__B0EFD53A_DFDA_42C6_81E4_98E817229EC3__INCLUDED_)
