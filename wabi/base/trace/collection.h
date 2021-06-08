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

#ifndef WABI_BASE_TRACE_COLLECTION_H
#define WABI_BASE_TRACE_COLLECTION_H

#include "wabi/wabi.h"

#include "wabi/base/trace/api.h"
#include "wabi/base/trace/event.h"
#include "wabi/base/trace/eventList.h"
#include "wabi/base/trace/threads.h"

#include "wabi/base/tf/mallocTag.h"

#include <map>
#include <unordered_map>

WABI_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
///
/// \class TraceCollection
///
/// This class owns lists of TraceEvent instances per thread, and allows
/// read access to them.
///
class TraceCollection {
 public:
  TF_MALLOC_TAG_NEW("Trace", "TraceCollection");

  using This = TraceCollection;

  using EventList    = TraceEventList;
  using EventListPtr = std::unique_ptr<EventList>;

  /// Constructor.
  TraceCollection() = default;

  /// Move constructor.
  TraceCollection(TraceCollection &&) = default;

  /// Move assignment operator.
  TraceCollection &operator=(TraceCollection &&) = default;

  // Collections should not be copied because TraceEvents contain
  // pointers to elements in the Key cache.
  TraceCollection(const TraceCollection &) = delete;
  TraceCollection &operator=(const TraceCollection &) = delete;

  /// Appends \p events to the collection. The collection will
  /// take ownership of the data.
  TRACE_API void AddToCollection(const TraceThreadId &id, EventListPtr &&events);

  ////////////////////////////////////////////////////////////////////////
  ///
  /// \class Visitor
  ///
  /// This interface provides a way to access data a TraceCollection.
  ///
  class Visitor {
   public:
    /// Destructor
    TRACE_API virtual ~Visitor();

    /// Called at the beginning of an iteration.
    virtual void OnBeginCollection() = 0;

    /// Called at the end of an iteration.
    virtual void OnEndCollection() = 0;

    /// Called before the first event of from the thread with
    /// \p threadId is encountered.
    virtual void OnBeginThread(const TraceThreadId &threadId) = 0;

    /// Called after the last event of from the thread with
    /// \p threadId is encountered.
    virtual void OnEndThread(const TraceThreadId &threadId) = 0;

    /// Called before an event with \p categoryId is visited. If the
    /// return value is false, the event will be visited.
    virtual bool AcceptsCategory(TraceCategoryId categoryId) = 0;

    /// Called for every event \p event with \p key on thread
    /// \p threadId if AcceptsCategory returns true.
    virtual void OnEvent(const TraceThreadId &threadId,
                         const TfToken &key,
                         const TraceEvent &event) = 0;
  };

  /// Forward iterates over the events of the collection and calls the
  /// \p visitor callbacks.
  TRACE_API void Iterate(Visitor &visitor) const;

  /// Reverse iterates over the events of the collection and calls the
  /// \p visitor callbacks.
  TRACE_API void ReverseIterate(Visitor &visitor) const;

 private:
  using KeyTokenCache = std::unordered_map<TraceKey, TfToken, TraceKey::HashFunctor>;

  /// Iterate through threads, then choose either forward or reverse
  /// iteration for the events in the threads
  void _Iterate(Visitor &visitor, bool doReverse) const;

  // Iterate through events in either forward or reverse order, depending on
  // the templated arguments
  template<class I>
  void _IterateEvents(Visitor &, KeyTokenCache &, const TraceThreadId &, I, I) const;

  using EventTable = std::map<TraceThreadId, EventListPtr>;

  EventTable _eventsPerThread;
};

WABI_NAMESPACE_END

#endif  // WABI_BASE_TRACE_COLLECTION_H
