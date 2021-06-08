//  Copyright 2021 Tangent Animation
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
//  including without limitation, as related to merchantability and fitness
//  for a particular purpose.
//
//  In no event shall any copyright holder be liable for any damages of any kind
//  arising from the use of this software, whether in contract, tort or otherwise.
//  See the License for the specific language governing permissions and
//  limitations under the License.

//  Usage of the debug codes follows Arnold's render delegate
//  https://github.com/Autodesk/arnold-usd/blob/11eb3ced2b6a148bb5737fddeb25e4e21273607f/render_delegate/

#ifndef HD_CYCLES_DEBUG_CODES_H
#define HD_CYCLES_DEBUG_CODES_H

#include <wabi/wabi.h>

#include <wabi/base/tf/debug.h>

WABI_NAMESPACE_BEGIN

// clang-format off
TF_DEBUG_CODES(
	HDCYCLES_MESH
)
// clang-format on

WABI_NAMESPACE_END

#endif  // HD_CYCLES_DEBUG_CODES_H