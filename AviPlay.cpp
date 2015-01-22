#include "stdafx.h"
#include "AviPlay.h"
#include "YuvToYuv.h"
#include "Round.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

// Ffmpeg libs
#pragma comment(lib, "ffmpeg\\libavcodec\\libavcodec.a")
#pragma comment(lib, "ffmpeg\\libavformat\\libavformat.a")
#pragma comment(lib, "ffmpeg\\libavutil\\libavutil.a")
#pragma comment(lib, "ffmpeg\\libswscale\\libswscale.a")
#pragma comment(lib, "ffmpeg\\libswresample\\libswresample.a")
// In 2014 I was happy to be able to build ffmpeg 2.2.1 with vs2010,
// but the result was extremelly slow: video codecs slower by a
// factor of 1.5 - 2 and swscale by a factor of 3 compared to mingw
#ifdef FFMPEG_TOOLCHAIN_MSVC
#pragma comment(lib, "ffmpeg\\msvc\\mp3lame.lib")
#else
// libcmt.lib(_pow_.obj) : error LNK2005: _pow already defined in libmingwex.a(pow.o)
// -> to correctly link we have to remove pow.o from libmingwex.a,
// perform the following in visual studio command prompt:
// 1. cd uimager\ffmpeg\lib
// 2. lib -remove:pow.o libmingwex.a
// 3. rename libmingwex.lib libmingwex.a
#pragma comment(lib, "ffmpeg\\mingw\\libgcc.a")
#pragma comment(lib, "ffmpeg\\mingw\\libmingwex.a")
#pragma comment(lib, "ffmpeg\\mingw\\libmp3lame.a")
#pragma comment(lib, "ffmpeg\\mingw\\libx264.a")
#endif

// Win32 libs
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "msacm32.lib")

static const TCHAR* WaveFormatTagTable[0x10000];
static const TCHAR WaveFormatTag_0x0000[] = _T("Microsoft Unknown Wave Format");
static const TCHAR WaveFormatTag_0x0001[] = _T("Uncompressed PCM");
static const TCHAR WaveFormatTag_0x0002[] = _T("Microsoft ADPCM");
static const TCHAR WaveFormatTag_0x0003[] = _T("IEEE Float");
static const TCHAR WaveFormatTag_0x0004[] = _T("Compaq Computer VSELP");
static const TCHAR WaveFormatTag_0x0005[] = _T("IBM CVSD");
static const TCHAR WaveFormatTag_0x0006[] = _T("Microsoft A-Law");
static const TCHAR WaveFormatTag_0x0007[] = _T("Microsoft mu-Law");
static const TCHAR WaveFormatTag_0x0008[] = _T("Microsoft DTS");
static const TCHAR WaveFormatTag_0x0010[] = _T("OKI ADPCM");
static const TCHAR WaveFormatTag_0x0011[] = _T("Intel DVI/IMA ADPCM");
static const TCHAR WaveFormatTag_0x0012[] = _T("Videologic MediaSpace ADPCM");
static const TCHAR WaveFormatTag_0x0013[] = _T("Sierra Semiconductor ADPCM");
static const TCHAR WaveFormatTag_0x0014[] = _T("Antex Electronics G.723 ADPCM");
static const TCHAR WaveFormatTag_0x0015[] = _T("DSP Solutions DigiSTD");
static const TCHAR WaveFormatTag_0x0016[] = _T("DSP Solutions DigiFIX");
static const TCHAR WaveFormatTag_0x0017[] = _T("Dialogic OKI ADPCM");
static const TCHAR WaveFormatTag_0x0018[] = _T("MediaVision ADPCM");
static const TCHAR WaveFormatTag_0x0019[] = _T("Hewlett-Packard CU");
static const TCHAR WaveFormatTag_0x0020[] = _T("Yamaha ADPCM");
static const TCHAR WaveFormatTag_0x0021[] = _T("Speech Compression Sonarc");
static const TCHAR WaveFormatTag_0x0022[] = _T("DSP Group TrueSpeech");
static const TCHAR WaveFormatTag_0x0023[] = _T("Echo Speech EchoSC1");
static const TCHAR WaveFormatTag_0x0024[] = _T("Audiofile AF36");
static const TCHAR WaveFormatTag_0x0025[] = _T("Audio Processing Technology APTX");
static const TCHAR WaveFormatTag_0x0026[] = _T("AudioFile AF10");
static const TCHAR WaveFormatTag_0x0027[] = _T("Prosody 1612");
static const TCHAR WaveFormatTag_0x0028[] = _T("LRC");
static const TCHAR WaveFormatTag_0x0030[] = _T("Dolby AC2");
static const TCHAR WaveFormatTag_0x0031[] = _T("Microsoft GSM 6.10");
static const TCHAR WaveFormatTag_0x0032[] = _T("MSNAudio");
static const TCHAR WaveFormatTag_0x0033[] = _T("Antex Electronics ADPCME");
static const TCHAR WaveFormatTag_0x0034[] = _T("Control Resources VQLPC");
static const TCHAR WaveFormatTag_0x0035[] = _T("DSP Solutions DigiREAL");
static const TCHAR WaveFormatTag_0x0036[] = _T("DSP Solutions DigiADPCM");
static const TCHAR WaveFormatTag_0x0037[] = _T("Control Resources CR10");
static const TCHAR WaveFormatTag_0x0038[] = _T("Natural MicroSystems VBXADPCM");
static const TCHAR WaveFormatTag_0x0039[] = _T("Crystal Semiconductor IMA ADPCM");
static const TCHAR WaveFormatTag_0x003A[] = _T("EchoSC3");
static const TCHAR WaveFormatTag_0x003B[] = _T("Rockwell ADPCM");
static const TCHAR WaveFormatTag_0x003C[] = _T("Rockwell Digit LK");
static const TCHAR WaveFormatTag_0x003D[] = _T("Xebec");
static const TCHAR WaveFormatTag_0x0040[] = _T("Antex Electronics G.721 ADPCM");
static const TCHAR WaveFormatTag_0x0041[] = _T("G.728 CELP");
static const TCHAR WaveFormatTag_0x0042[] = _T("MSG723");
static const TCHAR WaveFormatTag_0x0050[] = _T("MPEG Layer-2 or Layer-1");
static const TCHAR WaveFormatTag_0x0052[] = _T("RT24");
static const TCHAR WaveFormatTag_0x0053[] = _T("PAC");
static const TCHAR WaveFormatTag_0x0055[] = _T("MPEG Layer-3");
static const TCHAR WaveFormatTag_0x0059[] = _T("Lucent G.723");
static const TCHAR WaveFormatTag_0x0060[] = _T("Cirrus");
static const TCHAR WaveFormatTag_0x0061[] = _T("ESPCM");
static const TCHAR WaveFormatTag_0x0062[] = _T("Voxware");
static const TCHAR WaveFormatTag_0x0063[] = _T("Canopus Atrac");
static const TCHAR WaveFormatTag_0x0064[] = _T("G.726 ADPCM");
static const TCHAR WaveFormatTag_0x0065[] = _T("G.722 ADPCM");
static const TCHAR WaveFormatTag_0x0066[] = _T("DSAT");
static const TCHAR WaveFormatTag_0x0067[] = _T("DSAT Display");
static const TCHAR WaveFormatTag_0x0069[] = _T("Voxware Byte Aligned");
static const TCHAR WaveFormatTag_0x0070[] = _T("Voxware AC8");
static const TCHAR WaveFormatTag_0x0071[] = _T("Voxware AC10");
static const TCHAR WaveFormatTag_0x0072[] = _T("Voxware AC16");
static const TCHAR WaveFormatTag_0x0073[] = _T("Voxware AC20");
static const TCHAR WaveFormatTag_0x0074[] = _T("Voxware MetaVoice");
static const TCHAR WaveFormatTag_0x0075[] = _T("Voxware MetaSound");
static const TCHAR WaveFormatTag_0x0076[] = _T("Voxware RT29HW");
static const TCHAR WaveFormatTag_0x0077[] = _T("Voxware VR12");
static const TCHAR WaveFormatTag_0x0078[] = _T("Voxware VR18");
static const TCHAR WaveFormatTag_0x0079[] = _T("Voxware TQ40");
static const TCHAR WaveFormatTag_0x0080[] = _T("Softsound");
static const TCHAR WaveFormatTag_0x0081[] = _T("Voxware TQ60");
static const TCHAR WaveFormatTag_0x0082[] = _T("MSRT24");
static const TCHAR WaveFormatTag_0x0083[] = _T("G.729A");
static const TCHAR WaveFormatTag_0x0084[] = _T("MVI MV12");
static const TCHAR WaveFormatTag_0x0085[] = _T("DF G.726");
static const TCHAR WaveFormatTag_0x0086[] = _T("DF GSM610");
static const TCHAR WaveFormatTag_0x0088[] = _T("ISIAudio");
static const TCHAR WaveFormatTag_0x0089[] = _T("Onlive");
static const TCHAR WaveFormatTag_0x0091[] = _T("SBC24");
static const TCHAR WaveFormatTag_0x0092[] = _T("Dolby AC3 SPDIF");
static const TCHAR WaveFormatTag_0x0093[] = _T("MediaSonic G.723");
static const TCHAR WaveFormatTag_0x0094[] = _T("Aculab PLC Prosody 8kbps");
static const TCHAR WaveFormatTag_0x0097[] = _T("ZyXEL ADPCM");
static const TCHAR WaveFormatTag_0x0098[] = _T("Philips LPCBB");
static const TCHAR WaveFormatTag_0x0099[] = _T("Packed");
static const TCHAR WaveFormatTag_0x0100[] = _T("Rhetorex ADPCM");
static const TCHAR WaveFormatTag_0x0101[] = _T("IBM mu-law");
static const TCHAR WaveFormatTag_0x0102[] = _T("IBM A-law");
static const TCHAR WaveFormatTag_0x0103[] = _T("IBM AVC Adaptive Differential Pulse Code Modulation (ADPCM)");
static const TCHAR WaveFormatTag_0x0111[] = _T("Vivo G.723");
static const TCHAR WaveFormatTag_0x0112[] = _T("Vivo Siren");
static const TCHAR WaveFormatTag_0x0123[] = _T("Digital G.723");
static const TCHAR WaveFormatTag_0x0125[] = _T("Sanyo LD ADPCM");
static const TCHAR WaveFormatTag_0x0130[] = _T("Sipro Lab Telecom ACELP NET");
static const TCHAR WaveFormatTag_0x0131[] = _T("Sipro Lab Telecom ACELP 4800");
static const TCHAR WaveFormatTag_0x0132[] = _T("Sipro Lab Telecom ACELP 8V3");
static const TCHAR WaveFormatTag_0x0133[] = _T("Sipro Lab Telecom G.729");
static const TCHAR WaveFormatTag_0x0134[] = _T("Sipro Lab Telecom G.729A");
static const TCHAR WaveFormatTag_0x0135[] = _T("Sipro Lab Telecom Kelvin");
static const TCHAR WaveFormatTag_0x0140[] = _T("Windows Media Video V8");
static const TCHAR WaveFormatTag_0x0150[] = _T("Qualcomm PureVoice");
static const TCHAR WaveFormatTag_0x0151[] = _T("Qualcomm HalfRate");
static const TCHAR WaveFormatTag_0x0155[] = _T("Ring Zero Systems TUB GSM");
static const TCHAR WaveFormatTag_0x0160[] = _T("Microsoft Audio 1");
static const TCHAR WaveFormatTag_0x0161[] = _T("Windows Media Audio V7 / V8 / V9");
static const TCHAR WaveFormatTag_0x0162[] = _T("Windows Media Audio Professional V9");
static const TCHAR WaveFormatTag_0x0163[] = _T("Windows Media Audio Lossless V9");
static const TCHAR WaveFormatTag_0x0200[] = _T("Creative Labs ADPCM");
static const TCHAR WaveFormatTag_0x0202[] = _T("Creative Labs Fastspeech8");
static const TCHAR WaveFormatTag_0x0203[] = _T("Creative Labs Fastspeech10");
static const TCHAR WaveFormatTag_0x0210[] = _T("UHER Informatic GmbH ADPCM");
static const TCHAR WaveFormatTag_0x0220[] = _T("Quarterdeck");
static const TCHAR WaveFormatTag_0x0230[] = _T("I-link Worldwide VC");
static const TCHAR WaveFormatTag_0x0240[] = _T("Aureal RAW Sport");
static const TCHAR WaveFormatTag_0x0250[] = _T("Interactive Products HSX");
static const TCHAR WaveFormatTag_0x0251[] = _T("Interactive Products RPELP");
static const TCHAR WaveFormatTag_0x0260[] = _T("Consistent Software CS2");
static const TCHAR WaveFormatTag_0x0270[] = _T("Sony SCX");
static const TCHAR WaveFormatTag_0x0300[] = _T("Fujitsu FM Towns Snd");
static const TCHAR WaveFormatTag_0x0400[] = _T("BTV Digital");
static const TCHAR WaveFormatTag_0x0401[] = _T("Intel Music Coder");
static const TCHAR WaveFormatTag_0x0402[] = _T("Indeo Audio");
static const TCHAR WaveFormatTag_0x0450[] = _T("QDesign Music");
static const TCHAR WaveFormatTag_0x0680[] = _T("VME VMPCM");
static const TCHAR WaveFormatTag_0x0681[] = _T("AT&T Labs TPC");
static const TCHAR WaveFormatTag_0x08AE[] = _T("ClearJump LiteWave");
static const TCHAR WaveFormatTag_0x1000[] = _T("Olivetti GSM");
static const TCHAR WaveFormatTag_0x1001[] = _T("Olivetti ADPCM");
static const TCHAR WaveFormatTag_0x1002[] = _T("Olivetti CELP");
static const TCHAR WaveFormatTag_0x1003[] = _T("Olivetti SBC");
static const TCHAR WaveFormatTag_0x1004[] = _T("Olivetti OPR");
static const TCHAR WaveFormatTag_0x1100[] = _T("Lernout & Hauspie Codec");
static const TCHAR WaveFormatTag_0x1101[] = _T("Lernout & Hauspie CELP Codec");
static const TCHAR WaveFormatTag_0x1102[] = _T("Lernout & Hauspie SBC Codec");
static const TCHAR WaveFormatTag_0x1103[] = _T("Lernout & Hauspie SBC Codec");
static const TCHAR WaveFormatTag_0x1104[] = _T("Lernout & Hauspie SBC Codec");
static const TCHAR WaveFormatTag_0x1400[] = _T("Norris");
static const TCHAR WaveFormatTag_0x1401[] = _T("AT&T ISIAudio");
static const TCHAR WaveFormatTag_0x1500[] = _T("Soundspace Music Compression");
static const TCHAR WaveFormatTag_0x181C[] = _T("VoxWare RT24 Speech");
static const TCHAR WaveFormatTag_0x1FC4[] = _T("NCT Soft ALF2CD (www.nctsoft.com)");
static const TCHAR WaveFormatTag_0x2000[] = _T("Dolby AC3");
static const TCHAR WaveFormatTag_0x2001[] = _T("Dolby DTS");
static const TCHAR WaveFormatTag_0x2002[] = _T("WAVE_FORMAT_14_4");
static const TCHAR WaveFormatTag_0x2003[] = _T("WAVE_FORMAT_28_8");
static const TCHAR WaveFormatTag_0x2004[] = _T("WAVE_FORMAT_COOK");
static const TCHAR WaveFormatTag_0x2005[] = _T("WAVE_FORMAT_DNET");
static const TCHAR WaveFormatTag_0x566F[] = _T("Vorbis");
static const TCHAR WaveFormatTag_0x674F[] = _T("Ogg Vorbis 1");
static const TCHAR WaveFormatTag_0x6750[] = _T("Ogg Vorbis 2");
static const TCHAR WaveFormatTag_0x6751[] = _T("Ogg Vorbis 3");
static const TCHAR WaveFormatTag_0x676F[] = _T("Ogg Vorbis 1+");
static const TCHAR WaveFormatTag_0x6770[] = _T("Ogg Vorbis 2+");
static const TCHAR WaveFormatTag_0x6771[] = _T("Ogg Vorbis 3+");
static const TCHAR WaveFormatTag_0x77A1[] = _T("TTA");
static const TCHAR WaveFormatTag_0x7A21[] = _T("GSM-AMR (CBR, no SID)");
static const TCHAR WaveFormatTag_0x7A22[] = _T("GSM-AMR (VBR, including SID)");
static const TCHAR WaveFormatTag_0x00FF[] = _T("AAC");
static const TCHAR WaveFormatTag_0x706D[] = _T("AAC");
static const TCHAR WaveFormatTag_0xAAC0[] = _T("AAC");
static const TCHAR WaveFormatTag_0xF1AC[] = _T("Flac");
static const TCHAR WaveFormatTag_0xFFFE[] = _T("WAVE_FORMAT_EXTENSIBLE");
static const TCHAR WaveFormatTag_0xFFFF[] = _T("WAVE_FORMAT_DEVELOPMENT");
void CAVIPlay::InitWaveFormatTagTable()
{
	for (int i = 0 ; i < 0x10000 ; i++)
		WaveFormatTagTable[i] = _T("");
	WaveFormatTagTable[0x0000] = WaveFormatTag_0x0000;
	WaveFormatTagTable[0x0001] = WaveFormatTag_0x0001;
	WaveFormatTagTable[0x0002] = WaveFormatTag_0x0002;
	WaveFormatTagTable[0x0003] = WaveFormatTag_0x0003;
	WaveFormatTagTable[0x0004] = WaveFormatTag_0x0004;
	WaveFormatTagTable[0x0005] = WaveFormatTag_0x0005;
	WaveFormatTagTable[0x0006] = WaveFormatTag_0x0006;
	WaveFormatTagTable[0x0007] = WaveFormatTag_0x0007;
	WaveFormatTagTable[0x0008] = WaveFormatTag_0x0008;
	WaveFormatTagTable[0x0010] = WaveFormatTag_0x0010;
	WaveFormatTagTable[0x0011] = WaveFormatTag_0x0011;
	WaveFormatTagTable[0x0012] = WaveFormatTag_0x0012;
	WaveFormatTagTable[0x0013] = WaveFormatTag_0x0013;
	WaveFormatTagTable[0x0014] = WaveFormatTag_0x0014;
	WaveFormatTagTable[0x0015] = WaveFormatTag_0x0015;
	WaveFormatTagTable[0x0016] = WaveFormatTag_0x0016;
	WaveFormatTagTable[0x0017] = WaveFormatTag_0x0017;
	WaveFormatTagTable[0x0018] = WaveFormatTag_0x0018;
	WaveFormatTagTable[0x0019] = WaveFormatTag_0x0019;
	WaveFormatTagTable[0x0020] = WaveFormatTag_0x0020;
	WaveFormatTagTable[0x0021] = WaveFormatTag_0x0021;
	WaveFormatTagTable[0x0022] = WaveFormatTag_0x0022;
	WaveFormatTagTable[0x0023] = WaveFormatTag_0x0023;
	WaveFormatTagTable[0x0024] = WaveFormatTag_0x0024;
	WaveFormatTagTable[0x0025] = WaveFormatTag_0x0025;
	WaveFormatTagTable[0x0026] = WaveFormatTag_0x0026;
	WaveFormatTagTable[0x0027] = WaveFormatTag_0x0027;
	WaveFormatTagTable[0x0028] = WaveFormatTag_0x0028;
	WaveFormatTagTable[0x0030] = WaveFormatTag_0x0030;
	WaveFormatTagTable[0x0031] = WaveFormatTag_0x0031;
	WaveFormatTagTable[0x0032] = WaveFormatTag_0x0032;
	WaveFormatTagTable[0x0033] = WaveFormatTag_0x0033;
	WaveFormatTagTable[0x0034] = WaveFormatTag_0x0034;
	WaveFormatTagTable[0x0035] = WaveFormatTag_0x0035;
	WaveFormatTagTable[0x0036] = WaveFormatTag_0x0036;
	WaveFormatTagTable[0x0037] = WaveFormatTag_0x0037;
	WaveFormatTagTable[0x0038] = WaveFormatTag_0x0038;
	WaveFormatTagTable[0x0039] = WaveFormatTag_0x0039;
	WaveFormatTagTable[0x003A] = WaveFormatTag_0x003A;
	WaveFormatTagTable[0x003B] = WaveFormatTag_0x003B;
	WaveFormatTagTable[0x003C] = WaveFormatTag_0x003C;
	WaveFormatTagTable[0x003D] = WaveFormatTag_0x003D;
	WaveFormatTagTable[0x0040] = WaveFormatTag_0x0040;
	WaveFormatTagTable[0x0041] = WaveFormatTag_0x0041;
	WaveFormatTagTable[0x0042] = WaveFormatTag_0x0042;
	WaveFormatTagTable[0x0050] = WaveFormatTag_0x0050;
	WaveFormatTagTable[0x0052] = WaveFormatTag_0x0052;
	WaveFormatTagTable[0x0053] = WaveFormatTag_0x0053;
	WaveFormatTagTable[0x0055] = WaveFormatTag_0x0055;
	WaveFormatTagTable[0x0059] = WaveFormatTag_0x0059;
	WaveFormatTagTable[0x0060] = WaveFormatTag_0x0060;
	WaveFormatTagTable[0x0061] = WaveFormatTag_0x0061;
	WaveFormatTagTable[0x0062] = WaveFormatTag_0x0062;
	WaveFormatTagTable[0x0063] = WaveFormatTag_0x0063;
	WaveFormatTagTable[0x0064] = WaveFormatTag_0x0064;
	WaveFormatTagTable[0x0065] = WaveFormatTag_0x0065;
	WaveFormatTagTable[0x0066] = WaveFormatTag_0x0066;
	WaveFormatTagTable[0x0067] = WaveFormatTag_0x0067;
	WaveFormatTagTable[0x0069] = WaveFormatTag_0x0069;
	WaveFormatTagTable[0x0070] = WaveFormatTag_0x0070;
	WaveFormatTagTable[0x0071] = WaveFormatTag_0x0071;
	WaveFormatTagTable[0x0072] = WaveFormatTag_0x0072;
	WaveFormatTagTable[0x0073] = WaveFormatTag_0x0073;
	WaveFormatTagTable[0x0074] = WaveFormatTag_0x0074;
	WaveFormatTagTable[0x0075] = WaveFormatTag_0x0075;
	WaveFormatTagTable[0x0076] = WaveFormatTag_0x0076;
	WaveFormatTagTable[0x0077] = WaveFormatTag_0x0077;
	WaveFormatTagTable[0x0078] = WaveFormatTag_0x0078;
	WaveFormatTagTable[0x0079] = WaveFormatTag_0x0079;
	WaveFormatTagTable[0x0080] = WaveFormatTag_0x0080;
	WaveFormatTagTable[0x0081] = WaveFormatTag_0x0081;
	WaveFormatTagTable[0x0082] = WaveFormatTag_0x0082;
	WaveFormatTagTable[0x0083] = WaveFormatTag_0x0083;
	WaveFormatTagTable[0x0084] = WaveFormatTag_0x0084;
	WaveFormatTagTable[0x0085] = WaveFormatTag_0x0085;
	WaveFormatTagTable[0x0086] = WaveFormatTag_0x0086;
	WaveFormatTagTable[0x0088] = WaveFormatTag_0x0088;
	WaveFormatTagTable[0x0089] = WaveFormatTag_0x0089;
	WaveFormatTagTable[0x0091] = WaveFormatTag_0x0091;
	WaveFormatTagTable[0x0092] = WaveFormatTag_0x0092;
	WaveFormatTagTable[0x0093] = WaveFormatTag_0x0093;
	WaveFormatTagTable[0x0094] = WaveFormatTag_0x0094;
	WaveFormatTagTable[0x0097] = WaveFormatTag_0x0097;
	WaveFormatTagTable[0x0098] = WaveFormatTag_0x0098;
	WaveFormatTagTable[0x0099] = WaveFormatTag_0x0099;
	WaveFormatTagTable[0x00FF] = WaveFormatTag_0x00FF;
	WaveFormatTagTable[0x0100] = WaveFormatTag_0x0100;
	WaveFormatTagTable[0x0101] = WaveFormatTag_0x0101;
	WaveFormatTagTable[0x0102] = WaveFormatTag_0x0102;
	WaveFormatTagTable[0x0103] = WaveFormatTag_0x0103;
	WaveFormatTagTable[0x0111] = WaveFormatTag_0x0111;
	WaveFormatTagTable[0x0112] = WaveFormatTag_0x0112;
	WaveFormatTagTable[0x0123] = WaveFormatTag_0x0123;
	WaveFormatTagTable[0x0125] = WaveFormatTag_0x0125;
	WaveFormatTagTable[0x0130] = WaveFormatTag_0x0130;
	WaveFormatTagTable[0x0131] = WaveFormatTag_0x0131;
	WaveFormatTagTable[0x0132] = WaveFormatTag_0x0132;
	WaveFormatTagTable[0x0133] = WaveFormatTag_0x0133;
	WaveFormatTagTable[0x0134] = WaveFormatTag_0x0134;
	WaveFormatTagTable[0x0135] = WaveFormatTag_0x0135;
	WaveFormatTagTable[0x0140] = WaveFormatTag_0x0140;
	WaveFormatTagTable[0x0150] = WaveFormatTag_0x0150;
	WaveFormatTagTable[0x0151] = WaveFormatTag_0x0151;
	WaveFormatTagTable[0x0155] = WaveFormatTag_0x0155;
	WaveFormatTagTable[0x0160] = WaveFormatTag_0x0160;
	WaveFormatTagTable[0x0161] = WaveFormatTag_0x0161;
	WaveFormatTagTable[0x0162] = WaveFormatTag_0x0162;
	WaveFormatTagTable[0x0163] = WaveFormatTag_0x0163;
	WaveFormatTagTable[0x0200] = WaveFormatTag_0x0200;
	WaveFormatTagTable[0x0202] = WaveFormatTag_0x0202;
	WaveFormatTagTable[0x0203] = WaveFormatTag_0x0203;
	WaveFormatTagTable[0x0210] = WaveFormatTag_0x0210;
	WaveFormatTagTable[0x0220] = WaveFormatTag_0x0220;
	WaveFormatTagTable[0x0230] = WaveFormatTag_0x0230;
	WaveFormatTagTable[0x0240] = WaveFormatTag_0x0240;
	WaveFormatTagTable[0x0250] = WaveFormatTag_0x0250;
	WaveFormatTagTable[0x0251] = WaveFormatTag_0x0251;
	WaveFormatTagTable[0x0260] = WaveFormatTag_0x0260;
	WaveFormatTagTable[0x0270] = WaveFormatTag_0x0270;
	WaveFormatTagTable[0x0300] = WaveFormatTag_0x0300;
	WaveFormatTagTable[0x0400] = WaveFormatTag_0x0400;
	WaveFormatTagTable[0x0401] = WaveFormatTag_0x0401;
	WaveFormatTagTable[0x0402] = WaveFormatTag_0x0402;
	WaveFormatTagTable[0x0450] = WaveFormatTag_0x0450;
	WaveFormatTagTable[0x0680] = WaveFormatTag_0x0680;
	WaveFormatTagTable[0x0681] = WaveFormatTag_0x0681;
	WaveFormatTagTable[0x08AE] = WaveFormatTag_0x08AE;
	WaveFormatTagTable[0x1000] = WaveFormatTag_0x1000;
	WaveFormatTagTable[0x1001] = WaveFormatTag_0x1001;
	WaveFormatTagTable[0x1002] = WaveFormatTag_0x1002;
	WaveFormatTagTable[0x1003] = WaveFormatTag_0x1003;
	WaveFormatTagTable[0x1004] = WaveFormatTag_0x1004;
	WaveFormatTagTable[0x1100] = WaveFormatTag_0x1100;
	WaveFormatTagTable[0x1101] = WaveFormatTag_0x1101;
	WaveFormatTagTable[0x1102] = WaveFormatTag_0x1102;
	WaveFormatTagTable[0x1103] = WaveFormatTag_0x1103;
	WaveFormatTagTable[0x1104] = WaveFormatTag_0x1104;
	WaveFormatTagTable[0x1400] = WaveFormatTag_0x1400;
	WaveFormatTagTable[0x1401] = WaveFormatTag_0x1401;
	WaveFormatTagTable[0x1500] = WaveFormatTag_0x1500;
	WaveFormatTagTable[0x181C] = WaveFormatTag_0x181C;
	WaveFormatTagTable[0x1FC4] = WaveFormatTag_0x1FC4;
	WaveFormatTagTable[0x2000] = WaveFormatTag_0x2000;
	WaveFormatTagTable[0x2001] = WaveFormatTag_0x2001;
	WaveFormatTagTable[0x2002] = WaveFormatTag_0x2002;
	WaveFormatTagTable[0x2003] = WaveFormatTag_0x2003;
	WaveFormatTagTable[0x2004] = WaveFormatTag_0x2004;
	WaveFormatTagTable[0x2005] = WaveFormatTag_0x2005;
	WaveFormatTagTable[0x566F] = WaveFormatTag_0x566F;
	WaveFormatTagTable[0x674F] = WaveFormatTag_0x674F;
	WaveFormatTagTable[0x6750] = WaveFormatTag_0x6750;
	WaveFormatTagTable[0x6751] = WaveFormatTag_0x6751;
	WaveFormatTagTable[0x676F] = WaveFormatTag_0x676F;
	WaveFormatTagTable[0x6770] = WaveFormatTag_0x6770;
	WaveFormatTagTable[0x6771] = WaveFormatTag_0x6771;
	WaveFormatTagTable[0x706D] = WaveFormatTag_0x706D;
	WaveFormatTagTable[0x77A1] = WaveFormatTag_0x77A1;
	WaveFormatTagTable[0x7A21] = WaveFormatTag_0x7A21;
	WaveFormatTagTable[0x7A22] = WaveFormatTag_0x7A22;
	WaveFormatTagTable[0xAAC0] = WaveFormatTag_0xAAC0;
	WaveFormatTagTable[0xF1AC] = WaveFormatTag_0xF1AC;
	WaveFormatTagTable[0xFFFE] = WaveFormatTag_0xFFFE;
	WaveFormatTagTable[0xFFFF] = WaveFormatTag_0xFFFF;
}

