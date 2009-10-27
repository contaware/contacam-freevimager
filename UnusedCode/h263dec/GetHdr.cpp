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
//    Name    : GetHdr.cpp
//
/////////////////////////////////////////////////////////////////////////////

/************************************************************************
 *
 *  gethdr.c, header decoding for tmndecode (H.263 decoder)
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
 * based on mpeg2decode, (C) 1994, MPEG Software Simulation Group
 * and mpeg2play, (C) 1994 Stefan Eckart
 *                         <stefan@lis.e-technik.tu-muenchen.de>
 */

#include "stdafx.h"
#include "H263Dec.h"

/*
 * decode headers from one input stream
 * until an End of Sequence or picture start code
 * is found
 */

int CH263Dec::getheader()
{
  unsigned int code, gob;

  /* look for startcode */
  startcode();

//  #define PSC_LENGTH        17

  code = getbits(PSC_LENGTH);
  
  gob = getbits(5);
  
  if (gob == SE_CODE) 
  return 0;
  
  if (gob == 0) 
  {
    getpicturehdr();
   
	// reset decoder after receiving */
	if (m_syntax_arith_coding)        
		decoder_reset();	        /* fixed length PSC string */
  }
  
  return gob + 1;
}




/* align to start of next startcode */

void CH263Dec::startcode()
{


	/* search for new picture start code */
  while (showbits(PSC_LENGTH)!=1l) 
         flushbits(1);
}

/* decode picture header */
void CH263Dec::getpicturehdr()
{
  int pos, pei, tmp;

  pos = m_ld->bitcnt;
  m_prev_temp_ref = m_temp_ref;
  m_temp_ref = getbits(8);
  m_trd = m_temp_ref - m_prev_temp_ref;

  if (m_trd < 0)
    m_trd += 256;

  tmp = getbits(1); /* always "1" */
  
  if (!tmp)
    if (!m_quiet)
      fputs("warning: spare in picture header should be \"1\"\n",m_dlog);
  tmp = getbits(1); /* always "0" */
  
  if (tmp)
    if (!m_quiet)
      fputs("warning: H.261 distinction bit should be \"0\"\n",m_dlog);
  
  tmp = getbits(1); /* split_screen_indicator */
  if (tmp) 
  {
    if (!m_quiet)
      fputs("error: split-screen not supported in this version\n",m_dlog);
    return;
  }
  
  tmp = getbits(1); /* document_camera_indicator */
  if (tmp)
    if (!m_quiet)
      fputs("warning: document camera indicator not supported in this version\n",m_dlog);

  tmp = getbits(1); /* freeze_picture_release */
  if (tmp)
    if (!m_quiet)
      fputs("warning: frozen picture not supported in this version\n",m_dlog);

  m_source_format = getbits(3);
  m_pict_type = getbits(1);
  m_mv_outside_frame = getbits(1);
  m_long_vectors = (m_mv_outside_frame ? 1 : 0);
  m_syntax_arith_coding = getbits(1);
  m_adv_pred_mode = getbits(1);
  m_mv_outside_frame = (m_adv_pred_mode ? 1 : m_mv_outside_frame);
  m_pb_frame = getbits(1);
  m_quant = getbits(5);
  tmp = getbits(1);
  if (tmp) {
    if (!m_quiet)
      fputs("error: CPM not supported in this version\n",m_dlog);
	return;
  }

  if (m_pb_frame) {
    m_trb = getbits(3);
    m_bquant = getbits(2);
  }
  else {
    m_trb = 0;
  }


  pei = getbits(1);

pspare:
  
  if (pei) 
  {
     /* extra info for possible future backward compatible additions */
    getbits(8);  /* not used */
    pei = getbits(1);
    if (pei) goto pspare; /* keep on reading pspare until pei=0 */
  }
}



