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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#pragma once

#ifdef __cplusplus
#  include "wabi/base/arch/defines.h"

#  include "USD_scene.h"

#  include "KKE_api.h"
#  include "KKE_main.h"
#  include "KKE_robinhood.h"

#  include <wabi/base/arch/systemInfo.h>
#  include <wabi/base/tf/stringUtils.h>
#  include <wabi/base/tf/token.h>
#endif /* __cplusplus */

/** Aligned with #PropertyUnit and `kpyunits_ucategories_items` in `kpy_utils_units.cpp`. */
enum
{
  K_UNIT_NONE = 0,
  K_UNIT_LENGTH = 1,
  K_UNIT_AREA = 2,
  K_UNIT_VOLUME = 3,
  K_UNIT_MASS = 4,
  K_UNIT_ROTATION = 5,
  K_UNIT_TIME = 6,
  K_UNIT_TIME_ABSOLUTE = 7,
  K_UNIT_VELOCITY = 8,
  K_UNIT_ACCELERATION = 9,
  K_UNIT_CAMERA = 10,
  K_UNIT_POWER = 11,
  K_UNIT_TEMPERATURE = 12,
  K_UNIT_TYPE_TOT = 13,
};

// std::string kraken_exe_path_init(void);
// std::string kraken_system_tempdir_path(void);

// std::string kraken_datafiles_path_init(void);
// std::string kraken_fonts_path_init(void);
// std::string kraken_python_path_init(void);
// std::string kraken_icon_path_init(void);
// std::string kraken_startup_file_init(void);
// std::string kraken_ocio_file_init(void);

size_t KKE_unit_value_as_string(char *str,
                                int len_max,
                                double value,
                                int prec,
                                int type,
                                const UnitSettings *settings,
                                bool pad);
