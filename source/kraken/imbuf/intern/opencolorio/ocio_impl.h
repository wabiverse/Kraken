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

#ifndef __OCIO_IMPL_H__
#define __OCIO_IMPL_H__

#include "ocio_capi.h"

class IOCIOImpl
{
 public:

  virtual ~IOCIOImpl() {}

  virtual OCIO_ConstConfigRcPtr *getCurrentConfig(void) = 0;
  virtual void setCurrentConfig(const OCIO_ConstConfigRcPtr *config) = 0;

  virtual void configRelease(OCIO_ConstConfigRcPtr *config) = 0;

  virtual const char *configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config) = 0;
};

class OCIOImpl : public IOCIOImpl
{
 public:

  OCIOImpl(){};

  OCIO_ConstConfigRcPtr *getCurrentConfig(void);
  void setCurrentConfig(const OCIO_ConstConfigRcPtr *config);

  void configRelease(OCIO_ConstConfigRcPtr *config);

  const char *configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config);
};

#endif /* __OCIO_IMPL_H__ */