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
//    Name    : sac.cpp
//
/////////////////////////////////////////////////////////////////////////////

/************************************************************************
 *
 *  sac.c, part of tmndecode (H.263 decoder)
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

/*********************************************************************
 *        SAC Decoder Module
 *        Algorithm as Specified in H26P Annex -E
 *              (c) 1995 BT Labs
 *
 *	Author:	Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
 *
 *********************************************************************/


#include "stdafx.h"
#include "H263Dec.h"
 
#define   q1    16384
#define   q2    32768
#define   q3    49152
#define   top   65535

 /*********************************************************************
 *        SAC Decoder Algorithm as Specified in H26P Annex -E
 *
 *        Name:        decode_a_symbol
 *
 *	Description:	Decodes an Aritmetically Encoded Symbol
 *
 *	Input:			array holding cumulative freq. data
 *					also uses member data for decoding endpoints
 *					and the m_sac_code_value variable
 *
 *	Returns:		Index to relevant symbol model
 *
 *	Side Effects:	Modifies m_sac_low, m_sac_high, m_sac_length,
 *					m_sac_cum and m_sac_code_value
 *
 *	Author:			Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
 *
 *********************************************************************/
int CH263Dec::decode_a_symbol(const int cumul_freq[])
{

  m_sac_length = m_sac_high - m_sac_low + 1;
  m_sac_cum = (-1 + (m_sac_code_value - m_sac_low + 1) * cumul_freq[0]) / m_sac_length;
  for (m_sac_index = 1; cumul_freq[m_sac_index] > m_sac_cum; m_sac_index++);
  m_sac_high = m_sac_low - 1 + (m_sac_length * cumul_freq[m_sac_index-1]) / cumul_freq[0];
  m_sac_low += (m_sac_length * cumul_freq[m_sac_index]) / cumul_freq[0];

  for ( ; ; ) {  
    if (m_sac_high < q2) ;
    else if (m_sac_low >= q2) {
      m_sac_code_value -= q2; 
      m_sac_low -= q2; 
      m_sac_high -= q2;
    }
    else if (m_sac_low >= q1 && m_sac_high < q3) {
      m_sac_code_value -= q1; 
      m_sac_low -= q1; 
      m_sac_high -= q1;
    }
    else {
      break;
    }
 
    m_sac_low *= 2; 
    m_sac_high = 2*m_sac_high + 1;
    bit_out_psc_layer(); 
    m_sac_code_value = 2*m_sac_code_value + m_sac_bit;
  }

  return (m_sac_index-1);
}
 
/*********************************************************************
 *
 *        Name:        decoder_reset
 *
 *	Description:	Fills Decoder FIFO after a fixed word length
 *        string has been detected.
 *
 *	Input:        None
 *
 *	Returns:	Nothing
 *
 *	Side Effects:	Fills Arithmetic Decoder FIFO
 *
 *	Author:        Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
 *
 *********************************************************************/
void CH263Dec::decoder_reset()
{
  int i;
  m_sac_zerorun = 0;        /* clear consecutive zero's counter */
  m_sac_code_value = 0;
  m_sac_low = 0;
  m_sac_high = top;
  for (i = 1;   i <= 16;   i++) {
    bit_out_psc_layer(); 
    m_sac_code_value = 2 * m_sac_code_value + m_sac_bit;
  }
  if (m_trace)
    fputs("Arithmetic Decoder Reset \n",m_dlog);
}

/*********************************************************************
 *
 *        Name:        bit_out_psc_layer
 *
 *	Description:	Gets a bit from the Encoded Stream, Checks for
 *        and removes any PSC emulation prevention bits
 *        inserted at the decoder, provides 'zeros' to the
 *        Arithmetic Decoder FIFO to allow it to finish 
 *        data prior to the next PSC. (Garbage bits)
 *
 *	Input:        None
 *
 *	Returns:	Nothing
 *
 *	Side Effects:	Gets a bit from the Input Data Stream
 *
 *	Author:        Wayne Ellis <ellis_w_wayne@bt-web.bt.co.uk>
 *
 *********************************************************************/
void CH263Dec::bit_out_psc_layer()
{
  if (showbits(17)!=1) { /* check for startcode in Arithmetic Decoder FIFO */

    m_sac_bit = getbits(1);

    if(m_sac_zerorun > 13) {	/* if number of consecutive zeros = 14 */	 
      if (!m_sac_bit) {
        if (m_trace)
          fputs("PSC/GBSC, Header Data, or Encoded Stream Error \n",m_dlog);
        m_sac_zerorun = 1;        
      }
      else { /* if there is a 'stuffing bit present */
        if (m_trace)
          fputs("Removing Startcode Emulation Prevention bit \n",m_dlog);
        m_sac_bit = getbits(1);        /* overwrite the last bit */	
        m_sac_zerorun = !m_sac_bit;        /* m_sac_zerorun=1 if bit is a '0' */
      }
    }

    else { /* if consecutive zero's not exceeded 14 */

      if (!m_sac_bit)
        m_sac_zerorun++;
      else
        m_sac_zerorun = 0;
    }

  } /* end of if !(showbits(17)) */

  else {
    m_sac_bit = 0;
    if (m_trace)
      fputs("Startcode Found:Finishing Arithmetic Decoding using 'Garbage bits'\n",m_dlog);
  }     
}

