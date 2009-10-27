////////////////////////////////////////////////////////////////////////////
//
//
//    Project     : VideoNet version 1.1.
//    Description : Peer to Peer Video Conferencing over the LAN.
//	  Author      :	Nagareshwar Y Talekar ( nsry2002@yahoo.co.in)
//    Date        : 15-6-2004.
//
//    This is the modified version of tmndecode (H.263 decoder) 
//    written by Karl & Robert.It was in ANSI C. I have converted into C++
//    so that it can be integrated into any windows application. I have 
//    removed some of the files which had display and file storing 
//    functions.I have removed the unnecessary code and also added some
//    new files..
//	  Original library dealt with files. Input & Output , both were files.
//    I have done some major changes so that it can be used for real time 
//    decoding process. Now one can use this library for decoding H263 frames. 
//
//
//    File description : 
//    Name    : GetPic.cpp
//
/////////////////////////////////////////////////////////////////////////////



/************************************************************************
 *
 *  getpic.c, picture decoding for tmndecode (H.263 decoder)
 *  Copyright (C) 1996  Telenor R&D, Norway
 *        Karl Olav Lillevold <Karl.Lillevold@nta.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Karl Olav Lillevold               <Karl.Lillevold@nta.no>
 *  Telenor Research and Development
 *  P.O.Box 83                        tel.:   +47 63 84 84 00
 *  N-2007 Kjeller, Norway            fax.:   +47 63 81 00 76
 *
 *  Robert Danielsen                  e-mail: Robert.Danielsen@nta.no
 *  Telenor Research and Development  www:    http://www.nta.no/brukere/DVC/
 *  P.O.Box 83                        tel.:   +47 63 84 84 00
 *  N-2007 Kjeller, Norway            fax.:   +47 63 81 00 76
 *  
 ************************************************************************/

/*
 * modified by Wayne Ellis BT Labs to run Annex E Arithmetic Decoding
 *           <ellis_w_wayne@bt-web.bt.co.uk>
 *
 * based on mpeg2decode, (C) 1994, MPEG Software Simulation Group
 * and mpeg2play, (C) 1994 Stefan Eckart
 *                         <stefan@lis.e-technik.tu-muenchen.de>
 */

#include "stdafx.h"
#include "H263Dec.h"

/* decode one frame or field picture */
void CH263Dec::getpicture(unsigned __int64* framenum)
{
	int i;
	unsigned char *tmp;

	for (i=0; i<3; i++)
	{
		tmp = m_oldrefframe[i];
		m_oldrefframe[i] = m_refframe[i];
		m_refframe[i] = tmp;
		m_newframe[i] = m_refframe[i];
	}

	if (m_mv_outside_frame && *framenum > 0)
	{
		make_edge_image(m_oldrefframe[0],
						m_edgeframe[0],
						m_coded_picture_width,
						m_coded_picture_height,32);
		make_edge_image(m_oldrefframe[1],
						m_edgeframe[1],
						m_chrom_width,
						m_chrom_height,16);
		make_edge_image(m_oldrefframe[2],
						m_edgeframe[2],
						m_chrom_width,
						m_chrom_height,16);
	}
	getMBs();

	if (m_pb_frame)
	{
		if (m_expand && m_outtype == T_X11)
		{
			interpolate_image(	m_bframe[0],
								m_exnewframe[0],
								m_coded_picture_width,
								m_coded_picture_height);
			interpolate_image(	m_bframe[1],
								m_exnewframe[1],
								m_chrom_width,
								m_chrom_height);
			interpolate_image(	m_bframe[2],
								m_exnewframe[2],
								m_chrom_width,
								m_chrom_height);
			storeframe(m_exnewframe);
		}
		else
			storeframe(m_bframe);

		*framenum += m_pb_frame;
	}

	if (m_expand && m_outtype == T_X11)
	{
		interpolate_image(	m_newframe[0],
							m_exnewframe[0],
							m_coded_picture_width,
							m_coded_picture_height);
		interpolate_image(	m_newframe[1],
							m_exnewframe[1],
							m_chrom_width,
							m_chrom_height);
		interpolate_image(	m_newframe[2],
							m_exnewframe[2],
							m_chrom_width,
							m_chrom_height);
		storeframe(m_exnewframe);
	}
	else
		storeframe(m_newframe);
}


/**
*   Store the pointer to Y, U , V frames...
*  
*/
void CH263Dec::storeframe(unsigned char *src[])
{
	m_yp=src[0];		// Y component
	m_up=src[1];        // U component
	m_vp=src[2];        // V component
}


