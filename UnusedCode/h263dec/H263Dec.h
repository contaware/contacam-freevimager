#ifndef _INC_H263DECCLASS
#define _INC_H263DECCLASS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "decdef.h"

class CDib;

class CH263Dec
{	
	public:
		struct ld_struct
		{
			/* bit input */
			unsigned char rdbfr[2051];
			unsigned char *rdptr;
			unsigned char inbfr[16];
			int incnt;
			int bitcnt;

			/* block data */
			short block[12][64];
		};

		struct VLCtab
		{
			int val, len;
		};

		struct DCTtab
		{
			char run, level, len;
		};

		struct VLCtabI
		{
			int val, len;
		};

		struct RunCoef
		{
			int val, run, sign;
		};

	public:
		CH263Dec();
		virtual ~CH263Dec();
		bool DecodeH263(CDib* pDib, unsigned char* pH263Bits, unsigned int uiSize);

	protected:
		void error(char *text);
		void printbits(int code, int bits, int len);
		void Init();
		int DecompressFrame(unsigned char *cdata,
							int size,
							unsigned char *outdata,
							int outsize,
							int& first,
							unsigned __int64& framenum);
		void Free();
		void initdecoder();
		bool GetFrameSize(unsigned char* pH263Bits, unsigned int uiSize);

		// Get Bits
		void initbits();
		void fillbfr();
		unsigned int showbits(int n);
		unsigned int getbits1();
		void flushbits(int n);
		unsigned int getbits(int n);

		// Get Blocks
		RunCoef vlc_word_decode (int symbol_word, int *last); 
		RunCoef Decode_Escape_Char (int intra, int *last);
		int DecodeTCoef (int position, int intra);
		void getblock (int comp, int mode);
		void get_sac_block (int comp, int mode);

		// Get Header
		int getheader();
		void startcode();
		void getpicturehdr();

		// Get Picture
		void getpicture(unsigned __int64* framenum);
		void getMBs();
		void clearblock(int comp);
		int motion_decode(int vec,int pmv);
		int find_pmv(int x, int y, int block, int comp);
		void addblock(int comp, int bx, int by,int addflag);
		void reconblock_b(int comp,int bx,int by,int mode,int bdx, int bdy);
		void find_bidir_limits(int vec, int *start, int*stop, int nhv);
		void find_bidir_chroma_limits(int vec, int *start, int*stop);
		void make_edge_image(unsigned char *src, unsigned char *dst, int width, int height, int edge);
		void interpolate_image(unsigned char *in, unsigned char *out, int width, int height);
		void storeframe(unsigned char *src[]);

		// Get Vlc
		int getTMNMV();
		int getMCBPC();
		int getMODB();
		int getMCBPCintra();
		int getCBPY();

		// IDCT
		void init_idct();
		void idct(short *block);
		void idctrow(short *blk);
		void idctcol(short *blk);

		// IDCT Ref
		void init_idctref();
		void idctref(short *block);

