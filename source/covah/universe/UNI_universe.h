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
 * @file UNI_universe.h
 * @ingroup UNI
 * The @a central foundation for @a all data access.
 */

#pragma once

#include "UNI_api.h"

#include <wabi/base/tf/hashmap.h>
#include <wabi/base/tf/singleton.h>
#include <wabi/base/tf/token.h>
#include <wabi/base/tf/weakBase.h>

/** To Infinity and Beyond! 🌌 */
class UNIVERSE : public wabi::TfWeakBase {
 public:
  typedef UNIVERSE This;

 public:
  COVAH_UNIVERSE_API
  static UNIVERSE &GetInstance();

  UNIVERSE(UNIVERSE const &) = delete;
  UNIVERSE &operator=(UNIVERSE const &) = delete;

 private:
  UNIVERSE();

  friend class wabi::TfSingleton<UNIVERSE>;

 private:
  wabi::TfHashMap<std::string, wabi::TfHash> m_scene;
};