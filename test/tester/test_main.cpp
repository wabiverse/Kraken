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
 *  AUTO TESTER 5000 DELUXE -- ALL TESTS STARTUP & LOGGER
 * ---------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{
#ifdef _WIN32
  std::string LOGFILES_PATH = TfStringCatPaths(TfGetPathName(ArchGetExecutablePath()),
                                               "../../../Kraken/source/kraken/test/logs/win32/");
#elif __linux__
  std::string LOGFILES_PATH = TfStringCatPaths(TfGetPathName(ArchGetExecutablePath()),
                                               "../../../Kraken/source/kraken/test/logs/linux/");
#endif

#ifdef WITH_PLUGS_TEST

#endif
}
