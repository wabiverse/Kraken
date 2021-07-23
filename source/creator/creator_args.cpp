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

#include "KKE_context.h"
#include "KKE_main.h"

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
#include <filesystem>

#include <wabi/base/tf/diagnostic.h>
#include <wabi/base/tf/iterator.h>
#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_USING

/* NAMESPACES */
namespace CREATOR_ARGS = boost::program_options;

static bool run_diagnostics = false;

static std::string load_stage = "";
static std::string convert_stage = "";
static std::string resolve_asset = "";

void CREATOR_setup_args(int argc, const char **argv)
{
  CREATOR_ARGS::options_description options("Options");
  /* clang-format off */
  options.add_options()
    ("open", CREATOR_ARGS::value<std::string>(&load_stage),
    "Launch Kraken given a (usda|usd|usdc|usdz) file")

    ("convert", CREATOR_ARGS::value<std::string>(&convert_stage),
    "Converts a binary usd file to a human readable (.usda) file format")

    ("resolve", CREATOR_ARGS::value<std::string>(&resolve_asset),
    "Print resolved path of a given asset path using Pixar Asset Resolver")

    ("factory-startup", CREATOR_ARGS::bool_switch(&G.factory_startup),
    "Resets factory default settings and preferences on startup")

    ("server", CREATOR_ARGS::bool_switch(&G.background),
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
      /* Just show help, exit. */
      exit(KRAKEN_SUCCESS);
    }
  }
  catch (const CREATOR_ARGS::error &e)
  {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "%s\n", CHARALL(options));
    /* Let user fixup incorrect Arg, exit. */
    exit(KRAKEN_ERROR);
  }
}

int CREATOR_parse_args(int argc, const char **argv)
{
  if (run_diagnostics)
  {
    KKE_kraken_enable_debug_codes();
  }

  if (load_stage.length() > 2)
  {
    G.main->stage_id = load_stage;
  }

  if (convert_stage.size() > 2)
  {
    const std::filesystem::path fp = convert_stage;
    if (std::filesystem::exists(fp))
    {
      UNI_pixutil_convert_usd(fp, UsdUsdaFileFormatTokens->Id, /*verbose==*/true);
      exit(KRAKEN_SUCCESS);
    }

    TF_MSG_ERROR("File at %s does not exist.", CHARALL(fp.string()));
    exit(KRAKEN_ERROR);
  }

  if (resolve_asset.size() > 2)
  {
    !UNI_pixutil_resolve_asset(resolve_asset, /*verbose==*/true).empty() ? exit(KRAKEN_SUCCESS) :
                                                                           exit(KRAKEN_ERROR);
  }

  return 0;
}