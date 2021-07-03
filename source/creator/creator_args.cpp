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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Creator.
 * Creating Chaos.
 */

#ifdef _WIN32
#  include <Windows.h>
#endif

#include "CKE_context.h"
#include "CKE_main.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "UNI_api.h"
#include "UNI_pixar_utils.h"

#include "creator.h"

/* PIXAR */
#include <wabi/usd/usd/stage.h>

/* BOOST */
#include <boost/program_options.hpp>

#include <map>
#include <string>
#include <vector>

#include <wabi/base/tf/iterator.h>
#include <wabi/usd/usd/stage.h>

/* NAMESPACES */
namespace CREATOR_ARGS = boost::program_options;

static bool run_diagnostics = false;

static std::string load_stage = "";
static std::string convert_stage = "";

void CREATOR_setup_args(int argc, const char **argv)
{
  CREATOR_ARGS::options_description options("Options");
  /* clang-format off */
  options.add_options()
    ("open", CREATOR_ARGS::value<std::string>(&load_stage),
    "Launch Kraken given a (usda|usd|usdc|usdz) file")

    ("convert", CREATOR_ARGS::value<std::string>(&convert_stage),
    "Converts a file given a path to a new file format extension (usda|usd|usdc|usdz)")

    ("factory-startup", CREATOR_ARGS::bool_switch(&wabi::G.factory_startup),
    "Resets factory default settings and preferences on startup")

    ("server", CREATOR_ARGS::bool_switch(&wabi::G.server),
    "Puts KRAKEN in a headless client-serving server mode")

    ("diagnostics", CREATOR_ARGS::bool_switch(&run_diagnostics),
    "Run system diagnostics to debug the Kraken System")

    ("help", "Shows this help message")
  ;
  /* clang-format on */

  CREATOR_ARGS::variables_map arg_vars;
  try
  {
    CREATOR_ARGS::store(CREATOR_ARGS::parse_command_line(argc, (const char **)argv, options), arg_vars);
    CREATOR_ARGS::notify(arg_vars);

    if (arg_vars.count("help"))
    {
      std::cout << options << "\n";
    }
  }
  catch (const CREATOR_ARGS::error &e)
  {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "%s\n", wabi::CHARALL(options));
  }
}

int CREATOR_parse_args(int argc, const char **argv)
{
  if (run_diagnostics)
  {
    wabi::CKE_kraken_enable_debug_codes();
  }

  if (load_stage.length() > 2)
  {
    wabi::G.main->stage_id = load_stage;
  }

  if (convert_stage.size() > 2)
  {
    wabi::UNI_pixutil_convert(convert_stage);
    return 1;
  }

  return 0;
}