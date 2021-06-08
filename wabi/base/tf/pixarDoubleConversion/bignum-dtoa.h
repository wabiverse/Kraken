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

#ifndef DOUBLE_CONVERSION_BIGNUM_DTOA_H_
#define DOUBLE_CONVERSION_BIGNUM_DTOA_H_

#include "utils.h"

#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

namespace wabi_double_conversion {

enum BignumDtoaMode {
  // Return the shortest correct representation.
  // For example the output of 0.299999999999999988897 is (the less accurate but
  // correct) 0.3.
  BIGNUM_DTOA_SHORTEST,
  // Same as BIGNUM_DTOA_SHORTEST but for single-precision floats.
  BIGNUM_DTOA_SHORTEST_SINGLE,
  // Return a fixed number of digits after the decimal point.
  // For instance fixed(0.1, 4) becomes 0.1000
  // If the input number is big, the output will be big.
  BIGNUM_DTOA_FIXED,
  // Return a fixed number of digits, no matter what the exponent is.
  BIGNUM_DTOA_PRECISION
};

// Converts the given double 'v' to ascii.
// The result should be interpreted as buffer * 10^(point-length).
// The buffer will be null-terminated.
//
// The input v must be > 0 and different from NaN, and Infinity.
//
// The output depends on the given mode:
//  - SHORTEST: produce the least amount of digits for which the internal
//   identity requirement is still satisfied. If the digits are printed
//   (together with the correct exponent) then reading this number will give
//   'v' again. The buffer will choose the representation that is closest to
//   'v'. If there are two at the same distance, than the number is round up.
//   In this mode the 'requested_digits' parameter is ignored.
//  - FIXED: produces digits necessary to print a given number with
//   'requested_digits' digits after the decimal point. The produced digits
//   might be too short in which case the caller has to fill the gaps with '0's.
//   Example: toFixed(0.001, 5) is allowed to return buffer="1", point=-2.
//   Halfway cases are rounded up. The call toFixed(0.15, 2) thus returns
//     buffer="2", point=0.
//   Note: the length of the returned buffer has no meaning wrt the significance
//   of its digits. That is, just because it contains '0's does not mean that
//   any other digit would not satisfy the internal identity requirement.
//  - PRECISION: produces 'requested_digits' where the first digit is not '0'.
//   Even though the length of produced digits usually equals
//   'requested_digits', the function is allowed to return fewer digits, in
//   which case the caller has to fill the missing digits with '0's.
//   Halfway cases are again rounded up.
// 'BignumDtoa' expects the given buffer to be big enough to hold all digits
// and a terminating null-character.
void BignumDtoa(double v,
                BignumDtoaMode mode,
                int requested_digits,
                Vector<char> buffer,
                int *length,
                int *point);

}  // namespace wabi_double_conversion

WABI_NAMESPACE_END

#endif  // DOUBLE_CONVERSION_BIGNUM_DTOA_H_
