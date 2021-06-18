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

#ifndef WABI_BASE_TRACE_AGGREGATE_TREE_BUILDER_H
#define WABI_BASE_TRACE_AGGREGATE_TREE_BUILDER_H

#include "wabi/wabi.h"

#include "wabi/base/trace/aggregateTree.h"
#include "wabi/base/trace/api.h"
#include "wabi/base/trace/collection.h"
#include "wabi/base/trace/eventTree.h"

WABI_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////
/// \class Trace_AggregateTreeBuilder
///
/// This class populates a tree of TraceAggregateTree instances from
/// TraceCollection instances.
///
///
class Trace_AggregateTreeBuilder : private TraceCollection::Visitor
{
 public:
  static void AddEventTreeToAggregate(TraceAggregateTree *aggregateTree,
                                      const TraceEventTreeRefPtr &eventTree,
                                      const TraceCollection &collection);

 private:
  Trace_AggregateTreeBuilder(TraceAggregateTree *tree, const TraceEventTreeRefPtr &eventTree);

  void _ProcessCounters(const TraceCollection &collection);

  void _CreateAggregateNodes();

  // TraceCollection::Visitor interface
  virtual void OnBeginCollection() override;
  virtual void OnEndCollection() override;
  virtual void OnBeginThread(const TraceThreadId &threadId) override;
  virtual void OnEndThread(const TraceThreadId &threadId) override;
  virtual bool AcceptsCategory(TraceCategoryId categoryId) override;
  virtual void OnEvent(const TraceThreadId &threadIndex, const TfToken &key, const TraceEvent &e) override;

  void _OnCounterEvent(const TraceThreadId &threadIndex, const TfToken &key, const TraceEvent &e);

  TraceAggregateNodePtr _FindAggregateNode(const TraceThreadId &threadId,
                                           const TraceEvent::TimeStamp ts) const;

  TraceAggregateTree *_aggregateTree;
  TraceEventTreeRefPtr _tree;
};

WABI_NAMESPACE_END

#endif  // WABI_BASE_TRACE_AGGREGATE_TREE_BUILDER_H