		// Recon
		void reconstruct(int bx,int by,int P,int bdx,int bdy);
		void recon_comp(unsigned char *src, unsigned char *dst,int lx, int lx2, int w, int h, int x, int y, int dx, int dy, int flag);
		void recon_comp_obmc(unsigned char *src, unsigned char *dst,int lx,int lx2,int comp,int w,int h,int x,int y);
		void rec(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void recc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void reco(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
		void rech(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void rechc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void recho(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
		void recv(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void recvc(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void recvo(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);
		void rec4(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void rec4c(unsigned char *s, unsigned char *d, int lx, int lx2, int h);
		void rec4o(unsigned char *s, int *d, int lx, int lx2, int addflag,int c, int xa, int xb, int ya, int yb);

		// SAC (Syntax Arithmetic Coding)
		int decode_a_symbol(const int cumul_freq[]);
		void decoder_reset();
		void bit_out_psc_layer();

		// Vars

		int m_nFirst;
		unsigned __int64 m_FrameNum;
		int m_quiet;
		int m_trace;
		unsigned char *m_refframe[3], *m_oldrefframe[3], *m_bframe[3], *m_newframe[3];
		unsigned char *m_edgeframe[3], *m_edgeframeorig[3], *m_exnewframe[3];
		int m_MV[2][5][MBR+1][MBC+2];
		int m_modemap[MBR+1][MBC+2];
		unsigned char* m_clp_base; 
		unsigned char* m_clp;
		int m_horizontal_size, m_vertical_size, m_mb_width, m_mb_height;
		int m_coded_picture_width, m_coded_picture_height;
		int m_chrom_width, m_chrom_height, m_blk_cnt;
		int m_pict_type, m_newgob;
		int m_mv_outside_frame, m_syntax_arith_coding, m_adv_pred_mode, m_pb_frame;
		int m_long_vectors;
		int m_fault, m_expand;
		int m_refidct;	// Do Use Reference IDCT Flag
		int m_matrix_coefficients;
		int m_temp_ref, m_prev_temp_ref, m_quant, m_source_format;
		unsigned char* m_cframe;
		int m_csize;
		int m_cindex;
		ld_struct m_base;
		ld_struct* m_ld;
		unsigned char *m_yp,*m_up,*m_vp;
		int m_outtype;
		FILE* m_dlog; // log file for decoder
		int m_trd, m_trb, m_bscan, m_bquant;

		// Conversion from YUV420 to RGB24
		long int m_crv_tab[256];
		long int m_cbu_tab[256];
		long int m_cgu_tab[256];
		long int m_cgv_tab[256];
		long int m_tab_76309[256];
		unsigned char m_YUV420ToRGB24Clip[1024]; // for clip in CCIR601

		// IDCT
		short m_iclip[1024]; /* clipping table */
		short* m_iclp;

		// IDCT Ref
		double m_cos[8][8]; /* cosine transform matrix for 8x1 IDCT */

		// Sac
		long m_sac_low;
		long m_sac_high;
		long m_sac_code_value;
		long m_sac_bit;
		long m_sac_length;
		long m_sac_index;
		long m_sac_cum;
		long m_sac_zerorun;


		static const int m_bscan_tab[];
		static const int m_bquant_tab[];
		static const int m_roundtab[16];
		static const unsigned char m_zig_zag_scan[64];
		static const int m_convmat[8][4];
		static const int m_OM[5][8][8];
		static const VLCtabI m_DCT3Dtab0[];
		static const VLCtabI m_DCT3Dtab1[];
		static const VLCtabI m_DCT3Dtab2[];
		static const unsigned int m_msk[33];
		static const VLCtab m_TMNMVtab0[];
		static const VLCtab m_TMNMVtab1[];
		static const VLCtab m_TMNMVtab2[];
		static const VLCtab m_MCBPCtab[];
		static const VLCtab m_MCBPCtabintra[];
		static const VLCtab m_CBPYtab[48];

		static const int m_codtab[];
		static const int m_mcbpctab[];
		static const int m_mcbpc_intratab[];
		static const int m_modb_tab[];
		static const int m_ycbpb_tab[];
		static const int m_uvcbpb_tab[];
		static const int m_cbpytab[];
		static const int m_cbpy_intratab[];
		static const int m_dquanttab[];
		static const int m_mvdtab[];
		static const int m_intradctab[];
		static const int m_tcoeftab[];
		static const int m_signtab[];
		static const int m_lasttab[];
		static const int m_last_intratab[];
		static const int m_runtab[];
		static const int m_leveltab[];

		static const int m_cumf_COD[];
		static const int m_cumf_MCBPC[];
		static const int m_cumf_MCBPC_intra[];
		static const int m_cumf_MODB[];
		static const int m_cumf_YCBPB[];
		static const int m_cumf_UVCBPB[];
		static const int m_cumf_CBPY[];
		static const int m_cumf_CBPY_intra[];
		static const int m_cumf_DQUANT[];
		static const int m_cumf_MVD[];
		static const int m_cumf_INTRADC[];
		static const int m_cumf_TCOEF1[];
		static const int m_cumf_TCOEF2[];
		static const int m_cumf_TCOEF3[];
		static const int m_cumf_TCOEFr[];
		static const int m_cumf_TCOEF1_intra[];
		static const int m_cumf_TCOEF2_intra[];
		static const int m_cumf_TCOEF3_intra[];
		static const int m_cumf_TCOEFr_intra[];
		static const int m_cumf_SIGN[];
		static const int m_cumf_LAST[];
		static const int m_cumf_LAST_intra[];
		static const int m_cumf_RUN[];
		static const int m_cumf_RUN_intra[];
		static const int m_cumf_LEVEL[];
		static const int m_cumf_LEVEL_intra[];
};

#endif //!_INC_H263DECCLASS