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

#include <zmq.hpp>

#include <string>

#include "SERVER_api.h" /* Own Include. */

#include "KKE_api.h"

#include "WM_api.h"
#include "WM_msgbus.h"

#include <wabi/base/tf/diagnostic.h>
#include <wabi/base/tf/mallocTag.h>
#include <wabi/base/tf/staticTokens.h>
#include <wabi/wabi.h>

WABI_NAMESPACE_BEGIN

eServerNetworkPort ServerAttachNetworkPort()
{
  return KRAKEN_COMM_PORT_ALPHA;
}

static void PARSE_SIGNALS(std::string cmd)
{

#ifdef DEBUG_KRAKEN_SERVER
  printf("\nKRAKEN SERVER INCOMING: %s.\n", cmd.c_str());
#endif

  /* Kill the KRAKEN process if recieved 'KRAKENKILL' from world. */
  if (cmd == "KRAKENKILL") {
    exit(KRAKEN_SUCCESS);
  }
}

static void ServerRun(socket_t &sock)
{

  /**
   * SERVER: MAIN SERVER PROCESS
   * Listens for commands and performs
   * actions based on the commands given,
   * very tiny, very fast, very powerful.
   */

  KRAKENServerNotice("SERVER RUN").Send();
  while (true) {
    message_t push_notif;
    TF_UNUSED(sock.recv(push_notif, recv_flags::none));
    std::string cmd = push_notif.to_string();

    KRAKENServerNotice(cmd).Send();
    PARSE_SIGNALS(cmd);
    cmd = "";
  }
}

eServerErrorCode ServerStart(int argc, char *argv[])
{
  TF_UNUSED(argc);
  TF_UNUSED(argv);

  zmq::context_t ctx;
  zmq::socket_t sock(ctx, zmq::socket_type::pull);

  std::string server_port("tcp://127.0.0.1:");

  int status = -1;
  int reconnects = 0;
  while (status != SERVER_SUCCESS) {
    server_port = server_port.substr(0, server_port.find_last_of(":"));
    server_port += ":" + to_string(KRAKEN_COMM_PORT_ALPHA + reconnects);
    status = zmq::zmq_bind(sock, server_port.c_str());
    reconnects += 1;
    assert(reconnects <= KRAKEN_COMM_PORT_ZULU);
  }

  ServerRun(sock);
  return SERVER_SUCCESS;
}

eServerErrorCode ServerClose()
{
  return SERVER_SUCCESS;
}

WABI_NAMESPACE_END