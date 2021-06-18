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

#ifndef DOUBLE_CONVERSION_DIY_FP_H_
#define DOUBLE_CONVERSION_DIY_FP_H_

#include "utils.h"

#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

namespace wabi_double_conversion
{

// This "Do It Yourself Floating Point" class implements a floating-point number
// with a uint64 significand and an int exponent. Normalized DiyFp numbers will
// have the most significant bit of the significand set.
// Multiplication and Subtraction do not normalize their results.
// DiyFp are not designed to contain special doubles (NaN and Infinity).
class DiyFp
{
 public:
  static const int kSignificandSize = 64;

  DiyFp()
    : f_(0),
      e_(0)
  {}
  DiyFp(uint64_t significand, int exponent)
    : f_(significand),
      e_(exponent)
  {}

  // this = this - other.
  // The exponents of both numbers must be the same and the significand of this
  // must be bigger than the significand of other.
  // The result will not be normalized.
  void Subtract(const DiyFp &other)
  {
    ASSERT(e_ == other.e_);
    ASSERT(f_ >= other.f_);
    f_ -= other.f_;
  }

  // Returns a - b.
  // The exponents of both numbers must be the same and this must be bigger
  // than other. The result will not be normalized.
  static DiyFp Minus(const DiyFp &a, const DiyFp &b)
  {
    DiyFp result = a;
    result.Subtract(b);
    return result;
  }

  // this = this * other.
  void Multiply(const DiyFp &other);

  // returns a * b;
  static DiyFp Times(const DiyFp &a, const DiyFp &b)
  {
    DiyFp result = a;
    result.Multiply(b);
    return result;
  }

  void Normalize()
  {
    ASSERT(f_ != 0);
    uint64_t significand = f_;
    int exponent = e_;

    // This method is mainly called for normalizing boundaries. In general
    // boundaries need to be shifted by 10 bits. We thus optimize for this case.
    const uint64_t k10MSBits = UINT64_2PART_C(0xFFC00000, 00000000);
    while ((significand & k10MSBits) == 0)
    {
      significand <<= 10;
      exponent -= 10;
    }
    while ((significand & kUint64MSB) == 0)
    {
      significand <<= 1;
      exponent--;
    }
    f_ = significand;
    e_ = exponent;
  }

  static DiyFp Normalize(const DiyFp &a)
  {
    DiyFp result = a;
    result.Normalize();
    return result;
  }

  uint64_t f() const
  {
    return f_;
  }
  int e() const
  {
    return e_;
  }

  void set_f(uint64_t new_value)
  {
    f_ = new_value;
  }
  void set_e(int new_value)
  {
    e_ = new_value;
  }

 private:
  static const uint64_t kUint64MSB = UINT64_2PART_C(0x80000000, 00000000);

  uint64_t f_;
  int e_;
};

}  // namespace wabi_double_conversion

WABI_NAMESPACE_END

#endif  // DOUBLE_CONVERSION_DIY_FP_H_