/* decode all macroblocks of the current picture */
void CH263Dec::getMBs()
{
  int comp;
  int MBA, MBAmax;
  int bx, by;

  int COD=0,MCBPC, CBPY, CBP=0, CBPB=0, MODB=0, Mode=0, DQUANT;
  int COD_index, CBPY_index, MODB_index, DQUANT_index, MCBPC_index;
  int INTRADC_index, YCBPB_index, UVCBPB_index, mvdbx_index, mvdby_index;
  int mvx, mvy, mvy_index, mvx_index, pmv0, pmv1, xpos, ypos, gob, i,k;
  int mvdbx=0, mvdby=0, pmvdbx, pmvdby, gfid, YCBPB, UVCBPB, gobheader_read;
  int startmv,stopmv,offset,bsize,last_done=0,pCBP=0,pCBPB=0,pCOD=0;
  int DQ_tab[4] = {-1,-2,1,2};
  short *bp;

  /* number of macroblocks per picture */
  MBAmax = m_mb_width*m_mb_height;

  MBA = 0; /* macroblock address */
  m_newgob = 0;

  /* mark m_MV's above the picture */
  for (i = 1; i < m_mb_width+1; i++) {
    for (k = 0; k < 5; k++) {
      m_MV[0][k][0][i] = NO_VEC;
      m_MV[1][k][0][i] = NO_VEC;
    }
    m_modemap[0][i] = MODE_INTRA;
  }
  /* zero m_MV's on the sides of the picture */
  for (i = 0; i < m_mb_height+1; i++) {
    for (k = 0; k < 5; k++) {
      m_MV[0][k][i][0] = 0;
      m_MV[1][k][i][0] = 0;
      m_MV[0][k][i][m_mb_width+1] = 0;
      m_MV[1][k][i][m_mb_width+1] = 0;
    }
    m_modemap[i][0] = MODE_INTRA;
    m_modemap[i][m_mb_width+1] = MODE_INTRA;
  }

  m_fault = 0;
  gobheader_read = 0;
  
  for (;;) {
  resync:

    /* This version of the decoder does not resync on every possible
       error, and it does not do all possible error checks. It is not
       difficult to make it much more error robust, but I do not think
       it is necessary to include this in the freely available
       version. */

    if (m_fault) {
        if(m_trace)
		fputs("Warning: A Fault Condition Has Occurred - Resyncing \n",m_dlog);
      startcode();  /* sync on new startcode */
      m_fault = 0;
    }

    if (!(showbits(22)>>6)) { /* startcode */

      startcode();  
      /* in case of byte aligned start code, ie. PSTUF, GSTUF or ESTUF
         is used */
      
      if (showbits(22) == (32|SE_CODE)) { /* end of sequence */
        if (!(m_syntax_arith_coding && MBA < MBAmax)) {
          return;
        }
      }
      else if ((showbits(22) == PSC<<5) ) { /* new picture */
        if (!(m_syntax_arith_coding && MBA < MBAmax)) {
          return;
        }
      }
      else {
        if (!(m_syntax_arith_coding && MBA%m_mb_width)) {

          if (m_syntax_arith_coding) {   /* SAC hack to finish GOBs which   */
            gob = (showbits(22) & 31); /* end with MBs coded with no bits */
            if (gob * m_mb_width != MBA) 
              goto finish_gob;
          }

          gob = getheader() - 1;
          if (gob > m_mb_height) {
            if (!m_quiet)
              fputs("GN out of range\n",m_dlog);
            return;
          }
          
          /* GFID is not allowed to change unless PTYPE in picture header 
             changes */
          gfid = getbits(2);
          /* NB: in error-prone environments the decoder can use this
             value to determine whether a picture header where the PTYPE
             has changed, has been lost */
          
          m_quant = getbits(5);
          //if (trace)
          //  printf("GQUANT: %d\n", quant);
          xpos = 0;
          ypos = gob;
          MBA = ypos * m_mb_width;
          
          m_newgob = 1;
          gobheader_read = 1;
          if (m_syntax_arith_coding) 
            decoder_reset();	/* init. arithmetic decoder buffer after gob */
        }
      }
    }

  finish_gob:  /* SAC specific label */

    if (!gobheader_read) {
      xpos = MBA%m_mb_width;
      ypos = MBA/m_mb_width;
      if (xpos == 0 && ypos > 0)
        m_newgob = 0;
    }
    else 
      gobheader_read = 0;

    if (MBA>=MBAmax) 
      return; /* all macroblocks decoded */

  read_cod:
    if (m_syntax_arith_coding) { 
      if (m_pict_type == PCT_INTER) {
        COD_index = decode_a_symbol(m_cumf_COD);
        COD = m_codtab[COD_index];
        if (m_trace) {
          fputs("Arithmetic Decoding Debug \n",m_dlog);
        }
      }
      else
        COD = 0;  /* COD not used in I-pictures, set to zero */
    }
    else {
      if (m_pict_type == PCT_INTER) 
        COD = showbits(1);
      else
        COD = 0; /* Intra picture -> not skipped */
    }

    if (!COD) {  /* COD == 0 --> not skipped */    

      if (m_syntax_arith_coding)  {
        if (m_pict_type == PCT_INTER) {
          MCBPC_index = decode_a_symbol(m_cumf_MCBPC);
          MCBPC = m_mcbpctab[MCBPC_index];
        }	
        else {
          MCBPC_index = decode_a_symbol(m_cumf_MCBPC_intra);
          MCBPC = m_mcbpc_intratab[MCBPC_index];
        }
        //if (trace) 
        //  printf("MCBPC Index: %d MCBPC: %d \n",MCBPC_index, MCBPC);
      }

      else {
        if (m_pict_type == PCT_INTER)
          flushbits(1); /* flush COD bit */
        if (m_pict_type == PCT_INTRA) 
          MCBPC = getMCBPCintra();
        else
          MCBPC = getMCBPC();
      }

      if (m_fault) goto resync;
      
      if (MCBPC == 255) { /* stuffing */
        goto read_cod;   /* read next COD without advancing MB count */
      }

      else {             /* normal MB data */

        Mode = MCBPC & 7;

        /* MODB and CBPB */
        if (m_pb_frame) {
          CBPB = 0;
          if (m_syntax_arith_coding)  {
            MODB_index = decode_a_symbol(m_cumf_MODB);
            MODB = m_modb_tab[MODB_index];
          }
          else 
            MODB = getMODB();
          //if (trace)
           // printf("MODB: %d\n", MODB);
          if (MODB == PBMODE_CBPB_MVDB) {
            if (m_syntax_arith_coding)  {
              for(i=0; i<4; i++) {
        YCBPB_index = decode_a_symbol(m_cumf_YCBPB);
        YCBPB = m_ycbpb_tab[YCBPB_index];
        CBPB |= (YCBPB << (6-1-i));
              }
 
              for(i=4; i<6; i++) {
        UVCBPB_index = decode_a_symbol(m_cumf_UVCBPB);
        UVCBPB = m_uvcbpb_tab[UVCBPB_index];
        CBPB |= (UVCBPB << (6-1-i));
              }
            }
            else
              CBPB = getbits(6);
            //if (trace)
            //  printf("CBPB = %d\n",CBPB);
          }
        }

        if (m_syntax_arith_coding) {

          if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) { /* Intra */
            CBPY_index = decode_a_symbol(m_cumf_CBPY_intra);
            CBPY = m_cbpy_intratab[CBPY_index];
          }
          else {
            CBPY_index = decode_a_symbol(m_cumf_CBPY);
            CBPY = m_cbpytab[CBPY_index];
        
          }
         // if (trace)
         //   printf("CBPY Index: %d CBPY %d \n",CBPY_index, CBPY);

        }
        else 
          CBPY = getCBPY();
 
        /* Decode Mode and CBP */
        
        
        if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q)
          {/* Intra */
            if (!m_syntax_arith_coding)	
              CBPY = CBPY^15;        /* needed in huffman coding only */
          }

        CBP = (CBPY << 2) | (MCBPC >> 4);
      }

      if (Mode == MODE_INTER4V && !m_adv_pred_mode) 
        if (!m_quiet)
          fputs("8x8 vectors not allowed in normal prediction mode\n",m_dlog);
          /* Could set m_fault-flag and resync */


      if (Mode == MODE_INTER_Q || Mode == MODE_INTRA_Q) {
        /* Read DQUANT if necessary */

        if (m_syntax_arith_coding) {
          DQUANT_index = decode_a_symbol(m_cumf_DQUANT);
          DQUANT = m_dquanttab[DQUANT_index] - 2; 
          m_quant +=DQUANT;
        }
        else {
          DQUANT = getbits(2);
          m_quant += DQ_tab[DQUANT];
          if (m_trace) {
            fputs("DQUANT (",m_dlog);
            printbits(DQUANT,2,2);
            //printf("): %d = %d\n",DQUANT,DQ_tab[DQUANT]);
          }
        }

        if (m_quant > 31 || m_quant < 1) {
          if (!m_quiet)
            fputs("Quantizer out of range: clipping\n",m_dlog);
          m_quant = mmax(1,mmin(31,m_quant));
          /* could set m_fault-flag and resync here */
        }
      }

      /* motion vectors */
      if (Mode == MODE_INTER || Mode == MODE_INTER_Q || 
          Mode == MODE_INTER4V || m_pb_frame) {

        if (Mode == MODE_INTER4V) { startmv = 1; stopmv = 4;}
        else { startmv = 0; stopmv = 0;}

        for (k = startmv; k <= stopmv; k++) {
          if (m_syntax_arith_coding) {
            mvx_index = decode_a_symbol(m_cumf_MVD);
            mvx = m_mvdtab[mvx_index];
            mvy_index = decode_a_symbol(m_cumf_MVD);
            mvy = m_mvdtab[mvy_index];
            //if (trace)
            //  printf("mvx_index: %d mvy_index: %d \n", mvy_index, mvx_index);
          }
          else {
            mvx = getTMNMV();
            mvy = getTMNMV();
          }

          pmv0 = find_pmv(xpos,ypos,k,0);
          pmv1 = find_pmv(xpos,ypos,k,1);
          mvx = motion_decode(mvx, pmv0);
          mvy = motion_decode(mvy, pmv1);
         /* if (trace) {
            printf("mvx: %d\n", mvx);
            printf("mvy: %d\n", mvy);
          }
		  */
          /* Check mv's to prevent seg.faults when error rate is high */
          if (!m_mv_outside_frame) {
            bsize = k ? 8 : 16;
            offset = k ? (((k-1)&1)<<3) : 0;
            /* checking only integer component */
            if ((xpos<<4) + (mvx/2) + offset < 0 ||
        (xpos<<4) + (mvx/2) + offset > (m_mb_width<<4) - bsize) {
              if (!m_quiet)
				fputs("mvx out of range: searching for sync\n",m_dlog);
              m_fault = 1;
            }
            offset = k ? (((k-1)&2)<<2) : 0;
            if ((ypos<<4) + (mvy/2) + offset < 0 ||
        (ypos<<4) + (mvy/2) + offset > (m_mb_height<<4) - bsize) {
              if (!m_quiet)
				fputs("mvy out of range: searching for sync\n",m_dlog);
              m_fault = 1;
            }
          }
          m_MV[0][k][ypos+1][xpos+1] = mvx;
          m_MV[1][k][ypos+1][xpos+1] = mvy;
        }

        /* PB frame delta vectors */

        if (m_pb_frame) {
          if (MODB == PBMODE_MVDB || MODB == PBMODE_CBPB_MVDB) {
            if (m_syntax_arith_coding) {
              mvdbx_index = decode_a_symbol(m_cumf_MVD);
              mvdbx = m_mvdtab[mvdbx_index];
              
              mvdby_index = decode_a_symbol(m_cumf_MVD);
              mvdby = m_mvdtab[mvdby_index];
            }
            else {
              mvdbx = getTMNMV();
              mvdby = getTMNMV();
            }


            mvdbx = motion_decode(mvdbx, 0);
            mvdby = motion_decode(mvdby, 0);
            /* This will not work if the PB deltas are so large they
               require the second colums of the motion vector VLC
               table to be used.  To fix this it is necessary to
               calculate the MV predictor for the PB delta: TRB*MV/TRD
               here, and use this as the second parameter to
               motion_decode(). The B vector itself will then be
               returned from motion_decode(). This will have to be
               changed to the PB delta again, since it is the PB delta
               which is used later in this program. I don't think PB
               deltas outside the range mentioned above is useful, but
               you never know... */
		}
          else {
            mvdbx = 0; 
            mvdby = 0;
          }
        }
      }

      if (m_fault) goto resync;

    }
    else { /* COD == 1 --> skipped MB */
      if (MBA>=MBAmax)
        return; /* all macroblocks decoded */
      if (!m_syntax_arith_coding)
        if (m_pict_type == PCT_INTER)
          flushbits(1);

      Mode = MODE_INTER;
      
      /* Reset CBP */
      CBP = CBPB = 0;

      /* reset motion vectors */
      m_MV[0][0][ypos+1][xpos+1] = 0;
      m_MV[1][0][ypos+1][xpos+1] = 0;
      mvdbx = 0;
      mvdby = 0;
    }

    /* Store Mode*/
    m_modemap[ypos+1][xpos+1] = Mode;

    if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) 
      if (!m_pb_frame)
        m_MV[0][0][ypos+1][xpos+1]=m_MV[1][0][ypos+1][xpos+1] = 0;


  reconstruct_mb:

    /* pixel coordinates of top left corner of current macroblock */
    /* one delayed because of OBMC */
    if (xpos > 0) {
      bx = 16*(xpos-1);
      by = 16*ypos;
    }
    else {
      bx = m_coded_picture_width-16;
      by = 16*(ypos-1);
    }

    if (MBA > 0) {

      Mode = m_modemap[by/16+1][bx/16+1];

      /* forward motion compensation for B-frame */
      if (m_pb_frame)
        reconstruct(bx,by,0,pmvdbx,pmvdby);
      
      /* motion compensation for P-frame */
      if (Mode == MODE_INTER || Mode == MODE_INTER_Q || Mode == MODE_INTER4V)
        reconstruct(bx,by,1,0,0);

      /* copy or add block data into P-picture */
      for (comp=0; comp<m_blk_cnt; comp++) {
        /* inverse DCT */
        if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) {
          if (m_refidct)
            idctref(m_ld->block[comp]);
          else
            idct(m_ld->block[comp]);
          addblock(comp,bx,by,0);
        }
        else if ( (pCBP & (1<<(m_blk_cnt-1-comp))) ) {
          /* No need to to do this for blocks with no coeffs */
          if (m_refidct)
            idctref(m_ld->block[comp]);
          else
            idct(m_ld->block[comp]);
          addblock(comp,bx,by,1);
        }
      }
      
      
      if (m_pb_frame) {
        /* add block data into B-picture */
        for (comp = 6; comp<m_blk_cnt+6; comp++) {
          if (!pCOD || m_adv_pred_mode)
            reconblock_b(comp-6,bx,by,Mode,pmvdbx,pmvdby);
          if ( (pCBPB & (1<<(m_blk_cnt-1-comp%6))) ) {
            if (m_refidct)
              idctref(m_ld->block[comp]);
            else
              idct(m_ld->block[comp]);
            addblock(comp,bx,by,1);
          }
        }
      }
      
    } /* end if (MBA > 0) */

    if (!COD) {

      Mode = m_modemap[ypos+1][xpos+1];

      /* decode blocks */
      for (comp=0; comp<m_blk_cnt; comp++) {

        clearblock(comp);
        if (Mode == MODE_INTRA || Mode == MODE_INTRA_Q) { /* Intra */
          bp = m_ld->block[comp];
          if(m_syntax_arith_coding) {
            INTRADC_index = decode_a_symbol(m_cumf_INTRADC);
            bp[0] = m_intradctab[INTRADC_index];
            //if (trace)
            //  printf("INTRADC Index: %d INTRADC: %d \n", INTRADC_index, bp[0]);
          }
          else {
            bp[0] = getbits(8);
            /*if (trace) {
              printf("DC[%d]: (",comp);
              printbits((int)bp[0],8,8);
              printf("): %d\n",(int)bp[0]);
            }
			*/ 
			
		}

          if (bp[0] == 128)
            if (!m_quiet)
              fputs("Illegal DC-coeff: 1000000\n",m_dlog);
          if (bp[0] == 255)  /* Spec. in H.26P, not in TMN4 */
            bp[0] = 128;
          bp[0] *= 8; /* Iquant */
          if ( (CBP & (1<<(m_blk_cnt-1-comp))) ) {
            if (!m_syntax_arith_coding)
              getblock(comp,0);
            else 
              get_sac_block(comp,0);
          }
        }
        else { /* Inter */
          if ( (CBP & (1<<(m_blk_cnt-1-comp))) ) {
            if (!m_syntax_arith_coding)
              getblock(comp,1);
            else
              get_sac_block(comp,1);
          }

        }
        if (m_fault) goto resync;
      }

      /* Decode B blocks */
      if (m_pb_frame) {
        for (comp=6; comp<m_blk_cnt+6; comp++) {
          clearblock(comp);
          if ( (CBPB & (1<<(m_blk_cnt-1-comp%6))) ) {
            if (!m_syntax_arith_coding)
              getblock(comp,1);
            else
              get_sac_block(comp,1);
          }
          if (m_fault) goto resync;
        }
      }
          
    }

    /* advance to next macroblock */
    MBA++;

    pCBP = CBP; pCBPB = CBPB; pCOD = COD;
    pmvdbx = mvdbx; pmvdby = mvdby;
    fflush(stdout);

    if (MBA >= MBAmax && !last_done) {
      COD = 1;
      xpos = 0;
      ypos++;
      last_done = 1;
      goto reconstruct_mb;
    }

  }
}

