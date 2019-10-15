/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "tpdu.h"
#include "apdu.h"

TPDUPtr
TPDU::fromPacket (const CArray & c, TracePtr tr)
{
  TPDUPtr t;
  if (c.size() >= 1)
    {
      if ((c[0] & 0xfc) == 0)
        t = TPDUPtr(new T_DATA_XXX_REQ_PDU ());
      else if (c[0] == 0x80)
        t = TPDUPtr(new T_CONNECT_REQ_PDU ());
      else if (c[0] == 0x81)
        t = TPDUPtr(new T_DISCONNECT_REQ_PDU ());
      else if ((c[0] & 0xC3) == 0xC2)
        t = TPDUPtr(new T_ACK_PDU ());
      else if ((c[0] & 0xC3) == 0xC3)
        t = TPDUPtr(new T_NACK_PDU ());
      else if ((c[0] & 0xC0) == 0x40)
        t = TPDUPtr(new T_DATA_CONNECTED_REQ_PDU ());
    }
  if (t && t->init (c, tr))
    return t;

  t = TPDUPtr(new T_UNKNOWN_PDU ());
  t->init (c, tr);
  return t;
}


/* T_UNKNOWN  */

bool
T_UNKNOWN_PDU::init (const CArray & c, TracePtr)
{
  pdu = c;
  return true;
}

CArray T_UNKNOWN_PDU::ToPacket ()
{
  return pdu;
}

std::string T_UNKNOWN_PDU::Decode (TracePtr)
{
  std::string s ("Unknown TPDU: ");

  if (pdu.size() == 0)
    return "empty TPDU";

  ITER (i,pdu)
  addHex (s, *i);

  return s;
}

/* T_DATA_XXX_REQ  */

bool
T_DATA_XXX_REQ_PDU::init (const CArray & c, TracePtr)
{
  if (c.size() < 1)
    return false;
  data = c;
  return true;
}

CArray T_DATA_XXX_REQ_PDU::ToPacket ()
{
  assert (data.size() > 0);
  CArray pdu (data);
  pdu[0] = (pdu[0] & 0x3);
  return pdu;
}

std::string T_DATA_XXX_REQ_PDU::Decode (TracePtr t)
{
  APDUPtr a = APDU::fromPacket (data, t);
  std::string s ("T_DATA_XXX_REQ ");
  s += a->Decode (t);
  return s;
}

/* T_DATA_CONNECTED_REQ  */

bool
T_DATA_CONNECTED_REQ_PDU::init (const CArray & c, TracePtr)
{
  if (c.size() < 1)
    return false;
  data = c;
  serno = (c[0] >> 2) & 0x0f;
  return true;
}

CArray T_DATA_CONNECTED_REQ_PDU::ToPacket ()
{
  assert (data.size() > 0);
  assert ((serno & 0xf0) == 0);
  CArray pdu (data);
  pdu[0] = (pdu[0] & 0x3) | 0x40 | ((serno & 0x0f) << 2);
  return pdu;
}

std::string T_DATA_CONNECTED_REQ_PDU::Decode (TracePtr t)
{
  assert ((serno & 0xf0) == 0);
  APDUPtr a = APDU::fromPacket (data, t);
  std::string s ("T_DATA_CONNECTED_REQ serno:");
  addHex (s, serno);
  s += a->Decode (t);
  return s;
}

/* T_CONNECT_REQ  */

bool
T_CONNECT_REQ_PDU::init (const CArray & c, TracePtr)
{
  if (c.size() != 1)
    return false;
  return true;
}

CArray T_CONNECT_REQ_PDU::ToPacket ()
{
  uchar c = 0x80;
  return CArray (&c, 1);
}

std::string T_CONNECT_REQ_PDU::Decode (TracePtr)
{
  return "T_CONNECT_REQ";
}

/* T_DISCONNECT_REQ  */

bool
T_DISCONNECT_REQ_PDU::init (const CArray & c, TracePtr)
{
  if (c.size() != 1)
    return false;
  return true;
}

CArray T_DISCONNECT_REQ_PDU::ToPacket ()
{
  uchar c = 0x81;
  return CArray (&c, 1);
}

std::string T_DISCONNECT_REQ_PDU::Decode (TracePtr)
{
  return "T_DISCONNECT_REQ";
}

/* T_ACK */

bool
T_ACK_PDU::init (const CArray & c, TracePtr)
{
  if (c.size() != 1)
    return false;
  serno = (c[0] >> 2) & 0x0f;
  return true;
}

CArray T_ACK_PDU::ToPacket ()
{
  assert ((serno & 0xf0) == 0);
  uchar c = 0xC2 | ((serno & 0x0f) << 2);
  return CArray (&c, 1);
}

std::string T_ACK_PDU::Decode (TracePtr)
{
  assert ((serno & 0xf0) == 0);
  std::string s ("T_ACK Serno:");
  addHex (s, serno);
  return s;
}

/* T_NACK  */

bool T_NACK_PDU::init (const CArray & c, TracePtr)
{
  if (c.size() != 1)
    return false;
  serno = (c[0] >> 2) & 0x0f;
  return true;
}

CArray T_NACK_PDU::ToPacket ()
{
  assert ((serno & 0xf0) == 0);
  uchar c = 0xC3 | ((serno & 0x0f) << 2);
  return CArray (&c, 1);
}

std::string T_NACK_PDU::Decode (TracePtr)
{
  assert ((serno & 0xf0) == 0);
  std::string s ("T_NACK Serno:");
  addHex (s, serno);
  return s;
}
