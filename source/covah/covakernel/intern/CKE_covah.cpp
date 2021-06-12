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
 * COVAH Kernel.
 * Purple Underground.
 */

/* STDLIB */
#include <fstream>
#include <string>

#ifdef __GNUC__
#  include <cstdlib>
#endif /*__GNUC__ */

/* KERNEL */
#include "CKE_main.h"
#include "CKE_version.h"

/* ANCHOR */
#include "ANCHOR_api.h"
#include "ANCHOR_debug_codes.h"
#include "ANCHOR_vulkan.h"

/* WINDOW MANAGER */
#include "WM_init_exit.h"

/* UNIVERSE */
#include "UNI_context.h"
#include "UNI_diagnostics.h"

/* PIXAR */
#include <wabi/base/arch/hints.h>
#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/debug.h>
#include <wabi/base/tf/error.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/imaging/hd/debugCodes.h>
#include <wabi/usd/sdf/debugCodes.h>
#include <wabi/usd/usd/debugCodes.h>
#include <wabi/usd/usd/stage.h>
#include <wabi/wabi.h>

/* BOOST */
#include <boost/program_options.hpp>

/* NAMESPACES */
namespace CKE_ARGS = boost::program_options;

WABI_NAMESPACE_USING

Global G;

static std::string covah_version_string;

static void covah_version_init()
{
  covah_version_string = TfStringPrintf("%d.%02d.%d %s",
                                        COVAH_VERSION / 100,
                                        COVAH_VERSION % 100,
                                        COVAH_VERSION_PATCH,
                                        STRINGIFY(COVAH_VERSION_CYCLE));
  printf("Covah v%s\n\n", covah_version_string.c_str());
}

static std::string covah_get_version_decimal()
{
  return TfStringPrintf("%d.%02d", COVAH_VERSION / 100, COVAH_VERSION % 100);
}

static std::string covah_exe_path_init()
{
  return TfGetPathName(ArchGetExecutablePath());
}

static std::string covah_datafiles_path_init()
{
#ifdef _WIN32
  return TfStringCatPaths(G.main->exe_path, covah_get_version_decimal() + "/datafiles/");
#else
  /**
   * On Linux, datafiles directory lies outside of BIN
   * ex. BIN DATAFILES INCLUDE LIB PYTHON */
  return TfStringCatPaths(G.main->exe_path, "../datafiles/");
#endif
}

static std::string covah_icon_path_init()
{
  /* Ends with '/' to indicate a directory. */
  return TfStringCatPaths(G.main->datafiles_path, "icons/");
}

static std::string covah_styles_path_init()
{
  return TfStringCatPaths(G.main->datafiles_path, "styles/");
}

static std::string covah_startup_file_init()
{
  return TfStringCatPaths(G.main->exe_path, "startup.usda");
}

static std::string covah_system_tempdir_path()
{
  return std::filesystem::temp_directory_path().string();
}

Main *CKE_main_new(void)
{
  Main *wmain = new Main();
  return wmain;
}

void CKE_covah_globals_init()
{
  covah_version_init();

  memset(&G, 0, sizeof(Global));

  G.main = CKE_main_new();

  G.main->exe_path              = covah_exe_path_init();
  G.main->temp_dir              = covah_system_tempdir_path();
  G.main->datafiles_path        = covah_datafiles_path_init();
  G.main->icons_path            = covah_icon_path_init();
  G.main->styles_path           = covah_styles_path_init();
  G.main->stage_id              = covah_startup_file_init();
  G.main->covah_version_decimal = covah_get_version_decimal();
}

static bool run_diagnostics = false;

static std::string load_stage    = "";
static std::string convert_stage = "";
static std::string convert_to    = "";

static void covah_setup_args(int argc, char *argv[])
{
  CKE_ARGS::options_description options("Options");
  /* clang-format off */
  options.add_options()
    ("open", CKE_ARGS::value<std::string>(&load_stage),
    "Launch COVAH given a '.usd' '.usda' '.usdc' or '.usdz' file")

    ("convert", CKE_ARGS::value<std::string>(&convert_stage),
    "Converts a file given a path")

    ("to", CKE_ARGS::value<std::string>(&convert_to),
    "Converts file passed to '--convert' with extension: 'usda' 'usd' 'usdc' 'usdz'")

    ("factory-startup", CKE_ARGS::bool_switch(&G.factory_startup),
    "Resets factory default settings and preferences on startup")

    ("server", CKE_ARGS::bool_switch(&G.server),
    "Puts COVAH in a headless client-serving server mode")

    ("diagnostics", CKE_ARGS::bool_switch(&run_diagnostics),
    "Run diagnostic checks with Pixar API")

    ("help", "Shows this help message")
  ;
  /* clang-format on */

  CKE_ARGS::variables_map arg_vars;
  try {
    CKE_ARGS::store(CKE_ARGS::parse_command_line(argc, argv, options), arg_vars);
    CKE_ARGS::notify(arg_vars);

    if (arg_vars.count("help")) {
      std::cout << options << "\n";
      exit(COVAH_SUCCESS);
    }
  }
  catch (const CKE_ARGS::error &e) {
    fprintf(stderr, "%s\n", e.what());
    fprintf(stderr, "%s\n", TfStringify(options).c_str());
    exit(COVAH_ERROR);
  }
}