/* set block to zero */
void CH263Dec::clearblock(int comp)
{
  int *bp;
  int i;

  bp = (int *)m_ld->block[comp];

  for (i=0; i<8; i++)
  {
    bp[0] = bp[1] = bp[2] = bp[3] = 0;
    bp += 4;
  }
}


/* move/add 8x8-Block from block[comp] to refframe or bframe */
void CH263Dec::addblock(int comp,int bx,int by,int addflag)
{
  int cc,i, iincr, P = 1;
  unsigned char *rfp;
  short *bp;

  bp = m_ld->block[comp];

  if (comp >= 6) {
    /* This is a component for B-frame forward prediction */
    P = 0;
    addflag = 1;
    comp -= 6;
  }

  cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */

  if (cc==0) {
    /* luminance */
    
    /* frame DCT coding */
    if (P)
      rfp = m_newframe[0]
        + m_coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    else
      rfp = m_bframe[0]
        + m_coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    iincr = m_coded_picture_width;
  }
  else {
    /* chrominance */

    /* scale coordinates */
    bx >>= 1;
    by >>= 1;
    /* frame DCT coding */
    if (P)
      rfp = m_newframe[cc] + m_chrom_width*by + bx;
    else
      rfp = m_bframe[cc] + m_chrom_width*by + bx;
    iincr = m_chrom_width;
  }


  if (addflag) {
    for (i=0; i<8; i++) {
      rfp[0] = m_clp[bp[0]+rfp[0]];
      rfp[1] = m_clp[bp[1]+rfp[1]];
      rfp[2] = m_clp[bp[2]+rfp[2]];
      rfp[3] = m_clp[bp[3]+rfp[3]];
      rfp[4] = m_clp[bp[4]+rfp[4]];
      rfp[5] = m_clp[bp[5]+rfp[5]];
      rfp[6] = m_clp[bp[6]+rfp[6]];
      rfp[7] = m_clp[bp[7]+rfp[7]];
      bp += 8;
      rfp+= iincr;
    }
  }
  else  {
    for (i=0; i<8; i++) {
      rfp[0] = m_clp[bp[0]];
      rfp[1] = m_clp[bp[1]];
      rfp[2] = m_clp[bp[2]];
      rfp[3] = m_clp[bp[3]];
      rfp[4] = m_clp[bp[4]];
      rfp[5] = m_clp[bp[5]];
      rfp[6] = m_clp[bp[6]];
      rfp[7] = m_clp[bp[7]];
      bp += 8;
      rfp += iincr;
    }
  }
}

