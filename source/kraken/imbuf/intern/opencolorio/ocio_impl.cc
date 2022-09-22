/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include <cassert>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string.h>

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4251 4275)
#endif
#include <OpenColorIO/OpenColorIO.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace OCIO_NAMESPACE;

#include "ocio_impl.h"

#if !defined(WITH_ASSERT_ABORT)
#  define OCIO_abort()
#else
#  include <stdlib.h>
#  define OCIO_abort() abort()
#endif

#if defined(_MSC_VER)
#  define __func__ __FUNCTION__
#endif

static void OCIO_reportError(const char *err)
{
  std::cerr << "OpenColorIO Error: " << err << std::endl;

  OCIO_abort();
}

static void OCIO_reportException(Exception &exception)
{
  OCIO_reportError(exception.what());
}

OCIO_ConstConfigRcPtr *OCIOImpl::getCurrentConfig(void)
{
  ConstConfigRcPtr *config = new ConstConfigRcPtr();

  try {
    *config = GetCurrentConfig();

    if (*config)
      return (OCIO_ConstConfigRcPtr *)config;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  delete config;

  return NULL;
}

void OCIOImpl::setCurrentConfig(const OCIO_ConstConfigRcPtr *config)
{
  try {
    SetCurrentConfig(*(ConstConfigRcPtr *)config);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }
}

void OCIOImpl::configRelease(OCIO_ConstConfigRcPtr *config)
{
  delete config;
}

const char *OCIOImpl::configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getDefaultDisplay();
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}