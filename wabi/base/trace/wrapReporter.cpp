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

#include "wabi/wabi.h"

#include "wabi/base/trace/reporter.h"
#include "wabi/base/trace/reporterDataSourceCollector.h"

#include "wabi/base/tf/makePyConstructor.h"
#include "wabi/base/tf/pyEnum.h"
#include "wabi/base/tf/pyPtrHelpers.h"

#include <boost/python/class.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

WABI_NAMESPACE_USING

using namespace boost::python;

static void _Report(const TraceReporterPtr &self, int iterationCount)
{
  self->Report(std::cout, iterationCount);
}

static void _ReportToFile(const TraceReporterPtr &self,
                          const std::string &fileName,
                          int iterationCount,
                          bool append)
{
  std::ofstream os(fileName.c_str(), append ? std::ios_base::app : std::ios_base::out);
  self->Report(os, iterationCount);
}

static void _ReportTimes(TraceReporterPtr self)
{
  self->ReportTimes(std::cout);
}

static void _ReportChromeTracing(const TraceReporterPtr &self)
{
  self->ReportChromeTracing(std::cout);
}

static void _ReportChromeTracingToFile(const TraceReporterPtr &self, const std::string &fileName)
{
  std::ofstream os(fileName.c_str());
  self->ReportChromeTracing(os);
}

static TraceReporterRefPtr _Constructor1(const std::string &label)
{
  return TraceReporter::New(label, TraceReporterDataSourceCollector::New());
}

void wrapReporter()
{
  using This = TraceReporter;
  using ThisPtr = TraceReporterPtr;

  object reporter_class =
    class_<This, ThisPtr, boost::noncopyable>("Reporter", no_init)
      .def(TfPyRefAndWeakPtr())
      .def(TfMakePyConstructor(_Constructor1))

      .def("GetLabel", &This::GetLabel, return_value_policy<return_by_value>())

      .def("Report", &::_Report, (arg("iterationCount") = 1))

      .def("Report", &::_ReportToFile, (arg("iterationCount") = 1, arg("append") = false))

      .def("ReportTimes", &::_ReportTimes)

      .def("ReportChromeTracing", &::_ReportChromeTracing)
      .def("ReportChromeTracingToFile", &::_ReportChromeTracingToFile)

      .add_property("aggregateTreeRoot", &This::GetAggregateTreeRoot)

      .def("UpdateTraceTrees", &This::UpdateTraceTrees)

      .def("ClearTree", &This::ClearTree)

      .add_property("groupByFunction", &This::GetGroupByFunction, &This::SetGroupByFunction)

      .add_property("foldRecursiveCalls", &This::GetFoldRecursiveCalls, &This::SetFoldRecursiveCalls)

      .add_static_property("globalReporter", &This::GetGlobalReporter);
};
