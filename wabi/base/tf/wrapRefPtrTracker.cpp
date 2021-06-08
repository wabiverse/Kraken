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
/// \file wrapRefPtrTracker.cpp

#include "wabi/wabi.h"

#include "wabi/base/tf/pySingleton.h"
#include "wabi/base/tf/refPtr.h"
#include "wabi/base/tf/refPtrTracker.h"
#include "wabi/base/tf/weakPtr.h"

#include <boost/python/class.hpp>
#include <sstream>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace {

static std::string _ReportAllWatchedCounts(TfRefPtrTracker &tracker)
{
  std::ostringstream s;
  tracker.ReportAllWatchedCounts(s);
  return s.str();
}

static std::string _ReportAllTraces(TfRefPtrTracker &tracker)
{
  std::ostringstream s;
  tracker.ReportAllTraces(s);
  return s.str();
}

static std::string _ReportTracesForWatched(TfRefPtrTracker &tracker, uintptr_t ptr)
{
  std::ostringstream s;
  tracker.ReportTracesForWatched(s, (TfRefBase *)ptr);
  return s.str();
}

}  // anonymous namespace

void wrapRefPtrTracker()
{
  typedef TfRefPtrTracker This;
  typedef TfWeakPtr<TfRefPtrTracker> ThisPtr;

  class_<This, ThisPtr, boost::noncopyable>("RefPtrTracker", no_init)
      .def(TfPySingleton())

      .def("GetAllWatchedCountsReport", _ReportAllWatchedCounts)
      .def("GetAllTracesReport", _ReportAllTraces)
      .def("GetTracesReportForWatched", _ReportTracesForWatched);
}
