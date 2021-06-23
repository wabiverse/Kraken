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

#include "WM_tokens.h"

WABI_NAMESPACE_BEGIN

COVAH_OPERATOR_TOKENS_TYPE::COVAH_OPERATOR_TOKENS_TYPE()
  : COVAH_DEFINE_STATIC_TOKEN(WM_OT_open_mainfile),
    COVAH_DEFINE_STATIC_TOKEN(WM_OT_window_close),
    COVAH_DEFINE_STATIC_TOKEN(WM_OT_window_new),
    COVAH_DEFINE_STATIC_TOKEN(WM_OT_window_new_main),
    COVAH_DEFINE_STATIC_TOKEN(WM_OT_window_fullscreen_toggle),
    COVAH_DEFINE_STATIC_TOKEN(WM_OT_quit_covah),
    allTokens({WM_OT_window_close,
               WM_OT_window_new,
               WM_OT_window_new_main,
               WM_OT_window_fullscreen_toggle,
               WM_OT_quit_covah})
{}

TfStaticData<COVAH_OPERATOR_TOKENS_TYPE> COVAH_OPERATOR_TOKENS;

WABI_NAMESPACE_END
