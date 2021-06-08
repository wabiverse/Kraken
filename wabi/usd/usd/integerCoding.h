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
#ifndef WABI_USD_USD_INTEGER_CODING_H
#define WABI_USD_USD_INTEGER_CODING_H

#include "wabi/usd/usd/api.h"
#include "wabi/wabi.h"

#include <cstdint>
#include <memory>

WABI_NAMESPACE_BEGIN

class Usd_IntegerCompression {
 public:
  // Return the max compression buffer size required for \p numInts 32-bit
  // integers.
  USD_API
  static size_t GetCompressedBufferSize(size_t numInts);

  // Return the max decompression working space size required for \p numInts
  // 32-bit integers.
  USD_API
  static size_t GetDecompressionWorkingSpaceSize(size_t numInts);

  // Compress \p numInts ints from \p ints to \p compressed.  The
  // \p compressed space must point to at least
  // GetCompressedBufferSize(numInts) bytes.  Return the actual number
  // of bytes written to \p compressed.
  USD_API
  static size_t CompressToBuffer(int32_t const *ints, size_t numInts, char *compressed);

  // Compress \p numInts ints from \p ints to \p compressed.  The
  // \p compressed space must point to at least
  // GetCompressedBufferSize(numInts) bytes.  Return the actual number
  // of bytes written to \p compressed.
  USD_API
  static size_t CompressToBuffer(uint32_t const *ints, size_t numInts, char *compressed);

  // Decompress \p compressedSize bytes from \p compressed to produce
  // \p numInts 32-bit integers into \p ints.  Clients may supply
  // \p workingSpace to save allocations if several decompressions will be
  // done but it isn't required.  If supplied it must point to at least
  // GetDecompressionWorkingSpaceSize(numInts) bytes.
  USD_API
  static size_t DecompressFromBuffer(char const *compressed,
                                     size_t compressedSize,
                                     int32_t *ints,
                                     size_t numInts,
                                     char *workingSpace = nullptr);

  // Decompress \p compressedSize bytes from \p compressed to produce
  // \p numInts 32-bit integers into \p ints.  Clients may supply
  // \p workingSpace to save allocations if several decompressions will be
  // done but it isn't required.  If supplied it must point to at least
  // GetDecompressionWorkingSpaceSize(numInts) bytes.
  USD_API
  static size_t DecompressFromBuffer(char const *compressed,
                                     size_t compressedSize,
                                     uint32_t *ints,
                                     size_t numInts,
                                     char *workingSpace = nullptr);
};

class Usd_IntegerCompression64 {
 public:
  // Return the max compression buffer size required for \p numInts 64-bit
  // integers.
  USD_API
  static size_t GetCompressedBufferSize(size_t numInts);

  // Return the max decompression working space size required for \p numInts
  // 64-bit integers.
  USD_API
  static size_t GetDecompressionWorkingSpaceSize(size_t numInts);

  // Compress \p numInts ints from \p ints to \p compressed.  The
  // \p compressed space must point to at least
  // GetCompressedBufferSize(numInts) bytes.  Return the actual number
  // of bytes written to \p compressed.
  USD_API
  static size_t CompressToBuffer(int64_t const *ints, size_t numInts, char *compressed);

  // Compress \p numInts ints from \p ints to \p compressed.  The
  // \p compressed space must point to at least
  // GetCompressedBufferSize(numInts) bytes.  Return the actual number
  // of bytes written to \p compressed.
  USD_API
  static size_t CompressToBuffer(uint64_t const *ints, size_t numInts, char *compressed);

  // Decompress \p compressedSize bytes from \p compressed to produce
  // \p numInts 64-bit integers into \p ints.  Clients may supply
  // \p workingSpace to save allocations if several decompressions will be
  // done but it isn't required.  If supplied it must point to at least
  // GetDecompressionWorkingSpaceSize(numInts) bytes.
  USD_API
  static size_t DecompressFromBuffer(char const *compressed,
                                     size_t compressedSize,
                                     int64_t *ints,
                                     size_t numInts,
                                     char *workingSpace = nullptr);

  // Decompress \p compressedSize bytes from \p compressed to produce
  // \p numInts 64-bit integers into \p ints.  Clients may supply
  // \p workingSpace to save allocations if several decompressions will be
  // done but it isn't required.  If supplied it must point to at least
  // GetDecompressionWorkingSpaceSize(numInts) bytes.
  USD_API
  static size_t DecompressFromBuffer(char const *compressed,
                                     size_t compressedSize,
                                     uint64_t *ints,
                                     size_t numInts,
                                     char *workingSpace = nullptr);
};

WABI_NAMESPACE_END

#endif  // WABI_USD_USD_INTEGER_CODING_H
