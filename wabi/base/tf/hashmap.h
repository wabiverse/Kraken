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

#ifndef WABI_BASE_TF_HASHMAP_H
#define WABI_BASE_TF_HASHMAP_H

#include "wabi/base/arch/defines.h"
#include "wabi/wabi.h"
#include <unordered_map>

WABI_NAMESPACE_BEGIN

template<class Key,
         class Mapped,
         class HashFn = std::hash<Key>,
         class EqualKey = std::equal_to<Key>,
         class Alloc = std::allocator<std::pair<const Key, Mapped>>>
class TfHashMap : private std::unordered_map<Key, Mapped, HashFn, EqualKey, Alloc>
{
  typedef std::unordered_map<Key, Mapped, HashFn, EqualKey, Alloc> _Base;

 public:

  typedef typename _Base::key_type key_type;
  typedef typename _Base::mapped_type mapped_type;
  typedef typename _Base::value_type value_type;
  typedef typename _Base::hasher hasher;
  typedef typename _Base::key_equal key_equal;
  typedef typename _Base::size_type size_type;
  typedef typename _Base::difference_type difference_type;
  typedef typename _Base::pointer pointer;
  typedef typename _Base::const_pointer const_pointer;
  typedef typename _Base::reference reference;
  typedef typename _Base::const_reference const_reference;
  typedef typename _Base::iterator iterator;
  typedef typename _Base::const_iterator const_iterator;
  typedef typename _Base::allocator_type allocator_type;
  // No local_iterator nor any methods using them.

  TfHashMap() : _Base() {}
  explicit TfHashMap(size_type n,
                     const hasher &hf = hasher(),
                     const key_equal &eql = key_equal(),
                     const allocator_type &alloc = allocator_type())
    : _Base(n, hf, eql, alloc)
  {}
  explicit TfHashMap(const allocator_type &alloc) : _Base(alloc) {}
  template<class InputIterator>
  TfHashMap(InputIterator first,
            InputIterator last,
            size_type n = 0,
            const hasher &hf = hasher(),
            const key_equal &eql = key_equal(),
            const allocator_type &alloc = allocator_type())
    : _Base(first, last, n, hf, eql, alloc)
  {}
  TfHashMap(const TfHashMap &other) : _Base(other) {}

  TfHashMap &operator=(const TfHashMap &rhs)
  {
    _Base::operator=(rhs);
    return *this;
  }

  iterator begin()
  {
    return _Base::begin();
  }
  const_iterator begin() const
  {
    return _Base::begin();
  }
  // using _Base::bucket;
  using _Base::bucket_count;
  using _Base::bucket_size;
  const_iterator cbegin() const
  {
    return _Base::cbegin();
  }
  const_iterator cend() const
  {
    return _Base::cend();
  }
  using _Base::clear;
  using _Base::count;
  using _Base::empty;
  iterator end()
  {
    return _Base::end();
  }
  const_iterator end() const
  {
    return _Base::end();
  }
  using _Base::equal_range;
  size_type erase(const key_type &key)
  {
    return _Base::erase(key);
  }
  void erase(const_iterator position)
  {
    _Base::erase(position);
  }
  void erase(const_iterator first, const_iterator last)
  {
    _Base::erase(first, last);
  }
  using _Base::find;
  using _Base::get_allocator;
  using _Base::hash_function;
  std::pair<iterator, bool> insert(const value_type &v)
  {
    return _Base::insert(v);
  }
  iterator insert(const_iterator hint, const value_type &v)
  {
    return _Base::insert(hint, v);
  }
  template<class InputIterator> void insert(InputIterator first, InputIterator last)
  {
    _Base::insert(first, last);
  }
  template<class... Args> std::pair<iterator, bool> emplace(Args &&...args)
  {
    return _Base::emplace(std::forward<Args>(args)...);
  }
  template<class... Args> std::pair<iterator, bool> try_emplace(const Key &k, Args &&...args)
  {
    return _Base::try_emplace(k, std::forward<Args>(args)...);
  }
  using _Base::key_eq;
  using _Base::load_factor;
  using _Base::max_bucket_count;
  using _Base::max_load_factor;
  // using _Base::max_load_factor;
  using _Base::max_size;
  using _Base::rehash;
  using _Base::reserve;
  using _Base::size;
  void swap(TfHashMap &other)
  {
    _Base::swap(other);
  }
  mapped_type &operator[](const key_type &k)
  {
    return _Base::operator[](k);
  }

  template<class Key2, class Mapped2, class HashFn2, class EqualKey2, class Alloc2>
  friend bool operator==(const TfHashMap<Key2, Mapped2, HashFn2, EqualKey2, Alloc2> &,
                         const TfHashMap<Key2, Mapped2, HashFn2, EqualKey2, Alloc2> &);
};

template<class Key,
         class Mapped,
         class HashFn = std::hash<Key>,
         class EqualKey = std::equal_to<Key>,
         class Alloc = std::allocator<std::pair<const Key, Mapped>>>
class TfHashMultiMap : private std::unordered_multimap<Key, Mapped, HashFn, EqualKey, Alloc>
{
  typedef std::unordered_multimap<Key, Mapped, HashFn, EqualKey, Alloc> _Base;

 public:

