/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * Server.
 * Interprocess Online.
 */

#pragma once

#include <string>

/** \defgroup Server API
 *
 * Implementation of KRAKEN interprocess server networking capabilities.
 *
 *  \details
 * This API is used to create interprocess server architectures for use with 3D DCC
 * applications such as Blender, Maya, Arnold, Houdini, and the like.
 *
 * Note that the server API is built around ZMQ and it's schematic.
 *
 * \{
 */

/**
 * Network IP Addressing
 */
enum eServerNetworkPort
{
  KRAKEN_COMM_PORT_ALPHA = 5589,
  KRAKEN_COMM_PORT_BRAVO,
  KRAKEN_COMM_PORT_CHARLIE,
  KRAKEN_COMM_PORT_DELTA,
  KRAKEN_COMM_PORT_ECHO,
  KRAKEN_COMM_PORT_FOXTROT,
  KRAKEN_COMM_PORT_GOLF,
  KRAKEN_COMM_PORT_HOTEL,
  KRAKEN_COMM_PORT_INDIA,
  KRAKEN_COMM_PORT_JULIET,
  KRAKEN_COMM_PORT_KILO,
  KRAKEN_COMM_PORT_LIMA,
  KRAKEN_COMM_PORT_MIKE,
  KRAKEN_COMM_PORT_NOVEMBER,
  KRAKEN_COMM_PORT_OSCAR,
  KRAKEN_COMM_PORT_PAPA,
  KRAKEN_COMM_PORT_QUEBEC,
  KRAKEN_COMM_PORT_ROMEO,
  KRAKEN_COMM_PORT_SIERRA,
  KRAKEN_COMM_PORT_TANGO,
  KRAKEN_COMM_PORT_UNIFORM,
  KRAKEN_COMM_PORT_VICTOR,
  KRAKEN_COMM_PORT_WHISKEY,
  KRAKEN_COMM_PORT_XRAY,
  KRAKEN_COMM_PORT_YANKEE,
  KRAKEN_COMM_PORT_ZULU,
};

/**
 * Server error codes
 */

enum eServerErrorCode
{
  SERVER_SUCCESS = 0,
  SERVER_ERROR,
};

/**
 * Enum with the different server comm subtypes available within a server group.
 */

enum eServerCommType
{
  KRAKEN_COMM_TYPE_KRAKEN = 10,
  KRAKEN_COMM_TYPE_APP,
};

/**
 * Enum with the different server comm modes available for each \ref eServerCommType.
 */

enum eServerCommMode
{
  KRAKEN_COMM_MODE_INTERPROCESS = 20,
  KRAKEN_COMM_MODE_INTRAPROCESS,
};

/**
 * Enum with the different server comm colors (or 'teams') available for each \ref
 * eServerCommType.
 */

enum eServerCommColor
{
  KRAKEN_COMM_COLOR_BLACK = 30,
  KRAKEN_COMM_COLOR_GREEN,
};

/**
 * Enum with the different outside applications in which the server can establish a connection with
 * from \ref ServerStart.
 */

enum eServerCommConnect
{
  KRAKEN_COMM_CONNECT_BLENDER = 40,
  KRAKEN_COMM_CONNECT_CUSTOM1,
  KRAKEN_COMM_CONNECT_CUSTOM2,
  KRAKEN_COMM_CONNECT_CUSTOM3,
  KRAKEN_COMM_CONNECT_CUSTOM4,
  KRAKEN_COMM_CONNECT_CUSTOM5,
  KRAKEN_COMM_CONNECT_CUSTOM6
};

eServerErrorCode ServerStart(int argc, char *argv[]);
eServerErrorCode ServerClose(void);
eServerNetworkPort ServerAttachNetworkPort(void);

/*\}*/