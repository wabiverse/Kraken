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
 * Creator.
 * Creating Chaos.
 */

#pragma once

#include <wabi/base/arch/defines.h>

#include "KLI_utildefines.h"

/**
 *  -----  Kraken's Main Startup. ----- */

void CREATOR_kraken_main(int argc = 0, const char **argv = NULL);

/**
 *  -----  Creator's Args. ----- */

void CREATOR_setup_args(int argc, const char **argv);

int CREATOR_parse_args(int argc, const char **argv);

/* ------ */