static void CKE_CONVERT_STAGE_TO(std::string path, std::string extension)
{
  UsdStageRefPtr stage = UsdStage::Open(path);

  size_t pos = path.find_last_of(".");

  printf("%s\n", std::string(path.substr(0, pos) + extension).c_str());
  stage->Export(path.substr(0, pos) + "." + extension);
}

static ckeStatusCode covah_parse_args(int argc, char *argv[])
{
  if (load_stage.length() > 2) {
    G.main->stage_id = load_stage;
  }
  if (G.server) {
    // ServerStart(argc, argv);
    exit(COVAH_SUCCESS);
  }
  if (convert_stage.length() > 2) {
    CKE_CONVERT_STAGE_TO(convert_stage, convert_to);
    exit(COVAH_SUCCESS);
  }

  return COVAH_SUCCESS;
}

void CKE_covah_main_init(int argc, char *argv[])
{
  /* Init plugins. */
  CKE_covah_plugins_init();

  /* Init & parse args. */
  covah_setup_args(argc, argv);
  covah_parse_args(argc, argv);

  /* Determine stage to load (from user or factory default). */
  if (!std::filesystem::exists(G.main->stage_id) ||
      G.main->stage_id.string().find("startup.usda") != std::string::npos) {
    G.factory_startup = true;
  }

  /* Init & embed python. */
  CKE_covah_python_init();

  /* Init covah system paths, accessible from UNI.system.paths.xxx . */
  UNI_create(G.main->exe_path,
             G.main->temp_dir,
             G.main->styles_path,
             G.main->icons_path,
             G.main->datafiles_path,
             G.main->stage_id,
             G.main->build_commit_timestamp,
             G.main->build_hash,
             G.main->covah_version_decimal);

  if (run_diagnostics) {
    UNI_enable_all_debug_codes();
  }

  if (G.factory_startup) { /* Create default Pixar stage. */
    UNI_create_stage(TfStringCatPaths(G.main->temp_dir, "startup.usda"));
    UNI_author_gui();
    UNI_author_default_scene();
    UNI_save_stage();
  }
  else { /* Open user's stage. */
    UNI_open_stage(G.main->stage_id.string());
  }

  CKE_main_runtime(ANCHOR_SDL | ANCHOR_VULKAN);
}

bool CKE_has_kill_signal(ckeStatusCode signal)
{
  static ckeStatusCode kill_signal = COVAH_RUN;

  if (ARCH_UNLIKELY(signal != COVAH_RUN)) {
    kill_signal = signal;
  }

  return ARCH_UNLIKELY(kill_signal != COVAH_RUN);
}

/**
 * Where it all begins.
 * - Vulkan (and other backends to come)
 *   run the state of the application,
 *   each frame passed through (roughly
 *   3800-4000 FPS) maintains the runtime
 *   and immediate graphics API for the
 *   life of the application. */

ckeStatusCode CKE_main_runtime(int backend)
{
  ANCHOR_Status status     = ANCHOR_RUN;
  wmWindowManager *manager = new wmWindowManager();
  /** RUNTIMES ::: */
  switch (backend) {
    case (ANCHOR_SDL | ANCHOR_VULKAN): {
      VkResult vk_err;

      auto instance = ANCHOR_init_vulkan(vk_err);
      while (ARCH_LIKELY(status != ANCHOR_SUCCESS)) {
        status = ANCHOR_run_vulkan(instance.first, instance.second);
        WM_covah_runtime(manager);
        ANCHOR_render_vulkan(instance.second);
        if (ARCH_UNLIKELY(CKE_has_kill_signal())) {
          break;
        }
      }
      ANCHOR_clean_vulkan(instance.first, vk_err);
      break;
    }

    default:
      TF_CODING_ERROR("Specified a backend which is not implemented.");
      return COVAH_ERROR;
  }

  return COVAH_SUCCESS;
}

void CKE_main_free()
{
  delete G.main;
}