/* bidirectionally reconstruct 8x8-Block from block[comp] to bframe */

void CH263Dec::reconblock_b(int comp,int bx,int by,int mode,int bdx,int bdy)
{
  int cc,i,j,k, ii;
  unsigned char *bfr, *ffr;
  int BMVx, BMVy;
  int xa,xb,ya,yb,x,y,xvec,yvec,mvx,mvy;
  int xint,xhalf,yint,yhalf,pel;

  x = bx/16+1;y=by/16+1;

  if (mode == MODE_INTER4V) {
    if (comp < 4) {
      /* luma */
      mvx = m_MV[0][comp+1][y][x];
      mvy = m_MV[1][comp+1][y][x];
      BMVx = (bdx == 0 ? (m_trb-m_trd)* mvx/m_trd : m_trb * mvx/m_trd + bdx - mvx);
      BMVy = (bdy == 0 ? (m_trb-m_trd)* mvy/m_trd : m_trb * mvy/m_trd + bdy - mvy);
    }
    else {
      /* chroma */
      xvec = yvec = 0;
      for (k = 1; k <= 4; k++) {
        mvx = m_MV[0][k][y][x];
        mvy = m_MV[1][k][y][x];
        xvec += (bdx == 0 ? (m_trb-m_trd)* mvx/m_trd : m_trb * mvx/m_trd + bdx - mvx);
        yvec += (bdy == 0 ? (m_trb-m_trd)* mvy/m_trd : m_trb * mvy/m_trd + bdy - mvy);
      }
      
      /* chroma rounding (table 16/H.263) */
      BMVx = sign(xvec)*(m_roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
      BMVy = sign(yvec)*(m_roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);
    }
  }
  else {
    if (comp < 4) {
      /* luma */
      mvx = m_MV[0][0][y][x];
      mvy = m_MV[1][0][y][x];
      BMVx = (bdx == 0 ? (m_trb-m_trd)* mvx/m_trd : m_trb * mvx/m_trd + bdx - mvx);
      BMVy = (bdy == 0 ? (m_trb-m_trd)* mvy/m_trd : m_trb * mvy/m_trd + bdy - mvy);
    }
    else {
      /* chroma */
      mvx = m_MV[0][0][y][x];
      mvy = m_MV[1][0][y][x];
      xvec = (bdx == 0 ? (m_trb-m_trd)* mvx/m_trd : m_trb * mvx/m_trd + bdx - mvx);
      yvec = (bdy == 0 ? (m_trb-m_trd)* mvy/m_trd : m_trb * mvy/m_trd + bdy - mvy);
      xvec *= 4;
      yvec *= 4;
      
      /* chroma rounding (table 16/H.263) */
      BMVx = sign(xvec)*(m_roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
      BMVy = sign(yvec)*(m_roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);
    }
  }

  cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */

  if (cc==0) {
    /* luminance */
    find_bidir_limits(BMVx,&xa,&xb,comp&1);
    find_bidir_limits(BMVy,&ya,&yb,(comp&2)>>1);
    bfr = m_bframe[0] +
      m_coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    ffr = m_newframe[0] +
      m_coded_picture_width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
    ii = m_coded_picture_width;
  }
  else {
    /* chrominance */
    /* scale coordinates and vectors*/
    bx >>= 1;
    by >>= 1;

    find_bidir_chroma_limits(BMVx,&xa,&xb);
    find_bidir_chroma_limits(BMVy,&ya,&yb);

    bfr = m_bframe[cc]      + m_chrom_width*(by+((comp&2)<<2)) + bx + (comp&8);
    ffr = m_newframe[cc]    + m_chrom_width*(by+((comp&2)<<2)) + bx + (comp&8);
    ii = m_chrom_width;
  }

  xint = BMVx>>1;
  xhalf = BMVx - 2*xint;
  yint = BMVy>>1;
  yhalf = BMVy - 2*yint;

  ffr += xint + (yint+ya)*ii;
  bfr += ya*ii;
  
  if (!xhalf && !yhalf) {
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ffr[i];
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else if (xhalf && !yhalf) {
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ((unsigned int)(ffr[i]+ffr[i+1]+1))>>1;
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else if (!xhalf && yhalf) {
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ((unsigned int)(ffr[i]+ffr[ii+i]+1))>>1;
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  else { /* if (xhalf && yhalf) */
    for (j = ya; j < yb; j++) {
      for (i = xa; i < xb; i++) {
        pel = ((unsigned int)(ffr[i]+ffr[i+1]+ffr[ii+i]+ffr[ii+i+1]+2))>>2;
        bfr[i] = ((unsigned int)(pel + bfr[i]))>>1;
      }
      bfr += ii;
      ffr += ii;
    }
  }
  return;
}

int CH263Dec::motion_decode(int vec,int pmv)
{
  if (vec > 31) vec -= 64;
  vec += pmv;
  if (!m_long_vectors) {
    if (vec > 31)
      vec -= 64;
    if (vec < -32)
      vec += 64;
  }
  else {
    if (pmv < -31 && vec < -63)
      vec += 64;
    if (pmv > 32 && vec > 63)
      vec -= 64;
  }
  return vec;
}


int CH263Dec::find_pmv(int x,int  y,int  block,int comp)
{
  int p1,p2,p3;
  int xin1,xin2,xin3;
  int yin1,yin2,yin3;
  int vec1,vec2,vec3;
  int l8,o8,or8;

  x++;y++;

  l8 = (m_modemap[y][x-1] == MODE_INTER4V ? 1 : 0);
  o8 =  (m_modemap[y-1][x] == MODE_INTER4V ? 1 : 0);
  or8 = (m_modemap[y-1][x+1] == MODE_INTER4V ? 1 : 0);

  switch (block) {
  case 0: 
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 1:
    vec1 = (l8 ? 2 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = (o8 ? 3 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 2:
    vec1 = 1            ; yin1 = y  ; xin1 = x;
    vec2 = (o8 ? 4 : 0) ; yin2 = y-1; xin2 = x;
    vec3 = (or8? 3 : 0) ; yin3 = y-1; xin3 = x+1;
    break;
  case 3:
    vec1 = (l8 ? 4 : 0) ; yin1 = y  ; xin1 = x-1;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  case 4:
    vec1 = 3            ; yin1 = y  ; xin1 = x;
    vec2 = 1            ; yin2 = y  ; xin2 = x;
    vec3 = 2            ; yin3 = y  ; xin3 = x;
    break;
  default:
    if(m_trace)
		fputs("Illegal block number in find_pmv (getpic.c)\n",m_dlog);
    return 1;
  }
  p1 = m_MV[comp][vec1][yin1][xin1];
  p2 = m_MV[comp][vec2][yin2][xin2];
  p3 = m_MV[comp][vec3][yin3][xin3];

  if (m_newgob && (block == 0 || block == 1 || block == 2))
    p2 = NO_VEC;

  if (p2 == NO_VEC) { p2 = p3 = p1; }

  return p1+p2+p3 - mmax(p1,mmax(p2,p3)) - mmin(p1,mmin(p2,p3));
}

void CH263Dec::find_bidir_limits(int vec, int *start, int *stop, int nhv)
{
  /* limits taken from C loop in section G5 in H.263 */
  *start = mmax(0,(-vec+1)/2 - nhv*8);
  *stop = mmin(7,15-(vec+1)/2 - nhv*8);

  (*stop)++; /* I use < and not <= in the loop */
}

void CH263Dec::find_bidir_chroma_limits(int vec, int *start,int * stop)
{

  /* limits taken from C loop in section G5 in H.263 */
  *start = mmax(0,(-vec+1)/2);
  *stop = mmin(7,7-(vec+1)/2);

  (*stop)++; /* I use < and not <= in the loop */
  return;
}

void CH263Dec::make_edge_image(unsigned char *src,unsigned char *dst,int width,int height,int edge)
{
  int i,j;
  unsigned char *p1,*p2,*p3,*p4;
  unsigned char *o1,*o2,*o3,*o4;

  /* center image */
  p1 = dst;
  o1 = src;
  for (j = 0; j < height;j++) {
    for (i = 0; i < width; i++) {
      *(p1 + i) = *(o1 + i);
    }
    p1 += width + (edge<<1);
    o1 += width;
  }

  /* left and right edges */
  p1 = dst-1;
  o1 = src;
  for (j = 0; j < height;j++) {
    for (i = 0; i < edge; i++) {
      *(p1 - i) = *o1;
      *(p1 + width + i + 1) = *(o1 + width - 1);
    }
    p1 += width + (edge<<1);
    o1 += width;
  }    
    
  /* top and bottom edges */
  p1 = dst;
  p2 = dst + (width + (edge<<1))*(height-1);
  o1 = src;
  o2 = src + width*(height-1);
  for (j = 0; j < edge;j++) {
    p1 = p1 - (width + (edge<<1));
    p2 = p2 + (width + (edge<<1));
    for (i = 0; i < width; i++) {
      *(p1 + i) = *(o1 + i);
      *(p2 + i) = *(o2 + i);
    }
  }    

  /* corners */
  p1 = dst - (width+(edge<<1)) - 1;
  p2 = p1 + width + 1;
  p3 = dst + (width+(edge<<1))*(height)-1;
  p4 = p3 + width + 1;

  o1 = src;
  o2 = o1 + width - 1;
  o3 = src + width*(height-1);
  o4 = o3 + width - 1;
  for (j = 0; j < edge; j++) {
    for (i = 0; i < edge; i++) {
      *(p1 - i) = *o1;
      *(p2 + i) = *o2;
      *(p3 - i) = *o3;
      *(p4 + i) = *o4; 
    }
    p1 = p1 - (width + (edge<<1));
    p2 = p2 - (width + (edge<<1));
    p3 = p3 + width + (edge<<1);
    p4 = p4 + width + (edge<<1);
  }

}

/* only used for displayed interpolated frames, not reconstructed ones */
void CH263Dec::interpolate_image(unsigned char *in, unsigned char *out,int  width,int height)
{

  int x,xx,y,w2;

  unsigned char *pp,*ii;

  w2 = 2*width;

  /* Horizontally */
  pp = out;
  ii = in;
  for (y = 0; y < height-1; y++) {
    for (x = 0,xx=0; x < width-1; x++,xx+=2) {
      *(pp + xx) = *(ii + x);
      *(pp + xx+1) = ((unsigned int)(*(ii + x)  + *(ii + x + 1)))>>1;
      *(pp + w2 + xx) = ((unsigned int)(*(ii + x) + *(ii + x + width)))>>1;
      *(pp + w2 + xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 
           *(ii + x + width) + *(ii + x + width + 1)))>>2;
      
    }
    *(pp + w2 - 2) = *(ii + width - 1);
    *(pp + w2 - 1) = *(ii + width - 1);
    *(pp + w2 + w2 - 2) = *(ii + width + width - 1);
    *(pp + w2 + w2 - 1) = *(ii + width + width - 1);
    pp += w2<<1;
    ii += width;
  }

  /* last lines */
  for (x = 0,xx=0; x < width-1; x++,xx+=2) {
    *(pp+ xx) = *(ii + x);    
    *(pp+ xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 1))>>1;
    *(pp+ w2+ xx) = *(ii + x);    
    *(pp+ w2+ xx+1) = ((unsigned int)(*(ii + x) + *(ii + x + 1) + 1))>>1;
  }
  
  /* bottom right corner pels */
  *(pp + (width<<1) - 2) = *(ii + width -1);
  *(pp + (width<<1) - 1) = *(ii + width -1);
  *(pp + (width<<2) - 2) = *(ii + width -1);
  *(pp + (width<<2) - 1) = *(ii + width -1);
}
    


