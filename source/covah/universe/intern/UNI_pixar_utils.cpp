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
 * Universe.
 * Set the Stage.
 */

#include "UNI_pixar_utils.h"

#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_BEGIN


void UNI_pixutil_convert(const std::string &args)
{
  static std::vector<std::string> extra_args;
  extra_args.push_back(args);

  UsdStageRefPtr stage = UsdStage::Open(extra_args.at(0));

  SdfLayer::FileFormatArguments extrafmt = SdfLayer::FileFormatArguments();
  TF_FOR_ALL (arg, extra_args)
  {
    static int index = 0;
    static std::string prev = extra_args.at(index);
    if ((index += 1) < extra_args.size())
    {
      extrafmt.insert(std::make_pair(prev, extra_args.at(index)));
    }
  }

  stage->Export(extra_args.at(0), true, extrafmt);
}


WABI_NAMESPACE_END