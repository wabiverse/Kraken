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

# This file contains a set of flags/settings shared between our
# GCC and Clang configs. This allows clangdefaults and gccdefaults
# to remain minimal, marking the points where divergence is required.
include(Options)

# Enable all warnings.
set(_WABI_GCC_CLANG_STATIC_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS} -Wall")

# Errors are warnings in strict build mode.
if (${WABI_STRICT_BUILD_MODE})
    set(_WABI_GCC_CLANG_STATIC_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS} -Werror")
endif()

# We use hash_map, suppress deprecation warning.
# _disable_warning("deprecated")
# _disable_warning("deprecated-declarations")

# Suppress unused typedef warnings emanating from boost.
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
    NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.6)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR
        NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1)
            _disable_warning("unused-local-typedefs")
    endif()
endif()

# If using pthreads then tell the compiler.  This should automatically cause
# the linker to pull in the pthread library if necessary so we also clear
# WABI_THREAD_LIBS.
if(CMAKE_USE_PTHREADS_INIT)
    set(_WABI_GCC_CLANG_STATIC_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS} -pthread")
    set(WABI_THREAD_LIBS "")
endif()

# Enable CXX 20 features.
set(_WABI_GCC_CLANG_STATIC_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS} -std=c++20")

# Enable position independent code.
set(_WABI_GCC_CLANG_STATIC_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS} -fPIC")

# Enable debugging symbols.
set(_WABI_GCC_CLANG_STATIC_CXX_FLAGS "${_WABI_GCC_CLANG_STATIC_CXX_FLAGS} -g")

_add_define("UNICODE")
_add_define("_UNICODE")

# -----------------------------------------------------------------------------
# Extra Compile Flags

if(CMAKE_COMPILER_IS_GNUCC)

  add_check_c_compiler_flag(C_WARNINGS C_WARN_ALL -Wall)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ERROR_IMPLICIT_FUNCTION_DECLARATION -Werror=implicit-function-declaration)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ERROR_RETURN_TYPE  -Werror=return-type)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ERROR_VLA -Werror=vla)
  # system headers sometimes do this, disable for now, was: -Werror=strict-prototypes
  add_check_c_compiler_flag(C_WARNINGS C_WARN_STRICT_PROTOTYPES  -Wstrict-prototypes)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_MISSING_PROTOTYPES -Wmissing-prototypes)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_CHAR_SUBSCRIPTS -Wno-char-subscripts)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_POINTER_ARITH -Wpointer-arith)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_UNUSED_PARAMETER -Wunused-parameter)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_WRITE_STRINGS -Wwrite-strings)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_LOGICAL_OP -Wlogical-op)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_UNDEF -Wundef)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_INIT_SELF -Winit-self)  # needs -Wuninitialized
  add_check_c_compiler_flag(C_WARNINGS C_WARN_MISSING_INCLUDE_DIRS -Wmissing-include-dirs)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_DIV_BY_ZERO -Wno-div-by-zero)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_TYPE_LIMITS -Wtype-limits)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_FORMAT_SIGN -Wformat-signedness)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_RESTRICT -Wrestrict)

  # C-only.
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_NULL -Wnonnull)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ABSOLUTE_VALUE -Wabsolute-value)

  add_check_c_compiler_flag(C_WARNINGS C_WARN_UNINITIALIZED -Wuninitialized)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_UNINITIALIZED -Wuninitialized)

  add_check_c_compiler_flag(C_WARNINGS C_WARN_REDUNDANT_DECLS       -Wredundant-decls)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_REDUNDANT_DECLS -Wredundant-decls)

  add_check_c_compiler_flag(C_WARNINGS C_WARN_SHADOW -Wshadow)

  # disable because it gives warnings for printf() & friends.
  # add_check_c_compiler_flag(C_WARNINGS C_WARN_DOUBLE_PROMOTION -Wdouble-promotion -Wno-error=double-promotion)

  if(NOT APPLE)
    add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_ERROR_UNUSED_BUT_SET_VARIABLE -Wno-error=unused-but-set-variable)
  endif()

  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_ALL -Wall)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_INVALID_OFFSETOF -Wno-invalid-offsetof)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_SIGN_COMPARE -Wno-sign-compare)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_LOGICAL_OP -Wlogical-op)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_INIT_SELF -Winit-self)  # needs -Wuninitialized
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_MISSING_INCLUDE_DIRS -Wmissing-include-dirs)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_DIV_BY_ZERO -Wno-div-by-zero)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_TYPE_LIMITS -Wtype-limits)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_ERROR_RETURN_TYPE  -Werror=return-type)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_CHAR_SUBSCRIPTS -Wno-char-subscripts)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_POINTER_ARITH -Wpointer-arith)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_UNUSED_PARAMETER -Wunused-parameter)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_WRITE_STRINGS -Wwrite-strings)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_UNDEF -Wundef)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_FORMAT_SIGN -Wformat-signedness)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_RESTRICT -Wrestrict)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_SUGGEST_OVERRIDE  -Wno-suggest-override)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_UNINITIALIZED -Wuninitialized)

  # causes too many warnings
  if(NOT APPLE)
    add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_UNDEF -Wundef)
    add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_MISSING_DECLARATIONS -Wmissing-declarations)
  endif()

  # Use 'ATTR_FALLTHROUGH' macro to suppress.
  add_check_c_compiler_flag(C_WARNINGS C_WARN_IMPLICIT_FALLTHROUGH -Wimplicit-fallthrough=5)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_IMPLICIT_FALLTHROUGH -Wimplicit-fallthrough=5)

  # ---------------------
  # Suppress Strict Flags
  #
  # Exclude the following warnings from this list:
  # - `-Wno-address`:
  #   This can give useful hints that point to bugs/misleading logic.
  # - `-Wno-strict-prototypes`:
  #   No need to support older C-style prototypes.
  #
  # If code in `./extern/` needs to suppress these flags that can be done on a case-by-case basis.

  # flags to undo strict flags
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_DEPRECATED_DECLARATIONS -Wno-deprecated-declarations)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_PARAMETER        -Wno-unused-parameter)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_FUNCTION         -Wno-unused-function)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_TYPE_LIMITS             -Wno-type-limits)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_INT_IN_BOOL_CONTEXT     -Wno-int-in-bool-context)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_FORMAT                  -Wno-format)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_SWITCH                  -Wno-switch)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_VARIABLE         -Wno-unused-variable)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_VARIABLE         -Wno-uninitialized)

  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_CLASS_MEMACCESS     -Wno-class-memaccess)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_COMMENT             -Wno-comment)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_TYPEDEFS     -Wno-unused-local-typedefs)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_VARIABLE     -Wno-unused-variable)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_VARIABLE     -Wno-uninitialized)

  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_IMPLICIT_FALLTHROUGH    -Wno-implicit-fallthrough)

  if(NOT APPLE)
    add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_ERROR_UNUSED_BUT_SET_VARIABLE -Wno-error=unused-but-set-variable)
  endif()

elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")

  # strange, clang complains these are not supported, but then uses them.
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ALL -Wall)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ERROR_IMPLICIT_FUNCTION_DECLARATION -Werror=implicit-function-declaration)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_ERROR_RETURN_TYPE  -Werror=return-type)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_AUTOLOGICAL_COMPARE -Wno-tautological-compare)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_NO_CHAR_SUBSCRIPTS -Wno-char-subscripts)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_STRICT_PROTOTYPES  -Wstrict-prototypes)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_MISSING_PROTOTYPES -Wmissing-prototypes)
  add_check_c_compiler_flag(C_WARNINGS C_WARN_UNUSED_PARAMETER -Wunused-parameter)

  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_ALL -Wall)
  # Using C++20 features while having C++17 as the project language isn't allowed by MSVC.
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_CXX20_DESIGNATOR -Wc++20-designator)

  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_AUTOLOGICAL_COMPARE -Wno-tautological-compare)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_UNKNOWN_PRAGMAS     -Wno-unknown-pragmas)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_CHAR_SUBSCRIPTS     -Wno-char-subscripts)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_OVERLOADED_VIRTUAL  -Wno-overloaded-virtual)  # we get a lot of these, if its a problem a dev needs to look into it.
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_SIGN_COMPARE        -Wno-sign-compare)
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_INVALID_OFFSETOF    -Wno-invalid-offsetof)
  # Apple Clang (tested on version 12) doesn't support this flag while LLVM Clang 11 does.
  add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_NO_SUGGEST_OVERRIDE    -Wno-suggest-override)

  # gives too many unfixable warnings
  # add_check_c_compiler_flag(C_WARNINGS C_WARN_UNUSED_MACROS      -Wunused-macros)
  # add_check_cxx_compiler_flag(CXX_WARNINGS CXX_WARN_UNUSED_MACROS          -Wunused-macros)

  # ---------------------
  # Suppress Strict Flags

  # flags to undo strict flags
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_PARAMETER -Wno-unused-parameter)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_VARIABLE  -Wno-unused-variable)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_MACROS    -Wno-unused-macros)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISLEADING_INDENTATION    -Wno-misleading-indentation)

  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISSING_VARIABLE_DECLARATIONS -Wno-missing-variable-declarations)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_INCOMPAT_PTR_DISCARD_QUAL -Wno-incompatible-pointer-types-discards-qualifiers)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_FUNCTION -Wno-unused-function)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_INT_TO_VOID_POINTER_CAST -Wno-int-to-void-pointer-cast)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISSING_PROTOTYPES -Wno-missing-prototypes)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_DUPLICATE_ENUM -Wno-duplicate-enum)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNDEF -Wno-undef)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISSING_NORETURN -Wno-missing-noreturn)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_BUT_SET_VARIABLE -Wno-unused-but-set-variable)
  add_check_c_compiler_flag(C_REMOVE_STRICT_FLAGS C_WARN_NO_DEPRECATED_DECLARATIONS -Wno-deprecated-declarations)

  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_PARAMETER -Wno-unused-parameter)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_PRIVATE_FIELD -Wno-unused-private-field)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_CXX11_NARROWING -Wno-c++11-narrowing)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_NON_VIRTUAL_DTOR -Wno-non-virtual-dtor)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_MACROS -Wno-unused-macros)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_VARIABLE  -Wno-unused-variable)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_REORDER -Wno-reorder)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_COMMENT -Wno-comment)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_TYPEDEFS -Wno-unused-local-typedefs)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNDEFINED_VAR_TEMPLATE -Wno-undefined-var-template)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_INSTANTIATION_AFTER_SPECIALIZATION -Wno-instantiation-after-specialization)
  add_check_cxx_compiler_flag(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_MISLEADING_INDENTATION    -Wno-misleading-indentation)
endif()
