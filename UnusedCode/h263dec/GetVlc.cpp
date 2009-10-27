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
//    Name    : GetVlc.cpp
//
/////////////////////////////////////////////////////////////////////////////

/************************************************************************
 *
 *  getvlc.c, variable length decoding for tmndecode (H.263 decoder)
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
 *
 */

#include "stdafx.h"
#include "H263Dec.h"

int CH263Dec::getTMNMV()
{
  int code;

  if (m_trace)
    fputs("motion_code (",m_dlog);

  if (getbits1())
  {
    if (m_trace)
      fputs("1): 0\n",m_dlog);
    return 0;
  }

  if ((code = showbits(12))>=512)
  {
    code = (code>>8) - 2;
    flushbits(m_TMNMVtab0[code].len);

    if (m_trace)
    {
      fputs("0",m_dlog);
      printbits(code+2,4,m_TMNMVtab0[code].len);
    }

    return m_TMNMVtab0[code].val;
  }

  if (code>=128)
  {
    code = (code>>2) -32;
    flushbits(m_TMNMVtab1[code].len);

    if (m_trace)
    {
      fputs("0",m_dlog);
      printbits(code+32,10,m_TMNMVtab1[code].len);
    }

    return m_TMNMVtab1[code].val;
  }

  if ((code-=5)<0)
  {
    if (!m_quiet)
      fputs("Invalid motion_vector code\n",m_dlog);
    m_fault=1;
    return 0;
  }

  flushbits(m_TMNMVtab2[code].len);

  if (m_trace)
  {
    fputs("0",m_dlog);
    printbits(code+5,12,m_TMNMVtab2[code].len);
  }

  return m_TMNMVtab2[code].val;
}


int CH263Dec::getMCBPC()
{
  int code;

  if (m_trace)
    fputs("MCBPC (",m_dlog);

  code = showbits(9);

  if (code == 1) {
    /* macroblock stuffing */
    if (m_trace)
      fputs("000000001): stuffing\n",m_dlog);
    flushbits(9);
    return 255;
  }

  if (code == 0) {
    if (!m_quiet) 
      fputs("Invalid MCBPC code\n",m_dlog);
    m_fault = 1;
    return 0;
  }
    
  if (code>=256)
  {
    flushbits(1);
    return 0;
  }
    
  flushbits(m_MCBPCtab[code].len);

  if (m_trace)
  {
    printbits(code,9,m_MCBPCtab[code].len);
  }

  return m_MCBPCtab[code].val;
}

int CH263Dec::getMODB()
{
  int code;
  int MODB;

  if (m_trace)
    fputs("MODB (",m_dlog);

  code = showbits(2);

  if (code < 2) {
    if (m_trace)
      fputs("0): MODB = 0\n",m_dlog);
    MODB = 0;
    flushbits(1);
  }
  else if (code == 2) {
    if (m_trace)
      fputs("10): MODB = 1\n",m_dlog);
    MODB = 1;
    flushbits(2);
  }
  else { /* code == 3 */
    if (m_trace)
      fputs("11): MODB = 2\n",m_dlog);
    MODB = 2;
    flushbits(2);
  }
  return MODB;
}


int CH263Dec::getMCBPCintra()
{
  int code;

  if (m_trace)
    fputs("MCBPCintra (",m_dlog);

  code = showbits(9);

  if (code == 1) {
    /* macroblock stuffing */
    if (m_trace)
      fputs("000000001): stuffing\n",m_dlog);
    flushbits(9);
    return 255;
  }

  if (code < 8) {
    if (!m_quiet) 
      fputs("Invalid MCBPCintra code\n",m_dlog);
    m_fault = 1;
    return 0;
  }

  code >>= 3;
    
  if (code>=32)
  {
    flushbits(1);
    return 3;
  }

  flushbits(m_MCBPCtabintra[code].len);

  if (m_trace)
  {
    printbits(code,6,m_MCBPCtabintra[code].len);
  }

  return m_MCBPCtabintra[code].val;
}

int CH263Dec::getCBPY()
{
  int code;

  if (m_trace)
    fputs("CBPY (",m_dlog);

  code = showbits(6);
  if (code < 2) {
    if (!m_quiet) 
      fputs("Invalid CBPY code\n",m_dlog);
    m_fault = 1;
    return -1;
  }
    
  if (code>=48)
  {
    flushbits(2);
    return 0;
  }

  flushbits(m_CBPYtab[code].len);

  if (m_trace)
  {
    printbits(code,6,m_CBPYtab[code].len);
  }

  return m_CBPYtab[code].val;
}
