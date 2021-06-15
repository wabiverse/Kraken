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

#include "wabi/base/trace/counterAccumulator.h"

#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

void TraceCounterAccumulator::Update(const TraceCollection &col)
{
  col.Iterate(*this);
}

void TraceCounterAccumulator::OnBeginCollection()
{}

void TraceCounterAccumulator::OnEndCollection()
{
  // Convert the counter deltas and values to absolute values;
  for (const _CounterDeltaMap::value_type &c : _counterDeltas) {
    double curValue = _currentValues[c.first];

    for (const _CounterDeltaValues::value_type &v : c.second) {
      if (v.second.isDelta) {
        curValue += v.second.value;
      }
      else {
        curValue = v.second.value;
      }
      _counterValuesOverTime[c.first].emplace_back(v.first, curValue);
    }
    _currentValues[c.first] = curValue;
  }
  _counterDeltas.clear();
}

void TraceCounterAccumulator::OnBeginThread(const TraceThreadId &)
{
  // Do Nothing
}
void TraceCounterAccumulator::OnEndThread(const TraceThreadId &)
{
  // Do Nothing
}

void TraceCounterAccumulator::OnEvent(const TraceThreadId &, const TfToken &key, const TraceEvent &e)
{
  switch (e.GetType()) {
    case TraceEvent::EventType::CounterDelta: {
      _counterDeltas[key].insert(std::make_pair(e.GetTimeStamp(), _CounterValue{e.GetCounterValue(), true}));
      break;
    }
    case TraceEvent::EventType::CounterValue: {
      _counterDeltas[key].insert(
        std::make_pair(e.GetTimeStamp(), _CounterValue{e.GetCounterValue(), false}));
      break;
    }
    default:
      break;
  }
}
bool TraceCounterAccumulator::AcceptsCategory(TraceCategoryId id)
{
  return _AcceptsCategory(id);
}

void TraceCounterAccumulator::SetCurrentValues(const CounterMap &values)
{
  _currentValues = values;
}

WABI_NAMESPACE_END