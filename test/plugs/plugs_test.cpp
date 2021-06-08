#ifdef _WIN32
#  include <Windows.h>
#endif

#include <string>

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/plug/registry.h>
#include <wabi/base/tf/pathUtils.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/usd/ndr/registry.h>
#include <wabi/wabi.h>

WABI_NAMESPACE_USING

using namespace std;

/*
 * ---------------------------------------------------------------------
 *  MAIN -- STARTUP
 * ---------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{

  /*
   * ---------------------------------------------------------------------
   *  HIDE CONSOLE WINDOW ON STARTUP
   * ---------------------------------------------------------------------
   */

#ifdef _WIN32
  HWND hwnd = GetConsoleWindow();
  ShowWindow(hwnd, 0);
#endif

  /*
   * ---------------------------------------------------------------------
   *  TEST MAIN PLUGIN REGISTRATION
   * ---------------------------------------------------------------------
   */

  printf("\nPLUGS:::TEST REGISTRATION\n");

  std::string DATAFILES_PATH = TfStringCatPaths(TfGetPathName(ArchGetExecutablePath()),
                                                "/1.0/datafiles/");

  /* REGISTER CORE USD PLUGINS */
  const std::string core_plugin_path = TfStringCatPaths(DATAFILES_PATH, "usd/");
  printf("REGISTERING PLUGINS AT: %s\n", core_plugin_path.c_str());
  PlugRegistry::GetInstance().RegisterPlugins(core_plugin_path);

  /* REGISTER THIRD PARTY USD PLUGINS */
  const std::string thirdparty_plugin_path = TfStringCatPaths(DATAFILES_PATH, "plugin/");
  printf("REGISTERING PLUGINS AT: %s\n", thirdparty_plugin_path.c_str());
  PlugRegistry::GetInstance().RegisterPlugins(thirdparty_plugin_path);

  /*
   * ---------------------------------------------------------------------
   *  TEST MAIN NODE REGISTRY
   * ---------------------------------------------------------------------
   */

  printf("\nPLUGS:::TEST NODE REGISTRY\n");

  printf("REGISTERED SHADERS:\n");
  NdrStringVec shaders = NdrRegistry::GetInstance().GetNodeNames();
  for (auto shader = shaders.cbegin(); shader != shaders.cend(); ++shader) {
    printf("* %s\n", shader->c_str());
  }

  /*
   * ---------------------------------------------------------------------
   *  TEST COMPLETE
   * ---------------------------------------------------------------------
   */

  printf("\nPLUGS:::TEST COMPLETE\n");
}
