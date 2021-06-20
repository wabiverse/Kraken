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

#pragma once

#include "WM_api.h"

#include "UNI_api.h"

#include <wabi/base/tf/token.h>

WABI_NAMESPACE_BEGIN

struct COVAH_OPERATOR_TOKENS_TYPE
{
  COVAH_WM_API
  COVAH_OPERATOR_TOKENS_TYPE();

  COVAH_DECLARE_STATIC_TOKEN(WM_OT_window_close);
  COVAH_DECLARE_STATIC_TOKEN(WM_OT_window_new);
  COVAH_DECLARE_STATIC_TOKEN(WM_OT_window_new_main);
  COVAH_DECLARE_STATIC_TOKEN(WM_OT_window_fullscreen_toggle);
  COVAH_DECLARE_STATIC_TOKEN(WM_OT_quit_covah);

  const std::vector<TfToken> allTokens;
};

extern COVAH_WM_API
  TfStaticData<COVAH_OPERATOR_TOKENS_TYPE>
    COVAH_OPERATOR_TOKENS;

WABI_NAMESPACE_END