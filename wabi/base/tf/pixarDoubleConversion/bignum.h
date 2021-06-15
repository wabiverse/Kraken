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

#ifndef DOUBLE_CONVERSION_BIGNUM_H_
#define DOUBLE_CONVERSION_BIGNUM_H_

#include "utils.h"

#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

namespace wabi_double_conversion {

class Bignum {
 public:
  // 3584 = 128 * 28. We can represent 2^3584 > 10^1000 accurately.
  // This bignum can encode much bigger numbers, since it contains an
  // exponent.
  static const int kMaxSignificantBits = 3584;

  Bignum();
  void AssignUInt16(uint16_t value);
  void AssignUInt64(uint64_t value);
  void AssignBignum(const Bignum &other);

  void AssignDecimalString(Vector<const char> value);
  void AssignHexString(Vector<const char> value);

  void AssignPowerUInt16(uint16_t base, int exponent);

  void AddUInt64(uint64_t operand);
  void AddBignum(const Bignum &other);
  // Precondition: this >= other.
  void SubtractBignum(const Bignum &other);

  void Square();
  void ShiftLeft(int shift_amount);
  void MultiplyByUInt32(uint32_t factor);
  void MultiplyByUInt64(uint64_t factor);
  void MultiplyByPowerOfTen(int exponent);
  void Times10()
  {
    return MultiplyByUInt32(10);
  }
  // Pseudocode:
  //  int result = this / other;
  //  this = this % other;
  // In the worst case this function is in O(this/other).
  uint16_t DivideModuloIntBignum(const Bignum &other);

  bool ToHexString(char *buffer, int buffer_size) const;

  // Returns
  //  -1 if a < b,
  //   0 if a == b, and
  //  +1 if a > b.
  static int Compare(const Bignum &a, const Bignum &b);
  static bool Equal(const Bignum &a, const Bignum &b)
  {
    return Compare(a, b) == 0;
  }
  static bool LessEqual(const Bignum &a, const Bignum &b)
  {
    return Compare(a, b) <= 0;
  }
  static bool Less(const Bignum &a, const Bignum &b)
  {
    return Compare(a, b) < 0;
  }
  // Returns Compare(a + b, c);
  static int PlusCompare(const Bignum &a, const Bignum &b, const Bignum &c);
  // Returns a + b == c
  static bool PlusEqual(const Bignum &a, const Bignum &b, const Bignum &c)
  {
    return PlusCompare(a, b, c) == 0;
  }
  // Returns a + b <= c
  static bool PlusLessEqual(const Bignum &a, const Bignum &b, const Bignum &c)
  {
    return PlusCompare(a, b, c) <= 0;
  }
  // Returns a + b < c
  static bool PlusLess(const Bignum &a, const Bignum &b, const Bignum &c)
  {
    return PlusCompare(a, b, c) < 0;
  }

 private:
  typedef uint32_t Chunk;
  typedef uint64_t DoubleChunk;

  static const int kChunkSize = sizeof(Chunk) * 8;
  static const int kDoubleChunkSize = sizeof(DoubleChunk) * 8;
  // With bigit size of 28 we loose some bits, but a double still fits easily
  // into two chunks, and more importantly we can use the Comba multiplication.
  static const int kBigitSize = 28;
  static const Chunk kBigitMask = (1 << kBigitSize) - 1;
  // Every instance allocates kBigitLength chunks on the stack. Bignums cannot
  // grow. There are no checks if the stack-allocated space is sufficient.
  static const int kBigitCapacity = kMaxSignificantBits / kBigitSize;

  void EnsureCapacity(int size)
  {
    if (size > kBigitCapacity) {
      UNREACHABLE();
    }
  }
  void Align(const Bignum &other);
  void Clamp();
  bool IsClamped() const;
  void Zero();
  // Requires this to have enough capacity (no tests done).
  // Updates used_digits_ if necessary.
  // shift_amount must be < kBigitSize.
  void BigitsShiftLeft(int shift_amount);
  // BigitLength includes the "hidden" digits encoded in the exponent.
  int BigitLength() const
  {
    return used_digits_ + exponent_;
  }
  Chunk BigitAt(int index) const;
  void SubtractTimes(const Bignum &other, int factor);

  Chunk bigits_buffer_[kBigitCapacity];
  // A vector backed by bigits_buffer_. This way accesses to the array are
  // checked for out-of-bounds errors.
  Vector<Chunk> bigits_;
  int used_digits_;
  // The Bignum's value equals value(bigits_) * 2^(exponent_ * kBigitSize).
  int exponent_;

  DISALLOW_COPY_AND_ASSIGN(Bignum);
};

}  // namespace wabi_double_conversion

WABI_NAMESPACE_END

#endif  // DOUBLE_CONVERSION_BIGNUM_H_