  typedef typename _Base::key_type key_type;
  typedef typename _Base::mapped_type mapped_type;
  typedef typename _Base::value_type value_type;
  typedef typename _Base::hasher hasher;
  typedef typename _Base::key_equal key_equal;
  typedef typename _Base::size_type size_type;
  typedef typename _Base::difference_type difference_type;
  typedef typename _Base::pointer pointer;
  typedef typename _Base::const_pointer const_pointer;
  typedef typename _Base::reference reference;
  typedef typename _Base::const_reference const_reference;
  typedef typename _Base::iterator iterator;
  typedef typename _Base::const_iterator const_iterator;
  typedef typename _Base::allocator_type allocator_type;
  // No local_iterator nor any methods using them.

  TfHashMultiMap() : _Base() {}
  explicit TfHashMultiMap(size_type n,
                          const hasher &hf = hasher(),
                          const key_equal &eql = key_equal(),
                          const allocator_type &alloc = allocator_type())
    : _Base(n, hf, eql, alloc)
  {}
  explicit TfHashMultiMap(const allocator_type &alloc) : _Base(alloc) {}
  template<class InputIterator>
  TfHashMultiMap(InputIterator first,
                 InputIterator last,
                 size_type n = 0,
                 const hasher &hf = hasher(),
                 const key_equal &eql = key_equal(),
                 const allocator_type &alloc = allocator_type())
    : _Base(first, last, n, hf, eql, alloc)
  {}
  TfHashMultiMap(const TfHashMultiMap &other) : _Base(other) {}

  TfHashMultiMap &operator=(const TfHashMultiMap &rhs)
  {
    _Base::operator=(rhs);
    return *this;
  }

  iterator begin()
  {
    return _Base::begin();
  }
  const_iterator begin() const
  {
    return _Base::begin();
  }
  // using _Base::bucket;
  using _Base::bucket_count;
  using _Base::bucket_size;
  const_iterator cbegin() const
  {
    return _Base::cbegin();
  }
  const_iterator cend() const
  {
    return _Base::cend();
  }
  using _Base::clear;
  using _Base::count;
  using _Base::empty;
  iterator end()
  {
    return _Base::end();
  }
  const_iterator end() const
  {
    return _Base::end();
  }
  using _Base::equal_range;
  size_type erase(const key_type &key)
  {
    return _Base::erase(key);
  }
  void erase(const_iterator position)
  {
    _Base::erase(position);
  }
  void erase(const_iterator first, const_iterator last)
  {
    _Base::erase(first, last);
  }
  using _Base::find;
  using _Base::get_allocator;
  using _Base::hash_function;
  iterator insert(const value_type &v)
  {
    return _Base::insert(v);
  }
  iterator insert(const_iterator hint, const value_type &v)
  {
    return _Base::insert(hint, v);
  }
  template<class InputIterator> void insert(InputIterator first, InputIterator last)
  {
    _Base::insert(first, last);
  }
  using _Base::key_eq;
  using _Base::load_factor;
  using _Base::max_bucket_count;
  using _Base::max_load_factor;
  // using _Base::max_load_factor;
  using _Base::max_size;
  using _Base::rehash;
  using _Base::reserve;
  using _Base::size;
  void swap(TfHashMultiMap &other)
  {
    _Base::swap(other);
  }
  mapped_type &operator[](const key_type &k)
  {
    return _Base::operator[](k);
  }

  template<class Key2, class Mapped2, class HashFn2, class EqualKey2, class Alloc2>
  friend bool operator==(const TfHashMap<Key2, Mapped2, HashFn2, EqualKey2, Alloc2> &,
                         const TfHashMap<Key2, Mapped2, HashFn2, EqualKey2, Alloc2> &);
};

template<class Key, class Mapped, class HashFn, class EqualKey, class Alloc>
inline void swap(TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc> &lhs,
                 TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc> &rhs)
{
  lhs.swap(rhs);
}

template<class Key, class Mapped, class HashFn, class EqualKey, class Alloc>
inline bool operator==(const TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc> &lhs,
                       const TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc> &rhs)
{
  typedef typename TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc>::_Base _Base;
  return static_cast<const _Base &>(lhs) == static_cast<const _Base &>(rhs);
}

template<class Key, class Mapped, class HashFn, class EqualKey, class Alloc>
inline bool operator!=(const TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc> &lhs,
                       const TfHashMap<Key, Mapped, HashFn, EqualKey, Alloc> &rhs)
{
  return !(lhs == rhs);
}

template<class Key, class Mapped, class HashFn, class EqualKey, class Alloc>
inline void swap(TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc> &lhs,
                 TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc> &rhs)
{
  lhs.swap(rhs);
}

template<class Key, class Mapped, class HashFn, class EqualKey, class Alloc>
inline bool operator==(const TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc> &lhs,
                       const TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc> &rhs)
{
  typedef typename TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc>::_Base _Base;
  return static_cast<const _Base &>(lhs) == static_cast<const _Base &>(rhs);
}

template<class Key, class Mapped, class HashFn, class EqualKey, class Alloc>
inline bool operator!=(const TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc> &lhs,
                       const TfHashMultiMap<Key, Mapped, HashFn, EqualKey, Alloc> &rhs)
{
  return !(lhs == rhs);
}

WABI_NAMESPACE_END

#endif  // WABI_BASE_TF_HASHMAP_H