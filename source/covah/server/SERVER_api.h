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
 * Copyright 2021, Wabi.
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
 * Implementation of COVAH interprocess server networking capabilities.
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
  COVAH_COMM_PORT_ALPHA = 5589,
  COVAH_COMM_PORT_BRAVO,
  COVAH_COMM_PORT_CHARLIE,
  COVAH_COMM_PORT_DELTA,
  COVAH_COMM_PORT_ECHO,
  COVAH_COMM_PORT_FOXTROT,
  COVAH_COMM_PORT_GOLF,
  COVAH_COMM_PORT_HOTEL,
  COVAH_COMM_PORT_INDIA,
  COVAH_COMM_PORT_JULIET,
  COVAH_COMM_PORT_KILO,
  COVAH_COMM_PORT_LIMA,
  COVAH_COMM_PORT_MIKE,
  COVAH_COMM_PORT_NOVEMBER,
  COVAH_COMM_PORT_OSCAR,
  COVAH_COMM_PORT_PAPA,
  COVAH_COMM_PORT_QUEBEC,
  COVAH_COMM_PORT_ROMEO,
  COVAH_COMM_PORT_SIERRA,
  COVAH_COMM_PORT_TANGO,
  COVAH_COMM_PORT_UNIFORM,
  COVAH_COMM_PORT_VICTOR,
  COVAH_COMM_PORT_WHISKEY,
  COVAH_COMM_PORT_XRAY,
  COVAH_COMM_PORT_YANKEE,
  COVAH_COMM_PORT_ZULU,
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
  COVAH_COMM_TYPE_COVAH = 10,
  COVAH_COMM_TYPE_APP,
};

/**
 * Enum with the different server comm modes available for each \ref eServerCommType.
 */

enum eServerCommMode
{
  COVAH_COMM_MODE_INTERPROCESS = 20,
  COVAH_COMM_MODE_INTRAPROCESS,
};

/**
 * Enum with the different server comm colors (or 'teams') available for each \ref
 * eServerCommType.
 */

enum eServerCommColor
{
  COVAH_COMM_COLOR_BLACK = 30,
  COVAH_COMM_COLOR_GREEN,
};

/**
 * Enum with the different outside applications in which the server can establish a connection with
 * from \ref ServerStart.
 */

enum eServerCommConnect
{
  COVAH_COMM_CONNECT_BLENDER = 40,
  COVAH_COMM_CONNECT_CUSTOM1,
  COVAH_COMM_CONNECT_CUSTOM2,
  COVAH_COMM_CONNECT_CUSTOM3,
  COVAH_COMM_CONNECT_CUSTOM4,
  COVAH_COMM_CONNECT_CUSTOM5,
  COVAH_COMM_CONNECT_CUSTOM6
};

eServerErrorCode ServerStart(int argc, char *argv[]);
eServerErrorCode ServerClose(void);
eServerNetworkPort ServerAttachNetworkPort(void);

/*\}*/