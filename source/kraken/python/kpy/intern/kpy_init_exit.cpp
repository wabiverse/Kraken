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
 * KRAKEN Python.
 * It Bites.
 */

#include "KPY_init_exit.h" /* Own Include. */

#include "KKE_context.h"
#include "KKE_screen.h"
#include "KKE_utils.h"

#include "USD_object.h"

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/arch/vsnprintf.h>

#include <wabi/base/tf/error.h>
#include <wabi/base/tf/pathUtils.h>
#include <wabi/base/tf/pyInterpreter.h>
#include <wabi/base/tf/stringUtils.h>

/**
 *  -----  The Kraken Python Module. ----- */


#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace boost::python;

WABI_NAMESPACE_USING


/* ------ */


/**
 *  -----  Python Init & Exit. ----- */


void KPY_python_init(wabi::kContext *C)
{
  Main *kmain = CTX_data_main(C);

  setenv("PYTHONPATH", CHARSTR(kmain->python_path), true);

  TfPyInitialize();
}


void KPY_python_exit()
{
  /**
   * TODO: STUBBED */
}


/* ------ */
