/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#ifndef WABI_USD_PCP_STRENGTH_ORDERING_H
#define WABI_USD_PCP_STRENGTH_ORDERING_H

#include "wabi/usd/pcp/api.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

class PcpNodeRef;

/// Compares the strength of nodes \p a and \p b. These nodes must be siblings;
/// it is a coding error if \p a and \p b do not have the same parent node.
///
/// Returns -1 if a is stronger than b,
///          0 if a is equivalent to b,
///          1 if a is weaker than b
PCP_API
int PcpCompareSiblingNodeStrength(const PcpNodeRef &a, const PcpNodeRef &b);

/// Compares the strength of nodes \p a and \p b. These nodes must be part
/// of the same graph; it is a coding error if \p a and \p b do not have the
/// same root node.
///
/// Returns -1 if a is stronger than b,
///          0 if a is equivalent to b,
///          1 if a is weaker than b
PCP_API
int PcpCompareNodeStrength(const PcpNodeRef &a, const PcpNodeRef &b);

WABI_NAMESPACE_END

#endif  // WABI_USD_PCP_STRENGTH_ORDERING_H
