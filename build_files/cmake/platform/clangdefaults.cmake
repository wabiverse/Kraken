# 
#  Copyright 2021 Pixar. All Rights Reserved.
# 
#  Portions of this file are derived from original work by Pixar
#  distributed with Universal Scene Description, a project of the
#  Academy Software Foundation (ASWF). https://www.aswf.io/
# 
#  Licensed under the Apache License, Version 2.0 (the "Apache License")
#  with the following modification; you may not use this file except in
#  compliance with the Apache License and the following modification:
#  Section 6. Trademarks. is deleted and replaced with:
# 
#  6. Trademarks. This License does not grant permission to use the trade
#     names, trademarks, service marks, or product names of the Licensor
#     and its affiliates, except as required to comply with Section 4(c)
#     of the License and to reproduce the content of the NOTICE file.
#
#  You may obtain a copy of the Apache License at:
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the Apache License with the above modification is
#  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
#  ANY KIND, either express or implied. See the Apache License for the
#  specific language governing permissions and limitations under the
#  Apache License.
#
#  Modifications copyright (C) 2020-2021 Wabi.
#

include(gccclangshareddefaults)

set(_WABI_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS}")

# clang annoyingly warns about the -pthread option if it's only linking.
if(CMAKE_USE_PTHREADS_INIT)
    _disable_warning("unused-command-line-argument")
endif()

# Index while building to provide linting support of cxx as well as swift.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} -index-store-path ${CMAKE_BINARY_DIR}/symbol_index")

# Important that we don't get autolinking with macOS frameworks.
set(_WABI_CXX_FLAGS "${_WABI_CXX_FLAGS} -fno-autolink")

set(CMAKE_FIND_FRAMEWORK NEVER)
set(CMAKE_FIND_APPBUNDLE NEVER)

set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)