CString CAVIPlay::GetWaveFormatTagString(WORD wFormatTag)
{
	return WaveFormatTagTable[wFormatTag];
}

void CAVIPlay::CAVIStream::Free()
{
	if (m_pSrcFormat)
	{
		delete [] m_pSrcFormat;
		m_pSrcFormat = NULL;
	}
	if (m_pSuperIndexTable)
	{
		delete [] m_pSuperIndexTable;
		m_pSuperIndexTable = NULL;
	}
	if (m_pStdIndexHdrs)
	{
		delete [] m_pStdIndexHdrs;
		m_pStdIndexHdrs = NULL;
	}
	if (m_ppStdIndexTables)
	{
		for (int i = 0 ; i < (int)m_SuperIndexHdr.nEntriesInUse ; i++)
		{
			if (m_ppStdIndexTables[i])
				delete m_ppStdIndexTables[i];
		}
		delete [] m_ppStdIndexTables;
		m_ppStdIndexTables = NULL;
	}
	if (m_pOldIndexTable)
	{
		delete [] m_pOldIndexTable;
		m_pOldIndexTable = NULL;
	}
	m_pFile = NULL;
	memset(&m_Hdr, 0, sizeof(AVISTREAMHDR));
	m_dwSrcFormatSize = 0;
	memset(&m_SuperIndexHdr, 0, sizeof(AVISUPERINDEXHDR));
	m_llOldIndexBase = 0;
	m_dwOldIndexEntries = 0;
	m_dwChunksCount = 0;
	m_llBytesCount = 0;
	m_dwMinChunkSize = 0xFFFFFFFF;
	m_dwMaxChunkSize = 0;
}

void CAVIPlay::CAVIStream::CalcMinChunkSize()
{
	m_dwMinChunkSize = 0xFFFFFFFF;
	for (DWORD dwChunkNum = 0 ; dwChunkNum < m_dwChunksCount ; dwChunkNum++)
	{
		DWORD dwSize = 0;
		if (!GetChunkData(dwChunkNum, NULL, &dwSize))
			return;
		if (dwSize < m_dwMinChunkSize)
			m_dwMinChunkSize = dwSize;
	}
}

void CAVIPlay::CAVIStream::CalcMaxChunkSize()
{
	m_dwMaxChunkSize = 0;
	for (DWORD dwChunkNum = 0 ; dwChunkNum < m_dwChunksCount ; dwChunkNum++)
	{
		DWORD dwSize = 0;
		if (!GetChunkData(dwChunkNum, NULL, &dwSize))
			return;
		if (dwSize > m_dwMaxChunkSize)
			m_dwMaxChunkSize = dwSize;
	}
}

