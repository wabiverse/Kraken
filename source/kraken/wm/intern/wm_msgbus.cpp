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
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_area.h"
#include "USD_context.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_pixar_utils.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"

#include "WM_api.h"
#include "WM_debug_codes.h"
#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_window.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_utils.h"
#include "KKE_version.h"

#include <mutex>
#include <string>
#include <vector>


/**
 *  -----  The Kraken WindowManager. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  The MsgBus Callback. ----- */


MsgBusCallback::MsgBusCallback(wmNotifier *notice) : ref(1), notice(notice->notice), note(notice)
{}

void MsgBusCallback::wmCOMM(const TfNotice &notice, MsgBus const &sender)
{
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("MsgBus\n");

  switch (note->category) {
    case (NC_WM):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: WindowManager\n");
      break;
    case (NC_WINDOW):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Window\n");
      break;
    case (NC_SCREEN):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Screen\n");
      break;
    case (NC_SCENE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Scene\n");
      break;
    case (NC_OBJECT):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Object\n");
      break;
    case (NC_MATERIAL):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Material\n");
      break;
    case (NC_TEXTURE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Texture\n");
      break;
    case (NC_LAMP):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Lamp\n");
      break;
    case (NC_GROUP):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Group\n");
      break;
    case (NC_IMAGE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Image\n");
      break;
    case (NC_BRUSH):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Brush\n");
      break;
    case (NC_TEXT):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Text\n");
      break;
    case (NC_WORLD):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: World\n");
      break;
    case (NC_ANIMATION):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Animation\n");
      break;
    case (NC_SPACE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Space\n");
      break;
    case (NC_GEOM):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Geom\n");
      break;
    case (NC_NODE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Node\n");
      break;
    case (NC_ID):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: ID\n");
      break;
    case (NC_PAINTCURVE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: PaintCurve\n");
      break;
    case (NC_MOVIECLIP):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: MovieClip\n");
      break;
    case (NC_MASK):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Mask\n");
      break;
    case (NC_GPENCIL):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: GPencil\n");
      break;
    case (NC_LINESTYLE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: LineStyle\n");
      break;
    case (NC_CAMERA):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Camera\n");
      break;
    case (NC_LIGHTPROBE):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: LightProbe\n");
      break;
    case (NC_ASSET):
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Asset\n");
      break;
    default:
      TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Category: Undefined\n");
      break;
  }
  ++ref;
}


wmNotifier::wmNotifier()
  : window(nullptr),
    category(0),
    data(0),
    subtype(0),
    action(0),
    reference(nullptr),
    notice(TfNotice())
{}


void wmNotifier::Push()
{
  MsgBusCallback *cb = new MsgBusCallback(this);
  MsgBus invoker(cb);
  TfNotice::Register(invoker, &MsgBusCallback::wmCOMM, invoker);
  notice.Send(invoker);
}


MsgBusCallback::MsgBusCallback(wmOperatorType *ot) : ref(1), notice(ot->notice), op({ot}) {}

void MsgBusCallback::OperatorCOMM(const TfNotice &notice, MsgBus const &sender)
{
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("MsgBus\n");
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("  Type: Operator\n");
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("    ID: %s\n", CHARALL(op.type->idname));
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("    Name: %s\n", op.type->name);
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("    Description: %s\n", op.type->description);
  TF_DEBUG(KRAKEN_DEBUG_MSGBUS).Msg("\n");
  ++ref;
}


WABI_NAMESPACE_END