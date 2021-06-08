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

#include "wabi/base/arch/fileSystem.h"
#include "wabi/base/tf/pyUtils.h"
#include "wabi/base/tf/stackTrace.h"

#include <boost/python/def.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING

namespace {

static void _PrintStackTrace(object &obj, const std::string &reason)
{
#if PY_MAJOR_VERSION == 2
  if (PyFile_Check(obj.ptr())) {
    FILE *file = expect_non_null(PyFile_AsFile(obj.ptr()));
    if (file)
      TfPrintStackTrace(file, reason);
  }
#else
  int fd = PyObject_AsFileDescriptor(obj.ptr());
  if (fd >= 0) {
    FILE *file = expect_non_null(ArchFdOpen(fd, "w"));
    if (file) {
      TfPrintStackTrace(file, reason);
      fclose(file);
    }
  }
#endif
  else {
    // Wrong type for obj
    TfPyThrowTypeError("Expected file object.");
  }
}

}  // anonymous namespace

void wrapStackTrace()
{
  def("GetStackTrace",
      TfGetStackTrace,
      "GetStackTrace()\n\n"
      "Return both the C++ and the python stack as a string.");

  def("PrintStackTrace",
      _PrintStackTrace,
      "PrintStackTrace(file, str)\n\n"
      "Prints both the C++ and the python stack to the file provided.");

  def("LogStackTrace", TfLogStackTrace, (arg("reason"), arg("logToDb") = false));

  def("GetAppLaunchTime",
      TfGetAppLaunchTime,
      "GetAppLaunchTime() -> int \n\n"
      "Return the time (in seconds since the epoch) at which "
      "the application was started.");
}