bool CAVIPlay::CAVIStream::GetChunkData(DWORD dwChunkNum, LPBYTE pData, DWORD* pSize)
{
	try
	{
		// Enter CS
		::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

		// Check
		if (!m_pFile || !pSize)
		{
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}

		// Check
		if (dwChunkNum >= m_dwChunksCount)
		{
			*pSize = 0;
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}

		// Get Max Buf Size
		DWORD dwBufSize = *pSize;
		if (dwBufSize == 0)
			dwBufSize = 0xFFFFFFFF;

		if (m_pOldIndexTable)
		{
			// Note:
			// m_llOldIndexBase + dwOffset:	is the offset to the chunk header
			// dwSize:						is the size of the chunk data (no header!)

			// Position
			LONGLONG llPos = m_llOldIndexBase + m_pOldIndexTable[dwChunkNum].dwOffset + sizeof(RIFFCHUNK);
			m_pFile->Seek((LONGLONG)llPos, CFile::begin);
			*pSize = MIN(m_pOldIndexTable[dwChunkNum].dwSize, dwBufSize);
			if (pData)
			{
				if (*pSize == m_pFile->Read(pData, *pSize))
				{
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return true;
				}
				else
				{
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
			}
			else
			{
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return true;
			}
		}
		else if (m_pStdIndexHdrs && m_ppStdIndexTables)
		{
			// Find the Right Index
			DWORD dwEntries = 0;
			int index = 0;
			for (index = 0 ; index < (int)m_SuperIndexHdr.nEntriesInUse ; index++)
			{
				dwEntries += (DWORD)(m_pStdIndexHdrs[index].nEntriesInUse);
				if (dwChunkNum < dwEntries)
					break;
			}

			// Find the Right Position
			for (int i = 0 ; i < index ; i++)
				dwChunkNum -= (int)(m_pStdIndexHdrs[i].nEntriesInUse);
			
			// Note:
			// qwBaseOffset + dwOffset:		is the offset to the chunk data
			// dwSize:						is the size of the chunk data (no header!)

			// Position
			LONGLONG llPos = (LONGLONG)m_pStdIndexHdrs[index].qwBaseOffset + (LONGLONG)m_ppStdIndexTables[index][dwChunkNum].dwOffset;
			m_pFile->Seek((LONGLONG)llPos, CFile::begin);
			*pSize = MIN((m_ppStdIndexTables[index][dwChunkNum].dwSize & 0x7FFFFFFF), dwBufSize);
			if (pData)
			{
				if (*pSize == m_pFile->Read(pData, *pSize))
				{
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return true;
				}
				else
				{
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
			}
			else
			{
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return true;
			}
		}
		else
		{
			int nType = GetStreamType();
			RIFFCHUNK chunk;
			if (dwChunkNum >= m_dwLastReadMoviChunkNum)
			{
				chunk = m_LastReadMoviChunk;
				m_pFile->Seek(m_llLastReadMoviChunkOffset, CFile::begin);
				DWORD dwPos = dwChunkNum - m_dwLastReadMoviChunkNum;
				while (dwPos > 0)
				{
					if (!CAVIPlay::SeekToNextChunk(m_pFile, chunk))
					{
						*pSize = 0;
						::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
						return false;
					}
					else if (nType == CAVIPlay::GetChunkType(chunk.fcc))
						dwPos--;
				}
			}
			else
			{
				chunk = m_FirstMoviChunk;
				m_pFile->Seek(m_llFirstMoviChunkOffset, CFile::begin);
				DWORD dwPos = dwChunkNum;
				while (dwPos > 0)
				{
					if (!CAVIPlay::SeekToNextChunk(m_pFile, chunk))
					{
						*pSize = 0;
						::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
						return false;
					}
					else if (nType == CAVIPlay::GetChunkType(chunk.fcc))
						dwPos--;
				}
			}
			m_LastReadMoviChunk = chunk;
			m_dwLastReadMoviChunkNum = dwChunkNum;
			m_llLastReadMoviChunkOffset = m_pFile->GetPosition();
			*pSize = MIN(chunk.cb, dwBufSize);
			if (pData)
			{
				if (*pSize == m_pFile->Read(pData, *pSize))
				{
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return true;
				}
				else
				{
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
			}
			else
			{
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return true;
			}
		}
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		e->GetErrorMessage(szCause, 255);
		CString str(szCause);
		str += _T("\n");
		TRACE(str);
		if (m_pAVIPlay->IsShowMessageBoxOnError())
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}
}

void CAVIPlay::CAVIAudioStream::Free()
{
	// Release Stream Header
	m_AcmStreamHeader.pbSrc = m_pSrcBuf;
	m_AcmStreamHeader.pbDst = m_pDstBuf;
	m_AcmStreamHeader.cbSrcLength = m_dwSrcBufSize;
	m_AcmStreamHeader.cbDstLength = m_dwDstBufSize;
	if (m_hAcmStream)
		::acmStreamUnprepareHeader(m_hAcmStream, &m_AcmStreamHeader, 0);
	
	// Clean-Up
	if (m_pSrcBuf)
	{
		av_free(m_pSrcBuf);
		m_pSrcBuf = NULL;
	}
	if (m_pDstBuf)
	{
		av_free(m_pDstBuf);
		m_pDstBuf = NULL;
	}
	m_dwSrcBufSize = 0;
	m_dwDstBufSize = 0;

	// Close Stream
	if (m_hAcmStream)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
	}

	// Free LIBAVCODEC
	FreeAVCodec();

	if (m_pUncompressedWaveFormat)
	{
		delete [] m_pUncompressedWaveFormat;
		m_pUncompressedWaveFormat = NULL;
	}

	m_dwDstMinBytesCount = 0;
	m_dwSrcMinBytesCount = 0;
	m_dwNextChunk = 0;
	m_nCurrentChunk = -1;
	m_llCurrentSample = -1;
	m_nCurrentChunksCount = 0;
	m_dwDstBufSizeUsed = 0;
	m_dwDstBufOffset = 0;
	m_bFirstConversion = true;
	m_dwSrcBufUnconvertedBytesCount = 0;
	m_bHasDecompressor = false;
	memset(&m_MpegAudioFrameHdr, 0, sizeof(MpegAudioFrameHdr));
}

void CAVIPlay::CAVIAudioStream::FreeAVCodec()
{
	if (m_pCodecCtx)
	{
		/*
		Close a given AVCodecContext and free all the data associated with it
		(but not the AVCodecContext itself).
		Calling this function on an AVCodecContext that hasn't been opened will free
		the codec-specific data allocated in avcodec_alloc_context3() /
		avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
		do nothing.
		*/
		avcodec_close_thread_safe(m_pCodecCtx);

		// Free
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrame)
		av_frame_free(&m_pFrame);
	if (m_pAudioConvertCtx)
		swr_free(&m_pAudioConvertCtx);
} 

bool CAVIPlay::CAVIAudioStream::OpenDecompressionAVCodec()
{
	// Free
	FreeAVCodec();

	// Find the decoder for the audio stream
	AVCodecID id = AVCodecFormatTagToCodecID(GetFormatTag(true),
				((LPWAVEFORMATEX)m_pSrcFormat)->wBitsPerSample);
	
	// Open Codec
	m_pCodec = avcodec_find_decoder(id);
    if (!m_pCodec)
        return false;

	// Allocate Context
	/* if m_pCodec non-NULL, allocate private data and initialize defaults
	 * for the given codec. It is illegal to then call avcodec_open2()
	 * with a different codec.
	 * If NULL, then the codec-specific defaults won't be initialized,
	 * which may result in suboptimal default settings (this is
	 * important mainly for encoders, e.g. libx264).
	 */
	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (!m_pCodecCtx)
	{
		m_pCodec = NULL;
		return false;
	}

	// Init
	int nChannels = ((LPWAVEFORMATEX)m_pSrcFormat)->nChannels;
	int	nSamplesPerSec = ((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
	int	nBitsPerSec = ((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec * 8;
	m_pCodecCtx->channels = nChannels;
	m_pCodecCtx->sample_rate = nSamplesPerSec;
	m_pCodecCtx->bit_rate = nBitsPerSec;
	m_pCodecCtx->block_align = ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
	m_pCodecCtx->bits_per_coded_sample = ((LPWAVEFORMATEX)m_pSrcFormat)->wBitsPerSample;

	// Extra data
	if (m_dwSrcFormatSize > sizeof(WAVEFORMATEX))
	{
		m_pCodecCtx->extradata_size = m_dwSrcFormatSize - sizeof(WAVEFORMATEX);
		m_pCodecCtx->extradata = (uint8_t*)av_malloc(	m_pCodecCtx->extradata_size +
														FF_INPUT_BUFFER_PADDING_SIZE);
		if (m_pCodecCtx->extradata)
		{
			memcpy(	m_pCodecCtx->extradata,
					(LPBYTE)m_pSrcFormat + sizeof(WAVEFORMATEX),
					m_pCodecCtx->extradata_size);
		}
		else
			m_pCodecCtx->extradata_size = 0;
	}

	// Open codec
    if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
	{
		avcodec_close_thread_safe(m_pCodecCtx);
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
		m_pCodec = NULL;
		return false;
	}

	// Calc. Destination Buffer Sizes
	if (nChannels > 2 || nChannels < 1)
		nChannels = 2;
	if (nSamplesPerSec <= 11025) 
		m_dwDstMinBytesCount = 8 * AUDIO_PCM_MIN_BUF_SIZE * nChannels;
	else if (nSamplesPerSec <= 22050)
		m_dwDstMinBytesCount = 16 * AUDIO_PCM_MIN_BUF_SIZE * nChannels;
	else if (nSamplesPerSec <= 44100)
		m_dwDstMinBytesCount = 32 * AUDIO_PCM_MIN_BUF_SIZE * nChannels;
	else if (nSamplesPerSec <= 48000)
		m_dwDstMinBytesCount = 48 * AUDIO_PCM_MIN_BUF_SIZE * nChannels;
	else
		m_dwDstMinBytesCount = 64 * AUDIO_PCM_MIN_BUF_SIZE * nChannels;
	m_dwDstMinBytesCount = MAX(m_dwDstMinBytesCount, 192000); // at least 1 second of 48khz 16bit stereo
	m_dwDstBufSize = 3 * m_dwDstMinBytesCount;

	// Calc. Source Buffer Sizes
	if (m_bVBR)
	{
		m_dwSrcMinBytesCount = AUDIO_VBR_MIN_BUF_SIZE;
		m_dwSrcBufSize = 3 * AUDIO_VBR_MIN_BUF_SIZE;
	}
	else
	{
		// Always calc. with 16 bits samples
		int nMaxOutBytesPerSec = 2 * ((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec *
								((LPWAVEFORMATEX)m_pSrcFormat)->nChannels;
		int nCompression = 16;
		if (((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec > 0)
			nCompression = nMaxOutBytesPerSec / ((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec;
		if (nCompression < 1)
			nCompression = 16;

		// Be Safe
		nCompression *= 2;
	
		// Calc. Source Sizes
		m_dwSrcMinBytesCount = m_dwDstMinBytesCount / nCompression;
		m_dwSrcBufSize = m_dwDstBufSize / nCompression;
	}

	// Allocate audio frame
	if (m_pFrame)
		av_frame_free(&m_pFrame);
    m_pFrame = av_frame_alloc();
	if (!m_pFrame)
	{
		avcodec_close_thread_safe(m_pCodecCtx);
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
		m_pCodec = NULL;
		return false;
	}

	// Allocate Destination Buffer
	if (m_pDstBuf)
		av_free(m_pDstBuf);
	m_pDstBuf = (LPBYTE)av_malloc(m_dwDstBufSize + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!m_pDstBuf)
	{
		avcodec_close_thread_safe(m_pCodecCtx);
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
		m_pCodec = NULL;
		return false;
	}

	// Allocate Source Buffer
	if (m_pSrcBuf)
		av_free(m_pSrcBuf);
	m_pSrcBuf = (LPBYTE)av_malloc(m_dwSrcBufSize + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!m_pSrcBuf)
	{
		avcodec_close_thread_safe(m_pCodecCtx);
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
		m_pCodec = NULL;
		return false;
	}

	return true;
}

enum AVCodecID CAVIPlay::CAVIAudioStream::AVCodecFormatTagToCodecID(WORD wFormatTag, int nPcmBits/*=16*/)
{
	switch (wFormatTag)
	{
		case WAVE_FORMAT_PCM :					return nPcmBits == 16 ? AV_CODEC_ID_PCM_S16LE : AV_CODEC_ID_PCM_U8;
		case WAVE_FORMAT_MSAUDIO1 :				return AV_CODEC_ID_WMAV1; // = WMA7
		case WAVE_FORMAT_WMA8 :					return AV_CODEC_ID_WMAV2;
		case WAVE_FORMAT_MPEG :					return AV_CODEC_ID_MP2;
		case WAVE_FORMAT_MPEGLAYER3 :			return AV_CODEC_ID_MP3;
		case WAVE_FORMAT_DOLBY_AC3 :			return AV_CODEC_ID_AC3;
		case WAVE_FORMAT_DVD_DTS :				return AV_CODEC_ID_DTS;
		case WAVE_FORMAT_VORBIS :
		case WAVE_FORMAT_OGG1 :
		case WAVE_FORMAT_OGG2 :
		case WAVE_FORMAT_OGG3 :
		case WAVE_FORMAT_OGG1P :
		case WAVE_FORMAT_OGG2P :
		case WAVE_FORMAT_OGG3P :				return AV_CODEC_ID_VORBIS;
		case WAVE_FORMAT_FLAC :					return AV_CODEC_ID_FLAC;
		case WAVE_FORMAT_AAC1 :					return AV_CODEC_ID_AAC;
		case WAVE_FORMAT_AAC2 :					return AV_CODEC_ID_AAC;
		case WAVE_FORMAT_AAC4 :					return AV_CODEC_ID_AAC;
		case WAVE_FORMAT_AMR :					return AV_CODEC_ID_AMR_NB;
		case WAVE_FORMAT_VOXWARE :				return AV_CODEC_ID_ADPCM_IMA_DK3;
		case WAVE_FORMAT_DVI_ADPCM :			return AV_CODEC_ID_ADPCM_IMA_WAV;
		case WAVE_FORMAT_ESPCM :				return AV_CODEC_ID_ADPCM_IMA_DK4;
		case WAVE_FORMAT_ADPCM :				return AV_CODEC_ID_ADPCM_MS;
		case WAVE_FORMAT_CREATIVE_ADPCM :		return AV_CODEC_ID_ADPCM_CT;
		case WAVE_FORMAT_YAMAHA_ADPCM :			return AV_CODEC_ID_ADPCM_YAMAHA;
		case WAVE_FORMAT_G726_ADPCM :			return AV_CODEC_ID_ADPCM_G726;
		case WAVE_FORMAT_ALAW :					return AV_CODEC_ID_PCM_ALAW;
		case WAVE_FORMAT_MULAW :				return AV_CODEC_ID_PCM_MULAW;
		case WAVE_FORMAT_GSM610 :				return AV_CODEC_ID_GSM_MS;
		case WAVE_FORMAT_DSPGROUP_TRUESPEECH :	return AV_CODEC_ID_TRUESPEECH;
		case WAVE_FORMAT_TTA :					return AV_CODEC_ID_TTA;
		case WAVE_FORMAT_COOK :					return AV_CODEC_ID_COOK;
		case WAVE_FORMAT_SONY_SCX :				return AV_CODEC_ID_ATRAC3;
		default :								return AV_CODEC_ID_NONE;
	}
}

// sampling rates in hertz: 1. index = MPEG Version ID, 2. index = sampling rate index
const int CAVIPlay::CAVIAudioStream::m_nFreq[4][3] = 
{ 
	{11025, 12000, 8000,  },	// MPEG 2.5
	{0,     0,     0,     },	// reserved
	{22050, 24000, 16000, },	// MPEG 2
	{44100, 48000, 32000  }		// MPEG 1
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
const int CAVIPlay::CAVIAudioStream::m_nSamplesPerFrames[2][3] =
{
	{	// MPEG 1
		384,	// Layer1
		1152,	// Layer2	
		1152	// Layer3
	},
	{	// MPEG 2, 2.5
		384,	// Layer1
		1152,	// Layer2
		576		// Layer3
	}	
};

// Samples per Frame / 8: 1. index = LSF, 2. index = Layer
const int CAVIPlay::CAVIAudioStream::m_nCoefficients[2][3] =
{
	{	// MPEG 1
		12,		// Layer1	(must be multiplied with 4, because of slot size)
		144,	// Layer2
		144		// Layer3
	},
	{	// MPEG 2, 2.5
		12,		// Layer1	(must be multiplied with 4, because of slot size)
		144,	// Layer2
		72		// Layer3
	}	
};

// bitrates: 1. index = LSF, 2. index = Layer, 3. index = bitrate index
const int CAVIPlay::CAVIAudioStream::m_nBitrate[2][3][15] =
{
	{	// MPEG 1
		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},	// Layer1
		{0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},	// Layer2
		{0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}	// Layer3
	},
	{	// MPEG 2, 2.5		
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},		// Layer1
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},			// Layer2
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}			// Layer3
	}
};

// slot size per layer
const int CAVIPlay::CAVIAudioStream::m_nSlotSizes[3] =
{
	4,			// Layer1
	1,			// Layer2
	1			// Layer3
};

// Returns offset of Hdr Init (-1 on error)
// Input: *pSize is the buffer size
// Output: frame size in *pSize
//
// if pHdrFound is not NULL, the header is copied to it.
// if pHdrCompare is not NULL, the function returns success only
// if the found header is equal to the compare header.
int CAVIPlay::CAVIAudioStream::GetMpegAudioFrameHdr(LPBYTE pBuf,
													int* pSize,
													int nSearchForLayer/*=0*/, // If 0 search all layers otherwise consider only Layer 1,2 or 3
													MpegAudioFrameHdr* pHdrFound/*=NULL*/,
													MpegAudioFrameHdr* pHdrCompare/*=NULL*/)
{
	int nBufSize = *pSize;
    unsigned int x = 0; 
    unsigned int counter = 0;
	unsigned int i = 0 ;
	unsigned int d = 0;
	int nFrameSize = 0;
	int nFirstFrameOffset = 0;
	bool bOk = false;
	MpegAudioFrameHdr hdr;
	bool bFalsePositive = false;

	while (!bOk)
	{
		while (*pSize > 3)
		{ 
			d = *(unsigned int *)(pBuf + i); 
			_asm
			{
				mov eax, d;
				bswap eax;
				mov d, eax; 
			}
			if ((d & 0xFFE00000) == 0xFFE00000)
				break;
			i++;
			(*pSize)--;
		}
		if (*pSize <= 3) 
			return -1; // No Frame Found

		// Parse Header
		hdr.version = (d >> 19) & 0x3;
		int nLSF; // 1 means lower sampling frequencies (=MPEG2/MPEG2.5)
		if (hdr.version == MPEG1)
			nLSF = 0;
		else
			nLSF = 1;
		hdr.lay = 4 - ((d >> 17) & 0x3);
		hdr.error_protection = (d >> 16) & 0x1;
		hdr.bitrate_index = (d >> 12) & 0xF;
		hdr.sampling_frequency = (d >> 10) & 0x3;
		hdr.padding = (d >> 9) & 0x1;
		hdr.extension = (d >> 8) & 0x1;
		hdr.mode = (d >> 6) & 0x3;
		hdr.mode_ext = (d >> 4) & 0x3;
		hdr.copyright = (d >> 3) & 0x1;
		hdr.original = (d >> 2) & 0x1;
		hdr.emphasis = d & 0x3;
		nFirstFrameOffset = i;

		if (hdr.lay < 4 && hdr.sampling_frequency <= 2 && hdr.bitrate_index <= 14)
		{
			// Init Vars
			int channels = (hdr.mode == MPEG_MD_MONO) ? 1 : 2;
			int bitrate = m_nBitrate[nLSF][hdr.lay - 1][hdr.bitrate_index] * 1000;
			int frequency = m_nFreq[hdr.version][hdr.sampling_frequency];
			if (frequency > 0)
				nFrameSize = (m_nCoefficients[nLSF][hdr.lay - 1] * bitrate / frequency + hdr.padding) * m_nSlotSizes[hdr.lay - 1];
			
			// Check
			bOk =	((nSearchForLayer > 0) ? (nSearchForLayer == hdr.lay) : true) &&
					bitrate > 0			&&
					frequency > 0		&&
					frequency <= 48000	&&
					nFrameSize > 0		&&
					nFrameSize < 10000;

			// Better Check
			if (bOk && pHdrCompare)
			{		
				// Leave out error_protection, padding and mode_ext which may change between frames
				if (pHdrCompare->version != hdr.version							||
					pHdrCompare->lay != hdr.lay									||
					pHdrCompare->sampling_frequency != hdr.sampling_frequency	||
					pHdrCompare->extension != hdr.extension						||
					pHdrCompare->mode != hdr.mode								||
					pHdrCompare->copyright != hdr.copyright						||
					pHdrCompare->original != hdr.original						||
					pHdrCompare->emphasis != hdr.emphasis)
				{
					TRACE(	_T("\ni=%i, *pSize=%i, nBufSize=%i\nversion %i <-> %i\nlay %i <-> %i\nsampling_frequency %i <-> %i\n")
							_T("extension %i <-> %i\nmode %i <-> %i\ncopyright %i <-> %i\n")
							_T("original %i <-> %i\nemphasis %i <-> %i\n\n"),
							i, *pSize, nBufSize,
							hdr.version, pHdrCompare->version,
							hdr.lay, pHdrCompare->lay,
							hdr.sampling_frequency, pHdrCompare->sampling_frequency,
							hdr.extension, pHdrCompare->extension, 
							hdr.mode, pHdrCompare->mode,
							hdr.copyright, pHdrCompare->copyright,
							hdr.original, pHdrCompare->original,
							hdr.emphasis, pHdrCompare->emphasis);
					bOk = false;
					bFalsePositive = true;
				}
			}
		}

		// If not Ok continue to search the header begin
		if (!bOk)
		{
			i++;
			(*pSize)--;
		}
		else
		{
			if (bFalsePositive)
				TRACE(_T("\nOk: i=%i, *pSize=%i, nBufSize=%i\n"), i, *pSize, nBufSize);
		}
	}

	*pSize = nFrameSize;
	if (nBufSize >= nFirstFrameOffset + nFrameSize)
	{
		if (bFalsePositive)
			TRACE(_T("\nOk: Also nBufSize is enough\n\n\n"));
		if (pHdrFound)
			memcpy(pHdrFound, &hdr, sizeof(MpegAudioFrameHdr));
		return nFirstFrameOffset;
	}
	else
	{
		if (bFalsePositive)
			TRACE(_T("\nBad: nBufSize not enough\n\n\n"));
		return -1;
	}
}

CString CAVIPlay::CAVIAudioStream::GetACMDecompressorShortName()
{
	if (!m_hAcmStream)
		return _T("");

	HACMDRIVERID hId;
	MMRESULT res = ::acmDriverID((HACMOBJ)m_hAcmStream, &hId, 0);       
	if (res != MMSYSERR_NOERROR)
		return _T("");

	ACMDRIVERDETAILS DriverDetails;
	memset(&DriverDetails, 0, sizeof(ACMDRIVERDETAILS));
	DriverDetails.cbStruct = sizeof(ACMDRIVERDETAILS);
	res = ::acmDriverDetails(hId, &DriverDetails, 0);
	if (res != MMSYSERR_NOERROR)
		return _T("");
	else
		return CString(DriverDetails.szShortName);
}

CString CAVIPlay::CAVIAudioStream::GetACMDecompressorLongName()
{
	if (!m_hAcmStream)
		return _T("");

	HACMDRIVERID hId;
	MMRESULT res = ::acmDriverID((HACMOBJ)m_hAcmStream, &hId, 0);       
	if (res != MMSYSERR_NOERROR)
		return _T("");

	ACMDRIVERDETAILS DriverDetails;
	memset(&DriverDetails, 0, sizeof(ACMDRIVERDETAILS));
	DriverDetails.cbStruct = sizeof(ACMDRIVERDETAILS);
	res = ::acmDriverDetails(hId, &DriverDetails, 0);
	if (res != MMSYSERR_NOERROR)
		return _T("");
	else
		return CString(DriverDetails.szLongName);
}

bool CAVIPlay::CAVIAudioStream::OpenDecompressionACM()
{
	MMRESULT res;

	// Wave Format Pointer
	LPWAVEFORMATEX pWaveFormat = (LPWAVEFORMATEX)m_pSrcFormat;

	// Format Suggest
	res = ::acmFormatSuggest(NULL, pWaveFormat, m_pUncompressedWaveFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
	if (res != MMSYSERR_NOERROR)
		return false;

	// Open Decoder 
	res = ::acmStreamOpen(	(LPHACMSTREAM)&m_hAcmStream,	// Stream Handle
							NULL,							// Driver
							pWaveFormat,					// Source
							m_pUncompressedWaveFormat,		// Destination
							NULL,							// Filter
							NULL,							// CallBack
							0,								// User Data
							ACM_STREAMOPENF_NONREALTIME);	// Flags
	if (res != MMSYSERR_NOERROR)
	{
		if (m_hAcmStream)
		{
			::acmStreamClose(m_hAcmStream, 0);
			m_hAcmStream = NULL;
		}
		return false;
	}

	// Calc. the m_dwDstMinBytesCount variable
	if (!CalcMinBytesCount())
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}

	// Calc. the m_dwSrcMinBytesCount variable
	res = ::acmStreamSize(m_hAcmStream, m_dwDstMinBytesCount, (LPDWORD)&m_dwSrcMinBytesCount, ACM_STREAMSIZEF_DESTINATION);
	if (res != MMSYSERR_NOERROR)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}

	// Calc. the dwSuggestedDstBufSize
	DWORD dwSuggestedDstBufSize;
	DWORD dwSuggestedSrcBufSize = (m_Hdr.dwSuggestedBufferSize == 0) ? AUDIO_DEFAULT_SUGGESTED_BUFFER_SIZE : m_Hdr.dwSuggestedBufferSize;
	res = ::acmStreamSize(m_hAcmStream, dwSuggestedSrcBufSize, &dwSuggestedDstBufSize, ACM_STREAMSIZEF_SOURCE);
	if (res != MMSYSERR_NOERROR)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}

	// Allocate Dst (=Uncompressed) Buffer
	m_dwDstBufSize = 2 * MAX(dwSuggestedDstBufSize, m_dwDstMinBytesCount);	// Be Safe!
	if (m_pDstBuf)
		av_free(m_pDstBuf);
	m_pDstBuf = (LPBYTE)av_malloc(m_dwDstBufSize);
	if (!m_pDstBuf)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}

	// Calc. & Allocate Src Buffer
	// (for safety use the double)
	res = ::acmStreamSize(m_hAcmStream, m_dwDstBufSize, (LPDWORD)&m_dwSrcBufSize, ACM_STREAMSIZEF_DESTINATION);
	if (res != MMSYSERR_NOERROR)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}
	m_dwSrcBufSize *= 2;	// Be Safe!
	if (m_pSrcBuf)
		av_free(m_pSrcBuf);
	m_pSrcBuf = (LPBYTE)av_malloc(m_dwSrcBufSize);
	if (!m_pSrcBuf)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}

	// Check
	ASSERT(m_dwSrcBufSize >= m_dwSrcMinBytesCount);

	// Prepare Header
	::ZeroMemory(&m_AcmStreamHeader, sizeof(ACMSTREAMHEADER));
	m_AcmStreamHeader.cbStruct = sizeof(ACMSTREAMHEADER);
	m_AcmStreamHeader.pbSrc = m_pSrcBuf;
	m_AcmStreamHeader.pbDst = m_pDstBuf;
	m_AcmStreamHeader.cbSrcLength = m_dwSrcBufSize;
	m_AcmStreamHeader.cbDstLength = m_dwDstBufSize;
	res = ::acmStreamPrepareHeader(m_hAcmStream, &m_AcmStreamHeader, 0);
	if (res != MMSYSERR_NOERROR)
	{
		::acmStreamClose(m_hAcmStream, 0);
		m_hAcmStream = NULL;
		return false;
	}
	else
		return true;
}

bool CAVIPlay::CAVIAudioStream::OpenDecompression()
{
	// Clean-up
	Free();

	// Check
	if (!m_pSrcFormat || m_dwSrcFormatSize == 0)
		return false;
	
	// If PCM -> Decompression Not Needed
	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
	{
		// Calc. the m_dwDstMinBytesCount variable
		if (!CalcMinBytesCount())
		{
			Free();
			return false;
		}
		
		// Allocate Buffer
		DWORD dwSuggestedBufSize = (m_Hdr.dwSuggestedBufferSize == 0) ? AUDIO_DEFAULT_SUGGESTED_BUFFER_SIZE : m_Hdr.dwSuggestedBufferSize;
		m_dwDstBufSize = 2 * MAX(dwSuggestedBufSize, m_dwDstMinBytesCount);	// Be Safe!
		if (GetMaxChunkSize() > m_dwDstBufSize)
			m_dwDstBufSize = GetMaxChunkSize();
		m_pDstBuf = (LPBYTE)av_malloc(m_dwDstBufSize);
		if (!m_pDstBuf)
		{
			Free();
			return false;
		}

		// Start Getting Samples from the beginning
		if (GetNextChunksSamples())
		{
			m_bHasDecompressor = true;
			return true;
		}
		else
		{
			Free();
			return false;
		}
	}

	// Uncompressed Buffer
	m_pUncompressedWaveFormat = new WAVEFORMATEX;
	if (!m_pUncompressedWaveFormat)
	{
		Free();
		return false;
	}
	memset(m_pUncompressedWaveFormat, 0, sizeof(WAVEFORMATEX));
	m_pUncompressedWaveFormat->wFormatTag = WAVE_FORMAT_PCM;

	// Open Decompressor
	bool bFoundDecompressor = false;
	if (m_pAVIPlay->m_bAVCodecPriority)
	{
		if (!OpenDecompressionAVCodec() && !OpenDecompressionACM())
			bFoundDecompressor = false;
		else
			bFoundDecompressor = true;
	}
	else
	{
		if (!OpenDecompressionACM() && !OpenDecompressionAVCodec())
			bFoundDecompressor = false;
		else
			bFoundDecompressor = true;
	}
	if (!bFoundDecompressor)
	{
		// Error Message
		CString str;
		str.Format(_T("No Audio Codec Installed For %s, Tag ID 0x%X (%i)\n"),
					CAVIPlay::GetWaveFormatTagString(((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag),
					((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag,
					((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag);
#ifdef _DEBUG
		str = _T("OpenDecompression():\n") + str;
#endif
		TRACE(str);
		if (m_pAVIPlay->IsShowMessageBoxOnError())
			::AfxMessageBox(str, MB_ICONSTOP);

		Free();
		return false;
	}

	// Start Getting Samples from the beginning
	if (!GetChunksSamples(0))
	{
		Free();
		return false;
	}

	// Set Flag
	m_bHasDecompressor = true;

	return true;
}

bool CAVIPlay::CAVIAudioStream::CalcMinBytesCount()
{
	if (!m_pSrcFormat)
		return false;

	int nSamplesPerSec = (m_pUncompressedWaveFormat) ?
						m_pUncompressedWaveFormat->nSamplesPerSec :
						((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
	int nBlockAlign = (m_pUncompressedWaveFormat) ?
						m_pUncompressedWaveFormat->nBlockAlign :
						((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
	
	if (nSamplesPerSec <= 11025) 
		m_dwDstMinBytesCount = 1 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 22050)
		m_dwDstMinBytesCount = 2 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 44100)
		m_dwDstMinBytesCount = 4 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 48000)
		m_dwDstMinBytesCount = 6 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;
	else
		m_dwDstMinBytesCount = 8 * AUDIO_PCM_MIN_BUF_SIZE * nBlockAlign;

	return true;
}

bool CAVIPlay::CAVIAudioStream::SetIsVBR()
{
	// Check
	if (!m_pSrcFormat)
		return false;

	// Reset
	m_bVBR = false;
	m_nVBRSamplesPerChunk = 0;

	// Mpeg Audio Layer III
	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_MPEGLAYER3)
	{
		// Is VBR?
		if ((((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign >= 576		&&
			((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign <= 1152)	||
			GetHdr()->dwSampleSize == 0)
		{
			// Determine Samples Per Chunk
			// 1152 samples for 32KHz or higher
			// 576 samples for 24KHz or lower
			if (((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec >= 32000)
				m_nVBRSamplesPerChunk = 1152;
			else
				m_nVBRSamplesPerChunk = 576;
			m_bVBR = true;
		}
	}
	// Mpeg Audio Layer I (384) or II (1152)
	else if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_MPEG)
	{
		if (((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign == 1152	||
			GetHdr()->dwSampleSize == 0)
		{
			// Set as it was a Layer II,
			// but we have to check the Frame Header.
			// The first call to the GetMpegAudioFrameHdr()
			// function will set the correct m_nVBRSamplesPerChunk
			// value if we have a Layer I.
			m_nVBRSamplesPerChunk = 1152;
			m_bVBR = true;
		}
	}
	// Other Codecs which may be recorded with VBR
	else if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_DOLBY_AC3	||
			((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_DVD_DTS		||
			((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_VORBIS		||
			((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_FLAC			||
			((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_AAC1			||
			((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_AAC2			||
			((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_AAC4)
	{
		if (GetHdr()->dwSampleSize == 0)
		{
			// m_nVBRSamplesPerChunk will be set with first decoded frame
			m_bVBR = true;
		}
	}

	return true;
}

WORD CAVIPlay::CAVIAudioStream::GetFormatTag(bool bSource) const
{
	if (!m_pSrcFormat)
		return 0;

	if (bSource)
		return ((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag;
	else
	{
		if (m_pUncompressedWaveFormat) 
			return m_pUncompressedWaveFormat->wFormatTag;
		else
			return ((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag;
	}
}

WORD CAVIPlay::CAVIAudioStream::GetChannels(bool bSource) const
{
	if (!m_pSrcFormat)
		return 0;

	if (bSource)
		return ((LPWAVEFORMATEX)m_pSrcFormat)->nChannels;
	else
	{
		if (m_pUncompressedWaveFormat) 
			return m_pUncompressedWaveFormat->nChannels;
		else
			return ((LPWAVEFORMATEX)m_pSrcFormat)->nChannels;
	}
}

DWORD CAVIPlay::CAVIAudioStream::GetSampleRate(bool bSource) const
{
	if (!m_pSrcFormat)
		return 0;

	if (bSource)
		return ((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
	else
	{
		if (m_pUncompressedWaveFormat) 
			return m_pUncompressedWaveFormat->nSamplesPerSec;
		else
			return ((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
	}
}

DWORD CAVIPlay::CAVIAudioStream::CalcVBRBytesPerSeconds() const
{
	if (!m_pSrcFormat)
		return 0;

	if (m_bVBR && m_nVBRSamplesPerChunk > 0 && m_dwChunksCount > 0)
		return (DWORD)Round(((double)m_llBytesCount * (double)((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec) /
							((double)m_nVBRSamplesPerChunk * (double)m_dwChunksCount));
	else
		return 0;
}

DWORD CAVIPlay::CAVIAudioStream::GetBytesPerSeconds(bool bSource) const
{
	if (!m_pSrcFormat)
		return 0;

	if (bSource)
		return ((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec;
	else
	{		
		if (m_pUncompressedWaveFormat) 
			return m_pUncompressedWaveFormat->nAvgBytesPerSec;
		else
			return ((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec;
	}

}

WORD CAVIPlay::CAVIAudioStream::GetSampleSize(bool bSource) const
{
	if (!m_pSrcFormat)
		return 0;

	if (bSource)
		return ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
	else
	{
		if (m_pUncompressedWaveFormat) 
			return m_pUncompressedWaveFormat->nBlockAlign;
		else
			return ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
	}
}

WORD CAVIPlay::CAVIAudioStream::GetBits(bool bSource) const
{
	if (!m_pSrcFormat)
		return 0;

	if (bSource)
		return ((LPWAVEFORMATEX)m_pSrcFormat)->wBitsPerSample;
	else
	{
		if (m_pUncompressedWaveFormat) 
			return m_pUncompressedWaveFormat->wBitsPerSample;
		else
			return ((LPWAVEFORMATEX)m_pSrcFormat)->wBitsPerSample;
	}
}

LONGLONG CAVIPlay::CAVIAudioStream::GetTotalSamples() const
{
	if (!m_pSrcFormat)
		return 0;

	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
		return (m_llBytesCount / ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign);
	else if (m_bVBR)
		return (LONGLONG)m_nVBRSamplesPerChunk * (LONGLONG)m_dwChunksCount;
	else
		return ((LONGLONG)GetTotalTime() * (LONGLONG)((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec);
}

double CAVIPlay::CAVIAudioStream::GetTotalTime() const
{
	if (!m_pSrcFormat)
		return 0.0;

	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
		return ((double)m_llBytesCount / (double)((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign /
										(double)((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec);
	else if (m_bVBR)
		return (double)m_nVBRSamplesPerChunk * (double)m_dwChunksCount / (double)((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
	else
		return ((double)m_llBytesCount / (double)((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec);
}

DWORD CAVIPlay::CAVIAudioStream::SampleToChunk(LONGLONG llSampleNum)
{
	DWORD dwChunkNum;
	if (m_bVBR)
	{
		// From the Sample Position get the Chunk Position,
		// dwSampleNum & dwChunkNum are both updated!
		if (!VBRSampleToChunk(&llSampleNum, &dwChunkNum))
			return 0;

		return dwChunkNum;
	}
	else
	{
		// Convert Samples Position to Bytes Position
		LONGLONG llByteNum;
		if (!CBRSampleToByte(llSampleNum, &llByteNum))
			return 0;

		// From the Byte Position get the Chunk Position,
		// dwByteNum & dwChunkNum are both updated!
		if (!ByteToChunk(&llByteNum, &dwChunkNum))
			return 0;
	}

	return dwChunkNum;
}

bool CAVIPlay::CAVIAudioStream::ByteToChunk(LONGLONG* pByteNum, DWORD* pChunkNum)
{
	if (!pByteNum || !pChunkNum)
		return false;

	LONGLONG llBytesCount = 0;
	LONGLONG llPrevBytesCount = 0;
	DWORD dwChunkNum = 0;

	if (m_pOldIndexTable)
	{
		for (dwChunkNum = 0 ; dwChunkNum < m_dwChunksCount ; dwChunkNum++)
		{
			llPrevBytesCount = llBytesCount;
			llBytesCount += m_pOldIndexTable[dwChunkNum].dwSize;
			if (*pByteNum < llBytesCount)
			{
				*pChunkNum = dwChunkNum;
				*pByteNum = (*pByteNum) - llPrevBytesCount;
				return true;
			}
		}
	}

	if (m_pStdIndexHdrs && m_ppStdIndexTables)
	{
		for (unsigned int index = 0 ; index < m_SuperIndexHdr.nEntriesInUse ; index++)
		{
			for (unsigned int i = 0 ; i < m_pStdIndexHdrs[index].nEntriesInUse ; i++ , dwChunkNum++)
			{
				llPrevBytesCount = llBytesCount;
				llBytesCount += (m_ppStdIndexTables[index][i].dwSize & 0x7FFFFFFF);
				if (*pByteNum < llBytesCount)
				{
					*pChunkNum = dwChunkNum;
					*pByteNum = (*pByteNum) - llPrevBytesCount;
					return true;
				}
			}
		}
	}
	
	return false;
}

bool CAVIPlay::CAVIAudioStream::ChunkToByte(DWORD dwChunkNum, LONGLONG* pByteNum)
{
	if (!pByteNum)
		return false;

	if (dwChunkNum >= m_dwChunksCount)
		return false;

	*pByteNum = 0;
	DWORD dwChunkPos = 0;

	if (m_pOldIndexTable)
	{
		for (dwChunkPos = 0 ; dwChunkPos < m_dwChunksCount ; dwChunkPos++)
		{
			if (dwChunkPos >= dwChunkNum)
				return true;
			else
				(*pByteNum) += (LONGLONG)m_pOldIndexTable[dwChunkPos].dwSize;
		}
	}

	if (m_pStdIndexHdrs && m_ppStdIndexTables)
	{
		for (unsigned int index = 0 ; index < m_SuperIndexHdr.nEntriesInUse ; index++)
		{
			for (unsigned int i = 0 ; i < m_pStdIndexHdrs[index].nEntriesInUse ; i++ , dwChunkPos++)
			{
				if (dwChunkPos >= dwChunkNum)
					return true;
				else
					(*pByteNum) += (LONGLONG)(m_ppStdIndexTables[index][i].dwSize & 0x7FFFFFFF);
			}
		}
	}
	
	return false;
}

bool CAVIPlay::CAVIAudioStream::VBRSampleToChunk(LONGLONG* pSampleNum, DWORD* pChunkNum)
{
	if (!pSampleNum || !pChunkNum)
		return false;

	if (!m_bVBR || m_nVBRSamplesPerChunk <= 0)
		return false;

	*pChunkNum = (DWORD)((*pSampleNum) / m_nVBRSamplesPerChunk);
	*pSampleNum = (*pSampleNum) % m_nVBRSamplesPerChunk;

	return true;
}

bool CAVIPlay::CAVIAudioStream::VBRChunkToSample(DWORD dwChunkNum, LONGLONG* pSampleNum)
{
	if (!pSampleNum)
		return false;

	if (dwChunkNum >= m_dwChunksCount)
		return false;

	if (!m_bVBR || m_nVBRSamplesPerChunk <= 0)
		return false;

	*pSampleNum = (LONGLONG)dwChunkNum * (LONGLONG)m_nVBRSamplesPerChunk;

	return true;
}

bool CAVIPlay::CAVIAudioStream::CBRSampleToByte(LONGLONG llSampleNum, LONGLONG* pByteNum)
{
	if (!pByteNum)
		return false;

	if (!m_pSrcFormat)
		return false;

	if (m_bVBR)
		return false;

	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
		*pByteNum = llSampleNum * ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
	else
	{
		double dTime = (double)llSampleNum / (double)((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
		*pByteNum = (DWORD)Round(dTime * (double)((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec);
	}

	// Check
	if (*pByteNum < m_llBytesCount)
		return true;
	else
		return false;
}

bool CAVIPlay::CAVIAudioStream::CBRByteToSample(LONGLONG* pByteNum, LONGLONG* pSampleNum)
{
	if (!pByteNum || !pSampleNum)
		return false;

	if (!m_pSrcFormat)
		return false;

	if (*pByteNum >= m_llBytesCount)
		return false;

	if (m_bVBR)
		return false;

	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
	{
		*pSampleNum = *pByteNum / ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
		*pByteNum = *pByteNum % ((LPWAVEFORMATEX)m_pSrcFormat)->nBlockAlign;
	}
	else
	{
		double dTime = (double)(*pByteNum) / (double)((LPWAVEFORMATEX)m_pSrcFormat)->nAvgBytesPerSec;
		*pSampleNum = (DWORD)Round(dTime * (double)((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec);
	}

	return true;
}

bool CAVIPlay::CAVIAudioStream::ReOpenDecompressACM()
{
	MMRESULT res;

	// Check
	if (!m_hAcmStream				||	// Not Open? 
		!m_pUncompressedWaveFormat	||	// Not Init?
		!m_pSrcFormat)					// Not Init?
		return false;

	// Release Stream Header
	m_AcmStreamHeader.pbSrc = m_pSrcBuf;
	m_AcmStreamHeader.pbDst = m_pDstBuf;
	m_AcmStreamHeader.cbSrcLength = m_dwSrcBufSize;
	m_AcmStreamHeader.cbDstLength = m_dwDstBufSize;
	::acmStreamUnprepareHeader(m_hAcmStream, &m_AcmStreamHeader, 0);

	// Close
	::acmStreamClose(m_hAcmStream, 0);
	m_hAcmStream = NULL;

	// Open
	res = ::acmStreamOpen(	(LPHACMSTREAM)&m_hAcmStream,	// Stream Handle
							NULL,							// Driver
							(LPWAVEFORMATEX)m_pSrcFormat,		// Source
							m_pUncompressedWaveFormat,		// Destination
							NULL,							// Filter
							NULL,							// CallBack
							0,								// User Data
							ACM_STREAMOPENF_NONREALTIME);	// Flags
	if (res == MMSYSERR_NOERROR)
	{
		// Prepare Header
		::ZeroMemory(&m_AcmStreamHeader, sizeof(ACMSTREAMHEADER));
		m_AcmStreamHeader.cbStruct = sizeof(ACMSTREAMHEADER);
		m_AcmStreamHeader.pbSrc = m_pSrcBuf;
		m_AcmStreamHeader.pbDst = m_pDstBuf;
		m_AcmStreamHeader.cbSrcLength = m_dwSrcBufSize;
		m_AcmStreamHeader.cbDstLength = m_dwDstBufSize;
		res = ::acmStreamPrepareHeader(m_hAcmStream, &m_AcmStreamHeader, 0);
		if (res != MMSYSERR_NOERROR)
		{
			::acmStreamClose(m_hAcmStream, 0);
			m_hAcmStream = NULL;
			return false;
		}
		else
			return true;
	}
	else
	{
		if (m_hAcmStream)
		{
			::acmStreamClose(m_hAcmStream, 0);
			m_hAcmStream = NULL;
		}
		return false;
	}
}

bool CAVIPlay::CAVIAudioStream::GetNextChunksSamples()
{
	return GetChunksSamples(m_dwNextChunk);
}

int CAVIPlay::CAVIAudioStream::AVDecodeAudio(LPBYTE pDstBuf, int& out_size, AVPacket* pkt,
										int& out_channels, int& out_sample_rate, AVSampleFormat& out_sample_fmt)
{
	// Check
	if (!pDstBuf || !pkt)
		return -1;

	// Reset Frame Structure
	av_frame_unref(m_pFrame);

	// Decode
	int got_frame = 0;
	int len = avcodec_decode_audio4(m_pCodecCtx, m_pFrame, &got_frame, pkt); // returns the number of bytes consumed from the input
	
	// Set conversion params
	out_channels = MIN(2, m_pCodecCtx->channels); // not more channels than stereo
	out_sample_rate = m_pCodecCtx->sample_rate;
	out_sample_fmt = av_get_bytes_per_sample(m_pCodecCtx->sample_fmt) == 1 ?
			AV_SAMPLE_FMT_U8 : AV_SAMPLE_FMT_S16; // av_get_bytes_per_sample returns the bytes count per sample and per channel

	// Create resampler context
	if (!m_pAudioConvertCtx)
	{
		m_pAudioConvertCtx = swr_alloc();
		if (!m_pAudioConvertCtx)
			return -1;
		av_opt_set_int       (m_pAudioConvertCtx, "in_channel_count",   m_pCodecCtx->channels, 0);
		av_opt_set_int       (m_pAudioConvertCtx, "in_sample_rate",     m_pCodecCtx->sample_rate, 0);
		av_opt_set_sample_fmt(m_pAudioConvertCtx, "in_sample_fmt",      m_pCodecCtx->sample_fmt, 0);
		av_opt_set_int       (m_pAudioConvertCtx, "out_channel_count",  out_channels, 0);
		av_opt_set_int       (m_pAudioConvertCtx, "out_sample_rate",    out_sample_rate, 0);
		av_opt_set_sample_fmt(m_pAudioConvertCtx, "out_sample_fmt",     out_sample_fmt, 0);
		if (swr_init(m_pAudioConvertCtx) < 0)
			return -1;
	}

	// Resample
	int out_samples = (int)av_rescale_rnd(swr_get_delay(m_pAudioConvertCtx, m_pCodecCtx->sample_rate) + m_pFrame->nb_samples, out_sample_rate, m_pCodecCtx->sample_rate, AV_ROUND_UP);
	uint8_t* out_buffer = NULL;
	av_samples_alloc(&out_buffer, NULL, out_channels, out_samples, out_sample_fmt, 0);
	out_samples = swr_convert(m_pAudioConvertCtx, &out_buffer, out_samples, (const uint8_t **)m_pFrame->data, m_pFrame->nb_samples);
	if (out_samples < 0)
		return -1;

	// Copy to given destination buffer
	// Note: av_get_bytes_per_sample returns the bytes count per sample and per channel
	out_size = MIN(out_size, out_channels * out_samples * av_get_bytes_per_sample(out_sample_fmt));
	if (out_size > 0)
		memcpy(pDstBuf, out_buffer, out_size);
	av_freep(&out_buffer);

	return len;
}

bool CAVIPlay::CAVIAudioStream::GetChunksSamples(DWORD dwChunkNum)
{
	// Enter CS
	::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

	if (!m_pFile)
	{
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		m_dwDstBufOffset = 0;
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	if (dwChunkNum >= m_dwChunksCount)
	{
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		m_dwDstBufOffset = 0;
		m_dwNextChunk = m_dwChunksCount;
		m_llCurrentSample = GetTotalSamples();
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	if (!m_pSrcFormat || m_dwSrcFormatSize == 0)
	{
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		m_dwDstBufOffset = 0;
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// If PCM return
	if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag == WAVE_FORMAT_PCM)
	{
		m_nCurrentChunk = dwChunkNum;
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		DWORD dwBytes = m_dwDstBufSize - m_dwDstBufSizeUsed;
		while (GetChunkData(dwChunkNum, m_pDstBuf + m_dwDstBufSizeUsed, &dwBytes))
		{
			m_nCurrentChunksCount++;
			dwChunkNum++;
			m_dwDstBufSizeUsed += dwBytes;
			if (m_dwDstBufSizeUsed > m_dwDstMinBytesCount)
				break;
			dwBytes = m_dwDstBufSize - m_dwDstBufSizeUsed;
		}
		m_dwNextChunk = m_nCurrentChunk + m_nCurrentChunksCount;
		m_dwDstBufOffset = 0;

		// Calc. m_nCurrentSample
		LONGLONG llByteNum;
		ChunkToByte(m_nCurrentChunk, &llByteNum);
		CBRByteToSample(&llByteNum, (LONGLONG*)&m_llCurrentSample);
		
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

		return (m_nCurrentChunksCount > 0);
	}

	if (!m_pUncompressedWaveFormat)
	{
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		m_dwDstBufOffset = 0;
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}
	
	// Current Chunk
	m_nCurrentChunk = dwChunkNum;

	// Current Chunks Count
	m_nCurrentChunksCount = 0;

	// Length Used
	m_AcmStreamHeader.cbDstLengthUsed = 0;

	// Vars
	bool bOkGet;
	bool bSeek = false;
	DWORD dwSrcBufSizeUsed;

	// If Seeking or Begin -> Restart
	if (dwChunkNum != m_dwNextChunk || dwChunkNum == 0)
	{
		// Be First Conversion
		m_bFirstConversion = true;

		 // Throw them
		m_dwSrcBufUnconvertedBytesCount = 0;

		// Reset Converter
		//m_AcmStreamHeader.cbSrcLength = 0;
		//::acmStreamConvert(m_hAcmStream, &m_AcmStreamHeader, ACM_STREAMCONVERTF_END);
		ReOpenDecompressACM(); // For Some Decompressors the above code is not enough!

		// Set Seek Flag
		bSeek = true;

		// Flush Buffers
		if (m_pCodecCtx)
			avcodec_flush_buffers(m_pCodecCtx);
	}

	// Uncompression
	if (m_hAcmStream)
	{
		MMRESULT res;
		do
		{
			// Get Data
			dwSrcBufSizeUsed = 0;
			DWORD dwBytes = m_dwSrcBufSize - m_dwSrcBufUnconvertedBytesCount;
			while (bOkGet = GetChunkData(dwChunkNum, m_pSrcBuf + m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed, &dwBytes))
			{
				m_nCurrentChunksCount++;
				dwChunkNum++;
				dwSrcBufSizeUsed += dwBytes;
				if (dwSrcBufSizeUsed > m_dwSrcMinBytesCount)
					break;
				dwBytes = m_dwSrcBufSize - m_dwSrcBufUnconvertedBytesCount - dwSrcBufSizeUsed;
			}

			// If lay has the reserved value of 0 it means that the structure has not been initialized
			if ((GetFormatTag(true) == WAVE_FORMAT_MPEG			||
				GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)	&&
				m_MpegAudioFrameHdr.lay == 0)
			{
				int nFrameSize = (int)dwSrcBufSizeUsed;
				int nLayer = 0;
				if (GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)
					nLayer = 3;
				int nOffset = GetMpegAudioFrameHdr(m_pSrcBuf, &nFrameSize, nLayer, &m_MpegAudioFrameHdr);
				if (m_bVBR && m_MpegAudioFrameHdr.lay == 1)
					m_nVBRSamplesPerChunk = 384; // See SetIsVBR()
			}

			// Conversion
			if (dwSrcBufSizeUsed)
			{
				m_AcmStreamHeader.cbSrcLength = m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed;
				res = ::acmStreamConvert(m_hAcmStream, &m_AcmStreamHeader,
						(m_bFirstConversion ? ACM_STREAMCONVERTF_START : 0) | ACM_STREAMCONVERTF_BLOCKALIGN);
				m_bFirstConversion = false;
				if (res != MMSYSERR_NOERROR)
				{
					m_dwDstBufSizeUsed = 0;
					m_nCurrentChunksCount = 0;
					m_dwDstBufOffset = 0;
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
				if (m_AcmStreamHeader.cbSrcLengthUsed < (m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed))
				{
					m_dwSrcBufUnconvertedBytesCount = m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed - m_AcmStreamHeader.cbSrcLengthUsed;
					memmove(m_pSrcBuf, m_pSrcBuf + m_AcmStreamHeader.cbSrcLengthUsed, m_dwSrcBufUnconvertedBytesCount);
				}
				else
					m_dwSrcBufUnconvertedBytesCount = 0;
			}
			// Finish with unconverted bytes
			else
			{
				if (m_dwSrcBufUnconvertedBytesCount > 0)
				{
					m_AcmStreamHeader.cbSrcLength = m_dwSrcBufUnconvertedBytesCount;
					res = ::acmStreamConvert(m_hAcmStream, &m_AcmStreamHeader, ACM_STREAMCONVERTF_END);
					if (res != MMSYSERR_NOERROR)
					{
						m_dwDstBufSizeUsed = 0;
						m_nCurrentChunksCount = 0;
						m_dwDstBufOffset = 0;
						::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
						return false;
					}
				}
			}
		}
		while (bOkGet && dwSrcBufSizeUsed && (m_AcmStreamHeader.cbDstLengthUsed == 0));

		// Size & Offset
		m_dwDstBufSizeUsed = m_AcmStreamHeader.cbDstLengthUsed;
		m_dwDstBufOffset = 0;
	}
	else if (m_pCodecCtx)
	{
		int total_out_size = 0;
		do
		{
			// Get Data
			if (m_bVBR)
			{
				dwSrcBufSizeUsed = m_dwSrcBufSize - m_dwSrcBufUnconvertedBytesCount;
				if (bOkGet = GetChunkData(dwChunkNum, m_pSrcBuf + m_dwSrcBufUnconvertedBytesCount, &dwSrcBufSizeUsed))
				{
					m_nCurrentChunksCount++;
					dwChunkNum++;
				}
			}
			else
			{
				dwSrcBufSizeUsed = 0;
				DWORD dwBytes = m_dwSrcBufSize - m_dwSrcBufUnconvertedBytesCount;
				while (bOkGet = GetChunkData(dwChunkNum, m_pSrcBuf + m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed, &dwBytes))
				{
					m_nCurrentChunksCount++;
					dwChunkNum++;
					dwSrcBufSizeUsed += dwBytes;
					if ((m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed) > m_dwSrcMinBytesCount)
						break;
					dwBytes = m_dwSrcBufSize - m_dwSrcBufUnconvertedBytesCount - dwSrcBufSizeUsed;
				}
			}

			// Zero buffer
			memset(	m_pSrcBuf + m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed,
					0,
					FF_INPUT_BUFFER_PADDING_SIZE);

			// If lay has the reserved value of 0 it means that the structure has not been initialized
			int nOffset;
			int nFrameSize;
			if ((m_pCodecCtx->codec_id == AV_CODEC_ID_MP2	||
				m_pCodecCtx->codec_id == AV_CODEC_ID_MP3)	&&
				m_MpegAudioFrameHdr.lay == 0)
			{
				nFrameSize = (int)dwSrcBufSizeUsed;
				int nLayer = 0;
				if (GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)
					nLayer = 3;
				nOffset = GetMpegAudioFrameHdr(m_pSrcBuf, &nFrameSize, nLayer, &m_MpegAudioFrameHdr);
				if (m_bVBR && m_MpegAudioFrameHdr.lay == 1)
					m_nVBRSamplesPerChunk = 384; // See SetIsVBR()
			}

			// Conversion
			int len;
			LPBYTE inbuf_ptr = m_pSrcBuf;
			int nSrcBufSizeUsed = (int)(m_dwSrcBufUnconvertedBytesCount + dwSrcBufSizeUsed);
			while (nSrcBufSizeUsed > 0)
			{
				// Reset
				m_dwSrcBufUnconvertedBytesCount = 0;

				int out_size = 0;
				int out_channels = 2;
				int out_sample_rate = 44100;
				AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
				if ((m_pCodecCtx->codec_id == AV_CODEC_ID_MP2	||
					m_pCodecCtx->codec_id == AV_CODEC_ID_MP3)	&&
					!m_bVBR)
				{
					nFrameSize = nSrcBufSizeUsed;
					int nLayer = 0;
					if (GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)
						nLayer = 3;
					nOffset = GetMpegAudioFrameHdr(inbuf_ptr, &nFrameSize, nLayer, NULL, &m_MpegAudioFrameHdr);
					if (nOffset >= 0 && nFrameSize > 0)
					{
						// Copy source data to have a correctly aligned buffer
						// ending with FF_INPUT_BUFFER_PADDING_SIZE zero bytes
						AVPacket pkt;
						av_new_packet(&pkt, nFrameSize);
						memcpy(pkt.data, inbuf_ptr + nOffset, nFrameSize);
						out_size = (int)m_dwDstBufSize - total_out_size;
						len = AVDecodeAudio(m_pDstBuf + total_out_size, out_size, &pkt, out_channels, out_sample_rate, out_sample_fmt);
						av_free_packet(&pkt);

						// GetMpegAudioFrameHdr parser found a wrong frame start -> try again!
						if (len == -1)
						{
							out_size = 0;
							len = nOffset + 1;
						}
						else
						{
							// avcodec_decode_audio4 may not return the
							// correct used source size for one frame
							// -> set len manually!
							len = nOffset + nFrameSize;
						}
					}
					else
					{
						out_size = 0;
						len = 0;
						m_dwSrcBufUnconvertedBytesCount = nSrcBufSizeUsed;
						if (m_dwSrcBufUnconvertedBytesCount >= m_dwSrcBufSize / 2)
						{
							m_dwDstBufSizeUsed = 0;
							m_nCurrentChunksCount = 0;
							m_dwDstBufOffset = 0;
							::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
							return false;
						}
						memmove(m_pSrcBuf, inbuf_ptr, m_dwSrcBufUnconvertedBytesCount);
						nSrcBufSizeUsed = 0;
					}
				}
				else
				{
					if (nSrcBufSizeUsed > 0)
					{
						// Copy source data to have a correctly aligned buffer
						// ending with FF_INPUT_BUFFER_PADDING_SIZE zero bytes
						AVPacket pkt;
						av_new_packet(&pkt, nSrcBufSizeUsed);
						memcpy(pkt.data, inbuf_ptr, nSrcBufSizeUsed);
						out_size = (int)m_dwDstBufSize - total_out_size;
						len = AVDecodeAudio(m_pDstBuf + total_out_size, out_size, &pkt, out_channels, out_sample_rate, out_sample_fmt);
						av_free_packet(&pkt);
					}
				}	

				if (m_bFirstConversion && out_size > 0)
				{
					m_pUncompressedWaveFormat->nChannels = out_channels;
					if (out_sample_fmt == AV_SAMPLE_FMT_U8)
					{
						m_pUncompressedWaveFormat->wBitsPerSample = 8;
						m_pUncompressedWaveFormat->nBlockAlign = m_pUncompressedWaveFormat->nChannels;
					}
					else if (out_sample_fmt == AV_SAMPLE_FMT_S16)
					{
						m_pUncompressedWaveFormat->wBitsPerSample = 16;
						m_pUncompressedWaveFormat->nBlockAlign = 2 * m_pUncompressedWaveFormat->nChannels;
					}
					else
					{
						m_dwDstBufSizeUsed = 0;
						m_nCurrentChunksCount = 0;
						m_dwDstBufOffset = 0;
						::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
						return false;
					}
					m_pUncompressedWaveFormat->nSamplesPerSec = out_sample_rate;
					if (m_pUncompressedWaveFormat->nSamplesPerSec == 0)
						m_pUncompressedWaveFormat->nSamplesPerSec = ((LPWAVEFORMATEX)m_pSrcFormat)->nSamplesPerSec;
					m_pUncompressedWaveFormat->nAvgBytesPerSec = out_sample_rate * m_pUncompressedWaveFormat->nBlockAlign;
					if (m_pUncompressedWaveFormat->nAvgBytesPerSec == 0)
						m_pUncompressedWaveFormat->nAvgBytesPerSec = m_pUncompressedWaveFormat->nBlockAlign * m_pUncompressedWaveFormat->nSamplesPerSec;
					m_pUncompressedWaveFormat->cbSize = 0;

					// Set VBR Samples Per Chunk
					if (m_bVBR && m_nVBRSamplesPerChunk == 0)
						m_nVBRSamplesPerChunk = out_size / m_pUncompressedWaveFormat->nBlockAlign;

					// Reset Flag
					m_bFirstConversion = false;
				}

				// Skip Decodes from before the seek!
				if (bSeek && len == 0)
					out_size = 0;
				bSeek = false;

				// On Error
				if (len < 0)
				{
					m_dwDstBufSizeUsed = 0;
					m_nCurrentChunksCount = 0;
					m_dwDstBufOffset = 0;
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}

				// Update Sizes & Buffer Pointer
				nSrcBufSizeUsed -= len;
				inbuf_ptr += len;
				total_out_size += out_size;
			}
		}
		while (bOkGet && ((int)m_dwDstMinBytesCount > total_out_size));

		// Size & Offset
		m_dwDstBufSizeUsed = total_out_size;
		m_dwDstBufOffset = 0;
	}
	else
	{
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		m_dwDstBufOffset = 0;
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Next Chunk
	m_dwNextChunk = m_nCurrentChunk + m_nCurrentChunksCount;

	// Calc. m_llCurrentSample
	if (m_bVBR)
		VBRChunkToSample(m_nCurrentChunk, (LONGLONG*)&m_llCurrentSample);
	else
	{
		LONGLONG llByteNum;
		ChunkToByte(m_nCurrentChunk, &llByteNum);
		CBRByteToSample(&llByteNum, (LONGLONG*)&m_llCurrentSample);
	}

	// Leave CS
	::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

	return true;
}

bool CAVIPlay::CAVIAudioStream::GetSamples(LONGLONG llSampleNum)
{
	if (!m_pFile)
		return false;

	if (!m_pSrcFormat || m_dwSrcFormatSize == 0)
		return false;

	if (llSampleNum >= GetTotalSamples())
	{
		m_dwDstBufSizeUsed = 0;
		m_nCurrentChunksCount = 0;
		m_dwDstBufOffset = 0;
		m_dwNextChunk = m_dwChunksCount;
		m_llCurrentSample = GetTotalSamples();
		return false;
	}

	DWORD dwChunkNum;
	LONGLONG llByteNum;
	LONGLONG llSampleNumParam = llSampleNum;
	if (m_bVBR)
	{
		// Check
		if (!m_pUncompressedWaveFormat)
			return false;

		// From the Sample Position get the Chunk Position,
		// dwSampleNum & dwChunkNum are both updated!
		if (!VBRSampleToChunk(&llSampleNum, &dwChunkNum))
			return false;

		// Get the Chunk Samples
		if (!GetChunksSamples(dwChunkNum))
			return false;

		// Set the Offset
		m_dwDstBufOffset = (DWORD)llSampleNum * (DWORD)m_pUncompressedWaveFormat->nBlockAlign;

		// Set the Current Sample Pos
		m_llCurrentSample = llSampleNumParam;

		return true;
	}
	else
	{
		// Convert Samples Position to Bytes Position
		if (!CBRSampleToByte(llSampleNum, &llByteNum))
			return false;

		// From the Byte Position get the Chunk Position,
		// dwByteNum & dwChunkNum are both updated!
		if (!ByteToChunk(&llByteNum, &dwChunkNum))
			return false;

		// Get the Chunks Samples
		if (!GetChunksSamples(dwChunkNum))
			return false;

		if (((LPWAVEFORMATEX)m_pSrcFormat)->wFormatTag != WAVE_FORMAT_PCM)
		{
			// Check
			if (!m_pUncompressedWaveFormat)
				return false;

			// Calc. the Samples Offset
			CBRByteToSample(&llByteNum, &llSampleNum);

			// Set the Offset
			m_dwDstBufOffset = (DWORD)llSampleNum * (DWORD)m_pUncompressedWaveFormat->nBlockAlign;
		}
		else
		{
			// Set the Offset
			m_dwDstBufOffset = (DWORD)llByteNum;
		}

		// Set the Current Sample Pos
		m_llCurrentSample = llSampleNumParam;

		return true;
	}
}

/* Get Palette, as input pass the buffer size, as output it will return the effective palette size

Palette Format:

typedef struct
{
    BYTE          bFirstEntry;	// Specifies the index of the first palette entry to change
    BYTE          bNumEntries;	// Specifies the number of palette entries to change, or zero to change all 256 palette entries
    WORD          wFlags;		// Reserved
    PALETTEENTRY  peNew[];		// Size is: bNumEntries > 0 ? bNumEntries : 256
} AVIPALCHANGE;

typedef struct tagPALETTEENTRY {
    BYTE        peRed;
    BYTE        peGreen;
    BYTE        peBlue;
    BYTE        peFlags;
} PALETTEENTRY, *PPALETTEENTRY, FAR *LPPALETTEENTRY;

Different from the Palette passed after the BITMAPINFOHEADER:

typedef struct tagRGBQUAD {
    BYTE    rgbBlue; 
    BYTE    rgbGreen; 
    BYTE    rgbRed; 
    BYTE    rgbReserved; 
} RGBQUAD; 

*/

// Flips The Red And Blue Bytes of the palette
__forceinline void CAVIPlay::CAVIPaletteStream::RGBA2BGRA(void* palette, int entries)
{
	__asm
	{
		pushad
		mov ecx, entries				// Set Up A Counter
		mov ebx, palette				// Points ebx To Our Data (b)
		label:							// Label Used For Looping
			mov al,[ebx+0]				// Loads Value At ebx Into al
			mov ah,[ebx+2]				// Loads Value At ebx+2 Into ah
			mov [ebx+2],al				// Stores Value In al At ebx+2
			mov [ebx+0],ah				// Stores Value In ah At ebx
			
			add ebx,4					// Moves Through The Data By 4 Bytes
			dec ecx						// Decreases Our Loop Counter
			jnz label					// If Not Zero Jump Back To Label
		popad
	}
}

bool CAVIPlay::CAVIPaletteStream::GetPalette(DWORD dwChunkNum, LPBYTE pData, DWORD* pSize)
{
	// Check
	if (!pData || !pSize)
		return false;

	if (dwChunkNum < m_dwChangePaletteTableSize)
	{
		if (m_pChangePaletteTable[dwChunkNum] >= 0)
		{
			BYTE Buf[1028];
			DWORD dwSize = 1028;
			if (GetChunkData(m_pChangePaletteTable[dwChunkNum], (LPBYTE)Buf, &dwSize))
			{
				AVIPALCHANGE* pAviPalChange = (AVIPALCHANGE*)Buf;
				int nNumEntries = pAviPalChange->bNumEntries;
				if (nNumEntries == 0)
					nNumEntries = 256;
				RGBA2BGRA(pAviPalChange->peNew, nNumEntries);
				memcpy(	pData + 4 * pAviPalChange->bFirstEntry,
						pAviPalChange->peNew,
						MIN(*pSize - 4 * pAviPalChange->bFirstEntry, (DWORD)(4 * nNumEntries)));
				return true;
			}
			else
			{
				*pSize = 0;
				return false;
			}
		}
		else
		{
			*pSize = 0;
			return false;
		}
	}
	else
	{
		*pSize = 0;
		return false;
	}
}													

__forceinline int CAVIPlay::CAVIVideoStream::GetStride(LPBITMAPINFOHEADER pBMIH)
{
	int stride = ::CalcYUVStride(pBMIH->biCompression, (int)pBMIH->biWidth);
	if (stride > 0)
		return stride;
	else
		return DWALIGNEDWIDTHBYTES(	pBMIH->biBitCount *
									pBMIH->biWidth);
}

bool CAVIPlay::CAVIVideoStream::IsSrcFormatSupported()
{
	if (!m_pSrcFormat || (m_dwSrcFormatSize == 0))
		return false;

	if (m_hIC)
	{
		if (ICDecompressQuery(m_hIC, (LPBITMAPINFO)m_pSrcFormat, NULL) == ICERR_OK) 
			return true;
		else
			return false;
	}
	else if (m_pCodecCtx)
		return true;
	else
		return false;
}

bool CAVIPlay::CAVIVideoStream::IsDstFormatSupported(LPBITMAPINFO pDstBMI)
{
	if (!pDstBMI || !m_pSrcFormat || (m_dwSrcFormatSize == 0))
		return false;

	if (m_hIC)
	{
		if (ICDecompressQuery(m_hIC, (LPBITMAPINFO)m_pSrcFormat, pDstBMI) == ICERR_OK) 
			return true;
		else
			return false;
	}
	else if (m_pCodecCtx)
		return true;
	else
		return false;
}

bool CAVIPlay::CAVIVideoStream::InitDstFormat(LPBITMAPINFO pDstBMI/*=NULL*/)
{
	// Check
	if (!m_pSrcFormat || (m_dwSrcFormatSize == 0) || !m_hIC)
		return false;

	// Set to m_pDstBMI if NULL
	if (pDstBMI == NULL)
	{
		if (m_pDstBMI == NULL)
			return false;
		else
			pDstBMI = m_pDstBMI;
	}

	// Free?
	if ((pDstBMI != m_pDstBMI) && m_pDstBMI)
	{
		delete [] m_pDstBMI;
		m_pDstBMI = NULL;
	}

	// Get Destination Format Size
	if (m_pDstBMI == NULL)
	{
		m_dwDstFormatSize = ICDecompressGetFormatSize(m_hIC, (LPBITMAPINFO)m_pSrcFormat);
		if (m_dwDstFormatSize < sizeof(BITMAPINFOHEADER))
			m_dwDstFormatSize = sizeof(BITMAPINFOHEADER);
		m_pDstBMI = (LPBITMAPINFO)new BYTE[m_dwDstFormatSize];
		if (!m_pDstBMI)
		{
			m_dwDstFormatSize = 0;
			return false;
		}
		memset(m_pDstBMI, 0, m_dwDstFormatSize);
		memcpy(m_pDstBMI, pDstBMI, sizeof(BITMAPINFOHEADER));
	}
	
	// ICDecompressGetFormat is not well specified:
	// - Some Codecs always write the default destination format, with no regard
	//   to biBitCount and biCompression. I think this is the right way!
	// - Some Codecs look at biBitCount and biCompression, leave them (if supported)
	//   and adjust some other values like biClrUsed (like Microsoft Video 1).
	// - Some Codecs just want that this function is called, but they do not change
	//   the destination format if it is supported (like ffdshow).
	BITMAPINFOHEADER OrigBMIH;
	memcpy(&OrigBMIH, m_pDstBMI, sizeof(BITMAPINFOHEADER));
	if (ICDecompressGetFormat(m_hIC, (LPBITMAPINFO)m_pSrcFormat, m_pDstBMI) != ICERR_OK)
	{
		delete [] m_pDstBMI;
		m_dwDstFormatSize = 0;
		m_pDstBMI = NULL;
		return false;
	}
	else
	{
		// Now we have to restore the format for the right codecs,
		// because they will put the default format to m_pDstBMI!
		if ((m_pDstBMI->bmiHeader.biBitCount != OrigBMIH.biBitCount) ||
			(m_pDstBMI->bmiHeader.biCompression != OrigBMIH.biCompression))
			memcpy(m_pDstBMI, &OrigBMIH, sizeof(BITMAPINFOHEADER));
		return true;
	}
}

LPBITMAPINFO CAVIPlay::CAVIVideoStream::GetSuggestedFormat()
{
	// Check
	if (!m_pSrcFormat || (m_dwSrcFormatSize == 0) || !m_hIC)
		return NULL;

	// Get Suggested Format Size
	DWORD dwSuggestedFormatSize = ICDecompressGetFormatSize(m_hIC, (LPBITMAPINFO)m_pSrcFormat);
	if (dwSuggestedFormatSize < sizeof(BITMAPINFOHEADER))
		dwSuggestedFormatSize = sizeof(BITMAPINFOHEADER);
	LPBITMAPINFOHEADER pSuggestedBMIH = (LPBITMAPINFOHEADER)new BYTE[dwSuggestedFormatSize];
	if (!pSuggestedBMIH)
		return NULL;
	memset(pSuggestedBMIH, 0, dwSuggestedFormatSize);

	// Get Suggested Format
	if (ICDecompressGetFormat(m_hIC, (LPBITMAPINFO)m_pSrcFormat, (LPBITMAPINFO)pSuggestedBMIH) == ICERR_OK)
		return (LPBITMAPINFO)pSuggestedBMIH;
	else
	{
		delete [] pSuggestedBMIH;
		return NULL;
	}
}

bool CAVIPlay::CAVIVideoStream::PrepareWantedDstBMI(LPBITMAPINFOHEADER pDstBMIH)
{
	// Check
	if (!m_pSrcFormat || (m_dwSrcFormatSize == 0) || !pDstBMIH)
		return false;

	// Source Format Pointer
	LPBITMAPINFOHEADER pSrcBMIH = (LPBITMAPINFOHEADER)m_pSrcFormat;

	// Destination Format Init
	memset(pDstBMIH, 0 , sizeof(BITMAPINFOHEADER));
	pDstBMIH->biSize = sizeof(BITMAPINFOHEADER);
	pDstBMIH->biPlanes = 1;
	pDstBMIH->biWidth = pSrcBMIH->biWidth;
	pDstBMIH->biHeight = pSrcBMIH->biHeight;

	// Get Suggested Format
	LPBITMAPINFOHEADER pSuggestedBMIH = (LPBITMAPINFOHEADER)GetSuggestedFormat();

	// Decompress to YUV Format
	if (!m_bForceRgb)
	{
		// Set Flag
		bool bSet = true;

		// MPEG Family?
		if (IsYUV420Out(pSrcBMIH->biCompression))
		{
			pDstBMIH->biCompression = FCC('YV12');
			pDstBMIH->biBitCount = 12;
			int stride = ::CalcYUVStride(pDstBMIH->biCompression, (int)pDstBMIH->biWidth);
			pDstBMIH->biSizeImage = ::CalcYUVSize(pDstBMIH->biCompression, stride, (int)pDstBMIH->biHeight);
		}
		// MJPEG Family?
		else if (	pSrcBMIH->biCompression == FCC('MJPG')	||
					pSrcBMIH->biCompression == FCC('IJPG')	||
					pSrcBMIH->biCompression == FCC('dmb1')	||
					pSrcBMIH->biCompression == FCC('JPGL')	||
					pSrcBMIH->biCompression == FCC('QIVG')	||
					pSrcBMIH->biCompression == FCC('SP54')	||
					pSrcBMIH->biCompression == FCC('MJ2C'))
		{
			pDstBMIH->biCompression = FCC('YUY2');
			pDstBMIH->biBitCount = 16;
			int stride = ::CalcYUVStride(pDstBMIH->biCompression, (int)pDstBMIH->biWidth);
			pDstBMIH->biSizeImage = ::CalcYUVSize(pDstBMIH->biCompression, stride, (int)pDstBMIH->biHeight);
		}
		else
		{
			if (pSuggestedBMIH)
			{
				// YV12
				if (pSuggestedBMIH->biCompression == FCC('YV12'))
				{
					pDstBMIH->biCompression = FCC('YV12');
					pDstBMIH->biBitCount = 12;
					int stride = ::CalcYUVStride(pDstBMIH->biCompression, (int)pDstBMIH->biWidth);
					pDstBMIH->biSizeImage = ::CalcYUVSize(pDstBMIH->biCompression, stride, (int)pDstBMIH->biHeight);

				}
				// YUY2
				else if (pSuggestedBMIH->biCompression == FCC('YUY2'))
				{
					pDstBMIH->biCompression = FCC('YUY2');
					pDstBMIH->biBitCount = 16;
					int stride = ::CalcYUVStride(pDstBMIH->biCompression, (int)pDstBMIH->biWidth);
					pDstBMIH->biSizeImage = ::CalcYUVSize(pDstBMIH->biCompression, stride, (int)pDstBMIH->biHeight);
				}
				else
					bSet = false;
			}
			else
				bSet = false;
		}

		// Check
		if (bSet && IsDstFormatSupported((LPBITMAPINFO)pDstBMIH))
		{
			// Free
			FreeSuggestedFormat((LPBITMAPINFO)pSuggestedBMIH);

			// No change with display bpp change
			m_bNoBitCountChangeVCM = true;

			return true;
		}
	}

	// Do not set to display depth, because in case of a 16 or 24 bpp display
	// there may be some decoders which return buggy RGB16 and RGB24
	// images (no DWORD aligned lines) -> always use 32 bpp!
	//pDstBMIH->biBitCount = ::AfxGetMainFrame()->GetMonitorBpp();
	pDstBMIH->biBitCount = 32;
	pDstBMIH->biCompression = BI_RGB;
	pDstBMIH->biSizeImage = DWALIGNEDWIDTHBYTES(pDstBMIH->biBitCount	*
												pDstBMIH->biWidth)		*
												ABS(pDstBMIH->biHeight);

	// Try change with display bpp change
	m_bNoBitCountChangeVCM = false;

	// Display format supported?
	if (!IsDstFormatSupported((LPBITMAPINFO)pDstBMIH))
	{
		// Fall back to RGB24?
		pDstBMIH->biCompression = BI_RGB;
		pDstBMIH->biBitCount = 24;
		pDstBMIH->biSizeImage = DWALIGNEDWIDTHBYTES(pDstBMIH->biBitCount	*
													pDstBMIH->biWidth)		*
													ABS(pDstBMIH->biHeight);

		// If also RGB24 not supported use suggest one
		if (!IsDstFormatSupported((LPBITMAPINFO)pDstBMIH))
		{
			if (pSuggestedBMIH)
				memcpy(pDstBMIH, pSuggestedBMIH, sizeof(BITMAPINFOHEADER));
			else
			{
				// Panic! Do not know what to do with this codec...
				FreeSuggestedFormat((LPBITMAPINFO)pSuggestedBMIH);
				return false;
			}
		}
	}

	// Free
	FreeSuggestedFormat((LPBITMAPINFO)pSuggestedBMIH);

	return true;
}

bool CAVIPlay::CAVIVideoStream::IsYUV420Out(DWORD dwFourCC)
{
	enum AVCodecID id = AVCodecFourCCToCodecID(dwFourCC);
	if (id == AV_CODEC_ID_MPEG4			||
		id == AV_CODEC_ID_MSMPEG4V1		||
		id == AV_CODEC_ID_MSMPEG4V2		||
		id == AV_CODEC_ID_MSMPEG4V3		||
		id == AV_CODEC_ID_SNOW			||
		id == AV_CODEC_ID_WMV1			||
		id == AV_CODEC_ID_WMV2			||
		id == AV_CODEC_ID_FLV1			||
		id == AV_CODEC_ID_VP3			||
		id == AV_CODEC_ID_THEORA		||
		id == AV_CODEC_ID_VP5			||
		id == AV_CODEC_ID_VP6F			||
		id == AV_CODEC_ID_VP6			||
		id == AV_CODEC_ID_H261			||
		id == AV_CODEC_ID_H263			||
		id == AV_CODEC_ID_H263I			||
		id == AV_CODEC_ID_H264			||
		id == AV_CODEC_ID_VC1			||
		id == AV_CODEC_ID_FFVHUFF		||
		id == AV_CODEC_ID_FFV1			||
		id == AV_CODEC_ID_MPEG1VIDEO	||
		id == AV_CODEC_ID_MPEG2VIDEO)
		return true;
	else
		return false;
}

bool CAVIPlay::CAVIVideoStream::FindDecompressorVCM()
{
	bool bFoundDecompressor = false;
	BITMAPINFOHEADER DstBMIH;
	for (int i = 0 ; ICInfo(ICTYPE_VIDEO, i, &m_IcInfo) == TRUE ; i++) 
	{ 
		m_hIC = ICOpen(m_IcInfo.fccType, m_IcInfo.fccHandler, ICMODE_QUERY); 
		if (m_hIC)
		{
			// Skip this compressor if it can't handle the src format
			if (!IsSrcFormatSupported())
			{ 
				ICClose(m_hIC);
				m_hIC = NULL;
				continue; 
			}

			// Prepare Wanted Dib Format
			if (!PrepareWantedDstBMI((LPBITMAPINFOHEADER)&DstBMIH))
			{
				ICClose(m_hIC);
				m_hIC = NULL;
				continue;
			}

			// Init Destination Format
			if (!InitDstFormat((LPBITMAPINFO)&DstBMIH))
			{ 
				ICClose(m_hIC);
				m_hIC = NULL;
				continue; 
			}

			// Set Found Flag
			bFoundDecompressor = true;

			// Close Query
			ICClose(m_hIC); // Application verifier asserts here with "Locks violation detected"...
			m_hIC = NULL;

			break;
		} 
	}

	return bFoundDecompressor;
}

bool CAVIPlay::CAVIVideoStream::OpenDecompression(bool bForceRgb)
{
	// Clean-up
	Free();

	// Check
	if (!m_pSrcFormat || (m_dwSrcFormatSize == 0))
		return false;

	// Re-opening some codecs will crash the program
	// if biSizeImage does not contain the original value.
	// Remember that this value is changed each time
	// we call ICDecompress() to indicate the amount of
	// data to decompress.
	if (m_pOrigSrcBMI)	// Restore
		memcpy(m_pSrcFormat, m_pOrigSrcBMI, MIN(m_dwOrigSrcBMISize, m_dwSrcFormatSize));
	else				// Backup
	{
		m_pOrigSrcBMI = (LPBITMAPINFO)new BYTE[m_dwSrcFormatSize];
		if (!m_pOrigSrcBMI)
			return false;
		m_dwOrigSrcBMISize = m_dwSrcFormatSize;
		memcpy(m_pOrigSrcBMI, m_pSrcFormat, m_dwSrcFormatSize);
	}

	// Set whether to decompress to RGB only
	m_bForceRgb = bForceRgb;

	// Src Video Format Pointer
	LPBITMAPINFOHEADER pSrcBMIH = (LPBITMAPINFOHEADER)m_pSrcFormat;

	// Correct the wrong size of some avi files
	if (pSrcBMIH->biCompression == BI_RGB		||
		pSrcBMIH->biCompression == BI_BITFIELDS ||
		IsRLE(pSrcBMIH->biCompression))
		pSrcBMIH->biSize = sizeof(BITMAPINFOHEADER);

	// Decompression Not Needed?
	if (pSrcBMIH->biCompression == BI_RGB		||
		pSrcBMIH->biCompression == BI_BITFIELDS ||
		IsRLE(pSrcBMIH->biCompression)			||
		pSrcBMIH->biCompression == BI_RGB16		||
		pSrcBMIH->biCompression == BI_BGR16		||
		pSrcBMIH->biCompression == BI_RGB15		||
		pSrcBMIH->biCompression == BI_BGR15		||
		(!bForceRgb								&&
		(pSrcBMIH->biCompression == FCC('YV12')	||
		pSrcBMIH->biCompression == FCC('YUY2'))))
	{
		// Change to BI_BITFIELDS
		if (pSrcBMIH->biCompression == BI_RGB16	||
			pSrcBMIH->biCompression == BI_BGR16)
		{
			if (m_pSrcFormat)
				delete [] m_pSrcFormat;
			m_dwSrcFormatSize = sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD);
			m_pSrcFormat = new BYTE[m_dwSrcFormatSize];
			if (!m_pSrcFormat)
				return false;
			memcpy(m_pSrcFormat, m_pOrigSrcBMI, MIN(m_dwOrigSrcBMISize, m_dwSrcFormatSize));
			pSrcBMIH = (LPBITMAPINFOHEADER)m_pSrcFormat;
			LPBYTE pDstMask = m_pSrcFormat + sizeof(BITMAPINFOHEADER);
			*((DWORD*)(pDstMask)) = 0xF800; // Red Mask
			pDstMask = pDstMask + sizeof(DWORD);
			*((DWORD*)(pDstMask)) = 0x07E0; // Green Mask
			pDstMask = pDstMask + sizeof(DWORD);
			*((DWORD*)(pDstMask)) = 0x001F; // Blue Mask

			pSrcBMIH->biBitCount = 16;
			pSrcBMIH->biCompression = BI_BITFIELDS;
			pSrcBMIH->biSizeImage = DWALIGNEDWIDTHBYTES(pSrcBMIH->biWidth * pSrcBMIH->biBitCount) * ABS(pSrcBMIH->biHeight);
		}
		// Change to BI_RGB
		else if (	pSrcBMIH->biCompression == BI_RGB15	||
					pSrcBMIH->biCompression == BI_BGR15)
		{
			pSrcBMIH->biBitCount = 16;
			pSrcBMIH->biCompression = BI_RGB;
			pSrcBMIH->biSizeImage = DWALIGNEDWIDTHBYTES(pSrcBMIH->biWidth * pSrcBMIH->biBitCount) * ABS(pSrcBMIH->biHeight);
		}
		// Correct the size, some avi files have a wrong size
		else if (	pSrcBMIH->biCompression == BI_RGB		||
					pSrcBMIH->biCompression == BI_BITFIELDS)
			pSrcBMIH->biSizeImage = DWALIGNEDWIDTHBYTES(pSrcBMIH->biWidth * pSrcBMIH->biBitCount) * ABS(pSrcBMIH->biHeight);

		// Allocate Buffer
		m_dwSrcBufSize =	4 * pSrcBMIH->biWidth *
							ABS(pSrcBMIH->biHeight);
		m_pSrcBuf = (LPBYTE)av_malloc(m_dwSrcBufSize);
		if (!m_pSrcBuf)
			return false;

		// Set Flags
		m_bNoDecompression = true;
		m_bHasDecompressor = true;
		m_bNoBitCountChangeVCM = true;

		return true;
	}
	else if (::IsSupportedYuvToRgbFormat(pSrcBMIH->biCompression))
	{	
		m_dwDstFormatSize = sizeof(BITMAPINFOHEADER);
		m_pDstBMI = (LPBITMAPINFO)new BYTE[m_dwDstFormatSize];
		if (!m_pDstBMI)
			return false;
		memset(m_pDstBMI, 0, m_dwDstFormatSize);
		m_pDstBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_pDstBMI->bmiHeader.biBitCount = 32;
		m_pDstBMI->bmiHeader.biPlanes = 1;
		m_pDstBMI->bmiHeader.biHeight = pSrcBMIH->biHeight;
		m_pDstBMI->bmiHeader.biWidth = pSrcBMIH->biWidth;
		m_pDstBMI->bmiHeader.biCompression = BI_RGB;
		m_pDstBMI->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(	m_pDstBMI->bmiHeader.biBitCount	*
																m_pDstBMI->bmiHeader.biWidth)	*
																ABS(m_pDstBMI->bmiHeader.biHeight);
		m_pSrcBuf = (LPBYTE)av_malloc(m_dwSrcBufSize = pSrcBMIH->biSizeImage);
		if (!m_pSrcBuf)
			return false;
		m_pDstBuf = (LPBYTE)av_malloc(m_dwDstBufSize = m_pDstBMI->bmiHeader.biSizeImage);
		if (!m_pDstBuf)
			return false;
		m_bYuvToRgb32 = true;
		m_bHasDecompressor = true;
		m_bNoBitCountChangeVCM = true;
		return true;
	}

	// Find Decompressor
	bool bFoundDecompressorVCM = false;
	if (m_pAVIPlay->m_bAVCodecPriority)
	{
		if (OpenDecompressionAVCodec())
		{
			// Decode the first frame
			DWORD dwNextFrame = m_dwNextFrame;
			Rew();
			SkipFrame(1, TRUE);
			Rew();
			m_dwNextFrame = dwNextFrame;
			return true;
		}
		else
			bFoundDecompressorVCM = FindDecompressorVCM();
	}
	else
	{
		bFoundDecompressorVCM = FindDecompressorVCM();
		if (!bFoundDecompressorVCM)
		{
			if (OpenDecompressionAVCodec())
			{
				// Decode the first frame
				DWORD dwNextFrame = m_dwNextFrame;
				Rew();
				SkipFrame(1, TRUE);
				Rew();
				m_dwNextFrame = dwNextFrame;
				return true;
			}
		}
	}
	if (!bFoundDecompressorVCM)
	{
		// Error Message
		CString str;
		str.Format(_T("No Video Codec Installed For The FourCC: %s\n"),
					GetFourCCString(true));
#ifdef _DEBUG
		str = _T("OpenDecompression():\n") + str;
#endif
		TRACE(str);
		if (m_pAVIPlay->IsShowMessageBoxOnError())
			::AfxMessageBox(str, MB_ICONSTOP);

		Free();
		return false;
	}

	// Open
	m_hIC = ICOpen(m_IcInfo.fccType, m_IcInfo.fccHandler, ICMODE_DECOMPRESS);
	if (m_hIC == NULL)
	{
		Free();
		return false;
	}

	// Get the compressor name
	::memset(&m_IcInfo, 0, sizeof(ICINFO));
	ICGetInfo(m_hIC, &m_IcInfo, sizeof(m_IcInfo));
	
	// Set Format
	// (Necessary for some codecs like ffdshow)
	if (!InitDstFormat())
	{
		Free();
		return false;
	}

	// Allocate Src & Dst Buffers
	DWORD dwBufSize =	8 * m_pDstBMI->bmiHeader.biWidth * // Be Safe!
						ABS(m_pDstBMI->bmiHeader.biHeight);
	m_pSrcBuf = (LPBYTE)av_malloc(m_dwSrcBufSize = dwBufSize);
	if (!m_pSrcBuf)
	{
		Free();
		return false;
	}
	m_pDstBuf = (LPBYTE)av_malloc(m_dwDstBufSize = dwBufSize);
	if (!m_pDstBuf)
	{
		Free();
		return false;
	}

	// Init Decompressor
	if (ICDecompressBegin(m_hIC, (LPBITMAPINFO)pSrcBMIH, m_pDstBMI) != ICERR_OK) 
	{
		Free();
		return false;
	}

	// To init some codecs like XVID
	// we have to decode the first frame
	DWORD dwNextFrame = m_dwNextFrame;
	Rew();
	SkipFrame(1, TRUE);
	Rew();
	m_dwNextFrame = dwNextFrame;

	// Set Flag
	m_bHasDecompressor = true;

	return true;
}

bool CAVIPlay::CAVIVideoStream::OpenDecompressionAVCodec()
{
	// Src Video Format Pointer
	LPBITMAPINFOHEADER pSrcBMIH = (LPBITMAPINFOHEADER)m_pSrcFormat;

	// Free
	FreeAVCodec();

	// Get the codec id for the video stream
	DWORD dwFourCC = GetFourCC(true);
	AVCodecID id = AVCodecFourCCToCodecID(dwFourCC);

	// Always use mpeg2 decoder
	if (id == AV_CODEC_ID_MPEG1VIDEO)
		id = AV_CODEC_ID_MPEG2VIDEO;

    // Find the decoder for the video stream
	m_pCodec = avcodec_find_decoder(id);
    if (!m_pCodec)
        goto error;

	// Allocate Context
	/* if m_pCodec non-NULL, allocate private data and initialize defaults
	 * for the given codec. It is illegal to then call avcodec_open2()
	 * with a different codec.
	 * If NULL, then the codec-specific defaults won't be initialized,
	 * which may result in suboptimal default settings (this is
	 * important mainly for encoders, e.g. libx264).
	 */
	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (!m_pCodecCtx)
		goto error;

	// Set Width & Height
	m_pCodecCtx->coded_width = GetWidth();
	m_pCodecCtx->coded_height = GetHeight();

	// Some Decoders need that (like HuffYUV, TSCC)
	m_pCodecCtx->bits_per_coded_sample = pSrcBMIH->biBitCount;

	// Set FourCC
	m_pCodecCtx->codec_tag = dwFourCC;

	// Extra data
	if (m_dwSrcFormatSize > sizeof(BITMAPINFOHEADER))
	{
		m_pCodecCtx->extradata_size = m_dwSrcFormatSize - sizeof(BITMAPINFOHEADER);
		m_pCodecCtx->extradata = (uint8_t*)av_malloc(	m_pCodecCtx->extradata_size +
														FF_INPUT_BUFFER_PADDING_SIZE);
		if (m_pCodecCtx->extradata)
		{
			memcpy(	m_pCodecCtx->extradata,
					(LPBYTE)m_pSrcFormat + sizeof(BITMAPINFOHEADER),
					m_pCodecCtx->extradata_size);
			if (id == AV_CODEC_ID_MPEG1VIDEO || id == AV_CODEC_ID_MPEG2VIDEO)
				m_bAVDecodeExtraData = true;
			else
				m_bAVDecodeExtraData = false;
		}
		else
			m_pCodecCtx->extradata_size = 0;
	}

	// In case of buggy source header
	if (id == AV_CODEC_ID_MSVIDEO1)
		m_pCodecCtx->bits_per_coded_sample = m_pCodecCtx->extradata_size > 0 ? 8 : 16;

	// Special handling for palettized videos
	if (m_pCodecCtx->extradata_size > 0			&&
		m_pCodecCtx->bits_per_coded_sample <= 8	&&
		(id == CODEC_ID_MSRLE					||
		id == CODEC_ID_CSCD						||
		id == CODEC_ID_MSVIDEO1					||
		id == CODEC_ID_PNG						||
		id == CODEC_ID_QTRLE					||
		id == CODEC_ID_TSCC						||
		id == CODEC_ID_QPEG))
	{
		memcpy(	m_AVPalette, m_pCodecCtx->extradata,
				MIN(m_pCodecCtx->extradata_size, AVPALETTE_SIZE));
		m_bAVPaletteChanged = true;
	}

    // Open codec
    if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
        goto error;

    // Allocate video frames
    m_pFrame = av_frame_alloc();
	if (!m_pFrame)
        goto error;
    m_pFrameGdi = av_frame_alloc();
    if (!m_pFrameGdi)
        goto error;

	// Prepare Wanted Dib Format
	if (m_pDstBMI)
		delete [] m_pDstBMI;
	m_dwDstFormatSize = sizeof(BITMAPINFOHEADER);
	m_pDstBMI = (LPBITMAPINFO)new BYTE[m_dwDstFormatSize];
	if (!m_pDstBMI)
		goto error;
	if (!PrepareWantedDstBMI((LPBITMAPINFOHEADER)m_pDstBMI))
		goto error;

	// Allocate Src Buffer, be safe!
	m_dwSrcBufSize =	4 * pSrcBMIH->biWidth *
						ABS(pSrcBMIH->biHeight);
	if (m_pSrcBuf)
		av_free(m_pSrcBuf);
	m_pSrcBuf = (LPBYTE)av_malloc(m_dwSrcBufSize + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!m_pSrcBuf)
		goto error;

	// Set Flag
	m_bHasDecompressor = true;

	return true;

error:
	FreeAVCodec();
	return false;
}

void CAVIPlay::CAVIVideoStream::FreeAVCodec()
{
	if (m_pCodecCtx)
	{
		/*
		Close a given AVCodecContext and free all the data associated with it
		(but not the AVCodecContext itself).
		Calling this function on an AVCodecContext that hasn't been opened will free
		the codec-specific data allocated in avcodec_alloc_context3() /
		avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
		do nothing.
		*/
		avcodec_close_thread_safe(m_pCodecCtx);

		// Free
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrameGdi)
		av_frame_free(&m_pFrameGdi);
	if (m_pFrame)
		av_frame_free(&m_pFrame);
	
	m_bAVDecodeExtraData = false;
	m_bAVPaletteChanged = false;
	memset(&m_AVPalette, 0, AVPALETTE_SIZE);

	if (m_pImgConvertCtxGdi)
	{
		sws_freeContext(m_pImgConvertCtxGdi);
		m_pImgConvertCtxGdi = NULL;
	}
}

__forceinline bool CAVIPlay::CAVIVideoStream::AVCodecDecompressDib(bool bKeyFrame, bool bSeek)
{
	// Check
	if (!m_pCodecCtx)
		return false;

	// Flush Buffers
	if (bSeek)
		avcodec_flush_buffers(m_pCodecCtx);

	// Reset Frame Structure
	av_frame_unref(m_pFrame);

	// Set Key Frame, not necessary ... but it does not harm
	m_pFrame->key_frame = bKeyFrame ? 1 : 0;

	// Decode
	int got_picture;
	memset(	m_pSrcBuf + ((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage,
			0,
			FF_INPUT_BUFFER_PADDING_SIZE);
	if (m_bAVDecodeExtraData)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = m_pCodecCtx->extradata;
		pkt.size = m_pCodecCtx->extradata_size;
		got_picture = 0;
		avcodec_decode_video2(	m_pCodecCtx,
								m_pFrame,
								&got_picture,
								&pkt);
		av_free_packet(&pkt);
		m_bAVDecodeExtraData = false;
	}
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = (uint8_t*)m_pSrcBuf;
	pkt.size = ((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage;
	if (m_bAVPaletteChanged)
	{
		uint8_t* pal = av_packet_new_side_data(&pkt, AV_PKT_DATA_PALETTE, AVPALETTE_SIZE);
		if (pal)
			memcpy(pal, m_AVPalette, AVPALETTE_SIZE);
		m_bAVPaletteChanged = false;
	}
	got_picture = 0;
	int len = avcodec_decode_video2(m_pCodecCtx,
									m_pFrame,
									&got_picture,
									&pkt);
	av_free_packet(&pkt);
	if (m_pCodecCtx->has_b_frames)
		m_nOneFrameDelay = 1;
    if (len < 0)
		return true; // be tolerant!

	// Init Color Space Convert Context
	if (!m_pImgConvertCtxGdi)
	{
		// Make Sure Destination Format is set correctly
		if (!m_bForceRgb)
		{
			if (m_pCodecCtx->pix_fmt == AV_PIX_FMT_YUV420P	||
				m_pCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ420P)
			{
				m_pDstBMI->bmiHeader.biCompression = FCC('YV12');
				m_pDstBMI->bmiHeader.biBitCount = 12;
				int stride = ::CalcYUVStride(m_pDstBMI->bmiHeader.biCompression,
											(int)m_pDstBMI->bmiHeader.biWidth);
				m_pDstBMI->bmiHeader.biSizeImage = ::CalcYUVSize(	m_pDstBMI->bmiHeader.biCompression,
																	stride,
																	(int)m_pDstBMI->bmiHeader.biHeight);
			}
			else if (	m_pCodecCtx->pix_fmt == AV_PIX_FMT_YUYV422	||	// Note: AV_PIX_FMT_YUV422 = AV_PIX_FMT_YUYV422
						m_pCodecCtx->pix_fmt == AV_PIX_FMT_YUV422P	||
						m_pCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ422P)
			{
				m_pDstBMI->bmiHeader.biCompression = FCC('YUY2');
				m_pDstBMI->bmiHeader.biBitCount = 16;
				int stride = ::CalcYUVStride(m_pDstBMI->bmiHeader.biCompression,
											(int)m_pDstBMI->bmiHeader.biWidth);
				m_pDstBMI->bmiHeader.biSizeImage = ::CalcYUVSize(	m_pDstBMI->bmiHeader.biCompression,
																	stride,
																	(int)m_pDstBMI->bmiHeader.biHeight);
			}
		}

		// Get Pix Format
		enum AVPixelFormat pix_fmt = AVCodecBMIToPixFormat(m_pDstBMI);
		
		// Determine required buffer size and allocate buffer if necessary
		int nBufSize = avpicture_get_size(	pix_fmt,
											m_pCodecCtx->width,
											m_pCodecCtx->height);
		if ((int)(m_dwDstBufSize) < nBufSize || m_pDstBuf == NULL)
		{
			if (m_pDstBuf)
				av_free(m_pDstBuf);
			m_pDstBuf = (LPBYTE)av_malloc(nBufSize + FF_INPUT_BUFFER_PADDING_SIZE);
			if (!m_pDstBuf)
				return false;
			m_dwDstBufSize = nBufSize;
			ClearDstBuf();
		}

		// Assign appropriate parts of buffer to image planes
		avpicture_fill((AVPicture*)m_pFrameGdi,
						(unsigned __int8 *)m_pDstBuf,
						pix_fmt,
						m_pCodecCtx->width,
						m_pCodecCtx->height);

		// Prepare Image Conversion Context
		if (m_pCodecCtx->pix_fmt != AV_PIX_FMT_NONE)
		{
			m_pImgConvertCtxGdi = sws_getContext(	GetWidth(),				// Source Width
													GetHeight(),			// Source Height
													m_pCodecCtx->pix_fmt,	// Source Format
													GetWidth(),				// Destination Width
													GetHeight(),			// Destination Height
													pix_fmt,				// Destination Format
													SWS_BICUBIC,			// Interpolation
													NULL,					// No Source Filter
													NULL,					// No Destination Filter
													NULL);					// Param
			if (!m_pImgConvertCtxGdi)
				return false;
		}

		// Flip U <-> V pointers
		if (pix_fmt == AV_PIX_FMT_YUV420P)
		{
			uint8_t* pTemp = m_pFrameGdi->data[1];
			m_pFrameGdi->data[1] = m_pFrameGdi->data[2];
			m_pFrameGdi->data[2] = pTemp;
			// Line Sizes for U and V are the same no need to swap
		}
	}

	// Color Space Conversion
	// (first try fast conversion, if not supported fall back to sws_scale)
	if (got_picture && m_pFrame->data[0] && m_pImgConvertCtxGdi)
	{
		BOOL bOk = ITU601JPEGConvert(m_pCodecCtx->pix_fmt,				// Source Format
									AVCodecBMIToPixFormat(m_pDstBMI),	// Destination Format
									m_pFrame->data,						// Source Data
									m_pFrame->linesize,					// Source Stride
									m_pFrameGdi->data,					// Destination Data
									m_pFrameGdi->linesize,				// Destination Stride
									GetWidth(),							// Width
									GetHeight());						// Height
		if (!bOk)
		{
			bOk = sws_scale(m_pImgConvertCtxGdi,						// Image Convert Context
							m_pFrame->data,								// Source Data
							m_pFrame->linesize,							// Source Stride
							0,											// Source Slice Y
							GetHeight(),								// Source Height
							m_pFrameGdi->data,							// Destination Data
							m_pFrameGdi->linesize) > 0;					// Destination Stride
		}
		return bOk ? true : false;
	}
	else
		return true;
}

bool CAVIPlay::CAVIVideoStream::ReOpenDecompressVCM()
{
	m_nLastDecompressedDibFrame = -2;
	if (m_hIC)
	{
		// Close
		ICDecompressEnd(m_hIC);
		ICClose(m_hIC);

		// Re-opening some codecs will crash the program
		// if biSizeImage does not contain the original value.
		// Remember that this value is changed each time
		// we call ICDecompress() to indicate the amount of
		// data to decompress.
		if (m_pOrigSrcBMI)	// Restore
			memcpy(m_pSrcFormat, m_pOrigSrcBMI, MIN(m_dwOrigSrcBMISize, m_dwSrcFormatSize));
		else				// Backup
		{
			m_pOrigSrcBMI = (LPBITMAPINFO)new BYTE[m_dwSrcFormatSize];
			if (!m_pOrigSrcBMI)
				return false;
			m_dwOrigSrcBMISize = m_dwSrcFormatSize;
			memcpy(m_pOrigSrcBMI, m_pSrcFormat, m_dwSrcFormatSize);
		}

		// Re-Open
		m_hIC = ICOpen(m_IcInfo.fccType, m_IcInfo.fccHandler, ICMODE_DECOMPRESS);

		// Init Destination Format
		if (!InitDstFormat())
		{ 
			ICClose(m_hIC);
			m_hIC = NULL; 
		}
		
		// Check and Begin Decompression
		if (m_hIC == NULL)
			return false;
		else
		{
			if (ICDecompressBegin(m_hIC, (LPBITMAPINFO)m_pSrcFormat, m_pDstBMI) != ICERR_OK)
				return false;
			else
			{
				// To init some codecs like XVID
				// we have to read the first frame
				DWORD dwNextFrame = m_dwNextFrame;
				Rew();
				SkipFrame(1, TRUE);
				Rew();
				m_dwNextFrame = dwNextFrame;
				return true;
			}
		}
	}
	else
		return false;
}

void CAVIPlay::CAVIVideoStream::Free()
{
	// Close Decompressor
	if (m_hIC)
	{
		ICDecompressEnd(m_hIC);
		ICClose(m_hIC);
		m_hIC = NULL;
		::memset(&m_IcInfo, 0, sizeof(ICINFO));
	}

	// Free LIBAVCODEC
	FreeAVCodec();

	// Clean-Up
	if (m_pSrcBuf)
	{
		av_free(m_pSrcBuf);
		m_pSrcBuf = NULL;
	}
	if (m_pDstBuf)
	{
		av_free(m_pDstBuf);
		m_pDstBuf = NULL;
	}
	if (m_pDstBMI)
	{
		delete [] m_pDstBMI;
		m_pDstBMI = NULL;
	}
	if (m_pPrevRLEDib)
	{
		delete m_pPrevRLEDib;
		m_pPrevRLEDib = NULL;
	}
	if (m_pCurrentRLEDib)
	{
		delete m_pCurrentRLEDib;
		m_pCurrentRLEDib = NULL;
	}

	m_dwSrcBufSize = 0;
	m_dwDstBufSize = 0;
	m_dwDstFormatSize = 0;
	m_nLastDecompressedDibFrame = -2;
	m_bNoDecompression = false;
	m_bYuvToRgb32 = false;
	m_bHasDecompressor = false;
	m_nOneFrameDelay = 0;
}

void CAVIPlay::CAVIVideoStream::SetFrameRate(DWORD dwRate, DWORD dwScale)
{
	m_Hdr.dwRate = dwRate;
	m_Hdr.dwScale = dwScale;
}

bool CAVIPlay::CAVIVideoStream::SetCurrentFramePos(int nCurrentFramePos)
{
	if ((nCurrentFramePos < -1) ||
		(nCurrentFramePos >= (int)GetTotalFrames()))
		return false;
	else
	{
		m_dwNextFrame = nCurrentFramePos + 1 + m_nOneFrameDelay;
		return true;
	}
}

bool CAVIPlay::CAVIVideoStream::ChangeGetFrameBitsVCM(WORD wNewBitCount)
{
	// Check
	if (!m_pSrcFormat || (m_dwSrcFormatSize == 0))
		return false;

	// Src Format
	LPBITMAPINFOHEADER pSrcBMIH = (LPBITMAPINFOHEADER)m_pSrcFormat;

	// Dst Format
	BITMAPINFOHEADER DstBMIH;
	memset(&DstBMIH, 0 , sizeof(DstBMIH));
	DstBMIH.biSize = sizeof(DstBMIH);
	DstBMIH.biBitCount = wNewBitCount;
	DstBMIH.biPlanes = 1;
	DstBMIH.biHeight = pSrcBMIH->biHeight;
	DstBMIH.biWidth = pSrcBMIH->biWidth;
	DstBMIH.biCompression = BI_RGB;
	DstBMIH.biSizeImage = DWALIGNEDWIDTHBYTES(	DstBMIH.biBitCount	*
												DstBMIH.biWidth)	*
												ABS(DstBMIH.biHeight);

	// Try Changing Format
	if (IsDstFormatSupported((LPBITMAPINFO)&DstBMIH) &&
		InitDstFormat((LPBITMAPINFO)&DstBMIH))
	{
		if (m_pDstBMI->bmiHeader.biBitCount != wNewBitCount)
			m_bNoBitCountChangeVCM = true;
		return ReOpenDecompressVCM();
	}

	// If not possible disable change
	m_bNoBitCountChangeVCM = true;
	
	return false;
}

bool CAVIPlay::CAVIVideoStream::SkipFrame(int nNumOfFrames/*=1*/, BOOL bForceDecompress/*=FALSE*/)
{
	// Simple cases
	if (nNumOfFrames < 1)
		return true;
	else if (nNumOfFrames == 1)
	{
		::EnterCriticalSection(&m_pAVIPlay->m_csAVI);
		bool res = SkipFrameHelper(bForceDecompress);
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return res;
	}

	// Enter CS
	::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

	int nNewNextFrame = m_dwNextFrame + nNumOfFrames - 1;
	int nPrevKeyFrame = GetPrevKeyFrame(nNewNextFrame);
	int nSkipCount;
	if (nPrevKeyFrame <= (int)m_dwNextFrame)
		nSkipCount = nNumOfFrames;
	else
	{
		nSkipCount = nNewNextFrame - nPrevKeyFrame + 1;
		m_dwNextFrame = nPrevKeyFrame;
	}
	while (nSkipCount--)
	{
		if (!SkipFrameHelper(bForceDecompress))
		{
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}
	}
	
	// Leave CS
	::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

	// Make sure it is correct
	ASSERT(nNewNextFrame + 1 == (int)m_dwNextFrame);

	return true;
}

// Do not use it directly, has no CS!
__forceinline bool CAVIPlay::CAVIVideoStream::SkipFrameHelper(BOOL bForceDecompress)
{
	// If end of File -> return
	if ((GetTotalFrames() > 0) &&
		(m_dwNextFrame >= GetTotalFrames()))
		return false;

	// Reset
	m_nLastDecompressedDibFrame = -2;

	// Decompress?
	if (bForceDecompress ||
		(!m_bNoDecompression && !m_bYuvToRgb32 && GetTotalFrames() > GetTotalKeyFrames()))
	{
		// Get Compressed Data
		DWORD dwSrcBufSizeUsed = m_dwSrcBufSize;
		if (GetChunkData(	m_dwNextFrame,
							m_pSrcBuf,
							&dwSrcBufSizeUsed))
		{
			UpdatePalette(m_dwNextFrame);

			// Get the Frame
			// Note: ICDecompress() returns ICERR_OK or ICERR_DONTDRAW,
			// which have the values 0 and 1 ->
			// - Positive return values are OK.
			// - Negative return values are Errors.
			((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
			if (m_hIC)
			{
				if ((int)ICDecompress(	m_hIC,
										(IsKeyFrame(m_dwNextFrame) ? 0 : ICDECOMPRESS_NOTKEYFRAME) | ICDECOMPRESS_PREROLL,
										(LPBITMAPINFOHEADER)m_pSrcFormat,
										(LPVOID)m_pSrcBuf,                  
										(LPBITMAPINFOHEADER)m_pDstBMI,        
										(LPVOID)m_pDstBuf) >= 0)
					m_nLastDecompressedDibFrame = (int)m_dwNextFrame;
			}
			else
			{
				if (AVCodecDecompressDib(IsKeyFrame(m_dwNextFrame), false))
					m_nLastDecompressedDibFrame = (int)m_dwNextFrame;
			}
		}
	}
	
	// Inc. Pos
	m_dwNextFrame++;

	return true;
}

bool CAVIPlay::CAVIVideoStream::GetUncompressedFrameAt(	CDib* pDib,
														DWORD dwFrame,
														volatile int& nLastDecompressedFrame)
{
	// Check
	if (!pDib)
		return false;
	
	// Enter CS
	::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

	// Check Range
	if (dwFrame >= GetTotalFrames())
	{
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Get Frame?
	if (nLastDecompressedFrame != (int)dwFrame)
	{
		DWORD dwSrcBufSizeUsed = 0;

		// Get Frame
		if (((dwFrame == m_dwNextFrame)								&&
			((nLastDecompressedFrame + 1) == (int)dwFrame))	||
			!IsRLE(((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression))
		{
			// Reset
			nLastDecompressedFrame = -2;

			dwSrcBufSizeUsed = m_dwSrcBufSize;
			if (!GetChunkData(	dwFrame,
								m_pSrcBuf,
								&dwSrcBufSizeUsed))
			{
				pDib->Free();
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return false;
			}
			UpdatePalette(dwFrame);
			((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
			if (!GetUncompressedDib(pDib))
			{
				pDib->Free();
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return false;
			}
		}
		// Get Frame At
		else
		{
			int nPrevKeyFrame = GetPrevKeyFrame(dwFrame);
			if (nPrevKeyFrame == -1)
			{
				nLastDecompressedFrame = -2;
				pDib->Free();
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return false;
			}
			else if (nPrevKeyFrame < (int)dwFrame && ((int)dwFrame - nPrevKeyFrame < MAX_KEYFRAMES_SPACING))
			{
				// Reset
				nLastDecompressedFrame = -2;

				// Get Key-Frame
				dwSrcBufSizeUsed = m_dwSrcBufSize;
				if (!GetChunkData(	nPrevKeyFrame,
									m_pSrcBuf,
									&dwSrcBufSizeUsed))
				{
					pDib->Free();
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
				UpdatePalette(nPrevKeyFrame);
				((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
				if (!GetUncompressedDib(pDib))
				{
					pDib->Free();
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}

				// Get Middle Delta Frames
				for (int i = nPrevKeyFrame + 1 ; i < (int)dwFrame ; i++)
				{
					dwSrcBufSizeUsed = m_dwSrcBufSize;
					if (!GetChunkData(	i,
										m_pSrcBuf,
										&dwSrcBufSizeUsed))
					{
						pDib->Free();
						::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
						return false;
					}
					UpdatePalette(i);
					((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
					if (!GetUncompressedDib(pDib))
					{
						pDib->Free();
						::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
						return false;
					}
				}

				// Get Wanted Delta Frame
				dwSrcBufSizeUsed = m_dwSrcBufSize;
				if (!GetChunkData(	dwFrame,
									m_pSrcBuf,
									&dwSrcBufSizeUsed))
				{
					pDib->Free();
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
				UpdatePalette(dwFrame);
				((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
				if (!GetUncompressedDib(pDib))
				{
					pDib->Free();
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
			}
			else
			{
				// Reset
				nLastDecompressedFrame = -2;

				// Get Key-Frame
				dwSrcBufSizeUsed = m_dwSrcBufSize;
				if (!GetChunkData(	dwFrame,
									m_pSrcBuf,
									&dwSrcBufSizeUsed))
				{
					pDib->Free();
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
				UpdatePalette(dwFrame);
				((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
				if (!GetUncompressedDib(pDib))
				{
					pDib->Free();
					::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
					return false;
				}
			}
		}
	}

	// Update Last Decompressed Frame
	nLastDecompressedFrame = (int)dwFrame;

	// Inc. Pos
	m_dwNextFrame = dwFrame + 1;

	// Leave CS
	::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

	return true;
}

bool CAVIPlay::CAVIVideoStream::GetUncompressedDib(CDib* pDib)
{
	// Some RLE Videos have empty chunks or
	// a 2 bytes end of bitmap RLE sequence
	// to indicate a repetition of the last frame
	if (((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage == 0	||
		(((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage == 2	&&
		m_pSrcBuf[0] == 0 && m_pSrcBuf[1] == 1))
	{
		// Restore Previous RLE Dib
		if (m_pPrevRLEDib && m_pPrevRLEDib->IsValid())
			*pDib = *m_pPrevRLEDib;
		return true;
	}
	else
	{
		// Allocate Bits?
		if (!pDib->GetBits()																||
			!pDib->GetBMI()																	||
			(pDib->GetBMISize() != m_dwSrcFormatSize)										||
			(pDib->GetImageSize() < ((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage)		||
			(pDib->GetCompression() != ((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression)	||
			(pDib->GetBitCount() != ((LPBITMAPINFOHEADER)m_pSrcFormat)->biBitCount))
		{
			if (!pDib->AllocateBitsFast(((LPBITMAPINFOHEADER)m_pSrcFormat)->biBitCount,
										((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression,
										((LPBITMAPINFOHEADER)m_pSrcFormat)->biWidth,
										ABS(((LPBITMAPINFOHEADER)m_pSrcFormat)->biHeight),
										(RGBQUAD*)(m_pSrcFormat + ((LPBITMAPINFOHEADER)m_pSrcFormat)->biSize),
										((LPBITMAPINFOHEADER)m_pSrcFormat)->biClrUsed))
			{
				return false;
			}
		}
		if (!m_pPrevRLEDib && IsRLE(((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression))
		{
			m_pPrevRLEDib = new CDib;
			if (!m_pPrevRLEDib)
				return false;
		}

		// Set Bitmap Info
		memcpy(	pDib->GetBMI(),
				m_pSrcFormat,
				MIN(pDib->GetBMISize(), m_dwSrcFormatSize));

		// Flip Vertically?
		if (pDib->GetBMIH()->biHeight < 0)
		{
			pDib->GetBMIH()->biHeight *= -1;			
			int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(	pDib->GetBMIH()->biBitCount *
															pDib->GetBMIH()->biWidth);
			LPBYTE lpBits = pDib->GetBits() + ((int)pDib->GetBMIH()->biHeight - 1) * nDWAlignedLineSize;
			for (int nCurLine = 0 ; nCurLine < (int)pDib->GetBMIH()->biHeight ; nCurLine++)
			{
				memcpy((void*)lpBits, (void*)(m_pSrcBuf + nCurLine*nDWAlignedLineSize), nDWAlignedLineSize); 
				lpBits -= nDWAlignedLineSize;
			}
		}
		// Copy Bits
		else
		{
			memcpy(	pDib->GetBits(),
					m_pSrcBuf,
					MIN(pDib->GetImageSize(), ((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage));
		}

		// Decode RLE
		if (IsRLE(((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression))
		{
			if (pDib->DecompressRLE(m_pPrevRLEDib))
			{
				// Allocate Destination Format
				if (m_dwDstFormatSize != pDib->GetBMISize())
				{
					if (m_pDstBMI)
						delete [] m_pDstBMI;
					m_dwDstFormatSize = pDib->GetBMISize();
					m_pDstBMI = (LPBITMAPINFO)new BYTE[m_dwDstFormatSize];
					if (!m_pDstBMI)
						return false;
				}

				// Copy Destination Format
				memcpy(m_pDstBMI, pDib->GetBMI(), MIN(pDib->GetBMISize(), m_dwDstFormatSize));
			}
		}

		// Store Previous RLE Dib
		if (m_pPrevRLEDib)
			*m_pPrevRLEDib = *pDib;
		
		return true;
	}
}

bool CAVIPlay::CAVIVideoStream::GetYUVFrameAt(CDib* pDib, DWORD dwFrame)
{
	// Check
	if (!pDib)
		return false;
	
	// Enter CS
	::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

	// Check Range
	if (dwFrame >= GetTotalFrames())
	{
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Get Frame?
	if (m_nLastDecompressedDibFrame != (int)dwFrame)
	{
		// Reset
		m_nLastDecompressedDibFrame = -2;

		// Get Encoded Frame
		DWORD dwSrcBufSizeUsed = m_dwSrcBufSize;
		if (!GetChunkData(	dwFrame,
							m_pSrcBuf,
							&dwSrcBufSizeUsed))
		{
			pDib->Free();
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}

		// Decode
		if (!::YUVToRGB32(	((LPBITMAPINFOHEADER)m_pSrcFormat)->biCompression,
							m_pSrcBuf,
							m_pDstBuf,
							m_pDstBMI->bmiHeader.biWidth,
							m_pDstBMI->bmiHeader.biHeight))
		{
			pDib->Free();
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}
	}

	// Allocate?
	if (!AllocateDstDib(pDib))
	{
		m_nLastDecompressedDibFrame = -2;
		pDib->Free();
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Copy Bits
	memcpy(pDib->GetBits(), m_pDstBuf, pDib->GetImageSize());

	// Update Last Decompressed Frame
	m_nLastDecompressedDibFrame = (int)dwFrame;

	// Inc. Pos
	m_dwNextFrame = dwFrame + 1;

	// Leave CS
	::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

	return true;
}

__forceinline bool CAVIPlay::CAVIVideoStream::AllocateDstDib(CDib* pDib)
{
	// Allocate Bits?
	if (!pDib->GetBits()												||
		!pDib->GetBMI()													||
		(pDib->GetImageSize() != m_pDstBMI->bmiHeader.biSizeImage)		||
		(pDib->GetCompression() != m_pDstBMI->bmiHeader.biCompression)	||
		(pDib->GetBitCount() != m_pDstBMI->bmiHeader.biBitCount))
	{
		if (!pDib->AllocateBitsFast(m_pDstBMI->bmiHeader.biBitCount,
									m_pDstBMI->bmiHeader.biCompression,
									m_pDstBMI->bmiHeader.biWidth,
									m_pDstBMI->bmiHeader.biHeight,
									(RGBQUAD*)((LPBYTE)m_pDstBMI + m_pDstBMI->bmiHeader.biSize),
									m_pDstBMI->bmiHeader.biClrUsed))
		{
			return false;
		}
	}
	
	// Set Bitmap Info
	memcpy(	pDib->GetBMI(),
			m_pDstBMI,
			MIN(pDib->GetBMISize(), m_dwDstFormatSize));

	return true;
}

__forceinline void  CAVIPlay::CAVIVideoStream::UpdatePalette(DWORD dwFrame)
{
	if (m_pPaletteStream)
	{
		if (m_pCodecCtx)
		{
			DWORD dwPaletteSize = 0;
			if (dwFrame == 0)
			{
				if (m_dwSrcFormatSize > sizeof(BITMAPINFOHEADER))
				{
					dwPaletteSize = m_dwSrcFormatSize - sizeof(BITMAPINFOHEADER);
					memcpy(	(LPBYTE)m_AVPalette,
							(LPBYTE)m_pSrcFormat + sizeof(BITMAPINFOHEADER),
							MIN(dwPaletteSize, AVPALETTE_SIZE));
					m_bAVPaletteChanged = true;
				}
			}
			if (m_pPaletteStream->DoChangePalette(dwFrame))
			{
				dwPaletteSize = AVPALETTE_SIZE;
				m_pPaletteStream->GetPalette(dwFrame,
											(LPBYTE)m_AVPalette,
											&dwPaletteSize);
				m_bAVPaletteChanged = true;
			}
		}
		else
		{
			DWORD dwSrcPaletteSize = 0;
			DWORD dwDstPaletteSize = 0;
			if (m_dwSrcFormatSize > sizeof(BITMAPINFOHEADER))
				dwSrcPaletteSize = m_dwSrcFormatSize - sizeof(BITMAPINFOHEADER);
			if (m_dwDstFormatSize > sizeof(BITMAPINFOHEADER))
				dwDstPaletteSize = m_dwDstFormatSize - sizeof(BITMAPINFOHEADER);
			bool bChanged = false;
			if (dwSrcPaletteSize > 0)
			{
				// Get Palette
				if (dwFrame == 0)
				{
					memcpy(	(LPBYTE)m_pSrcFormat + sizeof(BITMAPINFOHEADER),
							(LPBYTE)m_pOrigSrcBMI + sizeof(BITMAPINFOHEADER),
							dwSrcPaletteSize);
					bChanged = true;
				}
				if (m_pPaletteStream->DoChangePalette(dwFrame))
				{
					m_pPaletteStream->GetPalette(dwFrame,
												(LPBYTE)m_pSrcFormat + sizeof(BITMAPINFOHEADER),
												&dwSrcPaletteSize);
					bChanged = true;
				}

				// Update
				if (bChanged)
				{
					// End if not decompressing to same bit depth
					if (m_hIC && dwDstPaletteSize == 0)
						ICDecompressEnd(m_hIC);

					// Update Destination Palette
					memcpy(	(LPBYTE)m_pDstBMI + sizeof(BITMAPINFOHEADER),
							(LPBYTE)m_pSrcFormat + sizeof(BITMAPINFOHEADER),
							MIN(dwSrcPaletteSize, dwDstPaletteSize));

					// Re-Begin if not decompressing to same bit depth
					if (m_hIC && dwDstPaletteSize == 0)
						ICDecompressBegin(m_hIC, (LPBITMAPINFO)m_pSrcFormat, m_pDstBMI);
				}
			}
		}
	}
}

bool CAVIPlay::CAVIVideoStream::GetFrame(CDib* pDib)
{
	// Check
	if (!pDib)
		return false;

	// Uncompressed Frame?
	if (m_bNoDecompression)
		return GetUncompressedFrameAt(pDib, m_dwNextFrame, m_nLastDecompressedDibFrame);
	else if (m_bYuvToRgb32)
		return GetYUVFrameAt(pDib, m_dwNextFrame);

	// Enter CS
	::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

	// Check
	if (GetTotalFrames() <= 0)
	{
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Special Handling for delayed codecs
	bool bFlushLastFrame = false;
	if (m_nOneFrameDelay)
	{
		// Flush Last Frame?
		if (m_dwNextFrame == GetTotalFrames())
			bFlushLastFrame = true;
		// End of File -> Return
		else if (m_dwNextFrame > GetTotalFrames())
		{
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}
	}
	else
	{
		// End of File -> Return
		if (m_dwNextFrame >= GetTotalFrames())
		{
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}
	}

	// Not in Sequence Frame?
	if ((m_nLastDecompressedDibFrame + 1) != (int)m_dwNextFrame)
	{
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return GetFrameAt(pDib, m_dwNextFrame);
	}

	// Get Frame?
	if (m_nLastDecompressedDibFrame != (int)m_dwNextFrame)
	{
		// Reset
		m_nLastDecompressedDibFrame = -2;

		// Get Compressed Data
		DWORD dwSrcBufSizeUsed = m_dwSrcBufSize;
		if (!GetChunkData(	bFlushLastFrame ?  GetTotalFrames() - 1 : m_dwNextFrame,
							m_pSrcBuf,
							&dwSrcBufSizeUsed))
		{
			pDib->Free();
			::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
			return false;
		}
		UpdatePalette(m_dwNextFrame);

		// Decompress
		((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
		if (m_hIC)
		{
			if ((int)ICDecompress(	m_hIC,
									IsKeyFrame(m_dwNextFrame) ?
									0 : ICDECOMPRESS_NOTKEYFRAME,
									(LPBITMAPINFOHEADER)m_pSrcFormat,
									(LPVOID)m_pSrcBuf,                  
									(LPBITMAPINFOHEADER)m_pDstBMI,        
									(LPVOID)m_pDstBuf) < 0)
			{
				pDib->Free();
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return false;
			}
		}
		else
		{
			if (!AVCodecDecompressDib(IsKeyFrame(bFlushLastFrame ?  GetTotalFrames() - 1 : m_dwNextFrame), false))
			{
				pDib->Free();
				::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
				return false;
			}
		}
	}

	// Allocate?
	if (!AllocateDstDib(pDib))
	{
		m_nLastDecompressedDibFrame = -2;
		pDib->Free();
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Copy Bits
	if (!m_hIC)
	{
		if (m_pDstBMI->bmiHeader.biCompression == BI_RGB ||
			m_pDstBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			// Flip Vertically
			int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(	m_pDstBMI->bmiHeader.biBitCount *
															m_pDstBMI->bmiHeader.biWidth);
			LPBYTE lpBits = pDib->GetBits() + ((int)m_pDstBMI->bmiHeader.biHeight - 1) * nDWAlignedLineSize;
			for (int nCurLine = 0 ; nCurLine < (int)GetHeight() ; nCurLine++)
			{
				memcpy((void*)lpBits, (void*)(m_pDstBuf + nCurLine*nDWAlignedLineSize), nDWAlignedLineSize); 
				lpBits -= nDWAlignedLineSize;
			}
		}
		else
		{
			// Copy
			int stride = ::CalcYUVStride(m_pDstBMI->bmiHeader.biCompression, (int)m_pDstBMI->bmiHeader.biWidth);
			int chromawidth = 0;
			int chromaheight = 0;
			if (m_pDstBMI->bmiHeader.biCompression == FCC('YV12')	||	
				m_pDstBMI->bmiHeader.biCompression == FCC('I420')	||
				m_pDstBMI->bmiHeader.biCompression == FCC('IYUV'))
			{
				chromawidth = stride >> 1;
				chromaheight = m_pDstBMI->bmiHeader.biHeight >> 1;
			}
			else if (m_pDstBMI->bmiHeader.biCompression == FCC('YVU9')	||
					m_pDstBMI->bmiHeader.biCompression == FCC('YUV9'))
			{
				chromawidth = stride >> 2;
				chromaheight = m_pDstBMI->bmiHeader.biHeight >> 2;
			}
			if (m_pFrameGdi->linesize[0] > 0 && m_pFrameGdi->data[0])
			{
				int line;
				LPBYTE pSrc = (LPBYTE)m_pFrameGdi->data[0];
				LPBYTE pDst = (LPBYTE)pDib->GetBits();
				for (line = 0 ; line < m_pDstBMI->bmiHeader.biHeight ; line++)
				{
					memcpy(pDst, pSrc, stride);
					pSrc += m_pFrameGdi->linesize[0];
					pDst += stride;
				}
				if (m_pFrameGdi->linesize[1] > 0 && m_pFrameGdi->data[1] &&
					m_pFrameGdi->linesize[2] > 0 && m_pFrameGdi->data[2])
				{
					LPBYTE pFirst, pSecond; 
					int nFirstSize, nSecondSize;
					if (m_pFrameGdi->data[1] < m_pFrameGdi->data[2])
					{
						pFirst = (LPBYTE)m_pFrameGdi->data[1];
						pSecond = (LPBYTE)m_pFrameGdi->data[2];
						nFirstSize = m_pFrameGdi->linesize[1];
						nSecondSize = m_pFrameGdi->linesize[2];
					}
					else
					{
						pFirst = (LPBYTE)m_pFrameGdi->data[2];
						pSecond = (LPBYTE)m_pFrameGdi->data[1];
						nFirstSize = m_pFrameGdi->linesize[2];
						nSecondSize = m_pFrameGdi->linesize[1];
					}
					pSrc = pFirst;
					for (line = 0 ; line < chromaheight ; line++)
					{
						memcpy(pDst, pSrc, chromawidth);
						pSrc += nFirstSize;
						pDst += chromawidth;
					}
					pSrc = pSecond;
					for (line = 0 ; line < chromaheight ; line++)
					{
						memcpy(pDst, pSrc, chromawidth);
						pSrc += nSecondSize;
						pDst += chromawidth;
					}
				}
			}
		}
	}
	else
		memcpy(pDib->GetBits(), m_pDstBuf, pDib->GetImageSize());

	// Update Last Decompressed Frame
	m_nLastDecompressedDibFrame = (int)m_dwNextFrame;

	// Inc. Pos
	m_dwNextFrame++;

	// Leave CS
	::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

	return true;
}

bool CAVIPlay::CAVIVideoStream::GetFrameAt(CDib* pDib, DWORD dwFrame)
{
	bool res;

	// Check
	if (!pDib)
		return false;
	
	// Uncompressed Frame?
	if (m_bNoDecompression)
		return GetUncompressedFrameAt(pDib, dwFrame, m_nLastDecompressedDibFrame);
	else if (m_bYuvToRgb32)
		return GetYUVFrameAt(pDib, dwFrame);

	// Enter CS
	::EnterCriticalSection(&m_pAVIPlay->m_csAVI);

	// Check Range
	if (dwFrame >= GetTotalFrames())
	{
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return false;
	}

	// Just Call GetFrame()?
	if ((dwFrame == m_dwNextFrame) &&
		((m_nLastDecompressedDibFrame + 1) == (int)dwFrame))
	{
		::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);
		return GetFrame(pDib);
	}

	// Get Frame at direct
	res = GetFrameAtDirect(pDib, dwFrame);
	::LeaveCriticalSection(&m_pAVIPlay->m_csAVI);

	// Has One Frame Delay?
	if (m_nOneFrameDelay)
	{	
		if (res)
			return GetFrame(pDib);
		else
			return false;
	}
	else
		return res;
}

bool CAVIPlay::CAVIVideoStream::GetFrameAtDirect(CDib* pDib, DWORD dwFrame)
{
	// Get Frame?
	if (m_nLastDecompressedDibFrame != (int)dwFrame)
	{
		DWORD dwSrcBufSizeUsed;
		int nPrevKeyFrame = GetPrevKeyFrame(dwFrame);
		if (nPrevKeyFrame == -1)
		{
			m_nLastDecompressedDibFrame = -2;
			pDib->Free();
			return false;
		}
		else if (nPrevKeyFrame < (int)dwFrame && ((int)dwFrame - nPrevKeyFrame < MAX_KEYFRAMES_SPACING))
		{	
			// Clear Dst Buffer if First Frame
			if (nPrevKeyFrame == 0)
				ClearDstBuf();

			// Reset
			m_nLastDecompressedDibFrame = -2;

			// Get Key-Frame
			dwSrcBufSizeUsed = m_dwSrcBufSize;
			if (!GetChunkData(	nPrevKeyFrame,
								m_pSrcBuf,
								&dwSrcBufSizeUsed))
			{
				pDib->Free();
				return false;
			}
			UpdatePalette(nPrevKeyFrame);
			((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
			if (m_hIC)
			{
				if ((int)ICDecompress(	m_hIC,
										ICDECOMPRESS_PREROLL,
										(LPBITMAPINFOHEADER)m_pSrcFormat,
										(LPVOID)m_pSrcBuf,                  
										(LPBITMAPINFOHEADER)m_pDstBMI,        
										(LPVOID)m_pDstBuf) < 0)
				{
					pDib->Free();
					return false;
				}
			}
			else
			{
				if (!AVCodecDecompressDib(true, true))
				{
					pDib->Free();
					return false;
				}
			}
			
			// Get Middle Delta Frames
			for (int i = nPrevKeyFrame + 1 ; i < (int)dwFrame ; i++)
			{
				dwSrcBufSizeUsed = m_dwSrcBufSize;
				if (!GetChunkData(	i,
									m_pSrcBuf,
									&dwSrcBufSizeUsed))
				{
					pDib->Free();
					return false;
				}
				UpdatePalette(i);
				((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
				if (m_hIC)
				{
					if ((int)ICDecompress(	m_hIC,
											ICDECOMPRESS_NOTKEYFRAME | ICDECOMPRESS_PREROLL,
											(LPBITMAPINFOHEADER)m_pSrcFormat,
											(LPVOID)m_pSrcBuf,                  
											(LPBITMAPINFOHEADER)m_pDstBMI,        
											(LPVOID)m_pDstBuf) < 0)
					{
						pDib->Free();
						return false;
					}
				}
				else
				{
					if (!AVCodecDecompressDib(false, false))
					{
						pDib->Free();
						return false;
					}
				}
			}

			// Get Wanted Delta Frame
			dwSrcBufSizeUsed = m_dwSrcBufSize;
			if (!GetChunkData(	dwFrame,
								m_pSrcBuf,
								&dwSrcBufSizeUsed))
			{
				pDib->Free();
				return false;
			}
			UpdatePalette(dwFrame);
			((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
			if (m_hIC)
			{
				if ((int)ICDecompress(	m_hIC,
										ICDECOMPRESS_NOTKEYFRAME,
										(LPBITMAPINFOHEADER)m_pSrcFormat,
										(LPVOID)m_pSrcBuf,                  
										(LPBITMAPINFOHEADER)m_pDstBMI,        
										(LPVOID)m_pDstBuf) < 0)
				{
					pDib->Free();
					return false;
				}
			}
			else
			{
				if (!AVCodecDecompressDib(false, false))
				{
					pDib->Free();
					return false;
				}
			}
		}
		else
		{
			// Clear Dst Buffer if First Frame
			if (dwFrame == 0)
				ClearDstBuf();

			// Reset
			m_nLastDecompressedDibFrame = -2;

			// Get Key-Frame
			dwSrcBufSizeUsed = m_dwSrcBufSize;
			if (!GetChunkData(	dwFrame,
								m_pSrcBuf,
								&dwSrcBufSizeUsed))
			{
				pDib->Free();
				return false;
			}
			UpdatePalette(dwFrame);
			
			((LPBITMAPINFOHEADER)m_pSrcFormat)->biSizeImage = dwSrcBufSizeUsed;
			if (m_hIC)
			{
				if ((int)ICDecompress(	m_hIC,
										0,
										(LPBITMAPINFOHEADER)m_pSrcFormat,
										(LPVOID)m_pSrcBuf,                  
										(LPBITMAPINFOHEADER)m_pDstBMI,        
										(LPVOID)m_pDstBuf) < 0)
				{
					pDib->Free();
					return false;
				}
			}
			else
			{
				if (!AVCodecDecompressDib(true, true))
				{
					pDib->Free();
					return false;
				}
			}
		}
	}

	// Allocate and set to black?
	if (!AllocateDstDib(pDib))
	{
		m_nLastDecompressedDibFrame = -2;
		pDib->Free();
		return false;
	}

	// Copy Bits
	if (!m_hIC)
	{
		if (m_pDstBMI->bmiHeader.biCompression == BI_RGB ||
			m_pDstBMI->bmiHeader.biCompression == BI_BITFIELDS)
		{
			// Flip Vertically
			int nDWAlignedLineSize = DWALIGNEDWIDTHBYTES(	m_pDstBMI->bmiHeader.biBitCount *
															m_pDstBMI->bmiHeader.biWidth);
			LPBYTE lpBits = pDib->GetBits() + ((int)m_pDstBMI->bmiHeader.biHeight - 1) * nDWAlignedLineSize;
			for (int nCurLine = 0 ; nCurLine < (int)GetHeight() ; nCurLine++)
			{
				memcpy((void*)lpBits, (void*)(m_pDstBuf + nCurLine*nDWAlignedLineSize), nDWAlignedLineSize); 
				lpBits -= nDWAlignedLineSize;
			}
		}
		else
		{
			// Copy
			int stride = ::CalcYUVStride(m_pDstBMI->bmiHeader.biCompression, (int)m_pDstBMI->bmiHeader.biWidth);
			int chromawidth = 0;
			int chromaheight = 0;
			if (m_pDstBMI->bmiHeader.biCompression == FCC('YV12')	||	
				m_pDstBMI->bmiHeader.biCompression == FCC('I420')	||
				m_pDstBMI->bmiHeader.biCompression == FCC('IYUV'))
			{
				chromawidth = stride >> 1;
				chromaheight = m_pDstBMI->bmiHeader.biHeight >> 1;
			}
			else if (m_pDstBMI->bmiHeader.biCompression == FCC('YVU9')	||
					m_pDstBMI->bmiHeader.biCompression == FCC('YUV9'))
			{
				chromawidth = stride >> 2;
				chromaheight = m_pDstBMI->bmiHeader.biHeight >> 2;
			}
			if (m_pFrameGdi->linesize[0] > 0 && m_pFrameGdi->data[0])
			{
				int line;
				LPBYTE pSrc = (LPBYTE)m_pFrameGdi->data[0];
				LPBYTE pDst = (LPBYTE)pDib->GetBits();
				for (line = 0 ; line < m_pDstBMI->bmiHeader.biHeight ; line++)
				{
					memcpy(pDst, pSrc, stride);
					pSrc += m_pFrameGdi->linesize[0];
					pDst += stride;
				}
				if (m_pFrameGdi->linesize[1] > 0 && m_pFrameGdi->data[1] &&
					m_pFrameGdi->linesize[2] > 0 && m_pFrameGdi->data[2])
				{
					LPBYTE pFirst, pSecond; 
					int nFirstSize, nSecondSize;
					if (m_pFrameGdi->data[1] < m_pFrameGdi->data[2])
					{
						pFirst = (LPBYTE)m_pFrameGdi->data[1];
						pSecond = (LPBYTE)m_pFrameGdi->data[2];
						nFirstSize = m_pFrameGdi->linesize[1];
						nSecondSize = m_pFrameGdi->linesize[2];
					}
					else
					{
						pFirst = (LPBYTE)m_pFrameGdi->data[2];
						pSecond = (LPBYTE)m_pFrameGdi->data[1];
						nFirstSize = m_pFrameGdi->linesize[2];
						nSecondSize = m_pFrameGdi->linesize[1];
					}
					pSrc = pFirst;
					for (line = 0 ; line < chromaheight ; line++)
					{
						memcpy(pDst, pSrc, chromawidth);
						pSrc += nFirstSize;
						pDst += chromawidth;
					}
					pSrc = pSecond;
					for (line = 0 ; line < chromaheight ; line++)
					{
						memcpy(pDst, pSrc, chromawidth);
						pSrc += nSecondSize;
						pDst += chromawidth;
					}
				}
			}
		}
	}
	else
		memcpy(pDib->GetBits(), m_pDstBuf, pDib->GetImageSize());

	// Update Last Decompressed Frame
	m_nLastDecompressedDibFrame = (int)dwFrame;

	// Inc. Pos
	m_dwNextFrame = dwFrame + 1;

	return true;
}

void CAVIPlay::CAVIVideoStream::ClearDstBuf()
{
	if (m_pDstBuf && m_dwDstBufSize > 0)
	{
		if (m_pDstBMI)
		{
			if (m_pDstBMI->bmiHeader.biCompression == FCC('I420')	||			// YUV420P
				m_pDstBMI->bmiHeader.biCompression == FCC('IYUV')	||
				m_pDstBMI->bmiHeader.biCompression == FCC('YV12'))
			{
				memset(	m_pDstBuf,
						16,
						m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight);
				memset(	m_pDstBuf + (m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight),
						128,
						(m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight) >> 1);
			}
			else if (	m_pDstBMI->bmiHeader.biCompression == FCC('YUV9')	||	// YUV410P
						m_pDstBMI->bmiHeader.biCompression == FCC('YVU9'))
			{
				memset(	m_pDstBuf,
						16,
						m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight);
				memset(	m_pDstBuf + (m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight),
						128,
						(m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight) >> 3);
			}
			else if (	m_pDstBMI->bmiHeader.biCompression == FCC('Y41B'))		// YUV411P
			{
				memset(	m_pDstBuf,
						16,
						m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight);
				memset(	m_pDstBuf + (m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight),
						128,
						(m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight) >> 1);
			}
			else if (	m_pDstBMI->bmiHeader.biCompression == FCC('YV16')	||	// YUV422P
						m_pDstBMI->bmiHeader.biCompression == FCC('Y42B'))
			{
				memset(	m_pDstBuf,
						16,
						m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight);
				memset(	m_pDstBuf + (m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight),
						128,
						m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight);
			}
			else if (	m_pDstBMI->bmiHeader.biCompression == FCC('YUY2')	||	// Packed 422: YUYV or YVYU
						m_pDstBMI->bmiHeader.biCompression == FCC('YUNV')	||
						m_pDstBMI->bmiHeader.biCompression == FCC('VYUY')	||
						m_pDstBMI->bmiHeader.biCompression == FCC('V422')	||
						m_pDstBMI->bmiHeader.biCompression == FCC('YUYV')	||
						m_pDstBMI->bmiHeader.biCompression == FCC('YVYU'))
			{
				int size = (m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight) << 1;
				int i = 0;
				while (i < size)
				{
					m_pDstBuf[i++] = 16;
					m_pDstBuf[i++] = 128;
				}
			}
			else if (	m_pDstBMI->bmiHeader.biCompression == FCC('UYVY')	||	// Packed 422: UYVY
						m_pDstBMI->bmiHeader.biCompression == FCC('Y422')	||
						m_pDstBMI->bmiHeader.biCompression == FCC('UYNV'))
			{
				int size = (m_pDstBMI->bmiHeader.biWidth * m_pDstBMI->bmiHeader.biHeight) << 1;
				int i = 0;
				while (i < size)
				{
					m_pDstBuf[i++] = 128;
					m_pDstBuf[i++] = 16;
				}
			}
			else
				memset(m_pDstBuf, 0, m_dwDstBufSize);
		}
		else
			memset(m_pDstBuf, 0, m_dwDstBufSize);
	}
}

CAVIPlay::CAVIVideoStream* CAVIPlay::GetVideoStream(int nStreamNum) const
{
	if ((nStreamNum >= 0) && (nStreamNum < m_VideoStreams.GetSize()))
		return m_VideoStreams[nStreamNum];
	else
		return NULL;
}

CAVIPlay::CAVIAudioStream* CAVIPlay::GetAudioStream(int nStreamNum) const
{
	if ((nStreamNum >= 0) && (nStreamNum < m_AudioStreams.GetSize()))
		return m_AudioStreams[nStreamNum];
	else
		return NULL;
}

CAVIPlay::CAVIPaletteStream* CAVIPlay::GetPaletteStream(int nStreamNum) const
{
	if ((nStreamNum >= 0) && (nStreamNum < m_PaletteStreams.GetSize()))
		return m_PaletteStreams[nStreamNum];
	else
		return NULL;
}

bool CAVIPlay::Open(LPCTSTR szFileName,
					bool bOnlyHeaders,
					bool bShowMessageBoxOnError)
{
	m_bShowMessageBoxOnError = bShowMessageBoxOnError;
	return Open(szFileName, bOnlyHeaders);
}

bool CAVIPlay::Open(	LPCTSTR szFileName,
						bool bOnlyHeader/*=false*/)
{
	CAVIVideoStream* pVideoStream = NULL;
	CAVIAudioStream* pAudioStream = NULL;
	m_sFileName = szFileName;
	m_uiOpenFlags = CFile::modeRead | CFile::shareDenyNone;

	try
	{		
		// Vars
		LONGLONG llPos;
		RIFFLIST list, hdrl_list;
		LONGLONG hdrl_pos;
		RIFFCHUNK chunk;
		DWORD dwVideoStreamsCount = 0;
		DWORD dwAudioStreamsCount = 0;
		bool bHasIndex = false;
		int i;

		// Close
		Close();

		// Open File
		m_pFile = new CFile(szFileName, m_uiOpenFlags);
		if (!m_pFile)
			throw (int)OUT_OF_MEM;

		// Read Start Avi List
		if (m_pFile->Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('RIFF')) ||
			(list.fccListType != FCC('AVI ') && list.fccListType != FCC('AVIX')))
			throw (int)WRONG_LIST_TYPE;

		// Read hdrl List
		if (m_pFile->Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
			throw (int)UNEXPECTED_EOF;
		hdrl_pos = m_pFile->GetPosition();
		if ((list.fcc != FCC('LIST')) ||
			list.fccListType != FCC('hdrl'))
			throw (int)WRONG_LIST_TYPE;
		hdrl_list = list;

		// Read avih Chunk
		if (m_pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
			throw (int)UNEXPECTED_EOF;
		if (chunk.fcc != FCC('avih'))
			throw (int)WRONG_CHUNK_TYPE;
		if (m_pFile->Read(&m_AviMainHdr, sizeof(AVIMAINHDR)) < sizeof(AVIMAINHDR))
			throw (int)UNEXPECTED_EOF;
		// Adjust File Position in case sizeof(AVIMAINHDR) != chunk.cb
		m_pFile->Seek(-((LONGLONG)sizeof(AVIMAINHDR)), CFile::current);
		m_pFile->Seek((LONGLONG)chunk.cb, CFile::current);

		// Read the streams
		for (DWORD dwStreamNum = 0 ; dwStreamNum < m_AviMainHdr.dwStreams ; dwStreamNum++)
		{
			// Read strl List
			while (TRUE)
			{
				if (m_pFile->Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
					throw (int)UNEXPECTED_EOF;
				if ((list.fcc != FCC('LIST')) ||
					list.fccListType != FCC('strl'))
					m_pFile->Seek(RIFFROUND((LONGLONG)list.cb - 4), CFile::current);
				else
					break;
			}
			
			// Calc the Next List Pos
			DWORD dwListSize = list.cb - 4;
			LONGLONG llNextListPos = m_pFile->GetPosition() + dwListSize;

			// Read strh Chunk
			AVISTREAMHDR AviStreamHdr;
			if (m_pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
				throw (int)UNEXPECTED_EOF;
			if (chunk.fcc != FCC('strh'))
				throw (int)WRONG_CHUNK_TYPE;
			if (m_pFile->Read(&AviStreamHdr, sizeof(AVISTREAMHDR)) < sizeof(AVISTREAMHDR))
				throw (int)UNEXPECTED_EOF;
			m_pFile->Seek(-((LONGLONG)sizeof(AVISTREAMHDR)), CFile::current);

			if (AviStreamHdr.fccType == streamtypeVIDEO)
			{
				// New Video Stream Object
				pVideoStream = (CAVIVideoStream*)new CAVIVideoStream(dwVideoStreamsCount, this, m_pFile);
				if (!pVideoStream)
					throw (int)OUT_OF_MEM;

				// Copy Stream Header
				memcpy(&pVideoStream->m_Hdr, &AviStreamHdr, sizeof(AVISTREAMHDR));

				// Go to Next Chunk
				while (llPos = SeekToNextChunk(m_pFile, chunk, llNextListPos))
				{
					// Check if it is an strf Chunk
					if (chunk.fcc == FCC('strf'))
					{
						if (chunk.cb < sizeof(BITMAPINFOHEADER))
							throw (int)WRONG_VIDEOSTREAM_HEADER;
						pVideoStream->m_dwSrcFormatSize = chunk.cb;
						pVideoStream->m_pSrcFormat = new BYTE[chunk.cb];
						if (!pVideoStream->m_pSrcFormat)
							throw (int)OUT_OF_MEM;
						if (m_pFile->Read(pVideoStream->m_pSrcFormat, chunk.cb) < chunk.cb)
							throw (int)UNEXPECTED_EOF;
					}
					// Check if it is an indx Chunk
					else if (chunk.fcc == FCC('indx'))
					{
						// Read Super Index Header
						if (m_pFile->Read(&pVideoStream->m_SuperIndexHdr, sizeof(AVISUPERINDEXHDR)) < sizeof(AVISUPERINDEXHDR))
							throw (int)UNEXPECTED_EOF;
						if ((pVideoStream->m_SuperIndexHdr.wLongsPerEntry != 4) ||
							(pVideoStream->m_SuperIndexHdr.bIndexType != AVI_INDEX_OF_INDEXES))
							throw (int)WRONG_SUPERINDEX_HEADER;

						// Read Super Index Entries
						pVideoStream->m_pSuperIndexTable = new AVISUPERINDEXENTRY[pVideoStream->m_SuperIndexHdr.nEntriesInUse];
						if (!pVideoStream->m_pSuperIndexTable)
							throw (int)OUT_OF_MEM;
						if (m_pFile->Read(pVideoStream->m_pSuperIndexTable, pVideoStream->m_SuperIndexHdr.nEntriesInUse * sizeof(AVISUPERINDEXENTRY)) < pVideoStream->m_SuperIndexHdr.nEntriesInUse * sizeof(AVISUPERINDEXENTRY))
							throw (int)UNEXPECTED_EOF;

						// Allocate Standard Index Headers
						pVideoStream->m_pStdIndexHdrs = new AVISTDINDEXHDR[pVideoStream->m_SuperIndexHdr.nEntriesInUse];
						if (!pVideoStream->m_pStdIndexHdrs)
							throw (int)OUT_OF_MEM;

						// Allocate Standard Index Tables
						pVideoStream->m_ppStdIndexTables = new LPAVISTDINDEXENTRY[pVideoStream->m_SuperIndexHdr.nEntriesInUse];
						if (!pVideoStream->m_ppStdIndexTables)
							throw (int)OUT_OF_MEM;
						for (i = 0 ; i < (int)pVideoStream->m_SuperIndexHdr.nEntriesInUse ; i++)
							pVideoStream->m_ppStdIndexTables[i] = NULL;

						// Read Indexes
						for (i = 0 ; i < (int)pVideoStream->m_SuperIndexHdr.nEntriesInUse ; i++)
						{
							// Seek To Standard Index
							m_pFile->Seek((LONGLONG)pVideoStream->m_pSuperIndexTable[i].qwOffset, CFile::begin);
							
							// Read Chunk
							if (m_pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
								throw (int)UNEXPECTED_EOF;

							// Read Standard Index Header
							if (m_pFile->Read(&pVideoStream->m_pStdIndexHdrs[i], sizeof(AVISTDINDEXHDR)) < sizeof(AVISTDINDEXHDR))
								throw (int)UNEXPECTED_EOF;
							if ((pVideoStream->m_pStdIndexHdrs[i].wLongsPerEntry != 2) ||
								(pVideoStream->m_pStdIndexHdrs[i].bIndexSubType != 0) ||
								(pVideoStream->m_pStdIndexHdrs[i].bIndexType != AVI_INDEX_OF_CHUNKS))
								throw (int)WRONG_STANDARDINDEX_HEADER;

							// Read Standard Index Entries
							pVideoStream->m_ppStdIndexTables[i] = new AVISTDINDEXENTRY[pVideoStream->m_pStdIndexHdrs[i].nEntriesInUse];
							if (!pVideoStream->m_ppStdIndexTables[i])
								throw (int)OUT_OF_MEM;
							if (m_pFile->Read(pVideoStream->m_ppStdIndexTables[i], (pVideoStream->m_pStdIndexHdrs[i].nEntriesInUse) * sizeof(AVISTDINDEXENTRY)) < (pVideoStream->m_pStdIndexHdrs[i].nEntriesInUse) * sizeof(AVISTDINDEXENTRY))
								throw (int)UNEXPECTED_EOF;

							// Count Bytes and Key-Frames
							for (int index = 0 ; index < (int)(pVideoStream->m_pStdIndexHdrs[i].nEntriesInUse) ; index++)
							{
								pVideoStream->m_dwChunksCount++;
								pVideoStream->m_llBytesCount += (pVideoStream->m_ppStdIndexTables[i][index].dwSize & 0x7FFFFFFF);
								if ((pVideoStream->m_ppStdIndexTables[i][index].dwSize & 0x80000000) != 0x80000000)
									pVideoStream->m_dwKeyFramesCount++;
							}
						}

						// Set Index Flag
						if (pVideoStream->m_SuperIndexHdr.nEntriesInUse > 0)
							bHasIndex = true; 
					}
					// Restore Position
					m_pFile->Seek((LONGLONG)llPos, CFile::begin);
				}
				++dwVideoStreamsCount;
				m_VideoStreams.Add(pVideoStream);
				pVideoStream = NULL;
			}
			else if (AviStreamHdr.fccType == streamtypeAUDIO)
			{
				// New Audio Stream Object
				pAudioStream = (CAVIAudioStream*)new CAVIAudioStream(dwAudioStreamsCount, this, m_pFile);
				if (!pAudioStream)
					throw (int)OUT_OF_MEM;

				// Copy Stream Header
				memcpy(&pAudioStream->m_Hdr, &AviStreamHdr, sizeof(AVISTREAMHDR));

				// Go to Next Chunk
				while (llPos = SeekToNextChunk(m_pFile, chunk, llNextListPos))
				{
					// Check if it is an strf Chunk
					if (chunk.fcc == FCC('strf'))
					{
						if (chunk.cb < sizeof(WAVEFORMAT))
							throw (int)WRONG_AUDIOSTREAM_HEADER;
						else if (chunk.cb <= sizeof(WAVEFORMATEX))
						{
							pAudioStream->m_pSrcFormat = new BYTE[sizeof(WAVEFORMATEX)];
							if (!pAudioStream->m_pSrcFormat)
								throw (int)OUT_OF_MEM;
							memset(pAudioStream->m_pSrcFormat, 0, sizeof(WAVEFORMATEX));
							if (m_pFile->Read(pAudioStream->m_pSrcFormat, chunk.cb) < chunk.cb)
								throw (int)UNEXPECTED_EOF;
							pAudioStream->m_dwSrcFormatSize = chunk.cb;
						}
						else
						{
							pAudioStream->m_pSrcFormat = new BYTE[chunk.cb];
							if (!pAudioStream->m_pSrcFormat)
								throw (int)OUT_OF_MEM;
							memset(pAudioStream->m_pSrcFormat, 0, chunk.cb);
							if (m_pFile->Read(pAudioStream->m_pSrcFormat, chunk.cb) < chunk.cb) 
								throw (int)UNEXPECTED_EOF;
							pAudioStream->m_dwSrcFormatSize = chunk.cb;
						}

						// Correct Some Brocken MPEG Audio Headers
						if (((LPWAVEFORMATEX)pAudioStream->m_pSrcFormat)->wFormatTag == WAVE_FORMAT_MPEG ||
							((LPWAVEFORMATEX)pAudioStream->m_pSrcFormat)->wFormatTag == WAVE_FORMAT_MPEGLAYER3)
							((LPWAVEFORMATEX)pAudioStream->m_pSrcFormat)->wBitsPerSample = 0;

						// Set VBR Flag
						pAudioStream->SetIsVBR();
					}
					// Check if it is an indx Chunk
					else if (chunk.fcc == FCC('indx'))
					{
						// Read Super Index Header
						if (m_pFile->Read(&pAudioStream->m_SuperIndexHdr, sizeof(AVISUPERINDEXHDR)) < sizeof(AVISUPERINDEXHDR))
							throw (int)UNEXPECTED_EOF;
						if ((pAudioStream->m_SuperIndexHdr.wLongsPerEntry != 4) ||
							(pAudioStream->m_SuperIndexHdr.bIndexType != AVI_INDEX_OF_INDEXES))
							throw (int)WRONG_SUPERINDEX_HEADER;

						// Read Super Index Entries
						pAudioStream->m_pSuperIndexTable = new AVISUPERINDEXENTRY[pAudioStream->m_SuperIndexHdr.nEntriesInUse];
						if (!pAudioStream->m_pSuperIndexTable)
							throw (int)OUT_OF_MEM;
						if (m_pFile->Read(pAudioStream->m_pSuperIndexTable, pAudioStream->m_SuperIndexHdr.nEntriesInUse * sizeof(AVISUPERINDEXENTRY)) < pAudioStream->m_SuperIndexHdr.nEntriesInUse * sizeof(AVISUPERINDEXENTRY))
							throw (int)UNEXPECTED_EOF;

						// Allocate Standard Index Headers
						pAudioStream->m_pStdIndexHdrs = new AVISTDINDEXHDR[pAudioStream->m_SuperIndexHdr.nEntriesInUse];
						if (!pAudioStream->m_pStdIndexHdrs)
							throw (int)OUT_OF_MEM;

						// Allocate Standard Index Tables
						pAudioStream->m_ppStdIndexTables = new LPAVISTDINDEXENTRY[pAudioStream->m_SuperIndexHdr.nEntriesInUse];
						if (!pAudioStream->m_ppStdIndexTables)
							throw (int)OUT_OF_MEM;
						for (i = 0 ; i < (int)pAudioStream->m_SuperIndexHdr.nEntriesInUse ; i++)
							pAudioStream->m_ppStdIndexTables[i] = NULL;

						// Read Indexes
						for (i = 0 ; i < (int)pAudioStream->m_SuperIndexHdr.nEntriesInUse ; i++)
						{
							// Seek To Standard Index
							m_pFile->Seek((LONGLONG)pAudioStream->m_pSuperIndexTable[i].qwOffset, CFile::begin);
							
							// Read Chunk
							if (m_pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
								throw (int)UNEXPECTED_EOF;

							// Read Standard Index Header
							if (m_pFile->Read(&pAudioStream->m_pStdIndexHdrs[i], sizeof(AVISTDINDEXHDR)) < sizeof(AVISTDINDEXHDR))
								throw (int)UNEXPECTED_EOF;
							if ((pAudioStream->m_pStdIndexHdrs[i].wLongsPerEntry != 2) ||
								(pAudioStream->m_pStdIndexHdrs[i].bIndexSubType != 0) ||
								(pAudioStream->m_pStdIndexHdrs[i].bIndexType != AVI_INDEX_OF_CHUNKS))
								throw (int)WRONG_STANDARDINDEX_HEADER;

							// Read Standard Index Entries
							pAudioStream->m_ppStdIndexTables[i] = new AVISTDINDEXENTRY[pAudioStream->m_pStdIndexHdrs[i].nEntriesInUse];
							if (!pAudioStream->m_ppStdIndexTables[i])
								throw (int)OUT_OF_MEM;
							if (m_pFile->Read(pAudioStream->m_ppStdIndexTables[i], (pAudioStream->m_pStdIndexHdrs[i].nEntriesInUse) * sizeof(AVISTDINDEXENTRY)) < (pAudioStream->m_pStdIndexHdrs[i].nEntriesInUse) * sizeof(AVISTDINDEXENTRY))
								throw (int)UNEXPECTED_EOF;

							// Count Bytes
							for (int index = 0 ; index < (int)(pAudioStream->m_pStdIndexHdrs[i].nEntriesInUse) ; index++)
							{
								pAudioStream->m_dwChunksCount++;
								pAudioStream->m_llBytesCount += (pAudioStream->m_ppStdIndexTables[i][index].dwSize & 0x7FFFFFFF);
							}
						}

						// Set Index Flag
						if (pAudioStream->m_SuperIndexHdr.nEntriesInUse > 0)
							bHasIndex = true; 
					}
					// Restore Position
					m_pFile->Seek((LONGLONG)llPos, CFile::begin);
				}
				++dwAudioStreamsCount;
				m_AudioStreams.Add(pAudioStream);
				pAudioStream = NULL;
			}

			// Go to next strl List
			m_pFile->Seek((LONGLONG)llNextListPos, CFile::begin);
		}

		// Seek To Next List After hdrl
		list = hdrl_list;
		m_pFile->Seek((LONGLONG)hdrl_pos, CFile::begin);
		if (!SeekToNextList(m_pFile, list))
			throw (int)UNEXPECTED_EOF;

		// Find the legacy idx1 Chunk
		if (!bHasIndex)
		{
			RIFFCHUNK firstmovichunk;
			LONGLONG llFirstMoviChunkOffset = 0;
			do
			{
				if (list.fccListType == FCC('movi'))
				{
					// Store movi list position
					LONGLONG llMoviAddr = m_pFile->GetPosition();

					// Init vars in case of no index
					if (m_pFile->Read(&firstmovichunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
						throw (int)UNEXPECTED_EOF;
					llFirstMoviChunkOffset = llMoviAddr + sizeof(RIFFCHUNK);
					m_pFile->Seek(llMoviAddr, CFile::begin);

					// Skip movi List and read idx1 Chunk
					if (!SeekToNextChunkFromList(m_pFile, list, chunk))
						break; // Go ahead without index
					// Has index?
					if (chunk.fcc != FCC('idx1'))
						break; // Go ahead without index
				
					// Parse Legacy Index
					if (!ParseLegacyIndex(chunk.cb, llMoviAddr - 0x4))
						break; // Go ahead without index
					else
						return true;
				}
			}
			while (SeekToNextList(m_pFile, list));
			InitNoIndex(firstmovichunk, llFirstMoviChunkOffset);
		}

		return true;
	}
	catch (CFileException* e)
	{
		if (pVideoStream)
			delete pVideoStream;
		if (pAudioStream)
			delete pAudioStream;
		
		TCHAR szCause[255];
		e->GetErrorMessage(szCause, 255);
		CString str(szCause);
		str += _T("\n");
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();

		return false;
	}
	catch (int error_code)
	{
		if (pVideoStream)
			delete pVideoStream;
		if (pAudioStream)
			delete pAudioStream;

		CString str;
#ifdef _DEBUG
		str.Format(_T("Open(%s):\n"), szFileName);
#else
		str = _T("Error Opening AVI File:\n");
#endif
		switch (error_code)
		{
			case UNEXPECTED_EOF :				str += _T("Unexpected End-Of-File\n");
			break;
			case WRONG_LIST_TYPE :				str += _T("Wrong List Type\n");
			break;
			case WRONG_CHUNK_TYPE :				str += _T("Wrong Chunk Type\n");
			break;
			case WRONG_AVIMAINHEADER_SIZE :		str += _T("Wrong AVIMAINHEADER size\n");
			break;
			case WRONG_VIDEOSTREAM_HEADER :		str += _T("Wrong Video Stream Header\n");
			break;
			case WRONG_AUDIOSTREAM_HEADER :		str += _T("Wrong Audio Stream Header\n");
			break;
			case WRONG_SUPERINDEX_HEADER :		str += _T("Wrong Super Index Header\n");
			break;
			case WRONG_STANDARDINDEX_HEADER :	str += _T("Wrong Standard Index Header\n");
			break;
			case OUT_OF_MEM :					str += _T("Insufficient memory\n");
			break;
			default:							str += _T("Unspecified error\n");
			break;
		}

		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
	
		return false;
	}
}

__forceinline int CAVIPlay::GetChunkType(DWORD fcc)
{
	char Id = (char)(fcc >> 16);
	if (Id == 'd' || Id == '3') // Video Chunk, found some Avi's with FCC('0031') for video (see file IV31.avi)
		return 0;	
	else if (Id == 'w') // Audio Chunk
		return 1;
	else if (Id == 'p') // Palette change for palettized videos
		return 2;
	else
		return -1;
}

void CAVIPlay::InitNoIndex(const RIFFCHUNK& firstmovichunk, LONGLONG llFirstMoviChunkOffset)
{
	// Get stream(s) and file size
	CAVIVideoStream* pVideoStream = NULL;
	CAVIAudioStream* pAudioStream = NULL;
	if (m_VideoStreams.GetSize() > 0)
		pVideoStream = m_VideoStreams.GetAt(0);
	if (m_AudioStreams.GetSize() > 0)
		pAudioStream = m_AudioStreams.GetAt(0);
	ULARGE_INTEGER FileSize = GetFileSize();

	// Note:
	// If both streams present estimate a 15% audio bytes and 85% video bytes
	// Ignore multiple video or audio streams
	
	// Video
	if (pVideoStream)
	{
		// Init vars
		RIFFCHUNK chunk = firstmovichunk;
		m_pFile->Seek(llFirstMoviChunkOffset, CFile::begin);
		while (GetChunkType(chunk.fcc) != 0)
		{
			if (!SeekToNextChunk(m_pFile, chunk))
				throw (int)UNEXPECTED_EOF;
		}
		pVideoStream->m_FirstMoviChunk = chunk;
		pVideoStream->m_llFirstMoviChunkOffset = m_pFile->GetPosition();
		pVideoStream->m_LastReadMoviChunk = pVideoStream->m_FirstMoviChunk;
		pVideoStream->m_llLastReadMoviChunkOffset = pVideoStream->m_llFirstMoviChunkOffset;
		pVideoStream->m_dwLastReadMoviChunkNum = 0;
		pVideoStream->m_llBytesCount = pAudioStream ? FileSize.QuadPart * 85 / 100 : FileSize.QuadPart;

		// Estimate chunk number
		BOOL bExact = FALSE;
		DWORD dwChunksCount = 1;
		DWORD dwBytesCount = chunk.cb;
		while (dwChunksCount <= NOINDEX_STAT_CHUNKS)
		{
			if (!CAVIPlay::SeekToNextChunk(m_pFile, chunk))
			{
				// We now know the exact count!
				pVideoStream->m_dwChunksCount = dwChunksCount;
				pVideoStream->m_dwKeyFramesCount = dwChunksCount;
				bExact = TRUE;
				break;
			}
			else if (GetChunkType(chunk.fcc) == 0)
			{
				dwChunksCount++;
				dwBytesCount += chunk.cb;
			}
		}
		if (!bExact)
		{
			DWORD dwAvgChunkSize = dwBytesCount / dwChunksCount;
			pVideoStream->m_dwChunksCount = (DWORD)(pVideoStream->m_llBytesCount / (LONGLONG)dwAvgChunkSize);
			pVideoStream->m_dwKeyFramesCount = pVideoStream->m_dwChunksCount;
		}
	}

	// Audio
	if (pAudioStream)
	{
		// Init vars
		RIFFCHUNK chunk = firstmovichunk;
		m_pFile->Seek(llFirstMoviChunkOffset, CFile::begin);
		while (GetChunkType(chunk.fcc) != 1)
		{
			if (!SeekToNextChunk(m_pFile, chunk))
				throw (int)UNEXPECTED_EOF;
		}
		pAudioStream->m_FirstMoviChunk = chunk;
		pAudioStream->m_llFirstMoviChunkOffset = m_pFile->GetPosition();
		pAudioStream->m_LastReadMoviChunk = pAudioStream->m_FirstMoviChunk;
		pAudioStream->m_llLastReadMoviChunkOffset = pAudioStream->m_llFirstMoviChunkOffset;
		pAudioStream->m_dwLastReadMoviChunkNum = 0;
		pAudioStream->m_llBytesCount = pVideoStream ? FileSize.QuadPart * 15 / 100 : FileSize.QuadPart;

		// Estimate chunk number
		BOOL bExact = FALSE;
		DWORD dwChunksCount = 1;
		DWORD dwBytesCount = chunk.cb;
		while (dwChunksCount <= NOINDEX_STAT_CHUNKS)
		{
			if (!CAVIPlay::SeekToNextChunk(m_pFile, chunk))
			{
				// We now know the exact count!
				pAudioStream->m_dwChunksCount = dwChunksCount;
				bExact = TRUE;
				break;
			}
			else if (GetChunkType(chunk.fcc) == 1)
			{
				dwChunksCount++;
				dwBytesCount += chunk.cb;
			}
		}
		if (!bExact)
		{
			DWORD dwAvgChunkSize = dwBytesCount / dwChunksCount;
			pAudioStream->m_dwChunksCount = (DWORD)(pAudioStream->m_llBytesCount / (LONGLONG)dwAvgChunkSize);
		}
	}
}

bool CAVIPlay::ParseLegacyIndex(int nSize, LONGLONG llMoviAddr)
{
	// Vars
	bool bOldIndexBaseOk = false;
	LONGLONG llOldIndexBase = 0;
	int index, nCount;
	int nMinNumIndex;
	unsigned long Num, MinNum;
	int nDstIndex;
	DWORD dwStreamNum;
	bool res = true;

	DWORD VideoFourCCs[MAX_VIDEO_STREAMS];
	DWORD VideoFourCCsOrdered[MAX_VIDEO_STREAMS];
	int VideoEntriesCount[MAX_VIDEO_STREAMS];
	int nTotalVideoEntriesCount = 0;
	memset(VideoEntriesCount, 0, sizeof(int)*MAX_VIDEO_STREAMS);
	int nVideoFourCCsCount = 0;

	DWORD AudioFourCCs[MAX_AUDIO_STREAMS];
	DWORD AudioFourCCsOrdered[MAX_AUDIO_STREAMS];
	int AudioEntriesCount[MAX_AUDIO_STREAMS];
	int nTotalAudioEntriesCount = 0;
	memset(AudioEntriesCount, 0, sizeof(int)*MAX_AUDIO_STREAMS);
	int nAudioFourCCsCount = 0;

	DWORD PaletteFourCCs[MAX_VIDEO_STREAMS];
	DWORD PaletteFourCCsOrdered[MAX_VIDEO_STREAMS];
	int PaletteEntriesCount[MAX_VIDEO_STREAMS];
	int nTotalPaletteEntriesCount = 0;
	memset(PaletteEntriesCount, 0, sizeof(int)*MAX_VIDEO_STREAMS);
	int nPaletteFourCCsCount = 0;

	// Allocate & read index, find streams numeration and determine base address
	AVIOLDINDEXENTRY* pAviOldIndexTable = (AVIOLDINDEXENTRY*)BIGALLOC(nSize);
	if (!pAviOldIndexTable)
		goto error;
	if (m_pFile->Read(pAviOldIndexTable, nSize) < (UINT)nSize)
		goto error;
	LONGLONG orig_pos = m_pFile->GetPosition();
	for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
	{
		// First Find The Streams Numeration
		DWORD dwFourCC = (DWORD)(pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF);
		char Id = (char)(dwFourCC >> 16);
		if (Id == 'd' || Id == '3') // Video Chunk, found some Avi's with FCC('0031') for video (see file IV31.avi)
		{
			if (nVideoFourCCsCount > 0) // Next Video Streams
			{
				if (!IsFourCCInArray(dwFourCC, VideoFourCCs, nVideoFourCCsCount))
					VideoFourCCs[nVideoFourCCsCount++] = dwFourCC;
			}
			else // First Video Stream
				VideoFourCCs[nVideoFourCCsCount++] = dwFourCC;
			nTotalVideoEntriesCount++;
		}
		else if (Id == 'w') // Audio Chunk
		{
			if (nAudioFourCCsCount > 0) // Next Audio Streams
			{
				if (!IsFourCCInArray(dwFourCC, AudioFourCCs, nAudioFourCCsCount))
					AudioFourCCs[nAudioFourCCsCount++] = dwFourCC;
			}
			else // First Audio Stream
				AudioFourCCs[nAudioFourCCsCount++] = dwFourCC;
			nTotalAudioEntriesCount++;
		}
		else if (Id == 'p') // Palette change for palettized videos
		{
			if (nPaletteFourCCsCount > 0) // Next Palette Streams
			{
				if (!IsFourCCInArray(dwFourCC, PaletteFourCCs, nPaletteFourCCsCount))
					PaletteFourCCs[nPaletteFourCCsCount++] = dwFourCC;
			}
			else // First Palette Stream
				PaletteFourCCs[nPaletteFourCCsCount++] = dwFourCC;
			nTotalPaletteEntriesCount++;
		}

		// Notes on Base Address:
		// The Specifications does not states if the Old Index
		// Offsets are with respect to the file begin or with
		// respect to the Movi Begin.

		/*	But if the Offsets are
			smaller than the Movi Begin the Offsets have to have
			a base address of llMoviAddr. Right.
			-> Not working with Vista, because this new system is creating Avi files
			with a base address of llMoviAddr and some junk at the beginning
			of the movi list. This makes Offsets which are not smaller than llMoviAddr.
			-> The following code is not detecting the right offset.
			Also some other programs have the same problem, was that intentionally done
			by microsoft to create unplayable avi files?!
		if (!bOldIndexBaseOk && ((LONGLONG)pAviOldIndexTable[index].dwOffset < llMoviAddr))
		{
			bOldIndexBaseOk = true;
			llOldIndexBase = llMoviAddr;
		}
		*/

		// Working Code
		if (!bOldIndexBaseOk)
		{
			RIFFCHUNK chunk0, chunk1;
			m_pFile->Seek((LONGLONG)pAviOldIndexTable[index].dwOffset, CFile::begin);
			if (m_pFile->Read(&chunk0, sizeof(RIFFCHUNK)) != sizeof(RIFFCHUNK))
				goto error;
			m_pFile->Seek((LONGLONG)(llMoviAddr + pAviOldIndexTable[index].dwOffset), CFile::begin);
			if (m_pFile->Read(&chunk1, sizeof(RIFFCHUNK)) != sizeof(RIFFCHUNK))
				goto error;
			DWORD dwChunkFourCC0 = chunk0.fcc & 0x00FFFFFF;
			char cChunkId0 = (char)(dwChunkFourCC0 >> 16);
			DWORD dwChunkFourCC1 = chunk1.fcc & 0x00FFFFFF;
			char cChunkId1 = (char)(dwChunkFourCC1 >> 16);
			if (cChunkId0 == Id && pAviOldIndexTable[index].dwSize == chunk0.cb)
			{
				bOldIndexBaseOk = true;
			}
			else if (cChunkId1 == Id && pAviOldIndexTable[index].dwSize == chunk1.cb)
			{
				bOldIndexBaseOk = true;
				llOldIndexBase = llMoviAddr;
			}
		}
	}
	m_pFile->Seek((LONGLONG)orig_pos, CFile::begin);
	bOldIndexBaseOk = true;

	// Create Palette Streams
	for (dwStreamNum = 0 ; dwStreamNum < (DWORD)nPaletteFourCCsCount ; dwStreamNum++)
	{
		CAVIPaletteStream* pPaletteStream = (CAVIPaletteStream*)new CAVIPaletteStream(dwStreamNum, this, m_pFile);
		if (!pPaletteStream)
			goto error;
		m_PaletteStreams.Add(pPaletteStream);
	}

	// Order the Video Streams Enumeration
	for (nDstIndex = 0 ; nDstIndex < nVideoFourCCsCount ; nDstIndex++)
	{
		MinNum = 0xFFFFFFFF;
		for (int nSrcIndex = 0 ; nSrcIndex < nVideoFourCCsCount ; nSrcIndex++)
		{
			if (VideoFourCCs[nSrcIndex])
			{
				CString Str = FourCCToString(VideoFourCCs[nSrcIndex]);
				Num = _tcstoul(Str.Left(2), NULL, 10);
				if (Num < MinNum)
				{
					MinNum = Num;
					nMinNumIndex = nSrcIndex;
				}
			}
		}
		VideoFourCCsOrdered[nDstIndex] = VideoFourCCs[nMinNumIndex];
		VideoFourCCs[nMinNumIndex] = 0;
	}

	// Order the Audio Streams Enumeration
	for (nDstIndex = 0 ; nDstIndex < nAudioFourCCsCount ; nDstIndex++)
	{
		MinNum = 0xFFFFFFFF;
		for (int nSrcIndex = 0 ; nSrcIndex < nAudioFourCCsCount ; nSrcIndex++)
		{
			if (AudioFourCCs[nSrcIndex])
			{
				CString Str = FourCCToString(AudioFourCCs[nSrcIndex]);
				Num = _tcstoul(Str.Left(2), NULL, 10);
				if (Num < MinNum)
				{
					MinNum = Num;
					nMinNumIndex = nSrcIndex;
				}
			}
		}
		AudioFourCCsOrdered[nDstIndex] = AudioFourCCs[nMinNumIndex];
		AudioFourCCs[nMinNumIndex] = 0;
	}

	// Order the Palette Streams Enumeration
	for (nDstIndex = 0 ; nDstIndex < nPaletteFourCCsCount ; nDstIndex++)
	{
		MinNum = 0xFFFFFFFF;
		for (int nSrcIndex = 0 ; nSrcIndex < nPaletteFourCCsCount ; nSrcIndex++)
		{
			if (PaletteFourCCs[nSrcIndex])
			{
				CString Str = FourCCToString(PaletteFourCCs[nSrcIndex]);
				Num = _tcstoul(Str.Left(2), NULL, 10);
				if (Num < MinNum)
				{
					MinNum = Num;
					nMinNumIndex = nSrcIndex;
				}
			}
		}
		PaletteFourCCsOrdered[nDstIndex] = PaletteFourCCs[nMinNumIndex];
		PaletteFourCCs[nMinNumIndex] = 0;
	}

	// Count the Video Entries for each Video Stream
	for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
	{
		for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
		{
			if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == VideoFourCCsOrdered[dwStreamNum])
				++VideoEntriesCount[dwStreamNum];
		}
	}

	// Count the Audio Entries for each Audio Stream
	for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
	{
		for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
		{
			if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == AudioFourCCsOrdered[dwStreamNum])
				++AudioEntriesCount[dwStreamNum];
		}
	}

	// Count the Palette Entries for each Palette Stream
	for (dwStreamNum = 0 ; dwStreamNum < GetPaletteStreamsCount() ; dwStreamNum++)
	{
		for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
		{
			if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == PaletteFourCCsOrdered[dwStreamNum])
				++PaletteEntriesCount[dwStreamNum];
		}
	}

	// Create the Video Index for each Video Stream
	for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
	{
		CAVIVideoStream* p = m_VideoStreams.GetAt(dwStreamNum);
		p->m_llOldIndexBase = llOldIndexBase;
		p->m_dwOldIndexEntries = VideoEntriesCount[dwStreamNum];
		p->m_pOldIndexTable = (AVIOLDINDEXENTRY*)new AVIOLDINDEXENTRY[VideoEntriesCount[dwStreamNum]];
		if (!p->m_pOldIndexTable)
			goto error;
		nDstIndex = 0;
		for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
		{
			if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == VideoFourCCsOrdered[dwStreamNum])
				memcpy(&p->m_pOldIndexTable[nDstIndex++], &pAviOldIndexTable[index], sizeof(AVIOLDINDEXENTRY));
		}
	}

	// Create the Audio Index for each Audio Stream
	for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
	{
		CAVIAudioStream* p = m_AudioStreams.GetAt(dwStreamNum);
		p->m_llOldIndexBase = llOldIndexBase;
		p->m_dwOldIndexEntries = AudioEntriesCount[dwStreamNum];
		p->m_pOldIndexTable = (AVIOLDINDEXENTRY*)new AVIOLDINDEXENTRY[AudioEntriesCount[dwStreamNum]];
		if (!p->m_pOldIndexTable)
			goto error;
		nDstIndex = 0;
		for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
		{
			if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == AudioFourCCsOrdered[dwStreamNum])
				memcpy(&p->m_pOldIndexTable[nDstIndex++], &pAviOldIndexTable[index], sizeof(AVIOLDINDEXENTRY));
		}
	}

	// Create the Palette Index for each Palette Stream
	for (dwStreamNum = 0 ; dwStreamNum < GetPaletteStreamsCount() ; dwStreamNum++)
	{
		CAVIPaletteStream* p = m_PaletteStreams.GetAt(dwStreamNum);
		p->m_llOldIndexBase = llOldIndexBase;
		p->m_dwOldIndexEntries = PaletteEntriesCount[dwStreamNum];
		p->m_pOldIndexTable = (AVIOLDINDEXENTRY*)new AVIOLDINDEXENTRY[PaletteEntriesCount[dwStreamNum]];
		if (!p->m_pOldIndexTable)
			goto error;
		p->m_dwChangePaletteTableSize = VideoEntriesCount[dwStreamNum];
		p->m_pChangePaletteTable = (int*)new int[p->m_dwChangePaletteTableSize];
		if (!p->m_pChangePaletteTable)
			goto error;
		nDstIndex = 0;
		int nPrevDstIndex = 0;
		DWORD dwVideoChunkPos = 0;
		for (nCount = index = 0 ; nCount < (int)nSize ; nCount += sizeof(AVIOLDINDEXENTRY) , index++)
		{
			if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == VideoFourCCsOrdered[dwStreamNum])
			{
				if (nPrevDstIndex < nDstIndex)
				{
					p->m_pChangePaletteTable[dwVideoChunkPos++] = nPrevDstIndex;
					nPrevDstIndex = nDstIndex;
				}
				else
					p->m_pChangePaletteTable[dwVideoChunkPos++] = -1;
			}
			else if ((pAviOldIndexTable[index].dwChunkId & 0x00FFFFFF) == PaletteFourCCsOrdered[dwStreamNum])
				memcpy(&p->m_pOldIndexTable[nDstIndex++], &pAviOldIndexTable[index], sizeof(AVIOLDINDEXENTRY));
		}
	}

	// Calc. Video Streams Sizes in Bytes and the Key Frames Count
	for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
	{
		CAVIVideoStream* p = m_VideoStreams.GetAt(dwStreamNum);
		p->m_dwChunksCount = 0;
		p->m_llBytesCount = 0;
		p->m_dwKeyFramesCount = 0;
		for (index = 0 ; index < (int)p->m_dwOldIndexEntries ; index++)
		{
			p->m_dwChunksCount++;
			p->m_llBytesCount += p->m_pOldIndexTable[index].dwSize;
			if ((p->m_pOldIndexTable[index].dwFlags & AVIIF_KEYFRAME) == AVIIF_KEYFRAME)
				p->m_dwKeyFramesCount++;
		}
	}

	// Calc. Audio Streams Sizes in Bytes
	for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
	{
		CAVIAudioStream* p = m_AudioStreams.GetAt(dwStreamNum);
		p->m_dwChunksCount = 0;
		p->m_llBytesCount = 0;
		for (index = 0 ; index < (int)p->m_dwOldIndexEntries ; index++)
		{
			p->m_dwChunksCount++;
			p->m_llBytesCount += p->m_pOldIndexTable[index].dwSize;
		}
	}

	// Calc. Palette Streams Sizes in Bytes
	for (dwStreamNum = 0 ; dwStreamNum < GetPaletteStreamsCount() ; dwStreamNum++)
	{
		CAVIPaletteStream* p = m_PaletteStreams.GetAt(dwStreamNum);
		p->m_dwChunksCount = 0;
		p->m_llBytesCount = 0;
		for (index = 0 ; index < (int)p->m_dwOldIndexEntries ; index++)
		{
			p->m_dwChunksCount++;
			p->m_llBytesCount += p->m_pOldIndexTable[index].dwSize;
		}
	}

	// Assign Palette Streams to Video Streams
	for (dwStreamNum = 0 ; dwStreamNum < MIN(GetPaletteStreamsCount(), GetVideoStreamsCount()) ; dwStreamNum++)
	{
		CAVIVideoStream* pVideoStream = m_VideoStreams.GetAt(dwStreamNum);
		CAVIPaletteStream* pPaletteStream = m_PaletteStreams.GetAt(dwStreamNum);
		pVideoStream->SetPaletteStreamPointer(pPaletteStream);
	}

	goto free;
	
	// Error
error:
	res = false;

	// Free
free:
	if (pAviOldIndexTable)
		BIGFREE(pAviOldIndexTable);

	return res;
}

void CAVIPlay::CloseFile()
{
	if (m_pFile)
	{
		m_pFile->Close();
		delete m_pFile;
		m_pFile = NULL;
	}
}

bool CAVIPlay::IsFourCCInArray(DWORD dwFourCC, DWORD* pFourCCs, DWORD nSize)
{
	for (unsigned int i = 0 ; i < nSize ; i++)
		if (dwFourCC == pFourCCs[i])
			return true;

	return false;
}

CAVIPlay::CAVIPlay(bool bShowMessageBoxOnError/*=true*/) :
	m_bShowMessageBoxOnError(bShowMessageBoxOnError)
{
	m_bAVCodecPriority = true;
	InitWaveFormatTagTable();
	memset(&m_AviMainHdr, 0, sizeof(AVIMAINHDR));
	m_pFile = NULL;
	m_uiOpenFlags = 0;
	m_sFileName = _T("");
	::InitializeCriticalSection(&m_csAVI);
}

CAVIPlay::CAVIPlay(	LPCTSTR lpszFileName,
					bool bShowMessageBoxOnError/*=true*/) :
	m_bShowMessageBoxOnError(bShowMessageBoxOnError)
{
	m_bAVCodecPriority = true;
	InitWaveFormatTagTable();
	memset(&m_AviMainHdr, 0, sizeof(AVIMAINHDR));
	m_pFile = NULL;
	m_uiOpenFlags = 0;
	::InitializeCriticalSection(&m_csAVI);
	Open(lpszFileName);
}

void CAVIPlay::Close()
{
	ClearStreamsArrays();
	CloseFile();
	memset(&m_AviMainHdr, 0, sizeof(AVIMAINHDR));
}

// Input:	file, must point to the data begin of current chunk
//			chunk, current chunk	
//
// Output:	file,  points to the data begin of the seeked chunk
//			chunk, seeked chunk 
// Return:	position of file (data begin of the seeked chunk),
//			0 if an error happened
__forceinline LONGLONG CAVIPlay::SeekToNextChunk(CFile* pFile, RIFFCHUNK& chunk)
{
	if (!pFile)
		return 0;

	try
	{
		pFile->Seek((LONGLONG)RIFFROUND(chunk.cb), CFile::current);

		// Skip JUNK
		if (pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
			return 0;
		if (chunk.fcc == FCC('JUNK'))
			return SeekToNextChunk(pFile, chunk);
		else
			return (LONGLONG)pFile->GetPosition();
	}
	catch (CFileException* e)
	{
		e->Delete();	
		return 0;
	}
}

// Input:	file, must point to the data begin of current chunk
//			chunk, current chunk	
//			llNextListPos, for limitating the Seek
//
// Output:	file,  points to the data begin of the seeked chunk
//			chunk, seeked chunk
// Return:	position of file (data begin of the seeked chunk),
//			0 if an error happened
__forceinline LONGLONG CAVIPlay::SeekToNextChunk(CFile* pFile, RIFFCHUNK& chunk, LONGLONG llNextListPos)
{
	if (!pFile)
		return 0;

	try
	{
		pFile->Seek((LONGLONG)RIFFROUND(chunk.cb), CFile::current);

		// Skip JUNK
		if (pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
			return 0;
		if (chunk.fcc == FCC('JUNK'))
			return SeekToNextChunk(pFile, chunk, llNextListPos);
		else
		{
			if ((LONGLONG)pFile->GetPosition() < llNextListPos)
				return (LONGLONG)pFile->GetPosition();
			else
				return 0;
		}
	}
	catch (CFileException* e)
	{
		e->Delete();	
		return 0;
	}
}

// Input:	file, must point to the data begin of given list	
//			list, current list
//
// Output:	file,  points to the data begin of the seeked chunk
//			chunk, seeked chunk
// Return:	position of file (data begin of the seeked chunk),
//			0 if an error happened
__forceinline LONGLONG CAVIPlay::SeekToNextChunkFromList(CFile* pFile, const RIFFLIST& list, RIFFCHUNK& chunk)
{
	if (!pFile)
		return 0;

	try
	{
		pFile->Seek(RIFFROUND((LONGLONG)list.cb - 4), CFile::current);

		// Skip JUNK
		if (pFile->Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
			return 0;
		if (chunk.fcc == FCC('JUNK'))
			return SeekToNextChunk(pFile, chunk);
		else
			return (LONGLONG)pFile->GetPosition();
	}
	catch (CFileException* e)
	{
		e->Delete();	
		return 0;
	}
}

// Input:	file, must point to the data begin of current list
//			list, current list	
//
// Output:	file,  points to the data begin of the seeked list
//			list, seeked list
// Return:	position of file (data begin of the seeked list),
//			0 if an error happened
__forceinline LONGLONG CAVIPlay::SeekToNextList(CFile* pFile, RIFFLIST& list)
{
	if (!pFile)
		return 0;

	try
	{
		pFile->Seek(RIFFROUND((LONGLONG)list.cb - 4), CFile::current);

		// Skip JUNK
		if (pFile->Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
			return 0;
		if (list.fcc == FCC('JUNK'))
			return SeekToNextList(pFile, list);
		else
			return (LONGLONG)pFile->GetPosition();
	}
	catch (CFileException* e)
	{
		e->Delete();	
		return 0;
	}
}

void CAVIPlay::ClearStreamsArrays()
{
	while (m_VideoStreams.GetSize() > 0)
	{
		delete m_VideoStreams.GetAt(m_VideoStreams.GetUpperBound());
		m_VideoStreams.RemoveAt(m_VideoStreams.GetUpperBound());
	}
	while (m_AudioStreams.GetSize() > 0)
	{
		delete m_AudioStreams.GetAt(m_AudioStreams.GetUpperBound());
		m_AudioStreams.RemoveAt(m_AudioStreams.GetUpperBound());
	}
	while (m_PaletteStreams.GetSize() > 0)
	{
		delete m_PaletteStreams.GetAt(m_PaletteStreams.GetUpperBound());
		m_PaletteStreams.RemoveAt(m_PaletteStreams.GetUpperBound());
	}
}

DWORD CAVIPlay::StringToFourCC(CString sFourCC)
{
	char FourCC[5];
	wcstombs(FourCC, sFourCC, 5);
	return ((DWORD)(BYTE)(FourCC[0]) |
			((DWORD)(BYTE)(FourCC[1]) << 8) |
			((DWORD)(BYTE)(FourCC[2]) << 16) |
			((DWORD)(BYTE)(FourCC[3]) << 24 ));
}

CString CAVIPlay::FourCCToString(DWORD dwFourCC)
{
	char ch0 = (char)(dwFourCC & 0xFF);
	char ch1 = (char)((dwFourCC >> 8) & 0xFF);
	char ch2 = (char)((dwFourCC >> 16) & 0xFF);
	char ch3 = (char)((dwFourCC >> 24) & 0xFF);
	WCHAR wch0, wch1, wch2, wch3;
	mbtowc(&wch0, &ch0, sizeof(WCHAR));
	mbtowc(&wch1, &ch1, sizeof(WCHAR));
	mbtowc(&wch2, &ch2, sizeof(WCHAR));
	mbtowc(&wch3, &ch3, sizeof(WCHAR));
	return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
}

CString CAVIPlay::FourCCToStringLowerCase(DWORD dwFourCC)
{
	CString sFourCC = FourCCToString(dwFourCC);
	sFourCC.MakeLower();
	return sFourCC;
}

CString CAVIPlay::FourCCToStringUpperCase(DWORD dwFourCC)
{
	CString sFourCC = FourCCToString(dwFourCC);
	sFourCC.MakeUpper();
	return sFourCC;
}

DWORD CAVIPlay::FourCCMakeLowerCase(DWORD dwFourCC)
{
	return StringToFourCC(FourCCToStringLowerCase(dwFourCC));
}

DWORD CAVIPlay::FourCCMakeUpperCase(DWORD dwFourCC)
{
	return StringToFourCC(FourCCToStringUpperCase(dwFourCC));
}

ULARGE_INTEGER CAVIPlay::GetFileSize()
{
	ULARGE_INTEGER Size;
	Size.QuadPart = 0;

	if (m_sFileName == _T(""))
		return Size;

	HANDLE hFile = ::CreateFile(m_sFileName,
								0, // Only Query Access
								FILE_SHARE_READ |
								FILE_SHARE_WRITE,
								NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return Size;
	
	Size.LowPart = ::GetFileSize(hFile, &(Size.HighPart));

	::CloseHandle(hFile);

	return Size;
}

DWORD CAVIPlay::GetMinChunkSize()
{
	int nStreamNum;
	DWORD dwMinChunkSize = 0xFFFFFFFF;
	for (nStreamNum = 0 ; nStreamNum < m_VideoStreams.GetSize() ; nStreamNum++)
	{
		if (m_VideoStreams[nStreamNum])
		{
			if (m_VideoStreams[nStreamNum]->GetMinChunkSize() < dwMinChunkSize)
				dwMinChunkSize = m_VideoStreams[nStreamNum]->GetMinChunkSize();
		}
	}
	for (nStreamNum = 0 ; nStreamNum < m_AudioStreams.GetSize() ; nStreamNum++)
	{
		if (m_AudioStreams[nStreamNum])
		{
			if (m_AudioStreams[nStreamNum]->GetMinChunkSize() < dwMinChunkSize)
				dwMinChunkSize = m_AudioStreams[nStreamNum]->GetMinChunkSize();
		}
	}
	return dwMinChunkSize;
}

DWORD CAVIPlay::GetMaxChunkSize()
{
	int nStreamNum;
	DWORD dwMaxChunkSize = 0;
	for (nStreamNum = 0 ; nStreamNum < m_VideoStreams.GetSize() ; nStreamNum++)
	{
		if (m_VideoStreams[nStreamNum])
		{
			if (m_VideoStreams[nStreamNum]->GetMaxChunkSize() > dwMaxChunkSize)
				dwMaxChunkSize = m_VideoStreams[nStreamNum]->GetMaxChunkSize();
		}
	}
	for (nStreamNum = 0 ; nStreamNum < m_AudioStreams.GetSize() ; nStreamNum++)
	{
		if (m_AudioStreams[nStreamNum])
		{
			if (m_AudioStreams[nStreamNum]->GetMaxChunkSize() > dwMaxChunkSize)
				dwMaxChunkSize = m_AudioStreams[nStreamNum]->GetMaxChunkSize();
		}
	}
	return dwMaxChunkSize;
}

bool CAVIPlay::AviChangeVideoFrameRate(	LPCTSTR szFileName,
										DWORD dwVideoStreamNum,
										double dFrameRate,
										bool bShowMessageBoxOnError)
{
	DWORD dwRate;
	DWORD dwScale;
#ifndef DEFAULT_FRAME_RATE_BASE
#define DEFAULT_FRAME_RATE_BASE 1001000
#endif
	AVRational FrameRate = av_d2q(dFrameRate, DEFAULT_FRAME_RATE_BASE);
	dwRate = (DWORD)FrameRate.num;
	dwScale = (DWORD)FrameRate.den;
	return AviChangeVideoFrameRate(szFileName,
								   dwVideoStreamNum,
								   dwRate,
								   dwScale,
								   bShowMessageBoxOnError);
}

bool CAVIPlay::AviChangeVideoFrameRate(LPCTSTR szFileName,
									   DWORD dwVideoStreamNum,
									   DWORD dwRate,
									   DWORD dwScale,
									   bool bShowMessageBoxOnError)
{
	try
	{
		CFile f(szFileName,
				CFile::modeReadWrite |
				CFile::shareDenyNone);	
		RIFFLIST list;
		RIFFCHUNK chunk;
		DWORD dwCurrentVideoStreamNum = 0;

		// Read Start Avi List
		if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('RIFF')) ||
			(list.fccListType != FCC('AVI ') && list.fccListType != FCC('AVIX')))
			throw (int)WRONG_LIST_TYPE;

		// Read hdrl List
		if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('LIST')) ||
			list.fccListType != FCC('hdrl'))
			throw (int)WRONG_LIST_TYPE;

		// Read avih Chunk
		AVIMAINHDR AviMainHdr;
		if (f.Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK)) 
			throw (int)UNEXPECTED_EOF;
		if (chunk.fcc != FCC('avih'))
			throw (int)WRONG_CHUNK_TYPE;
		if (f.Read(&AviMainHdr, sizeof(AVIMAINHDR)) < sizeof(AVIMAINHDR))
			throw (int)UNEXPECTED_EOF;
		// Adjust File Position in case sizeof(AVIMAINHDR) != chunk.cb
		f.Seek(-((LONGLONG)sizeof(AVIMAINHDR)), CFile::current);
		f.Seek((LONGLONG)chunk.cb, CFile::current);

		// Read the streams
		for (DWORD dwStreamNum = 0 ; dwStreamNum < AviMainHdr.dwStreams ; dwStreamNum++)
		{
			// Read strl List
			while (TRUE)
			{
				if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
					throw (int)UNEXPECTED_EOF;
				if ((list.fcc != FCC('LIST')) ||
					list.fccListType != FCC('strl'))
					f.Seek(RIFFROUND((LONGLONG)list.cb - 4), CFile::current);
				else
					break;
			}

			// Calc the Next List Pos
			DWORD dwListSize = list.cb - 4;
			LONGLONG llNextListPos = f.GetPosition() + dwListSize;

			// Read strh Chunk
			AVISTREAMHDR AviStreamHdr;
			if (f.Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK)) 
				throw (int)UNEXPECTED_EOF;
			if (chunk.fcc != FCC('strh'))
				throw (int)WRONG_CHUNK_TYPE;
			LONGLONG llAviStreamHdrPos = f.GetPosition();
			if (f.Read(&AviStreamHdr, sizeof(AVISTREAMHDR)) < sizeof(AVISTREAMHDR))
				throw (int)UNEXPECTED_EOF;
			f.Seek(-((LONGLONG)sizeof(AVISTREAMHDR)), CFile::current);
			if (AviStreamHdr.fccType == streamtypeVIDEO)
			{
				// Change Avi Stream Header
				if (dwCurrentVideoStreamNum == dwVideoStreamNum)
				{
					AviStreamHdr.dwRate = dwRate;
					AviStreamHdr.dwScale = dwScale;
					f.Seek((LONGLONG)llAviStreamHdrPos, CFile::begin);
					f.Write(&AviStreamHdr, sizeof(AVISTREAMHDR));
					f.Flush();
				}
				
				++dwCurrentVideoStreamNum;
			}

			// Go to next strl List
			f.Seek((LONGLONG)llNextListPos, CFile::begin);
		}

		f.Close();

		return true;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		e->GetErrorMessage(szCause, 255);
		CString str(szCause);
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return false;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("AviChangeVideoFrameRate(%s):\n"), szFileName);
#else
		str = _T("Error while changing Video Frame Rate:\n");
#endif
		switch (error_code)
		{
			case UNEXPECTED_EOF :			str += _T("Unexpected End-Of-File\n");
			break;
			case WRONG_LIST_TYPE :			str += _T("Wrong List Type\n");
			break;
			case WRONG_CHUNK_TYPE :			str += _T("Wrong Chunk Type\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return false;
	}
}

bool CAVIPlay::AviChangeVideoStartOffset(LPCTSTR szFileName,
										 DWORD dwVideoStreamNum,
										 DWORD dwStart,
										 bool bShowMessageBoxOnError)
{
	try
	{
		CFile f(szFileName,
				CFile::modeReadWrite |
				CFile::shareDenyNone);	
		RIFFLIST list;
		RIFFCHUNK chunk;
		DWORD dwCurrentVideoStreamNum = 0;

		// Read Start Avi List
		if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST)) 
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('RIFF')) ||
			(list.fccListType != FCC('AVI ') && list.fccListType != FCC('AVIX')))
			throw (int)WRONG_LIST_TYPE;

		// Read hdrl List
		if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST)) 
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('LIST')) ||
			list.fccListType != FCC('hdrl'))
			throw (int)WRONG_LIST_TYPE;

		// Read avih Chunk
		AVIMAINHDR AviMainHdr;
		if (f.Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
			throw (int)UNEXPECTED_EOF;
		if (chunk.fcc != FCC('avih'))
			throw (int)WRONG_CHUNK_TYPE;
		if (f.Read(&AviMainHdr, sizeof(AVIMAINHDR)) < sizeof(AVIMAINHDR))
			throw (int)UNEXPECTED_EOF;
		// Adjust File Position in case sizeof(AVIMAINHDR) != chunk.cb
		f.Seek(-((LONGLONG)sizeof(AVIMAINHDR)), CFile::current);
		f.Seek((LONGLONG)chunk.cb, CFile::current);

		// Read the streams
		for (DWORD dwStreamNum = 0 ; dwStreamNum < AviMainHdr.dwStreams ; dwStreamNum++)
		{
			// Read strl List
			while (TRUE)
			{
				if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
					throw (int)UNEXPECTED_EOF;
				if ((list.fcc != FCC('LIST')) ||
					list.fccListType != FCC('strl'))
					f.Seek(RIFFROUND((LONGLONG)list.cb - 4), CFile::current);
				else
					break;
			}

			// Calc the Next List Pos
			DWORD dwListSize = list.cb - 4;
			LONGLONG llNextListPos = f.GetPosition() + dwListSize;

			// Read strh Chunk
			AVISTREAMHDR AviStreamHdr;
			if (f.Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
				throw (int)UNEXPECTED_EOF;
			if (chunk.fcc != FCC('strh'))
				throw (int)WRONG_CHUNK_TYPE;
			LONGLONG llAviStreamHdrPos = f.GetPosition();
			if (f.Read(&AviStreamHdr, sizeof(AVISTREAMHDR)) < sizeof(AVISTREAMHDR))
				throw (int)UNEXPECTED_EOF;
			f.Seek(-((LONGLONG)sizeof(AVISTREAMHDR)), CFile::current);
			if (AviStreamHdr.fccType == streamtypeVIDEO)
			{
				// Change Avi Stream Header
				if (dwCurrentVideoStreamNum == dwVideoStreamNum)
				{
					AviStreamHdr.dwStart = dwStart;
					f.Seek((LONGLONG)llAviStreamHdrPos, CFile::begin);
					f.Write(&AviStreamHdr, sizeof(AVISTREAMHDR));
					f.Flush();
				}
				
				++dwCurrentVideoStreamNum;
			}

			// Go to next strl List
			f.Seek((LONGLONG)llNextListPos, CFile::begin);
		}

		f.Close();

		return true;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		e->GetErrorMessage(szCause, 255);
		CString str(szCause);
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return false;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("AviChangeVideoStartOffset(%s):\n"), szFileName);
#else
		str = _T("Error while changing Video Start Offset:\n");
#endif	
		switch (error_code)
		{
			case UNEXPECTED_EOF :			str += _T("Unexpected End-Of-File\n");
			break;
			case WRONG_LIST_TYPE :			str += _T("Wrong List Type\n");
			break;
			case WRONG_CHUNK_TYPE :			str += _T("Wrong Chunk Type\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return false;
	}
}

bool CAVIPlay::AviChangeAudioStartOffset(LPCTSTR szFileName,
										 DWORD dwAudioStreamNum,
										 DWORD dwStart,
										 bool bShowMessageBoxOnError)
{
	try
	{
		CFile f(szFileName,
				CFile::modeReadWrite |
				CFile::shareDenyNone);
		RIFFLIST list;
		RIFFCHUNK chunk;
		DWORD dwCurrentAudioStreamNum = 0;

		// Read Start Avi List
		if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST)) 
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('RIFF')) ||
			(list.fccListType != FCC('AVI ') && list.fccListType != FCC('AVIX')))
			throw (int)WRONG_LIST_TYPE;

		// Read hdrl List
		if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST)) 
			throw (int)UNEXPECTED_EOF;
		if ((list.fcc != FCC('LIST')) ||
			list.fccListType != FCC('hdrl'))
			throw (int)WRONG_LIST_TYPE;

		// Read avih Chunk
		AVIMAINHDR AviMainHdr;
		if (f.Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
			throw (int)UNEXPECTED_EOF;
		if (chunk.fcc != FCC('avih'))
			throw (int)WRONG_CHUNK_TYPE;
		if (f.Read(&AviMainHdr, sizeof(AVIMAINHDR)) < sizeof(AVIMAINHDR))
			throw (int)UNEXPECTED_EOF;
		// Adjust File Position in case sizeof(AVIMAINHDR) != chunk.cb
		f.Seek(-((LONGLONG)sizeof(AVIMAINHDR)), CFile::current);
		f.Seek((LONGLONG)chunk.cb, CFile::current);

		// Read the streams
		for (DWORD dwStreamNum = 0 ; dwStreamNum < AviMainHdr.dwStreams ; dwStreamNum++)
		{
			// Read strl List
			while (TRUE)
			{
				if (f.Read(&list, sizeof(RIFFLIST)) < sizeof(RIFFLIST))
					throw (int)UNEXPECTED_EOF;
				if ((list.fcc != FCC('LIST')) ||
					list.fccListType != FCC('strl'))
					f.Seek(RIFFROUND((LONGLONG)list.cb - 4), CFile::current);
				else
					break;
			}

			// Calc the Next List Pos
			DWORD dwListSize = list.cb - 4;
			LONGLONG llNextListPos = f.GetPosition() + dwListSize;

			// Read strh Chunk
			AVISTREAMHDR AviStreamHdr;
			if (f.Read(&chunk, sizeof(RIFFCHUNK)) < sizeof(RIFFCHUNK))
				throw (int)UNEXPECTED_EOF;
			if (chunk.fcc != FCC('strh'))
				throw (int)WRONG_CHUNK_TYPE;
			LONGLONG llAviStreamHdrPos = f.GetPosition();
			if (f.Read(&AviStreamHdr, sizeof(AVISTREAMHDR)) < sizeof(AVISTREAMHDR))
				throw (int)UNEXPECTED_EOF;
			f.Seek(-((LONGLONG)sizeof(AVISTREAMHDR)), CFile::current);
			if (AviStreamHdr.fccType == streamtypeAUDIO)
			{
				// Change Avi Stream Header
				if (dwCurrentAudioStreamNum == dwAudioStreamNum)
				{
					AviStreamHdr.dwStart = dwStart;
					f.Seek((LONGLONG)llAviStreamHdrPos, CFile::begin);
					f.Write(&AviStreamHdr, sizeof(AVISTREAMHDR));
					f.Flush();
				}
				
				++dwCurrentAudioStreamNum;
			}

			// Go to next strl List
			f.Seek((LONGLONG)llNextListPos, CFile::begin);
		}

		f.Close();

		return true;
	}
	catch (CFileException* e)
	{
		TCHAR szCause[255];
		e->GetErrorMessage(szCause, 255);
		CString str(szCause);
		str += _T("\n");
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		e->Delete();
		return false;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("AviChangeAudioStartOffset(%s):\n"), szFileName);
#else
		str = _T("Error while changing Audio Start Offset:\n");
#endif	
		switch (error_code)
		{
			case UNEXPECTED_EOF :			str += _T("Unexpected End-Of-File\n");
			break;
			case WRONG_LIST_TYPE :			str += _T("Wrong List Type\n");
			break;
			case WRONG_CHUNK_TYPE :			str += _T("Wrong Chunk Type\n");
			break;
			default:						str += _T("Unspecified error\n");
			break;
		}
		
		TRACE(str);
		if (bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);

		return false;
	}
}