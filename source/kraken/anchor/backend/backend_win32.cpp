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
 * Anchor.
 * Bare Metal.
 */


/**
 * Workaround min max hell
 * Keeping NOMINMAX defined, 
 * using the same Windows Min 
 * Max, but prefixed with win
 * as to not conflict with std:: */
#define winmax(a, b) (((a) > (b)) ? (a) : (b))
#define winmin(a, b) (((a) < (b)) ? (a) : (b))

#define VK_USE_PLATFORM_WIN32_KHR

#include "ANCHOR_BACKEND_vulkan.h"
#include "ANCHOR_BACKEND_win32.h"
#include "ANCHOR_Rect.h"
#include "ANCHOR_api.h"
#include "ANCHOR_buttons.h"
#include "ANCHOR_debug_codes.h"
#include "ANCHOR_event.h"
#include "ANCHOR_window.h"

#include <vulkan/vulkan.h>

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/diagnostic.h>
#include <wabi/imaging/hgiVulkan/diagnostic.h>
#include <wabi/imaging/hgiVulkan/hgi.h>
#include <wabi/imaging/hgiVulkan/instance.h>

#ifndef _WIN32_IE
#  define _WIN32_IE 0x0501 /* shipped before XP, so doesn't impose additional requirements */
#endif

#include <commctrl.h>
#include <psapi.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <windowsx.h>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <dwmapi.h>
#include <tchar.h>
#include <windows.h>
#include <winnt.h>

// Configuration flags to add in your imconfig.h file:
//#define ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD              // Disable gamepad support. This was meaningful before <1.81 but we now load XInput dynamically so the option is now less relevant.

// Using XInput for gamepad (will load DLL dynamically)
#ifndef ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD
#  include <xinput.h>
typedef DWORD(WINAPI *PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES *);
typedef DWORD(WINAPI *PFN_XInputGetState)(DWORD, XINPUT_STATE *);
#endif

WABI_NAMESPACE_USING

/* clang-format off */

/**
 * PIXAR HYDRA GRAPHICS INTERFACE. */
static HgiVulkan         *g_PixarHydra      = nullptr;
static HgiVulkanInstance *g_PixarVkInstance = nullptr;

/**
 * HYDRA RENDERING PARAMS. */
static UsdApolloRenderParams g_HDPARAMS_Apollo;

/**
 * ANCHOR VULKAN GLOBALS. */
static VkAllocationCallbacks   *g_Allocator      = NULL;
static VkInstance               g_Instance       = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device         = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily    = (uint32_t)-1;
static VkQueue                  g_Queue          = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport    = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache  = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ANCHOR_VulkanGPU_Surface  g_MainWindowData;
static uint32_t                  g_MinImageCount    = 2;
static bool                      g_SwapChainRebuild = false;

/* clang-format on */

struct ANCHOR_ImplWin32_Data
{
  HWND hWnd;
  INT64 Time;
  INT64 TicksPerSecond;
  ANCHOR_MouseCursor LastMouseCursor;
  bool HasGamepad;
  bool WantUpdateHasGamepad;

#ifndef ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD
  HMODULE XInputDLL;
  PFN_XInputGetCapabilities XInputGetCapabilities;
  PFN_XInputGetState XInputGetState;
#endif

  ANCHOR_ImplWin32_Data()
  {
    memset(this, 0, sizeof(*this));
  }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple ANCHOR contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single ANCHOR context + multiple windows) instead of multiple ANCHOR contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static ANCHOR_ImplWin32_Data *ANCHOR_ImplWin32_GetBackendData()
{
  return ANCHOR::GetCurrentContext() ? (ANCHOR_ImplWin32_Data *)ANCHOR::GetIO().BackendPlatformUserData : NULL;
}

// Functions
bool ANCHOR_ImplWin32_Init(void *hwnd)
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

  INT64 perf_frequency, perf_counter;
  if (!::QueryPerformanceFrequency((LARGE_INTEGER *)&perf_frequency))
    return false;
  if (!::QueryPerformanceCounter((LARGE_INTEGER *)&perf_counter))
    return false;

  // Setup backend capabilities flags
  ANCHOR_ImplWin32_Data *bd = IM_NEW(ANCHOR_ImplWin32_Data)();
  io.BackendPlatformUserData = (void *)bd;
  io.BackendPlatformName = "imgui_impl_win32";
  io.BackendFlags |= ANCHORBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor() values (optional)
  io.BackendFlags |= ANCHORBackendFlags_HasSetMousePos;   // We can honor io.WantSetMousePos requests (optional, rarely used)

  bd->hWnd = (HWND)hwnd;
  bd->WantUpdateHasGamepad = true;
  bd->TicksPerSecond = perf_frequency;
  bd->Time = perf_counter;
  bd->LastMouseCursor = ANCHOR_MouseCursor_COUNT;

  io.ImeWindowHandle = hwnd;

  // Keyboard mapping. ANCHOR will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
  io.KeyMap[ANCHOR_Key_Tab] = VK_TAB;
  io.KeyMap[ANCHOR_Key_LeftArrow] = VK_LEFT;
  io.KeyMap[ANCHOR_Key_RightArrow] = VK_RIGHT;
  io.KeyMap[ANCHOR_Key_UpArrow] = VK_UP;
  io.KeyMap[ANCHOR_Key_DownArrow] = VK_DOWN;
  io.KeyMap[ANCHOR_Key_PageUp] = VK_PRIOR;
  io.KeyMap[ANCHOR_Key_PageDown] = VK_NEXT;
  io.KeyMap[ANCHOR_Key_Home] = VK_HOME;
  io.KeyMap[ANCHOR_Key_End] = VK_END;
  io.KeyMap[ANCHOR_Key_Insert] = VK_INSERT;
  io.KeyMap[ANCHOR_Key_Delete] = VK_DELETE;
  io.KeyMap[ANCHOR_Key_Backspace] = VK_BACK;
  io.KeyMap[ANCHOR_Key_Space] = VK_SPACE;
  io.KeyMap[ANCHOR_Key_Enter] = VK_RETURN;
  io.KeyMap[ANCHOR_Key_Escape] = VK_ESCAPE;
  io.KeyMap[ANCHOR_Key_KeyPadEnter] = VK_RETURN;
  io.KeyMap[ANCHOR_Key_A] = 'A';
  io.KeyMap[ANCHOR_Key_C] = 'C';
  io.KeyMap[ANCHOR_Key_V] = 'V';
  io.KeyMap[ANCHOR_Key_X] = 'X';
  io.KeyMap[ANCHOR_Key_Y] = 'Y';
  io.KeyMap[ANCHOR_Key_Z] = 'Z';

  // Dynamically load XInput library
#ifndef ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD
  const char *xinput_dll_names[] =
    {
      "xinput1_4.dll",    // Windows 8+
      "xinput1_3.dll",    // DirectX SDK
      "xinput9_1_0.dll",  // Windows Vista, Windows 7
      "xinput1_2.dll",    // DirectX SDK
      "xinput1_1.dll"     // DirectX SDK
    };
  for (int n = 0; n < ANCHOR_ARRAYSIZE(xinput_dll_names); n++)
    if (HMODULE dll = ::LoadLibraryA(xinput_dll_names[n]))
    {
      bd->XInputDLL = dll;
      bd->XInputGetCapabilities = (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");
      bd->XInputGetState = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
      break;
    }
#endif  // ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD

  return true;
}

void ANCHOR_ImplWin32_Shutdown()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ImplWin32_Data *bd = ANCHOR_ImplWin32_GetBackendData();

  // Unload XInput library
#ifndef ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD
  if (bd->XInputDLL)
    ::FreeLibrary(bd->XInputDLL);
#endif  // ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD

  io.BackendPlatformName = NULL;
  io.BackendPlatformUserData = NULL;
  IM_DELETE(bd);
}

static bool ANCHOR_ImplWin32_UpdateMouseCursor()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  if (io.ConfigFlags & ANCHORConfigFlags_NoMouseCursorChange)
    return false;

  ANCHOR_MouseCursor imgui_cursor = ANCHOR::GetMouseCursor();
  if (imgui_cursor == ANCHOR_MouseCursor_None || io.MouseDrawCursor)
  {
    // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
    ::SetCursor(NULL);
  }
  else
  {
    // Show OS mouse cursor
    LPTSTR win32_cursor = IDC_ARROW;
    switch (imgui_cursor)
    {
      case ANCHOR_MouseCursor_Arrow:
        win32_cursor = IDC_ARROW;
        break;
      case ANCHOR_MouseCursor_TextInput:
        win32_cursor = IDC_IBEAM;
        break;
      case ANCHOR_MouseCursor_ResizeAll:
        win32_cursor = IDC_SIZEALL;
        break;
      case ANCHOR_MouseCursor_ResizeEW:
        win32_cursor = IDC_SIZEWE;
        break;
      case ANCHOR_MouseCursor_ResizeNS:
        win32_cursor = IDC_SIZENS;
        break;
      case ANCHOR_MouseCursor_ResizeNESW:
        win32_cursor = IDC_SIZENESW;
        break;
      case ANCHOR_MouseCursor_ResizeNWSE:
        win32_cursor = IDC_SIZENWSE;
        break;
      case ANCHOR_MouseCursor_Hand:
        win32_cursor = IDC_HAND;
        break;
      case ANCHOR_MouseCursor_NotAllowed:
        win32_cursor = IDC_NO;
        break;
    }
    ::SetCursor(::LoadCursor(NULL, win32_cursor));
  }
  return true;
}

static void ANCHOR_ImplWin32_UpdateMousePos()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ImplWin32_Data *bd = ANCHOR_ImplWin32_GetBackendData();
  ANCHOR_ASSERT(bd->hWnd != 0);

  // Set OS mouse position if requested (rarely used, only when ANCHORConfigFlags_NavEnableSetMousePos is enabled by user)
  if (io.WantSetMousePos)
  {
    POINT pos = {(int)io.MousePos[0], (int)io.MousePos[1]};
    if (::ClientToScreen(bd->hWnd, &pos))
      ::SetCursorPos(pos.x, pos.y);
  }

  // Set mouse position
  io.MousePos = wabi::GfVec2f(-FLT_MAX, -FLT_MAX);
  POINT pos;
  if (HWND active_window = ::GetForegroundWindow())
    if (active_window == bd->hWnd || ::IsChild(active_window, bd->hWnd))
      if (::GetCursorPos(&pos) && ::ScreenToClient(bd->hWnd, &pos))
        io.MousePos = wabi::GfVec2f((float)pos.x, (float)pos.y);
}

// Gamepad navigation mapping
static void ANCHOR_ImplWin32_UpdateGamepads()
{
#ifndef ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD
  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ImplWin32_Data *bd = ANCHOR_ImplWin32_GetBackendData();
  memset(io.NavInputs, 0, sizeof(io.NavInputs));
  if ((io.ConfigFlags & ANCHORConfigFlags_NavEnableGamepad) == 0)
    return;

  // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
  // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after receiving WM_DEVICECHANGE.
  if (bd->WantUpdateHasGamepad)
  {
    XINPUT_CAPABILITIES caps;
    bd->HasGamepad = bd->XInputGetCapabilities ? (bd->XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS) : false;
    bd->WantUpdateHasGamepad = false;
  }

  io.BackendFlags &= ~ANCHORBackendFlags_HasGamepad;
  XINPUT_STATE xinput_state;
  if (bd->HasGamepad && bd->XInputGetState && bd->XInputGetState(0, &xinput_state) == ERROR_SUCCESS)
  {
    const XINPUT_GAMEPAD &gamepad = xinput_state.Gamepad;
    io.BackendFlags |= ANCHORBackendFlags_HasGamepad;

#  define MAP_BUTTON(NAV_NO, BUTTON_ENUM) \
    { \
      io.NavInputs[NAV_NO] = (gamepad.wButtons & BUTTON_ENUM) ? 1.0f : 0.0f; \
    }
#  define MAP_ANALOG(NAV_NO, VALUE, V0, V1) \
    { \
      float vn = (float)(VALUE - V0) / (float)(V1 - V0); \
      if (vn > 1.0f) \
        vn = 1.0f; \
      if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) \
        io.NavInputs[NAV_NO] = vn; \
    }
    MAP_BUTTON(ANCHOR_NavInput_Activate, XINPUT_GAMEPAD_A);                // Cross / A
    MAP_BUTTON(ANCHOR_NavInput_Cancel, XINPUT_GAMEPAD_B);                  // Circle / B
    MAP_BUTTON(ANCHOR_NavInput_Menu, XINPUT_GAMEPAD_X);                    // Square / X
    MAP_BUTTON(ANCHOR_NavInput_Input, XINPUT_GAMEPAD_Y);                   // Triangle / Y
    MAP_BUTTON(ANCHOR_NavInput_DpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);        // D-Pad Left
    MAP_BUTTON(ANCHOR_NavInput_DpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);      // D-Pad Right
    MAP_BUTTON(ANCHOR_NavInput_DpadUp, XINPUT_GAMEPAD_DPAD_UP);            // D-Pad Up
    MAP_BUTTON(ANCHOR_NavInput_DpadDown, XINPUT_GAMEPAD_DPAD_DOWN);        // D-Pad Down
    MAP_BUTTON(ANCHOR_NavInput_FocusPrev, XINPUT_GAMEPAD_LEFT_SHOULDER);   // L1 / LB
    MAP_BUTTON(ANCHOR_NavInput_FocusNext, XINPUT_GAMEPAD_RIGHT_SHOULDER);  // R1 / RB
    MAP_BUTTON(ANCHOR_NavInput_TweakSlow, XINPUT_GAMEPAD_LEFT_SHOULDER);   // L1 / LB
    MAP_BUTTON(ANCHOR_NavInput_TweakFast, XINPUT_GAMEPAD_RIGHT_SHOULDER);  // R1 / RB
    MAP_ANALOG(ANCHOR_NavInput_LStickLeft, gamepad.sThumbLX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(ANCHOR_NavInput_LStickRight, gamepad.sThumbLX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(ANCHOR_NavInput_LStickUp, gamepad.sThumbLY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(ANCHOR_NavInput_LStickDown, gamepad.sThumbLY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32767);
#  undef MAP_BUTTON
#  undef MAP_ANALOG
  }
#endif  // #ifndef ANCHOR_IMPL_WIN32_DISABLE_GAMEPAD
}

void ANCHOR_ImplWin32_NewFrame()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ImplWin32_Data *bd = ANCHOR_ImplWin32_GetBackendData();
  ANCHOR_ASSERT(bd != NULL && "Did you call ANCHOR_ImplWin32_Init()?");

  // Setup display size (every frame to accommodate for window resizing)
  RECT rect = {0, 0, 0, 0};
  ::GetClientRect(bd->hWnd, &rect);
  io.DisplaySize = wabi::GfVec2f((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

  // Setup time step
  INT64 current_time = 0;
  ::QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
  io.DeltaTime = (float)(current_time - bd->Time) / bd->TicksPerSecond;
  bd->Time = current_time;

  // Read keyboard modifiers inputs
  io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
  io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
  io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
  io.KeySuper = false;
  // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

  // Update OS mouse position
  ANCHOR_ImplWin32_UpdateMousePos();

  // Update OS mouse cursor with the cursor requested by imgui
  ANCHOR_MouseCursor mouse_cursor = io.MouseDrawCursor ? ANCHOR_MouseCursor_None : ANCHOR::GetMouseCursor();
  if (bd->LastMouseCursor != mouse_cursor)
  {
    bd->LastMouseCursor = mouse_cursor;
    ANCHOR_ImplWin32_UpdateMouseCursor();
  }

  // Update game controllers (if enabled and available)
  ANCHOR_ImplWin32_UpdateGamepads();
}

// Allow compilation with old Windows SDK. MinGW doesn't have default _WIN32_WINNT/WINVER versions.
#ifndef WM_MOUSEHWHEEL
#  define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
#  define DBT_DEVNODES_CHANGED 0x0007
#endif

// Win32 message handler (process Win32 mouse/keyboard inputs, etc.)
// Call from your application's message handler.
// When implementing your own backend, you can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if ANCHOR wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to ANCHOR, and hide them from your application based on those two flags.
// PS: In this Win32 handler, we use the capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinates when dragging mouse outside of our window bounds.
// PS: We treat DBLCLK messages as regular mouse down messages, so this code will work on windows classes that have the CS_DBLCLKS flag set. Our own example app code doesn't set this flag.
#if 0
// Copy this line into your .cpp file to forward declare the function.
extern ANCHOR_BACKEND_API LRESULT ANCHOR_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
ANCHOR_BACKEND_API LRESULT ANCHOR_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ANCHOR::GetCurrentContext() == NULL)
    return 0;

  ANCHOR_IO &io = ANCHOR::GetIO();
  ANCHOR_ImplWin32_Data *bd = ANCHOR_ImplWin32_GetBackendData();

  switch (msg)
  {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK: {
      int button = 0;
      if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
      {
        button = 0;
      }
      if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK)
      {
        button = 1;
      }
      if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK)
      {
        button = 2;
      }
      if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK)
      {
        button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
      }
      if (!ANCHOR::IsAnyMouseDown() && ::GetCapture() == NULL)
        ::SetCapture(hwnd);
      io.MouseDown[button] = true;
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP: {
      int button = 0;
      if (msg == WM_LBUTTONUP)
      {
        button = 0;
      }
      if (msg == WM_RBUTTONUP)
      {
        button = 1;
      }
      if (msg == WM_MBUTTONUP)
      {
        button = 2;
      }
      if (msg == WM_XBUTTONUP)
      {
        button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
      }
      io.MouseDown[button] = false;
      if (!ANCHOR::IsAnyMouseDown() && ::GetCapture() == hwnd)
        ::ReleaseCapture();
      return 0;
    }
    case WM_MOUSEWHEEL:
      io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
      return 0;
    case WM_MOUSEHWHEEL:
      io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
      return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      if (wParam < 256)
        io.KeysDown[wParam] = 1;
      return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      if (wParam < 256)
        io.KeysDown[wParam] = 0;
      return 0;
    case WM_KILLFOCUS:
      memset(io.KeysDown, 0, sizeof(io.KeysDown));
      return 0;
    case WM_CHAR:
      // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
      if (wParam > 0 && wParam < 0x10000)
        io.AddInputCharacterUTF16((unsigned short)wParam);
      return 0;
    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT && ANCHOR_ImplWin32_UpdateMouseCursor())
        return 1;
      return 0;
    case WM_DEVICECHANGE:
      if ((UINT)wParam == DBT_DEVNODES_CHANGED)
        bd->WantUpdateHasGamepad = true;
      return 0;
  }
  return 0;
}


//--------------------------------------------------------------------------------------------------------
// DPI-related helpers (optional)
//--------------------------------------------------------------------------------------------------------
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
//---------------------------------------------------------------------------------------------------------
// This is the scheme successfully used by GLFW (from which we borrowed some of the code) and other apps aiming to be highly portable.
// ANCHOR_ImplWin32_EnableDpiAwareness() is just a helper called by main.cpp, we don't call it automatically.
// If you are trying to implement your own backend for your own engine, you may ignore that noise.
//---------------------------------------------------------------------------------------------------------

// Perform our own check with RtlVerifyVersionInfo() instead of using functions from <VersionHelpers.h> as they
// require a manifest to be functional for checks above 8.1. See https://github.com/ocornut/imgui/issues/4200
static BOOL _IsWindowsVersionOrGreater(WORD major, WORD minor, WORD)
{
  typedef LONG(WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW *, ULONG, ULONGLONG);
  static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfoFn = NULL;
  if (RtlVerifyVersionInfoFn == NULL)
    if (HMODULE ntdllModule = ::GetModuleHandleA("ntdll.dll"))
      RtlVerifyVersionInfoFn = (PFN_RtlVerifyVersionInfo)GetProcAddress(ntdllModule, "RtlVerifyVersionInfo");
  if (RtlVerifyVersionInfoFn == NULL)
    return FALSE;

  RTL_OSVERSIONINFOEXW versionInfo = {};
  ULONGLONG conditionMask = 0;
  versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
  versionInfo.dwMajorVersion = major;
  versionInfo.dwMinorVersion = minor;
  VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
  VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
  return (RtlVerifyVersionInfoFn(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) == 0) ? TRUE : FALSE;
}

#define _IsWindowsVistaOrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0600), LOBYTE(0x0600), 0)    // _WIN32_WINNT_VISTA
#define _IsWindows8OrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0602), LOBYTE(0x0602), 0)        // _WIN32_WINNT_WIN8
#define _IsWindows8Point1OrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0603), LOBYTE(0x0603), 0)  // _WIN32_WINNT_WINBLUE
#define _IsWindows10OrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0A00), LOBYTE(0x0A00), 0)       // _WIN32_WINNT_WINTHRESHOLD / _WIN32_WINNT_WIN10

#ifndef DPI_ENUMS_DECLARED
typedef enum
{
  PROCESS_DPI_UNAWARE = 0,
  PROCESS_SYSTEM_DPI_AWARE = 1,
  PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum
{
  MDT_EFFECTIVE_DPI = 0,
  MDT_ANGULAR_DPI = 1,
  MDT_RAW_DPI = 2,
  MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif
#ifndef _DPI_AWARENESS_CONTEXTS_
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#  define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE (DPI_AWARENESS_CONTEXT) - 3
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#  define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (DPI_AWARENESS_CONTEXT) - 4
#endif
typedef HRESULT(WINAPI *PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);                      // Shcore.lib + dll, Windows 8.1+
typedef HRESULT(WINAPI *PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT *, UINT *);       // Shcore.lib + dll, Windows 8.1+
typedef DPI_AWARENESS_CONTEXT(WINAPI *PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);  // User32.lib + dll, Windows 10 v1607+ (Creators Update)

// Helper function to enable DPI awareness without setting up a manifest
void ANCHOR_ImplWin32_EnableDpiAwareness()
{
  if (_IsWindows10OrGreater())
  {
    static HINSTANCE user32_dll = ::LoadLibraryA("user32.dll");  // Reference counted per-process
    if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext"))
    {
      SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
      return;
    }
  }
  if (_IsWindows8Point1OrGreater())
  {
    static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll");  // Reference counted per-process
    if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn = (PFN_SetProcessDpiAwareness)::GetProcAddress(shcore_dll, "SetProcessDpiAwareness"))
    {
      SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
      return;
    }
  }
#if _WIN32_WINNT >= 0x0600
  ::SetProcessDPIAware();
#endif
}

#if defined(_MSC_VER) && !defined(NOGDI)
#  pragma comment(lib, "gdi32")  // Link with gdi32.lib for GetDeviceCaps(). MinGW will require linking with '-lgdi32'
#endif

float ANCHOR_ImplWin32_GetDpiScaleForMonitor(void *monitor)
{
  UINT xdpi = 96, ydpi = 96;
  if (_IsWindows8Point1OrGreater())
  {
    static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll");  // Reference counted per-process
    static PFN_GetDpiForMonitor GetDpiForMonitorFn = NULL;
    if (GetDpiForMonitorFn == NULL && shcore_dll != NULL)
      GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor");
    if (GetDpiForMonitorFn != NULL)
    {
      GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
      ANCHOR_ASSERT(xdpi == ydpi);  // Please contact me if you hit this assert!
      return xdpi / 96.0f;
    }
  }
#ifndef NOGDI
  const HDC dc = ::GetDC(NULL);
  xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
  ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
  ANCHOR_ASSERT(xdpi == ydpi);  // Please contact me if you hit this assert!
  ::ReleaseDC(NULL, dc);
#endif
  return xdpi / 96.0f;
}

float ANCHOR_ImplWin32_GetDpiScaleForHwnd(void *hwnd)
{
  HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
  return ANCHOR_ImplWin32_GetDpiScaleForMonitor(monitor);
}

//---------------------------------------------------------------------------------------------------------
// Transparency related helpers (optional)
//--------------------------------------------------------------------------------------------------------

#if defined(_MSC_VER)
#  pragma comment(lib, "dwmapi")  // Link with dwmapi.lib. MinGW will require linking with '-ldwmapi'
#endif

// [experimental]
// Borrowed from GLFW's function updateFramebufferTransparency() in src/win32_window.c
// (the Dwm* functions are Vista era functions but we are borrowing logic from GLFW)
void ANCHOR_ImplWin32_EnableAlphaCompositing(void *hwnd)
{
  if (!_IsWindowsVistaOrGreater())
    return;

  BOOL composition;
  if (FAILED(::DwmIsCompositionEnabled(&composition)) || !composition)
    return;

  BOOL opaque;
  DWORD color;
  if (_IsWindows8OrGreater() || (SUCCEEDED(::DwmGetColorizationColor(&color, &opaque)) && !opaque))
  {
    HRGN region = ::CreateRectRgn(0, 0, -1, -1);
    DWM_BLURBEHIND bb = {};
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.hRgnBlur = region;
    bb.fEnable = TRUE;
    ::DwmEnableBlurBehindWindow((HWND)hwnd, &bb);
    ::DeleteObject(region);
  }
  else
  {
    DWM_BLURBEHIND bb = {};
    bb.dwFlags = DWM_BB_ENABLE;
    ::DwmEnableBlurBehindWindow((HWND)hwnd, &bb);
  }
}

// --------------------------------------------------------------------------------------------------------

ANCHOR_DisplayManagerWin32::ANCHOR_DisplayManagerWin32(void)
{}

eAnchorStatus ANCHOR_DisplayManagerWin32::getNumDisplays(AnchorU8 &numDisplays) const
{
  numDisplays = ::GetSystemMetrics(SM_CMONITORS);
  return numDisplays > 0 ? ANCHOR_SUCCESS : ANCHOR_ERROR;
}

static BOOL get_dd(DWORD d, DISPLAY_DEVICE *dd)
{
  dd->cb = sizeof(DISPLAY_DEVICE);
  return ::EnumDisplayDevices(NULL, d, dd, 0);
}

/*
 * When you call EnumDisplaySettings with iModeNum set to zero, the operating system
 * initializes and caches information about the display device. When you call
 * EnumDisplaySettings with iModeNum set to a non-zero value, the function returns
 * the information that was cached the last time the function was called with iModeNum
 * set to zero. */
eAnchorStatus ANCHOR_DisplayManagerWin32::getNumDisplaySettings(AnchorU8 display,
                                                                AnchorS32 &numSettings) const
{
  DISPLAY_DEVICE display_device;
  if (!get_dd(display, &display_device))
    return ANCHOR_ERROR;

  numSettings = 0;
  DEVMODE dm;
  while (::EnumDisplaySettings(display_device.DeviceName, numSettings, &dm))
  {
    numSettings++;
  }
  return ANCHOR_SUCCESS;
}

eAnchorStatus ANCHOR_DisplayManagerWin32::getDisplaySetting(AnchorU8 display,
                                                            AnchorS32 index,
                                                            ANCHOR_DisplaySetting &setting) const
{
  DISPLAY_DEVICE display_device;
  if (!get_dd(display, &display_device))
    return ANCHOR_ERROR;

  eAnchorStatus success;
  DEVMODE dm;
  if (::EnumDisplaySettings(display_device.DeviceName, index, &dm))
  {
    setting.xPixels = dm.dmPelsWidth;
    setting.yPixels = dm.dmPelsHeight;
    setting.bpp = dm.dmBitsPerPel;

    /**
     * When you call the EnumDisplaySettings function, the dmDisplayFrequency member
     * may return with the value 0 or 1. These values represent the display hardware's
     * default refresh rate. This default rate is typically set by switches on a display
     * card or computer motherboard, or by a configuration program that does not use
     * Win32 display functions such as ChangeDisplaySettings. */

    /**
     * First, we tried to explicitly set the frequency to 60 if EnumDisplaySettings
     * returned 0 or 1 but this doesn't work since later on an exact match will
     * be searched. And this will never happen if we change it to 60. Now we rely
     * on the default h/w setting. */
    setting.frequency = dm.dmDisplayFrequency;
    success = ANCHOR_SUCCESS;
  }
  else
  {
    success = ANCHOR_ERROR;
  }
  return success;
}

eAnchorStatus ANCHOR_DisplayManagerWin32::getCurrentDisplaySetting(AnchorU8 display,
                                                                   ANCHOR_DisplaySetting &setting) const
{
  return getDisplaySetting(display, ENUM_CURRENT_SETTINGS, setting);
}

eAnchorStatus ANCHOR_DisplayManagerWin32::setCurrentDisplaySetting(AnchorU8 display,
                                                                   const ANCHOR_DisplaySetting &setting)
{
  DISPLAY_DEVICE display_device;
  if (!get_dd(display, &display_device))
    return ANCHOR_ERROR;

  ANCHOR_DisplaySetting match;
  findMatch(display, setting, match);
  DEVMODE dm;
  int i = 0;
  while (::EnumDisplaySettings(display_device.DeviceName, i++, &dm))
  {
    if ((dm.dmBitsPerPel == match.bpp) && (dm.dmPelsWidth == match.xPixels) &&
        (dm.dmPelsHeight == match.yPixels) && (dm.dmDisplayFrequency == match.frequency))
    {
      break;
    }
  }

  /**
   * dm.dmBitsPerPel = match.bpp;
   * dm.dmPelsWidth = match.xPixels;
   * dm.dmPelsHeight = match.yPixels;
   * dm.dmDisplayFrequency = match.frequency;
   * dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
   * dm.dmSize = sizeof(DEVMODE);
   * dm.dmDriverExtra = 0; */

  LONG status = ::ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
  return status == DISP_CHANGE_SUCCESSFUL ? ANCHOR_SUCCESS : ANCHOR_ERROR;
}

//---------------------------------------------------------------------------------------------------------

/**
 * Key code values not found in winuser.h */
#ifndef VK_MINUS
#  define VK_MINUS 0xBD
#endif  // VK_MINUS
#ifndef VK_SEMICOLON
#  define VK_SEMICOLON 0xBA
#endif  // VK_SEMICOLON
#ifndef VK_PERIOD
#  define VK_PERIOD 0xBE
#endif  // VK_PERIOD
#ifndef VK_COMMA
#  define VK_COMMA 0xBC
#endif  // VK_COMMA
#ifndef VK_QUOTE
#  define VK_QUOTE 0xDE
#endif  // VK_QUOTE
#ifndef VK_BACK_QUOTE
#  define VK_BACK_QUOTE 0xC0
#endif  // VK_BACK_QUOTE
#ifndef VK_SLASH
#  define VK_SLASH 0xBF
#endif  // VK_SLASH
#ifndef VK_BACK_SLASH
#  define VK_BACK_SLASH 0xDC
#endif  // VK_BACK_SLASH
#ifndef VK_EQUALS
#  define VK_EQUALS 0xBB
#endif  // VK_EQUALS
#ifndef VK_OPEN_BRACKET
#  define VK_OPEN_BRACKET 0xDB
#endif  // VK_OPEN_BRACKET
#ifndef VK_CLOSE_BRACKET
#  define VK_CLOSE_BRACKET 0xDD
#endif  // VK_CLOSE_BRACKET
#ifndef VK_GR_LESS
#  define VK_GR_LESS 0xE2
#endif  // VK_GR_LESS

/**
 * Workaround for some laptop touchpads, some of which seems to
 * have driver issues which makes it so window function receives
 * the message, but PeekMessage doesn't pick those messages for
 * some reason.
 *
 * We send a dummy WM_USER message to force PeekMessage to receive
 * something, making it so kraken's window manager sees the new
 * messages coming in. */
#define BROKEN_PEEK_TOUCHPAD

static void initRawInput()
{
#define DEVICE_COUNT 1

  RAWINPUTDEVICE devices[DEVICE_COUNT];
  memset(devices, 0, DEVICE_COUNT * sizeof(RAWINPUTDEVICE));

  // Initiates WM_INPUT messages from keyboard
  // That way ANCHOR can retrieve true keys
  devices[0].usUsagePage = 0x01;
  devices[0].usUsage = 0x06; /* http://msdn.microsoft.com/en-us/windows/hardware/gg487473.aspx */

  RegisterRawInputDevices(devices, DEVICE_COUNT, sizeof(RAWINPUTDEVICE));

#undef DEVICE_COUNT
}

typedef BOOL(WINAPI *ANCHOR_WIN32_EnableNonClientDpiScaling)(HWND);

ANCHOR_SystemWin32::ANCHOR_SystemWin32()
  : m_win32_window(nullptr),
    m_hasPerformanceCounter(false),
    m_freq(0),
    m_start(0),
    m_lfstart(0)
{
  m_displayManager = new ANCHOR_DisplayManagerWin32();
  ANCHOR_ASSERT(m_displayManager);
  m_displayManager->initialize();

  m_consoleStatus = 1;

  /**
   * Tell Windows we are per monitor DPI aware. This
   * disables the default blurry scaling and enables
   * WM_DPICHANGED to allow us to draw at proper DPI. */
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

  /**
   * Check if current keyboard layout uses AltGr and save keylayout ID for
   * specialized handling if keys like VK_OEM_*. I.e. french keylayout
   * generates VK_OEM_8 for their exclamation key (key left of right shift). */
  this->handleKeyboardChange();

  /**
   * Require COM for ANCHOR_DropTargetWin32. */
  OleInitialize(0);
}

ANCHOR_SystemWin32::~ANCHOR_SystemWin32()
{
  /**
   * Shutdown COM. */
  OleUninitialize();
  toggleConsole(1);
}

AnchorU64 ANCHOR_SystemWin32::performanceCounterToMillis(__int64 perf_ticks) const
{
  // Calculate the time passed since system initialization.
  __int64 delta = (perf_ticks - m_start) * 1000;

  AnchorU64 t = (AnchorU64)(delta / m_freq);
  return t;
}

bool ANCHOR_SystemWin32::processEvents(bool waitForEvent)
{
  MSG msg;
  bool hasEventHandled = false;

  do
  {
    if (waitForEvent && !::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
      ::Sleep(1);
    }

    if (ANCHOR::GetTime())
    {
      hasEventHandled = true;
    }

    /**
     * Process all the events waiting for us. */
    while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) != 0)
    {
      /**
       * TranslateMessage doesn't alter the message, and doesn't change our raw keyboard data.
       * Needed for MapVirtualKey or if we ever need to get chars from wm_ime_char or similar. */
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
      hasEventHandled = true;
    }

    /* PeekMessage above is allowed to dispatch messages to the wndproc without us
     * noticing, so we need to check the event manager here to see if there are
     * events waiting in the queue.
     */
    hasEventHandled |= this->m_eventManager->getNumEvents() > 0;

  } while (waitForEvent && !hasEventHandled);

  return hasEventHandled;
}

eAnchorStatus ANCHOR_SystemWin32::getModifierKeys(ANCHOR_ModifierKeys &keys) const
{
  bool down = HIBYTE(::GetKeyState(VK_LSHIFT)) != 0;
  keys.set(ANCHOR_ModifierKeyLeftShift, down);
  down = HIBYTE(::GetKeyState(VK_RSHIFT)) != 0;
  keys.set(ANCHOR_ModifierKeyRightShift, down);

  down = HIBYTE(::GetKeyState(VK_LMENU)) != 0;
  keys.set(ANCHOR_ModifierKeyLeftAlt, down);
  down = HIBYTE(::GetKeyState(VK_RMENU)) != 0;
  keys.set(ANCHOR_ModifierKeyRightAlt, down);

  down = HIBYTE(::GetKeyState(VK_LCONTROL)) != 0;
  keys.set(ANCHOR_ModifierKeyLeftControl, down);
  down = HIBYTE(::GetKeyState(VK_RCONTROL)) != 0;
  keys.set(ANCHOR_ModifierKeyRightControl, down);

  bool lwindown = HIBYTE(::GetKeyState(VK_LWIN)) != 0;
  bool rwindown = HIBYTE(::GetKeyState(VK_RWIN)) != 0;
  if (lwindown || rwindown)
    keys.set(ANCHOR_ModifierKeyOS, true);
  else
    keys.set(ANCHOR_ModifierKeyOS, false);
  return ANCHOR_SUCCESS;
}

eAnchorStatus ANCHOR_SystemWin32::getButtons(ANCHOR_Buttons &buttons) const
{
  /**
   * Check for swapped buttons (left-handed mouse buttons)
   * GetAsyncKeyState() will give back the state of the
   * physical mouse buttons. */
  bool swapped = ::GetSystemMetrics(SM_SWAPBUTTON) == TRUE;

  bool down = HIBYTE(::GetAsyncKeyState(VK_LBUTTON)) != 0;
  buttons.set(swapped ? ANCHOR_ButtonMaskRight : ANCHOR_ButtonMaskLeft, down);

  down = HIBYTE(::GetAsyncKeyState(VK_MBUTTON)) != 0;
  buttons.set(ANCHOR_ButtonMaskMiddle, down);

  down = HIBYTE(::GetAsyncKeyState(VK_RBUTTON)) != 0;
  buttons.set(swapped ? ANCHOR_ButtonMaskLeft : ANCHOR_ButtonMaskRight, down);
  return ANCHOR_SUCCESS;
}

void ANCHOR_SystemWin32::getMainDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const
{}

void ANCHOR_SystemWin32::getAllDisplayDimensions(AnchorU32 &width, AnchorU32 &height) const
{}

eAnchorStatus ANCHOR_SystemWin32::init()
{
  eAnchorStatus success = ANCHOR_System::init();
  InitCommonControls();

  /* Disable scaling on high DPI displays on Vista */
  SetProcessDPIAware();
  initRawInput();

  m_lfstart = ::GetTickCount();

  /* Determine whether this system has a high frequency performance counter. */
  m_hasPerformanceCounter = ::QueryPerformanceFrequency((LARGE_INTEGER *)&m_freq) == TRUE;
  if (m_hasPerformanceCounter)
  {
    TF_DEBUG(ANCHOR_WIN32).Msg("High Frequency Performance Timer available\n");
    ::QueryPerformanceCounter((LARGE_INTEGER *)&m_start);
  }
  else
  {
    TF_DEBUG(ANCHOR_WIN32).Msg("High Frequency Performance Timer not available\n");
  }

  if (success)
  {
    WNDCLASSW wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = s_wndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ::GetModuleHandle(0);
    wc.hIcon = ::LoadIcon(wc.hInstance, "APPICON");

    if (!wc.hIcon)
    {
      ::LoadIcon(NULL, IDI_APPLICATION);
    }
    wc.hCursor = ::LoadCursor(0, IDC_ARROW);
    wc.hbrBackground =
#ifdef INW32_COMPISITING
      (HBRUSH)CreateSolidBrush
#endif
      (0x00000000);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"ANCHOR_WindowClass";

    // Use RegisterClassEx for setting small icon
    if (::RegisterClassW(&wc) == 0)
    {
      success = ANCHOR_ERROR;
    }
  }

  return success;
}

ANCHOR_ISystemWindow *ANCHOR_SystemWin32::createWindow(const char *title,
                                                       const char *icon,
                                                       AnchorS32 left,
                                                       AnchorS32 top,
                                                       AnchorU32 width,
                                                       AnchorU32 height,
                                                       eAnchorWindowState state,
                                                       eAnchorDrawingContextType type,
                                                       int vkSettings,
                                                       const bool exclusive,
                                                       const bool is_dialog,
                                                       const ANCHOR_ISystemWindow *parentWindow)
{
  ANCHOR_WindowWin32 *window = new ANCHOR_WindowWin32(this,
                                                      title,
                                                      icon,
                                                      left,
                                                      top,
                                                      width,
                                                      height,
                                                      state,
                                                      type,
                                                      (ANCHOR_WindowWin32 *)parentWindow,
                                                      is_dialog);

  if (window->getValid())
  {
    /**
     * Store the pointer to the window. */
    m_windowManager->addWindow(window);
    m_windowManager->setActiveWindow(window);
  }
  else
  {
    TF_DEBUG(ANCHOR_WIN32).Msg("Window invalid\n");
    delete window;
    window = NULL;
  }

  return window;
}

bool ANCHOR_SystemWin32::generateWindowExposeEvents()
{
  return true;
}

eAnchorStatus ANCHOR_SystemWin32::getCursorPosition(AnchorS32 &x, AnchorS32 &y) const
{
  POINT point;
  if (::GetCursorPos(&point))
  {
    x = point.x;
    y = point.y;
    return ANCHOR_SUCCESS;
  }
  return ANCHOR_ERROR;
}

eAnchorStatus ANCHOR_SystemWin32::setCursorPosition(AnchorS32 x, AnchorS32 y)
{
  if (!::GetActiveWindow())
    return ANCHOR_ERROR;
  return ::SetCursorPos(x, y) == TRUE ? ANCHOR_SUCCESS : ANCHOR_ERROR;
}

void ANCHOR_SystemWin32::processMinMaxInfo(MINMAXINFO *minmax)
{
  minmax->ptMinTrackSize.x = 320;
  minmax->ptMinTrackSize.y = 240;
}

static DWORD GetParentProcessID(void)
{
  HANDLE snapshot;
  PROCESSENTRY32 pe32 = {0};
  DWORD ppid = 0, pid = GetCurrentProcessId();
  snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE)
  {
    return -1;
  }
  pe32.dwSize = sizeof(pe32);
  if (!Process32First(snapshot, &pe32))
  {
    CloseHandle(snapshot);
    return -1;
  }
  do
  {
    if (pe32.th32ProcessID == pid)
    {
      ppid = pe32.th32ParentProcessID;
      break;
    }
  } while (Process32Next(snapshot, &pe32));
  CloseHandle(snapshot);
  return ppid;
}

static bool getProcessName(int pid, char *buffer, int max_len)
{
  bool result = false;
  HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (handle)
  {
    GetModuleFileNameEx(handle, 0, buffer, max_len);
    result = true;
  }
  CloseHandle(handle);
  return result;
}

static bool isStartedFromCommandPrompt()
{
  HWND hwnd = GetConsoleWindow();

  if (hwnd)
  {
    DWORD pid = (DWORD)-1;
    DWORD ppid = GetParentProcessID();
    char parent_name[MAX_PATH];
    bool start_from_launcher = false;

    GetWindowThreadProcessId(hwnd, &pid);
    if (getProcessName(ppid, parent_name, sizeof(parent_name)))
    {
      char *filename = strrchr(parent_name, '\\');
      if (filename != NULL)
      {
        start_from_launcher = strstr(filename, "kraken.exe") != NULL;
      }
    }

    /* When we're starting from a wrapper we need to compare with parent process ID. */
    if (pid != (start_from_launcher ? ppid : GetCurrentProcessId()))
      return true;
  }

  return false;
}

int ANCHOR_SystemWin32::toggleConsole(int action)
{
  HWND wnd = GetConsoleWindow();

  switch (action)
  {
    case 3:  // startup: hide if not started from command prompt
    {
      if (!isStartedFromCommandPrompt())
      {
        ShowWindow(wnd, SW_HIDE);
        m_consoleStatus = 0;
      }
      break;
    }
    case 0:  // hide
      ShowWindow(wnd, SW_HIDE);
      m_consoleStatus = 0;
      break;
    case 1:  // show
      ShowWindow(wnd, SW_SHOW);
      if (!isStartedFromCommandPrompt())
      {
        DeleteMenu(GetSystemMenu(wnd, FALSE), SC_CLOSE, MF_BYCOMMAND);
      }
      m_consoleStatus = 1;
      break;
    case 2:  // toggle
      ShowWindow(wnd, m_consoleStatus ? SW_HIDE : SW_SHOW);
      m_consoleStatus = !m_consoleStatus;
      if (m_consoleStatus && !isStartedFromCommandPrompt())
      {
        DeleteMenu(GetSystemMenu(wnd, FALSE), SC_CLOSE, MF_BYCOMMAND);
      }
      break;
  }

  return m_consoleStatus;
}

LRESULT WINAPI ANCHOR_SystemWin32::s_wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ANCHOR_Event *event = NULL;
  bool eventHandled = false;

  LRESULT lResult = 0;
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();
#ifdef WITH_INPUT_IME
  ANCHOR_EventManager *eventManager = system->getEventManager();
#endif
  ANCHOR_ASSERT(system);

  if (hwnd)
  {

    if (msg == WM_NCCREATE)
    {
      // Tell Windows to automatically handle scaling of non-client areas
      // such as the caption bar. EnableNonClientDpiScaling was introduced in Windows 10
      HMODULE m_user32 = ::LoadLibrary("User32.dll");
      if (m_user32)
      {
        ANCHOR_WIN32_EnableNonClientDpiScaling fpEnableNonClientDpiScaling =
          (ANCHOR_WIN32_EnableNonClientDpiScaling)::GetProcAddress(m_user32, "EnableNonClientDpiScaling");
        if (fpEnableNonClientDpiScaling)
        {
          fpEnableNonClientDpiScaling(hwnd);
        }
      }
    }

    ANCHOR_WindowWin32 *window = (ANCHOR_WindowWin32 *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (window)
    {
      switch (msg)
      {
        // we need to check if new key layout has AltGr
        case WM_INPUTLANGCHANGE: {
          system->handleKeyboardChange();
#ifdef WITH_INPUT_IME
          window->getImeInput()->SetInputLanguage();
#endif
          break;
        }
        ////////////////////////////////////////////////////////////////////////
        // Keyboard events, processed
        ////////////////////////////////////////////////////////////////////////
        case WM_INPUT: {
          RAWINPUT raw;
          RAWINPUT *raw_ptr = &raw;
          UINT rawSize = sizeof(RAWINPUT);

          GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw_ptr, &rawSize, sizeof(RAWINPUTHEADER));

          switch (raw.header.dwType)
          {
            case RIM_TYPEKEYBOARD:
              event = processKeyEvent(window, raw);
              if (!event)
              {
                TF_DEBUG(ANCHOR_WIN32).Msg("ANCHOR_SystemWin32::wndProc: key event ");
                TF_DEBUG(ANCHOR_WIN32).Msg(std::to_string(msg));
                TF_DEBUG(ANCHOR_WIN32).Msg(" key ignored\n");
              }
              break;
#ifdef WITH_INPUT_NDOF
            case RIM_TYPEHID:
              if (system->processNDOF(raw))
              {
                eventHandled = true;
              }
              break;
#endif
          }
          break;
        }
#ifdef WITH_INPUT_IME
        ////////////////////////////////////////////////////////////////////////
        // IME events, processed, read more in ANCHOR_IME.h
        ////////////////////////////////////////////////////////////////////////
        case WM_IME_SETCONTEXT: {
          ANCHOR_ImeWin32 *ime = window->getImeInput();
          ime->SetInputLanguage();
          ime->CreateImeWindow(hwnd);
          ime->CleanupComposition(hwnd);
          ime->CheckFirst(hwnd);
          break;
        }
        case WM_IME_STARTCOMPOSITION: {
          ANCHOR_ImeWin32 *ime = window->getImeInput();
          eventHandled = true;
          /* remove input event before start comp event, avoid redundant input */
          eventManager->removeTypeEvents(ANCHOR_EventTypeKeyDown, window);
          ime->CreateImeWindow(hwnd);
          ime->ResetComposition(hwnd);
          event = processImeEvent(ANCHOR_EventTypeImeCompositionStart, window, &ime->eventImeData);
          break;
        }
        case WM_IME_COMPOSITION: {
          ANCHOR_ImeWin32 *ime = window->getImeInput();
          eventHandled = true;
          ime->UpdateImeWindow(hwnd);
          ime->UpdateInfo(hwnd);
          if (ime->eventImeData.result_len)
          {
            /* remove redundant IME event */
            eventManager->removeTypeEvents(ANCHOR_EventTypeImeComposition, window);
          }
          event = processImeEvent(ANCHOR_EventTypeImeComposition, window, &ime->eventImeData);
          break;
        }
        case WM_IME_ENDCOMPOSITION: {
          ANCHOR_ImeWin32 *ime = window->getImeInput();
          eventHandled = true;
          /* remove input event after end comp event, avoid redundant input */
          eventManager->removeTypeEvents(ANCHOR_EventTypeKeyDown, window);
          ime->ResetComposition(hwnd);
          ime->DestroyImeWindow(hwnd);
          event = processImeEvent(ANCHOR_EventTypeImeCompositionEnd, window, &ime->eventImeData);
          break;
        }
#endif /* WITH_INPUT_IME */
        ////////////////////////////////////////////////////////////////////////
        // Keyboard events, ignored
        ////////////////////////////////////////////////////////////////////////
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        /* These functions were replaced by #WM_INPUT. */
        case WM_CHAR:
        /* The WM_CHAR message is posted to the window with the keyboard focus when
         * a WM_KEYDOWN message is translated by the TranslateMessage function. WM_CHAR
         * contains the character code of the key that was pressed.
         */
        case WM_DEADCHAR:
          /* The WM_DEADCHAR message is posted to the window with the keyboard focus when a
           * WM_KEYUP message is translated by the TranslateMessage function. WM_DEADCHAR
           * specifies a character code generated by a dead key. A dead key is a key that
           * generates a character, such as the umlaut (double-dot), that is combined with
           * another character to form a composite character. For example, the umlaut-O
           * character (Ö) is generated by typing the dead key for the umlaut character, and
           * then typing the O key.
           */
          break;
        case WM_SYSDEADCHAR:
        /* The WM_SYSDEADCHAR message is sent to the window with the keyboard focus when
         * a WM_SYSKEYDOWN message is translated by the TranslateMessage function.
         * WM_SYSDEADCHAR specifies the character code of a system dead key - that is,
         * a dead key that is pressed while holding down the alt key.
         */
        case WM_SYSCHAR:
          /* The WM_SYSCHAR message is sent to the window with the keyboard focus when
           * a WM_SYSCHAR message is translated by the TranslateMessage function.
           * WM_SYSCHAR specifies the character code of a dead key - that is,
           * a dead key that is pressed while holding down the alt key.
           * To prevent the sound, DefWindowProc must be avoided by return
           */
          break;
        case WM_SYSCOMMAND:
          /* The WM_SYSCOMMAND message is sent to the window when system commands such as
           * maximize, minimize  or close the window are triggered. Also it is sent when ALT
           * button is press for menu. To prevent this we must return preventing DefWindowProc.
           *
           * Note that the four low-order bits of the wParam parameter are used internally by the
           * OS. To obtain the correct result when testing the value of wParam, an application
           * must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator.
           */
          switch (wParam & 0xFFF0)
          {
            case SC_KEYMENU:
              eventHandled = true;
              break;
            case SC_RESTORE: {
              ::ShowWindow(hwnd, SW_RESTORE);
              window->setState(window->getState());

#ifdef WINDOWS_NEEDS_TABLET_SUPPORT
              ANCHOR_Wintab *wt = window->getWintab();
              if (wt)
              {
                wt->enable();
              }
#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */

              eventHandled = true;
              break;
            }
            case SC_MAXIMIZE: {

#ifdef WINDOWS_NEEDS_TABLET_SUPPORT
              ANCHOR_Wintab *wt = window->getWintab();
              if (wt)
              {
                wt->enable();
              }
#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */

              /* Don't report event as handled so that default handling occurs. */
              break;
            }
            case SC_MINIMIZE: {

#ifdef WINDOWS_NEEDS_TABLET_SUPPORT
              ANCHOR_Wintab *wt = window->getWintab();
              if (wt)
              {
                wt->disable();
              }
#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */

              /* Don't report event as handled so that default handling occurs. */
              break;
            }
          }
          break;

#ifdef WINDOWS_NEEDS_TABLET_SUPPORT

        ////////////////////////////////////////////////////////////////////////
        // Wintab events, processed
        ////////////////////////////////////////////////////////////////////////
        case WT_CSRCHANGE: {
          ANCHOR_Wintab *wt = window->getWintab();
          if (wt)
          {
            wt->updateCursorInfo();
          }
          eventHandled = true;
          break;
        }
        case WT_PROXIMITY: {
          ANCHOR_Wintab *wt = window->getWintab();
          if (wt)
          {
            bool inRange = LOWORD(lParam);
            if (inRange)
            {
              /* Some devices don't emit WT_CSRCHANGE events, so update cursor info here. */
              wt->updateCursorInfo();
            }
            else
            {
              wt->leaveRange();
            }
          }
          eventHandled = true;
          break;
        }
        case WT_INFOCHANGE: {
          ANCHOR_Wintab *wt = window->getWintab();
          if (wt)
          {
            wt->processInfoChange(lParam);

            if (window->usingTabletAPI(ANCHOR_TabletWintab))
            {
              window->resetPointerPenInfo();
            }
          }
          eventHandled = true;
          break;
        }
        case WT_PACKET:
          processWintabEvent(window);
          eventHandled = true;
          break;

#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */

        ////////////////////////////////////////////////////////////////////////
        // Pointer events, processed
        ////////////////////////////////////////////////////////////////////////
        case WM_POINTERUPDATE:
        case WM_POINTERDOWN:
        case WM_POINTERUP:
          processPointerEvent(msg, window, wParam, lParam, eventHandled);
          break;
        case WM_POINTERLEAVE: {
          AnchorU32 pointerId = GET_POINTERID_WPARAM(wParam);
          POINTER_INFO pointerInfo;
          if (!GetPointerInfo(pointerId, &pointerInfo))
          {
            break;
          }

          /* Reset pointer pen info if pen device has left tracking range. */
          if (pointerInfo.pointerType == PT_PEN)
          {
            window->resetPointerPenInfo();
            eventHandled = true;
          }
          break;
        }
        ////////////////////////////////////////////////////////////////////////
        // Mouse events, processed
        ////////////////////////////////////////////////////////////////////////
        case WM_LBUTTONDOWN:
          event = processButtonEvent(ANCHOR_EventTypeButtonDown, window, ANCHOR_ButtonMaskLeft);
          break;
        case WM_MBUTTONDOWN:
          event = processButtonEvent(ANCHOR_EventTypeButtonDown, window, ANCHOR_ButtonMaskMiddle);
          break;
        case WM_RBUTTONDOWN:
          event = processButtonEvent(ANCHOR_EventTypeButtonDown, window, ANCHOR_ButtonMaskRight);
          break;
        case WM_XBUTTONDOWN:
          if ((short)HIWORD(wParam) == XBUTTON1)
          {
            event = processButtonEvent(ANCHOR_EventTypeButtonDown, window, ANCHOR_ButtonMaskButton4);
          }
          else if ((short)HIWORD(wParam) == XBUTTON2)
          {
            event = processButtonEvent(ANCHOR_EventTypeButtonDown, window, ANCHOR_ButtonMaskButton5);
          }
          break;
        case WM_LBUTTONUP:
          event = processButtonEvent(ANCHOR_EventTypeButtonUp, window, ANCHOR_ButtonMaskLeft);
          break;
        case WM_MBUTTONUP:
          event = processButtonEvent(ANCHOR_EventTypeButtonUp, window, ANCHOR_ButtonMaskMiddle);
          break;
        case WM_RBUTTONUP:
          event = processButtonEvent(ANCHOR_EventTypeButtonUp, window, ANCHOR_ButtonMaskRight);
          break;
        case WM_XBUTTONUP:
          if ((short)HIWORD(wParam) == XBUTTON1)
          {
            event = processButtonEvent(ANCHOR_EventTypeButtonUp, window, ANCHOR_ButtonMaskButton4);
          }
          else if ((short)HIWORD(wParam) == XBUTTON2)
          {
            event = processButtonEvent(ANCHOR_EventTypeButtonUp, window, ANCHOR_ButtonMaskButton5);
          }
          break;
        case WM_MOUSEMOVE:
          if (!window->m_mousePresent)
          {
            TRACKMOUSEEVENT tme = {sizeof(tme)};
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            window->m_mousePresent = true;

#ifdef WINDOWS_NEEDS_TABLET_SUPPORT
            ANCHOR_Wintab *wt = window->getWintab();
            if (wt)
            {
              wt->gainFocus();
            }
#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */
          }
          event = processCursorEvent(window);

          break;
        case WM_MOUSEWHEEL: {
          /* The WM_MOUSEWHEEL message is sent to the focus window
           * when the mouse wheel is rotated. The DefWindowProc
           * function propagates the message to the window's parent.
           * There should be no internal forwarding of the message,
           * since DefWindowProc propagates it up the parent chain
           * until it finds a window that processes it.
           */
          processWheelEvent(window, wParam, lParam);
          eventHandled = true;
#ifdef BROKEN_PEEK_TOUCHPAD
          PostMessage(hwnd, WM_USER, 0, 0);
#endif
          break;
        }
        case WM_SETCURSOR:
          /* The WM_SETCURSOR message is sent to a window if the mouse causes the cursor
           * to move within a window and mouse input is not captured.
           * This means we have to set the cursor shape every time the mouse moves!
           * The DefWindowProc function uses this message to set the cursor to an
           * arrow if it is not in the client area.
           */
          if (LOWORD(lParam) == HTCLIENT)
          {
            // Load the current cursor
            window->loadCursor(window->getCursorVisibility(), window->getCursorShape());
            // Bypass call to DefWindowProc
            return 0;
          }
          else
          {
            // Outside of client area show standard cursor
            window->loadCursor(true, ANCHOR_StandardCursorDefault);
          }
          break;
        case WM_MOUSELEAVE: {
          window->m_mousePresent = false;
          if (window->getTabletData().Active == ANCHOR_TabletModeNone)
          {
            processCursorEvent(window);
          }
#ifdef WINDOWS_NEEDS_TABLET_SUPPORT
          ANCHOR_Wintab *wt = window->getWintab();
          if (wt)
          {
            wt->loseFocus();
          }
#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */
          break;
        }
        ////////////////////////////////////////////////////////////////////////
        // Mouse events, ignored
        ////////////////////////////////////////////////////////////////////////
        case WM_NCMOUSEMOVE:
        /* The WM_NCMOUSEMOVE message is posted to a window when the cursor is moved
         * within the non-client area of the window. This message is posted to the window that
         * contains the cursor. If a window has captured the mouse, this message is not posted.
         */
        case WM_NCHITTEST:
          /* The WM_NCHITTEST message is sent to a window when the cursor moves, or
           * when a mouse button is pressed or released. If the mouse is not captured,
           * the message is sent to the window beneath the cursor. Otherwise, the message
           * is sent to the window that has captured the mouse.
           */
          break;

        ////////////////////////////////////////////////////////////////////////
        // Window events, processed
        ////////////////////////////////////////////////////////////////////////
        case WM_CLOSE:
          /* The WM_CLOSE message is sent as a signal that a window
           * or an application should terminate. Restore if minimized. */
          if (IsIconic(hwnd))
          {
            ShowWindow(hwnd, SW_RESTORE);
          }
          event = processWindowEvent(ANCHOR_EventTypeWindowClose, window);
          break;
        case WM_ACTIVATE:
          /* The WM_ACTIVATE message is sent to both the window being activated and the window
           * being deactivated. If the windows use the same input queue, the message is sent
           * synchronously, first to the window procedure of the top-level window being
           * deactivated, then to the window procedure of the top-level window being activated.
           * If the windows use different input queues, the message is sent asynchronously,
           * so the window is activated immediately. */
          {
            ANCHOR_ModifierKeys modifiers;
            modifiers.clear();
            system->storeModifierKeys(modifiers);
            system->m_wheelDeltaAccum = 0;
            system->m_keycode_last_repeat_key = 0;
            event = processWindowEvent(LOWORD(wParam) ? ANCHOR_EventTypeWindowActivate :
                                                        ANCHOR_EventTypeWindowDeactivate,
                                       window);
            /* WARNING: Let DefWindowProc handle WM_ACTIVATE, otherwise WM_MOUSEWHEEL
             * will not be dispatched to OUR active window if we minimize one of OUR windows. */
            if (LOWORD(wParam) == WA_INACTIVE)
              window->lostMouseCapture();

            lResult = ::DefWindowProc(hwnd, msg, wParam, lParam);
            break;
          }
        case WM_ENTERSIZEMOVE:
          /* The WM_ENTERSIZEMOVE message is sent one time to a window after it enters the moving
           * or sizing modal loop. The window enters the moving or sizing modal loop when the user
           * clicks the window's title bar or sizing border, or when the window passes the
           * WM_SYSCOMMAND message to the DefWindowProc function and the wParam parameter of the
           * message specifies the SC_MOVE or SC_SIZE value. The operation is complete when
           * DefWindowProc returns.
           */
          window->m_inLiveResize = 1;
          break;
        case WM_EXITSIZEMOVE:
          window->m_inLiveResize = 0;
          break;
        case WM_PAINT:
          /* An application sends the WM_PAINT message when the system or another application
           * makes a request to paint a portion of an application's window. The message is sent
           * when the UpdateWindow or RedrawWindow function is called, or by the DispatchMessage
           * function when the application obtains a WM_PAINT message by using the GetMessage or
           * PeekMessage function.
           */
          if (!window->m_inLiveResize)
          {
            event = processWindowEvent(ANCHOR_EventTypeWindowUpdate, window);
            ::ValidateRect(hwnd, NULL);
          }
          else
          {
            eventHandled = true;
          }
          break;
        case WM_GETMINMAXINFO:
          /* The WM_GETMINMAXINFO message is sent to a window when the size or
           * position of the window is about to change. An application can use
           * this message to override the window's default maximized size and
           * position, or its default minimum or maximum tracking size.
           */
          processMinMaxInfo((MINMAXINFO *)lParam);
          /* Let DefWindowProc handle it. */
          break;
        case WM_SIZING:
          event = processWindowSizeEvent(window);
          break;
        case WM_SIZE:
          /* The WM_SIZE message is sent to a window after its size has changed.
           * The WM_SIZE and WM_MOVE messages are not sent if an application handles the
           * WM_WINDOWPOSCHANGED message without calling DefWindowProc. It is more efficient
           * to perform any move or size change processing during the WM_WINDOWPOSCHANGED
           * message without calling DefWindowProc.
           */
          event = processWindowSizeEvent(window);
          break;
        case WM_CAPTURECHANGED:
          window->lostMouseCapture();
          break;
        case WM_MOVING:
          /* The WM_MOVING message is sent to a window that the user is moving. By processing
           * this message, an application can monitor the size and position of the drag rectangle
           * and, if needed, change its size or position.
           */
        case WM_MOVE:
          /* The WM_SIZE and WM_MOVE messages are not sent if an application handles the
           * WM_WINDOWPOSCHANGED message without calling DefWindowProc. It is more efficient
           * to perform any move or size change processing during the WM_WINDOWPOSCHANGED
           * message without calling DefWindowProc.
           */
          /* See #WM_SIZE comment. */
          if (window->m_inLiveResize)
          {
            system->pushEvent(processWindowEvent(ANCHOR_EventTypeWindowMove, window));
            system->dispatchEvents();
          }
          else
          {
            event = processWindowEvent(ANCHOR_EventTypeWindowMove, window);
          }

          break;
        case WM_DPICHANGED:
          /* The WM_DPICHANGED message is sent when the effective dots per inch (dpi) for a
           * window has changed. The DPI is the scale factor for a window. There are multiple
           * events that can cause the DPI to change such as when the window is moved to a monitor
           * with a different DPI.
           */
          {
            // The suggested new size and position of the window.
            RECT *const suggestedWindowRect = (RECT *)lParam;

            // Push DPI change event first
            system->pushEvent(processWindowEvent(ANCHOR_EventTypeWindowDPIHintChanged, window));
            system->dispatchEvents();
            eventHandled = true;

            // Then move and resize window
            SetWindowPos(hwnd,
                         NULL,
                         suggestedWindowRect->left,
                         suggestedWindowRect->top,
                         suggestedWindowRect->right - suggestedWindowRect->left,
                         suggestedWindowRect->bottom - suggestedWindowRect->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
          }
          break;
        case WM_DISPLAYCHANGE: {
#ifdef WINDOWS_NEEDS_TABLET_SUPPORT
          ANCHOR_Wintab *wt = window->getWintab();
          if (wt)
          {
            wt->remapCoordinates();
          }
#endif /* WINDOWS_NEEDS_TABLET_SUPPORT */
          break;
        }
        case WM_KILLFOCUS:
          /* The WM_KILLFOCUS message is sent to a window immediately before it loses the keyboard
           * focus. We want to prevent this if a window is still active and it loses focus to
           * nowhere. */
          if (!wParam && hwnd == ::GetActiveWindow())
          {
            ::SetFocus(hwnd);
          }
          break;
        ////////////////////////////////////////////////////////////////////////
        // Window events, ignored
        ////////////////////////////////////////////////////////////////////////
        case WM_WINDOWPOSCHANGED:
        /* The WM_WINDOWPOSCHANGED message is sent to a window whose size, position, or place
         * in the Z order has changed as a result of a call to the SetWindowPos function or
         * another window-management function.
         * The WM_SIZE and WM_MOVE messages are not sent if an application handles the
         * WM_WINDOWPOSCHANGED message without calling DefWindowProc. It is more efficient
         * to perform any move or size change processing during the WM_WINDOWPOSCHANGED
         * message without calling DefWindowProc.
         */
        case WM_ERASEBKGND:
        /* An application sends the WM_ERASEBKGND message when the window background must be
         * erased (for example, when a window is resized). The message is sent to prepare an
         * invalidated portion of a window for painting.
         */
        case WM_NCPAINT:
        /* An application sends the WM_NCPAINT message to a window
         * when its frame must be painted. */
        case WM_NCACTIVATE:
        /* The WM_NCACTIVATE message is sent to a window when its non-client area needs to be
         * changed to indicate an active or inactive state. */
        case WM_DESTROY:
        /* The WM_DESTROY message is sent when a window is being destroyed. It is sent to the
         * window procedure of the window being destroyed after the window is removed from the
         * screen. This message is sent first to the window being destroyed and then to the child
         * windows (if any) as they are destroyed. During the processing of the message, it can
         * be assumed that all child windows still exist. */
        case WM_NCDESTROY:
          /* The WM_NCDESTROY message informs a window that its non-client area is being
           * destroyed. The DestroyWindow function sends the WM_NCDESTROY message to the window
           * following the WM_DESTROY message. WM_DESTROY is used to free the allocated memory
           * object associated with the window.
           */
          break;
        case WM_SHOWWINDOW:
        /* The WM_SHOWWINDOW message is sent to a window when the window is
         * about to be hidden or shown. */
        case WM_WINDOWPOSCHANGING:
        /* The WM_WINDOWPOSCHANGING message is sent to a window whose size, position, or place in
         * the Z order is about to change as a result of a call to the SetWindowPos function or
         * another window-management function.
         */
        case WM_SETFOCUS:
          /* The WM_SETFOCUS message is sent to a window after it has gained the keyboard focus. */
          break;
        ////////////////////////////////////////////////////////////////////////
        // Other events
        ////////////////////////////////////////////////////////////////////////
        case WM_GETTEXT:
        /* An application sends a WM_GETTEXT message to copy the text that
         * corresponds to a window into a buffer provided by the caller.
         */
        case WM_ACTIVATEAPP:
        /* The WM_ACTIVATEAPP message is sent when a window belonging to a
         * different application than the active window is about to be activated.
         * The message is sent to the application whose window is being activated
         * and to the application whose window is being deactivated.
         */
        case WM_TIMER:
          /* The WIN32 docs say:
           * The WM_TIMER message is posted to the installing thread's message queue
           * when a timer expires. You can process the message by providing a WM_TIMER
           * case in the window procedure. Otherwise, the default window procedure will
           * call the TimerProc callback function specified in the call to the SetTimer
           * function used to install the timer.
           *
           * In ANCHOR, we let DefWindowProc call the timer callback.
           */
          break;
      }
    }
    else
    {
      // Event found for a window before the pointer to the class has been set.
      TF_DEBUG(ANCHOR_WIN32).Msg("ANCHOR_SystemWin32::wndProc: ANCHOR window event before creation\n");
      /* These are events we typically miss at this point:
       * WM_GETMINMAXINFO 0x24
       * WM_NCCREATE          0x81
       * WM_NCCALCSIZE        0x83
       * WM_CREATE            0x01
       * We let DefWindowProc do the work.
       */
    }
  }
  else
  {
    // Events without valid hwnd
    TF_DEBUG(ANCHOR_WIN32).Msg("ANCHOR_SystemWin32::wndProc: event without window\n");
  }

  if (event)
  {
    system->pushEvent(event);
    eventHandled = true;
  }

  if (!eventHandled)
    lResult = ::DefWindowProcW(hwnd, msg, wParam, lParam);

  return lResult;
}

eAnchorKey ANCHOR_SystemWin32::convertKey(short vKey, short scanCode, short extend) const
{
  eAnchorKey key;

  if ((vKey >= '0') && (vKey <= '9'))
  {
    // VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39)
    key = (eAnchorKey)(vKey - '0' + ANCHOR_Key0);
  }
  else if ((vKey >= 'A') && (vKey <= 'Z'))
  {
    // VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A)
    key = (eAnchorKey)(vKey - 'A' + ANCHOR_KeyA);
  }
  else if ((vKey >= VK_F1) && (vKey <= VK_F24))
  {
    key = (eAnchorKey)(vKey - VK_F1 + ANCHOR_KeyF1);
  }
  else
  {
    switch (vKey)
    {
      case VK_RETURN:
        key = (extend) ? ANCHOR_KeyNumpadEnter : ANCHOR_KeyEnter;
        break;

      case VK_BACK:
        key = ANCHOR_KeyBackSpace;
        break;
      case VK_TAB:
        key = ANCHOR_KeyTab;
        break;
      case VK_ESCAPE:
        key = ANCHOR_KeyEsc;
        break;
      case VK_SPACE:
        key = ANCHOR_KeySpace;
        break;

      case VK_INSERT:
      case VK_NUMPAD0:
        key = (extend) ? ANCHOR_KeyInsert : ANCHOR_KeyNumpad0;
        break;
      case VK_END:
      case VK_NUMPAD1:
        key = (extend) ? ANCHOR_KeyEnd : ANCHOR_KeyNumpad1;
        break;
      case VK_DOWN:
      case VK_NUMPAD2:
        key = (extend) ? ANCHOR_KeyDownArrow : ANCHOR_KeyNumpad2;
        break;
      case VK_NEXT:
      case VK_NUMPAD3:
        key = (extend) ? ANCHOR_KeyDownPage : ANCHOR_KeyNumpad3;
        break;
      case VK_LEFT:
      case VK_NUMPAD4:
        key = (extend) ? ANCHOR_KeyLeftArrow : ANCHOR_KeyNumpad4;
        break;
      case VK_CLEAR:
      case VK_NUMPAD5:
        key = (extend) ? ANCHOR_KeyUnknown : ANCHOR_KeyNumpad5;
        break;
      case VK_RIGHT:
      case VK_NUMPAD6:
        key = (extend) ? ANCHOR_KeyRightArrow : ANCHOR_KeyNumpad6;
        break;
      case VK_HOME:
      case VK_NUMPAD7:
        key = (extend) ? ANCHOR_KeyHome : ANCHOR_KeyNumpad7;
        break;
      case VK_UP:
      case VK_NUMPAD8:
        key = (extend) ? ANCHOR_KeyUpArrow : ANCHOR_KeyNumpad8;
        break;
      case VK_PRIOR:
      case VK_NUMPAD9:
        key = (extend) ? ANCHOR_KeyUpPage : ANCHOR_KeyNumpad9;
        break;
      case VK_DECIMAL:
      case VK_DELETE:
        key = (extend) ? ANCHOR_KeyDelete : ANCHOR_KeyNumpadPeriod;
        break;

      case VK_SNAPSHOT:
        key = ANCHOR_KeyPrintScreen;
        break;
      case VK_PAUSE:
        key = ANCHOR_KeyPause;
        break;
      case VK_MULTIPLY:
        key = ANCHOR_KeyNumpadAsterisk;
        break;
      case VK_SUBTRACT:
        key = ANCHOR_KeyNumpadMinus;
        break;
      case VK_DIVIDE:
        key = ANCHOR_KeyNumpadSlash;
        break;
      case VK_ADD:
        key = ANCHOR_KeyNumpadPlus;
        break;

      case VK_SEMICOLON:
        key = ANCHOR_KeySemicolon;
        break;
      case VK_EQUALS:
        key = ANCHOR_KeyEqual;
        break;
      case VK_COMMA:
        key = ANCHOR_KeyComma;
        break;
      case VK_MINUS:
        key = ANCHOR_KeyMinus;
        break;
      case VK_PERIOD:
        key = ANCHOR_KeyPeriod;
        break;
      case VK_SLASH:
        key = ANCHOR_KeySlash;
        break;
      case VK_BACK_QUOTE:
        key = ANCHOR_KeyAccentGrave;
        break;
      case VK_OPEN_BRACKET:
        key = ANCHOR_KeyLeftBracket;
        break;
      case VK_BACK_SLASH:
        key = ANCHOR_KeyBackslash;
        break;
      case VK_CLOSE_BRACKET:
        key = ANCHOR_KeyRightBracket;
        break;
      case VK_QUOTE:
        key = ANCHOR_KeyQuote;
        break;
      case VK_GR_LESS:
        key = ANCHOR_KeyGrLess;
        break;

      case VK_SHIFT:
        /* Check single shift presses */
        if (scanCode == 0x36)
        {
          key = ANCHOR_KeyRightShift;
        }
        else if (scanCode == 0x2a)
        {
          key = ANCHOR_KeyLeftShift;
        }
        else
        {
          /* Must be a combination SHIFT (Left or Right) + a Key
           * Ignore this as the next message will contain
           * the desired "Key" */
          key = ANCHOR_KeyUnknown;
        }
        break;
      case VK_CONTROL:
        key = (extend) ? ANCHOR_KeyRightControl : ANCHOR_KeyLeftControl;
        break;
      case VK_MENU:
        key = (extend) ? ANCHOR_KeyRightAlt : ANCHOR_KeyLeftAlt;
        break;
      case VK_LWIN:
      case VK_RWIN:
        key = ANCHOR_KeyOS;
        break;
      case VK_APPS:
        key = ANCHOR_KeyApp;
        break;
      case VK_NUMLOCK:
        key = ANCHOR_KeyNumLock;
        break;
      case VK_SCROLL:
        key = ANCHOR_KeyScrollLock;
        break;
      case VK_CAPITAL:
        key = ANCHOR_KeyCapsLock;
        break;
      case VK_OEM_8:
        key = ((ANCHOR_SystemWin32 *)getSystem())->processSpecialKey(vKey, scanCode);
        break;
      case VK_MEDIA_PLAY_PAUSE:
        key = ANCHOR_KeyMediaPlay;
        break;
      case VK_MEDIA_STOP:
        key = ANCHOR_KeyMediaStop;
        break;
      case VK_MEDIA_PREV_TRACK:
        key = ANCHOR_KeyMediaFirst;
        break;
      case VK_MEDIA_NEXT_TRACK:
        key = ANCHOR_KeyMediaLast;
        break;
      default:
        key = ANCHOR_KeyUnknown;
        break;
    }
  }

  return key;
}

eAnchorStatus ANCHOR_WindowWin32::getPointerInfo(std::vector<ANCHOR_PointerInfoWin32> &outPointerInfo,
                                                 WPARAM wParam,
                                                 LPARAM lParam)
{
  AnchorS32 pointerId = GET_POINTERID_WPARAM(wParam);
  AnchorS32 isPrimary = IS_POINTER_PRIMARY_WPARAM(wParam);
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)ANCHOR_System::getSystem();
  AnchorU32 outCount = 0;

  if (!(GetPointerPenInfoHistory(pointerId, &outCount, NULL)))
  {
    return ANCHOR_ERROR;
  }

  std::vector<POINTER_PEN_INFO> pointerPenInfo(outCount);
  outPointerInfo.resize(outCount);

  if (!(GetPointerPenInfoHistory(pointerId, &outCount, pointerPenInfo.data())))
  {
    return ANCHOR_ERROR;
  }

  for (AnchorU32 i = 0; i < outCount; i++)
  {
    POINTER_INFO pointerApiInfo = pointerPenInfo[i].pointerInfo;
    // Obtain the basic information from the event
    outPointerInfo[i].pointerId = pointerId;
    outPointerInfo[i].isPrimary = isPrimary;

    switch (pointerApiInfo.ButtonChangeType)
    {
      case POINTER_CHANGE_FIRSTBUTTON_DOWN:
      case POINTER_CHANGE_FIRSTBUTTON_UP:
        outPointerInfo[i].buttonMask = ANCHOR_ButtonMaskLeft;
        break;
      case POINTER_CHANGE_SECONDBUTTON_DOWN:
      case POINTER_CHANGE_SECONDBUTTON_UP:
        outPointerInfo[i].buttonMask = ANCHOR_ButtonMaskRight;
        break;
      case POINTER_CHANGE_THIRDBUTTON_DOWN:
      case POINTER_CHANGE_THIRDBUTTON_UP:
        outPointerInfo[i].buttonMask = ANCHOR_ButtonMaskMiddle;
        break;
      case POINTER_CHANGE_FOURTHBUTTON_DOWN:
      case POINTER_CHANGE_FOURTHBUTTON_UP:
        outPointerInfo[i].buttonMask = ANCHOR_ButtonMaskButton4;
        break;
      case POINTER_CHANGE_FIFTHBUTTON_DOWN:
      case POINTER_CHANGE_FIFTHBUTTON_UP:
        outPointerInfo[i].buttonMask = ANCHOR_ButtonMaskButton5;
        break;
      default:
        break;
    }

    outPointerInfo[i].pixelLocation = pointerApiInfo.ptPixelLocation;
    outPointerInfo[i].tabletData.Active = ANCHOR_TabletModeStylus;
    outPointerInfo[i].tabletData.Pressure = 1.0f;
    outPointerInfo[i].tabletData.Xtilt = 0.0f;
    outPointerInfo[i].tabletData.Ytilt = 0.0f;
    outPointerInfo[i].time = system->performanceCounterToMillis(pointerApiInfo.PerformanceCount);

    if (pointerPenInfo[i].penMask & PEN_MASK_PRESSURE)
    {
      outPointerInfo[i].tabletData.Pressure = pointerPenInfo[i].pressure / 1024.0f;
    }

    if (pointerPenInfo[i].penFlags & PEN_FLAG_ERASER)
    {
      outPointerInfo[i].tabletData.Active = ANCHOR_TabletModeEraser;
    }

    if (pointerPenInfo[i].penMask & PEN_MASK_TILT_X)
    {
      outPointerInfo[i].tabletData.Xtilt = fmin(fabs(pointerPenInfo[i].tiltX / 90.0f), 1.0f);
    }

    if (pointerPenInfo[i].penMask & PEN_MASK_TILT_Y)
    {
      outPointerInfo[i].tabletData.Ytilt = fmin(fabs(pointerPenInfo[i].tiltY / 90.0f), 1.0f);
    }
  }

  if (!outPointerInfo.empty())
  {
    m_lastPointerTabletData = outPointerInfo.back().tabletData;
  }

  return ANCHOR_SUCCESS;
}

/**
 * @note this function can be extended to include other exotic cases as they arise. */
eAnchorKey ANCHOR_SystemWin32::processSpecialKey(short vKey, short scanCode) const
{
  eAnchorKey key = ANCHOR_KeyUnknown;
  switch (PRIMARYLANGID(m_langId))
  {
    case LANG_FRENCH:
      if (vKey == VK_OEM_8)
        key = ANCHOR_KeyF13;  // oem key; used purely for shortcuts .
      break;
    case LANG_ENGLISH:
      if (SUBLANGID(m_langId) == SUBLANG_ENGLISH_UK && vKey == VK_OEM_8)  // "`¬"
        key = ANCHOR_KeyAccentGrave;
      break;
  }

  return key;
}

ANCHOR_Event *ANCHOR_SystemWin32::processWindowEvent(eAnchorEventType type,
                                                     ANCHOR_WindowWin32 *window)
{
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();

  if (type == ANCHOR_EventTypeWindowActivate)
  {
    system->getWindowManager()->setActiveWindow(window);
  }

  return new ANCHOR_Event(ANCHOR::GetTime(), type, window);
}

void ANCHOR_SystemWin32::processWheelEvent(ANCHOR_WindowWin32 *window, WPARAM wParam, LPARAM lParam)
{
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();

  int acc = system->m_wheelDeltaAccum;
  int delta = GET_WHEEL_DELTA_WPARAM(wParam);

  if (acc * delta < 0)
  {
    // scroll direction reversed.
    acc = 0;
  }
  acc += delta;
  int direction = (acc >= 0) ? 1 : -1;
  acc = abs(acc);

  while (acc >= WHEEL_DELTA)
  {
    system->pushEvent(new ANCHOR_EventWheel(ANCHOR::GetTime(), window, direction));
    acc -= WHEEL_DELTA;
  }
  system->m_wheelDeltaAccum = acc * direction;
}

void ANCHOR_SystemWin32::processPointerEvent(UINT type, ANCHOR_WindowWin32 *window, WPARAM wParam, LPARAM lParam, bool &eventHandled)
{
  /* Pointer events might fire when changing windows for a device which is set to use Wintab,
   * even when Wintab is left enabled but set to the bottom of Wintab overlap order. */
  // if (!window->usingTabletAPI(ANCHOR_TabletWinPointer)) {
  //   return;
  // }

  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();
  std::vector<ANCHOR_PointerInfoWin32> pointerInfo;

  if (window->getPointerInfo(pointerInfo, wParam, lParam) != ANCHOR_SUCCESS)
  {
    return;
  }

  switch (type)
  {
    case WM_POINTERUPDATE:
      /* Coalesced pointer events are reverse chronological order, reorder chronologically.
       * Only contiguous move events are coalesced. */
      for (AnchorU32 i = pointerInfo.size(); i-- > 0;)
      {
        system->pushEvent(new ANCHOR_EventCursor(pointerInfo[i].time,
                                                 ANCHOR_EventTypeCursorMove,
                                                 window,
                                                 pointerInfo[i].pixelLocation.x,
                                                 pointerInfo[i].pixelLocation.y,
                                                 pointerInfo[i].tabletData));
      }

      /* Leave event unhandled so that system cursor is moved. */

      break;
    case WM_POINTERDOWN:
      /* Move cursor to point of contact because ANCHOR_EventButton does not include position. */
      system->pushEvent(new ANCHOR_EventCursor(pointerInfo[0].time,
                                               ANCHOR_EventTypeCursorMove,
                                               window,
                                               pointerInfo[0].pixelLocation.x,
                                               pointerInfo[0].pixelLocation.y,
                                               pointerInfo[0].tabletData));
      system->pushEvent(new ANCHOR_EventButton(pointerInfo[0].time,
                                               ANCHOR_EventTypeButtonDown,
                                               window,
                                               pointerInfo[0].buttonMask,
                                               pointerInfo[0].tabletData));
      window->updateMouseCapture(MousePressed);

      /* Mark event handled so that mouse button events are not generated. */
      eventHandled = true;

      break;
    case WM_POINTERUP:
      system->pushEvent(new ANCHOR_EventButton(pointerInfo[0].time,
                                               ANCHOR_EventTypeButtonUp,
                                               window,
                                               pointerInfo[0].buttonMask,
                                               pointerInfo[0].tabletData));
      window->updateMouseCapture(MouseReleased);

      /* Mark event handled so that mouse button events are not generated. */
      eventHandled = true;

      break;
    default:
      break;
  }
}

ANCHOR_EventCursor *ANCHOR_SystemWin32::processCursorEvent(ANCHOR_WindowWin32 *window)
{
  AnchorS32 x_screen, y_screen;
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();

  if (window->getTabletData().Active != ANCHOR_TabletModeNone)
  {
    /* While pen devices are in range, cursor movement is handled by tablet input processing. */
    return NULL;
  }

  system->getCursorPosition(x_screen, y_screen);

  if (window->getCursorGrabModeIsWarp())
  {
    AnchorS32 x_new = x_screen;
    AnchorS32 y_new = y_screen;
    AnchorS32 x_accum, y_accum;
    ANCHOR_Rect bounds;

    /* Fallback to window bounds. */
    if (window->getCursorGrabBounds(bounds) == ANCHOR_ERROR)
    {
      window->getClientBounds(bounds);
    }

    /* Could also clamp to screen bounds wrap with a window outside the view will fail atm.
     * Use inset in case the window is at screen bounds. */
    bounds.wrapPoint(x_new, y_new, 2, window->getCursorGrabAxis());

    window->getCursorGrabAccum(x_accum, y_accum);
    if (x_new != x_screen || y_new != y_screen)
    {
      /* When wrapping we don't need to add an event because the setCursorPosition call will cause
       * a new event after. */
      system->setCursorPosition(x_new, y_new); /* wrap */
      window->setCursorGrabAccum(x_accum + (x_screen - x_new), y_accum + (y_screen - y_new));
    }
    else
    {
      return new ANCHOR_EventCursor(ANCHOR::GetTime(),
                                    ANCHOR_EventTypeCursorMove,
                                    window,
                                    x_screen + x_accum,
                                    y_screen + y_accum,
                                    ANCHOR_TABLET_DATA_NONE);
    }
  }
  else
  {
    return new ANCHOR_EventCursor(ANCHOR::GetTime(),
                                  ANCHOR_EventTypeCursorMove,
                                  window,
                                  x_screen,
                                  y_screen,
                                  ANCHOR_TABLET_DATA_NONE);
  }
  return NULL;
}

ANCHOR_EventButton *ANCHOR_SystemWin32::processButtonEvent(eAnchorEventType type,
                                                           ANCHOR_WindowWin32 *window,
                                                           eAnchorButtonMask mask)
{
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();

  ANCHOR_TabletData td = window->getTabletData();

  /* Move mouse to button event position. */
  if (window->getTabletData().Active != ANCHOR_TabletModeNone)
  {
    /* Tablet should be handling in between mouse moves, only move to event position. */
    DWORD msgPos = ::GetMessagePos();
    int msgPosX = GET_X_LPARAM(msgPos);
    int msgPosY = GET_Y_LPARAM(msgPos);
    system->pushEvent(new ANCHOR_EventCursor(
      ::GetMessageTime(), ANCHOR_EventTypeCursorMove, window, msgPosX, msgPosY, td));
  }

  window->updateMouseCapture(type == ANCHOR_EventTypeButtonDown ? MousePressed : MouseReleased);
  return new ANCHOR_EventButton(ANCHOR::GetTime(), type, window, mask, td);
}

eAnchorKey ANCHOR_SystemWin32::hardKey(RAWINPUT const &raw, bool *r_keyDown, bool *r_is_repeated_modifier)
{
  bool is_repeated_modifier = false;

  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();
  eAnchorKey key = ANCHOR_KeyUnknown;
  ANCHOR_ModifierKeys modifiers;
  system->retrieveModifierKeys(modifiers);

  // RI_KEY_BREAK doesn't work for sticky keys release, so we also
  // check for the up message
  unsigned int msg = raw.data.keyboard.Message;
  *r_keyDown = !(raw.data.keyboard.Flags & RI_KEY_BREAK) && msg != WM_KEYUP && msg != WM_SYSKEYUP;

  key = this->convertKey(raw.data.keyboard.VKey,
                         raw.data.keyboard.MakeCode,
                         (raw.data.keyboard.Flags & (RI_KEY_E1 | RI_KEY_E0)));

  // extra handling of modifier keys: don't send repeats out from ANCHOR
  if (key >= ANCHOR_KeyLeftShift && key <= ANCHOR_KeyRightAlt)
  {
    bool changed = false;
    eAnchorModifierKeyMask modifier;
    switch (key)
    {
      case ANCHOR_KeyLeftShift: {
        changed = (modifiers.get(ANCHOR_ModifierKeyLeftShift) != *r_keyDown);
        modifier = ANCHOR_ModifierKeyLeftShift;
        break;
      }
      case ANCHOR_KeyRightShift: {
        changed = (modifiers.get(ANCHOR_ModifierKeyRightShift) != *r_keyDown);
        modifier = ANCHOR_ModifierKeyRightShift;
        break;
      }
      case ANCHOR_KeyLeftControl: {
        changed = (modifiers.get(ANCHOR_ModifierKeyLeftControl) != *r_keyDown);
        modifier = ANCHOR_ModifierKeyLeftControl;
        break;
      }
      case ANCHOR_KeyRightControl: {
        changed = (modifiers.get(ANCHOR_ModifierKeyRightControl) != *r_keyDown);
        modifier = ANCHOR_ModifierKeyRightControl;
        break;
      }
      case ANCHOR_KeyLeftAlt: {
        changed = (modifiers.get(ANCHOR_ModifierKeyLeftAlt) != *r_keyDown);
        modifier = ANCHOR_ModifierKeyLeftAlt;
        break;
      }
      case ANCHOR_KeyRightAlt: {
        changed = (modifiers.get(ANCHOR_ModifierKeyRightAlt) != *r_keyDown);
        modifier = ANCHOR_ModifierKeyRightAlt;
        break;
      }
      default:
        break;
    }

    if (changed)
    {
      modifiers.set(modifier, *r_keyDown);
      system->storeModifierKeys(modifiers);
    }
    else
    {
      is_repeated_modifier = true;
    }
  }

  *r_is_repeated_modifier = is_repeated_modifier;
  return key;
}

ANCHOR_Event *ANCHOR_SystemWin32::processWindowSizeEvent(ANCHOR_WindowWin32 *window)
{
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();
  ANCHOR_Event *sizeEvent = new ANCHOR_Event(
    ANCHOR::GetTime(), ANCHOR_EventTypeWindowSize, window);

  /* We get WM_SIZE before we fully init. Do not dispatch before we are continuously resizing. */
  if (window->m_inLiveResize)
  {
    system->pushEvent(sizeEvent);
    system->dispatchEvents();
    return NULL;
  }
  else
  {
    return sizeEvent;
  }
}

/** Error occurs when required parameter is missing. */
#define UTF_ERROR_NULL_IN (1 << 0)
/** Error if character is in illegal UTF range. */
#define UTF_ERROR_ILLCHAR (1 << 1)
/** Passed size is to small. It gives legal string with character missing at the end. */
#define UTF_ERROR_SMALL (1 << 2)
/** Error if sequence is broken and doesn't finish. */
#define UTF_ERROR_ILLSEQ (1 << 3)

static int conv_utf_16_to_8(const wchar_t *in16, char *out8, size_t size8)
{
  char *out8end = out8 + size8;
  wchar_t u = 0;
  int err = 0;
  if (!size8 || !in16 || !out8)
    return UTF_ERROR_NULL_IN;
  out8end--;

  for (; out8 < out8end && (u = *in16); in16++, out8++)
  {
    if (u < 0x0080)
    {
      *out8 = u;
    }
    else if (u < 0x0800)
    {
      if (out8 + 1 >= out8end)
        break;
      *out8++ = (0x3 << 6) | (0x1F & (u >> 6));
      *out8 = (0x1 << 7) | (0x3F & (u));
    }
    else if (u < 0xD800 || u >= 0xE000)
    {
      if (out8 + 2 >= out8end)
        break;
      *out8++ = (0x7 << 5) | (0xF & (u >> 12));
      *out8++ = (0x1 << 7) | (0x3F & (u >> 6));
      *out8 = (0x1 << 7) | (0x3F & (u));
    }
    else if (u < 0xDC00)
    {
      wchar_t u2 = *++in16;

      if (!u2)
        break;
      if (u2 >= 0xDC00 && u2 < 0xE000)
      {
        if (out8 + 3 >= out8end)
          break;
        else
        {
          unsigned int uc = 0x10000 + (u2 - 0xDC00) + ((u - 0xD800) << 10);

          *out8++ = (0xF << 4) | (0x7 & (uc >> 18));
          *out8++ = (0x1 << 7) | (0x3F & (uc >> 12));
          *out8++ = (0x1 << 7) | (0x3F & (uc >> 6));
          *out8 = (0x1 << 7) | (0x3F & (uc));
        }
      }
      else
      {
        out8--;
        err |= UTF_ERROR_ILLCHAR;
      }
    }
    else if (u < 0xE000)
    {
      out8--;
      err |= UTF_ERROR_ILLCHAR;
    }
  }

  *out8 = *out8end = 0;

  if (*in16)
    err |= UTF_ERROR_SMALL;

  return err;
}

static size_t count_utf_16_from_8(const char *string8)
{
  size_t count = 0;
  char u;
  char type = 0;
  unsigned int u32 = 0;

  if (!string8)
    return 0;

  for (; (u = *string8); string8++)
  {
    if (type == 0)
    {
      if ((u & 0x01 << 7) == 0)
      {
        count++;
        u32 = 0;
        continue;
      }  // 1 utf-8 char
      if ((u & 0x07 << 5) == 0xC0)
      {
        type = 1;
        u32 = u & 0x1F;
        continue;
      }  // 2 utf-8 char
      if ((u & 0x0F << 4) == 0xE0)
      {
        type = 2;
        u32 = u & 0x0F;
        continue;
      }  // 3 utf-8 char
      if ((u & 0x1F << 3) == 0xF0)
      {
        type = 3;
        u32 = u & 0x07;
        continue;
      }  // 4 utf-8 char
      continue;
    }
    else
    {
      if ((u & 0xC0) == 0x80)
      {
        u32 = (u32 << 6) | (u & 0x3F);
        type--;
      }
      else
      {
        u32 = 0;
        type = 0;
      }
    }

    if (type == 0)
    {
      if ((0 < u32 && u32 < 0xD800) || (0xE000 <= u32 && u32 < 0x10000))
        count++;
      else if (0x10000 <= u32 && u32 < 0x110000)
        count += 2;
      u32 = 0;
    }
  }

  return ++count;
}

static int conv_utf_8_to_16(const char *in8, wchar_t *out16, size_t size16)
{
  char u;
  char type = 0;
  unsigned int u32 = 0;
  wchar_t *out16end = out16 + size16;
  int err = 0;
  if (!size16 || !in8 || !out16)
    return UTF_ERROR_NULL_IN;
  out16end--;

  for (; out16 < out16end && (u = *in8); in8++)
  {
    if (type == 0)
    {
      if ((u & 0x01 << 7) == 0)
      {
        *out16 = u;
        out16++;
        u32 = 0;
        continue;
      }  // 1 utf-8 char
      if ((u & 0x07 << 5) == 0xC0)
      {
        type = 1;
        u32 = u & 0x1F;
        continue;
      }  // 2 utf-8 char
      if ((u & 0x0F << 4) == 0xE0)
      {
        type = 2;
        u32 = u & 0x0F;
        continue;
      }  // 3 utf-8 char
      if ((u & 0x1F << 3) == 0xF0)
      {
        type = 3;
        u32 = u & 0x07;
        continue;
      }  // 4 utf-8 char
      err |= UTF_ERROR_ILLCHAR;
      continue;
    }
    else
    {
      if ((u & 0xC0) == 0x80)
      {
        u32 = (u32 << 6) | (u & 0x3F);
        type--;
      }
      else
      {
        u32 = 0;
        type = 0;
        err |= UTF_ERROR_ILLSEQ;
      }
    }
    if (type == 0)
    {
      if ((0 < u32 && u32 < 0xD800) || (0xE000 <= u32 && u32 < 0x10000))
      {
        *out16 = u32;
        out16++;
      }
      else if (0x10000 <= u32 && u32 < 0x110000)
      {
        if (out16 + 1 >= out16end)
          break;
        u32 -= 0x10000;
        *out16 = 0xD800 + (u32 >> 10);
        out16++;
        *out16 = 0xDC00 + (u32 & 0x3FF);
        out16++;
      }
      u32 = 0;
    }
  }

  *out16 = *out16end = 0;

  if (*in8)
    err |= UTF_ERROR_SMALL;

  return err;
}

static wchar_t *alloc_utf16_from_8(const char *in8, size_t add)
{
  size_t bsize = count_utf_16_from_8(in8);
  wchar_t *out16 = NULL;
  if (!bsize)
    return NULL;
  out16 = (wchar_t *)malloc(sizeof(wchar_t) * (bsize + add));
  conv_utf_8_to_16(in8, out16, bsize);
  return out16;
}

ANCHOR_EventKey *ANCHOR_SystemWin32::processKeyEvent(ANCHOR_WindowWin32 *window, RAWINPUT const &raw)
{
  bool keyDown = false;
  bool is_repeated_modifier = false;
  ANCHOR_SystemWin32 *system = (ANCHOR_SystemWin32 *)getSystem();
  eAnchorKey key = system->hardKey(raw, &keyDown, &is_repeated_modifier);
  ANCHOR_EventKey *event;

  /* We used to check `if (key != ANCHOR_KeyUnknown)`, but since the message
   * values `WM_SYSKEYUP`, `WM_KEYUP` and `WM_CHAR` are ignored, we capture
   * those events here as well. */
  if (!is_repeated_modifier)
  {
    char vk = raw.data.keyboard.VKey;
    char utf8_char[6] = {0};
    char ascii = 0;
    bool is_repeat = false;

    /* Unlike on Linux, not all keys can send repeat events. E.g. modifier keys don't. */
    if (keyDown)
    {
      if (system->m_keycode_last_repeat_key == vk)
      {
        is_repeat = true;
      }
      system->m_keycode_last_repeat_key = vk;
    }
    else
    {
      if (system->m_keycode_last_repeat_key == vk)
      {
        system->m_keycode_last_repeat_key = 0;
      }
    }

    wchar_t utf16[3] = {0};
    BYTE state[256] = {0};
    int r;
    GetKeyboardState((PBYTE)state);
    bool ctrl_pressed = state[VK_CONTROL] & 0x80;
    bool alt_pressed = state[VK_MENU] & 0x80;

    /* No text with control key pressed (Alt can be used to insert special characters though!). */
    if (ctrl_pressed && !alt_pressed)
    {
      utf8_char[0] = '\0';
    }
    // Don't call ToUnicodeEx on dead keys as it clears the buffer and so won't allow diacritical
    // composition.
    else if (MapVirtualKeyW(vk, 2) != 0)
    {
      // todo: ToUnicodeEx can respond with up to 4 utf16 chars (only 2 here).
      // Could be up to 24 utf8 bytes.
      if ((r = ToUnicodeEx(
             vk, raw.data.keyboard.MakeCode, state, utf16, 2, 0, system->m_keylayout)))
      {
        if ((r > 0 && r < 3))
        {
          utf16[r] = 0;
          conv_utf_16_to_8(utf16, utf8_char, 6);
        }
        else if (r == -1)
        {
          utf8_char[0] = '\0';
        }
      }
    }

    if (!keyDown)
    {
      utf8_char[0] = '\0';
      ascii = '\0';
    }
    else
    {
      ascii = utf8_char[0] & 0x80 ? '?' : utf8_char[0];
    }

    event = new ANCHOR_EventKey(ANCHOR::GetTime(),
                                keyDown ? ANCHOR_EventTypeKeyDown : ANCHOR_EventTypeKeyUp,
                                window,
                                key,
                                ascii,
                                utf8_char,
                                is_repeat);
  }
  else
  {
    event = NULL;
  }

  return event;
}

//---------------------------------------------------------------------------------------------------------

const wchar_t *ANCHOR_WindowWin32::s_windowClassName = L"ANCHOR_WindowClass";
const int ANCHOR_WindowWin32::s_maxTitleLength = 128;

/* force NVidia Optimus to use dedicated graphics */
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

ANCHOR_WindowWin32::ANCHOR_WindowWin32(ANCHOR_SystemWin32 *system,
                                       const char *title,
                                       const char *icon,
                                       AnchorS32 left,
                                       AnchorS32 top,
                                       AnchorU32 width,
                                       AnchorU32 height,
                                       eAnchorWindowState state,
                                       eAnchorDrawingContextType type,
                                       const bool stereoVisual,
                                       const bool exclusive,
                                       ANCHOR_WindowWin32 *parentWindow,
                                       bool dialog)
  : ANCHOR_SystemWindow(width, height, state, stereoVisual, exclusive),
    m_system(system),
    m_hDC(0),
    m_isDialog(dialog),
    m_valid_setup(false),
    m_invalid_window(false),
    m_vulkan_context(nullptr),
    m_mousePresent(false),
    m_inLiveResize(false),
    m_hasMouseCaptured(false),
    m_hasGrabMouse(false),
    m_nPressedButtons(0),
    m_customCursor(0),
    m_lastPointerTabletData(ANCHOR_TABLET_DATA_NONE),
    m_user32(::LoadLibrary("user32.dll")),
    m_parentWindowHwnd(parentWindow ? parentWindow->m_hWnd : HWND_DESKTOP)
{
  DWORD style = parentWindow ?
                  WS_POPUPWINDOW | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX :
                  WS_OVERLAPPEDWINDOW;

  if (state == ANCHOR_WindowStateFullScreen)
  {
    style |= WS_MAXIMIZE;
  }

  /* Forces owned windows onto taskbar and allows minimization. */
  DWORD extended_style = parentWindow ? WS_EX_APPWINDOW : 0;

  RECT win_rect = {left, top, (long)(left + width), (long)(top + height)};
  adjustWindowRectForClosestMonitor(&win_rect, style, extended_style);

  wchar_t *title_16 = alloc_utf16_from_8((char *)title, 0);
  m_hWnd = ::CreateWindowExW(extended_style,                  // window extended style
                             s_windowClassName,               // pointer to registered class name
                             title_16,                        // pointer to window name
                             style,                           // window style
                             win_rect.left,                   // horizontal position of window
                             win_rect.top,                    // vertical position of window
                             win_rect.right - win_rect.left,  // window width
                             win_rect.bottom - win_rect.top,  // window height
                             m_parentWindowHwnd,              // handle to parent or owner window
                             0,                               // handle to menu or child-window identifier
                             ::GetModuleHandle(0),            // handle to application instance
                             0);                              // pointer to window-creation data
  free(title_16);

  if (m_hWnd == NULL)
  {
    return;
  }

  /*  Store the device context. */
  m_hDC = ::GetDC(m_hWnd);

  if (!setDrawingContextType(type))
  {
    ::DestroyWindow(m_hWnd);
    m_hWnd = NULL;
    return;
  }

  RegisterTouchWindow(m_hWnd, 0);

  /* Register as drop-target. #OleInitialize(0) required first. */
  // m_dropTarget = new ANCHOR_DropTargetWin32(this, m_system);
  // ::RegisterDragDrop(m_hWnd, m_dropTarget);

  /* Store a pointer to this class in the window structure. */
  ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

  if (!m_system->m_windowFocus)
  {
    /* If we don't want focus then lower to bottom. */
    ::SetWindowPos(m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }

  /* Show the window. */
  int nCmdShow;
  switch (state)
  {
    case ANCHOR_WindowStateMaximized:
      nCmdShow = SW_SHOWMAXIMIZED;
      break;
    case ANCHOR_WindowStateMinimized:
      nCmdShow = (m_system->m_windowFocus) ? SW_SHOWMINIMIZED : SW_SHOWMINNOACTIVE;
      break;
    case ANCHOR_WindowStateNormal:
    default:
      nCmdShow = (m_system->m_windowFocus) ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE;
      break;
  }

  ::ShowWindow(m_hWnd, nCmdShow);

#ifdef WIN32_COMPOSITING
  if (alphaBackground && parentwindowhwnd == 0)
  {

    HRESULT hr = S_OK;

    /* Create and populate the Blur Behind structure. */
    DWM_BLURBEHIND bb = {0};

    /* Enable Blur Behind and apply to the entire client area. */
    bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    bb.fEnable = true;
    bb.hRgnBlur = CreateRectRgn(0, 0, -1, -1);

    /* Apply Blur Behind. */
    hr = DwmEnableBlurBehindWindow(m_hWnd, &bb);
    DeleteObject(bb.hRgnBlur);
  }
#endif

  /* Force an initial paint of the window. */
  ::UpdateWindow(m_hWnd);

  /* Initialize Wintab. */
  // if (system->getTabletAPI() != ANCHOR_TabletWinPointer) {
  // loadWintab(ANCHOR_WindowStateMinimized != state);
  // }

  /* Allow the showing of a progress bar on the taskbar. */
  CoCreateInstance(
    CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (LPVOID *)&m_Bar);
}

ANCHOR_WindowWin32::~ANCHOR_WindowWin32()
{}

void ANCHOR_WindowWin32::adjustWindowRectForClosestMonitor(LPRECT win_rect,
                                                           DWORD dwStyle,
                                                           DWORD dwExStyle)
{
  /* Get Details of the closest monitor. */
  HMONITOR hmonitor = MonitorFromRect(win_rect, MONITOR_DEFAULTTONEAREST);
  MONITORINFOEX monitor;
  monitor.cbSize = sizeof(MONITORINFOEX);
  monitor.dwFlags = 0;
  GetMonitorInfo(hmonitor, &monitor);

  /* Constrain requested size and position to fit within this monitor. */
  LONG width = winmin(monitor.rcWork.right - monitor.rcWork.left, win_rect->right - win_rect->left);
  LONG height = winmin(monitor.rcWork.bottom - monitor.rcWork.top, win_rect->bottom - win_rect->top);
  win_rect->left = winmin(winmax(monitor.rcWork.left, win_rect->left), monitor.rcWork.right - width);
  win_rect->right = win_rect->left + width;
  win_rect->top = winmin(winmax(monitor.rcWork.top, win_rect->top), monitor.rcWork.bottom - height);
  win_rect->bottom = win_rect->top + height;

  /* With Windows 10 and newer we can adjust for chrome that differs with DPI and scale. */
  ANCHOR_WIN32_AdjustWindowRectExForDpi fpAdjustWindowRectExForDpi = nullptr;
  if (m_user32)
  {
    fpAdjustWindowRectExForDpi = (ANCHOR_WIN32_AdjustWindowRectExForDpi)::GetProcAddress(
      m_user32, "AdjustWindowRectExForDpi");
  }

  /* Adjust to allow for caption, borders, shadows, scaling, etc. Resulting values can be
   * correctly outside of monitor bounds. Note: You cannot specify WS_OVERLAPPED when calling. */
  if (fpAdjustWindowRectExForDpi)
  {
    UINT dpiX, dpiY;
    GetDpiForMonitor(hmonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    fpAdjustWindowRectExForDpi(win_rect, dwStyle & ~WS_OVERLAPPED, FALSE, dwExStyle, dpiX);
  }
  else
  {
    AdjustWindowRectEx(win_rect, dwStyle & ~WS_OVERLAPPED, FALSE, dwExStyle);
  }

  /* But never allow a top position that can hide part of the title bar. */
  win_rect->top = winmax(monitor.rcWork.top, win_rect->top);
}

std::string ANCHOR_WindowWin32::getTitle() const
{
  return std::string("KRAKEN");
}

bool ANCHOR_WindowWin32::getValid() const
{
  return ANCHOR_SystemWindow::getValid() && m_hWnd != 0 && m_hDC != 0;
}

void ANCHOR_WindowWin32::resetPointerPenInfo()
{
  m_lastPointerTabletData = ANCHOR_TABLET_DATA_NONE;
}

ANCHOR_TabletData ANCHOR_WindowWin32::getTabletData()
{
  // if (usingTabletAPI(ANCHOR_TabletWintab)) {
  //   return m_wintab ? m_wintab->getLastTabletData() : ANCHOR_TABLET_DATA_NONE;
  // }
  // else {
  return m_lastPointerTabletData;
  // }
}

void ANCHOR_WindowWin32::getClientBounds(ANCHOR_Rect &bounds) const
{
  RECT rect;
  POINT coord;
  if (!IsIconic(m_hWnd))
  {
    ::GetClientRect(m_hWnd, &rect);

    coord.x = rect.left;
    coord.y = rect.top;
    ::ClientToScreen(m_hWnd, &coord);

    bounds.m_l = coord.x;
    bounds.m_t = coord.y;

    coord.x = rect.right;
    coord.y = rect.bottom;
    ::ClientToScreen(m_hWnd, &coord);

    bounds.m_r = coord.x;
    bounds.m_b = coord.y;
  }
  else
  {
    bounds.m_b = 0;
    bounds.m_l = 0;
    bounds.m_r = 0;
    bounds.m_t = 0;
  }
}

void ANCHOR_WindowWin32::getWindowBounds(ANCHOR_Rect &bounds) const
{
  RECT rect;
  ::GetWindowRect(m_hWnd, &rect);
  bounds.m_b = rect.bottom;
  bounds.m_l = rect.left;
  bounds.m_r = rect.right;
  bounds.m_t = rect.top;
}

static void check_vk_result(VkResult err)
{
  if (err == 0)
    return;
  TF_CODING_ERROR("[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

static void SetupVulkan()
{
  VkResult err;

  /**
   * Setup VULKAN extensions. */
  std::vector<const char *> extensions {
    "VK_KHR_win32_surface",
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
  };

  /**
   * Create Vulkan Instance. */
  {
    VkInstanceCreateInfo create_info = {};

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    /**
     * Create Vulkan Instance. */
    err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
    g_PixarHydra = new HgiVulkan(g_PixarVkInstance = new HgiVulkanInstance(g_Instance));
    check_vk_result(err);
  }

  /**
   * Select GPU. */
  {
    uint32_t gpu_count;
    err = vkEnumeratePhysicalDevices(g_PixarVkInstance->GetVulkanInstance(), &gpu_count, NULL);
    check_vk_result(err);
    ANCHOR_ASSERT(gpu_count > 0);

    VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(g_PixarVkInstance->GetVulkanInstance(), &gpu_count, gpus);
    check_vk_result(err);

    /**
     * If a number >1 of GPUs got reported, find discrete GPU if present,
     * or use first one available. This covers most common cases (multi-gpu
     * & integrated+dedicated graphics).
     *
     * TODO: Handle more complicated setups (multiple dedicated GPUs). */
    int use_gpu = 0;
    for (int i = 0; i < (int)gpu_count; i++)
    {

      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(gpus[i], &properties);
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      {
        use_gpu = i;
        break;
      }
    }

    g_PhysicalDevice = gpus[use_gpu];
    free(gpus);
  }

  /**
   * Select graphics queue family. */
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
    VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) *
                                                                        count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        g_QueueFamily = i;
        break;
      }
    free(queues);
    ANCHOR_ASSERT(g_QueueFamily != (uint32_t)-1);
  }

  /**
   * Create Logical Device (with 1 queue). */
  {
    int device_extension_count = 1;
    const char *device_extensions[] = {"VK_KHR_swapchain"};
    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = g_QueueFamily;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = device_extension_count;
    create_info.ppEnabledExtensionNames = device_extensions;
    err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
    check_vk_result(err);
    vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
  }

  /**
   * Create Descriptor Pool. */
  {
    /* clang-format off */
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };
    /* clang-format on */
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * ANCHOR_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)ANCHOR_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
    check_vk_result(err);
  }
}

static void SetFont()
{
  ANCHOR_IO &io = ANCHOR::GetIO();
  io.Fonts->AddFontDefault();

  const static std::string exe_path = TfGetPathName(ArchGetExecutablePath());

  /* Gotham Font. */
  const static std::string gm_ttf("datafiles/fonts/GothamPro.ttf");
  const static char *gm_path = TfStringCatPaths(exe_path, gm_ttf).c_str();
  io.Fonts->AddFontFromFileTTF(gm_path, 11.0f);

  /* Dankmono Font. */
  const static std::string dm_ttf("datafiles/fonts/dankmono.ttf");
  const static char *dm_path = TfStringCatPaths(exe_path, dm_ttf).c_str();
  io.Fonts->AddFontFromFileTTF(dm_path, 12.0f);

  /* San Francisco Font (Default). */
  const static std::string sf_ttf("datafiles/fonts/SFProText-Medium.ttf");
  const static char *sf_path = TfStringCatPaths(exe_path, sf_ttf).c_str();
  io.FontDefault = io.Fonts->AddFontFromFileTTF(sf_path, 14.0f);
}

static void SetupVulkanWindow(ANCHOR_VulkanGPU_Surface *wd, VkSurfaceKHR surface, int width, int height)
{
  wd->Surface = surface;

  /**
   * Check for WSI support. */
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
  if (res != VK_TRUE)
  {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  /**
   * Select Surface Format. */
  /* clang-format off */
  const VkFormat requestSurfaceImageFormat[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8_UNORM,
    VK_FORMAT_R8G8B8_UNORM
  };
  /* clang-format on */
  const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

  wd->SurfaceFormat = ANCHOR_ImplVulkanH_SelectSurfaceFormat(
    g_PhysicalDevice,
    wd->Surface,
    requestSurfaceImageFormat,
    (size_t)ANCHOR_ARRAYSIZE(requestSurfaceImageFormat),
    requestSurfaceColorSpace);

  /**
   * Render at maximum possible FPS. */
  if (HgiVulkanIsMaxFPSEnabled())
  {

    TF_DEBUG(ANCHOR_WIN32).Msg("[Anchor] Rendering at maximum possible frames per second.\n");

    /* clang-format off */
    VkPresentModeKHR present_modes[] = {
      /** Removes screen tearing. */
      VK_PRESENT_MODE_MAILBOX_KHR,
      /** Present frames immediately. */
      VK_PRESENT_MODE_IMMEDIATE_KHR,
      /** Required for presentation. */
      VK_PRESENT_MODE_FIFO_KHR
    };
    /* clang-format on */

    wd->PresentMode = ANCHOR_ImplVulkanH_SelectPresentMode(
      g_PhysicalDevice, wd->Surface, &present_modes[0], ANCHOR_ARRAYSIZE(present_modes));
  }
  else
  { /** Throttled FPS ~75FPS */

    TF_DEBUG(ANCHOR_WIN32).Msg("[Anchor] Throttled maximum frames per second.\n");

    /* clang-format off */
    VkPresentModeKHR present_modes[] = {
      /** Required for presentation. */
      VK_PRESENT_MODE_FIFO_KHR
    };
    /* clang-format on */

    wd->PresentMode = ANCHOR_ImplVulkanH_SelectPresentMode(
      g_PhysicalDevice, wd->Surface, &present_modes[0], ANCHOR_ARRAYSIZE(present_modes));
  }

  TF_DEBUG(ANCHOR_WIN32).Msg("[Anchor] Selected PresentMode = %d\n", wd->PresentMode);

  /**
   * Create SwapChain, RenderPass, Framebuffer, etc. */
  ANCHOR_ASSERT(g_MinImageCount >= 2);
  ANCHOR_ImplVulkanH_CreateOrResizeWindow(g_PixarVkInstance->GetVulkanInstance(),
                                          g_PhysicalDevice,
                                          g_Device,
                                          wd,
                                          g_QueueFamily,
                                          g_Allocator,
                                          width,
                                          height,
                                          g_MinImageCount);
}

void ANCHOR_WindowWin32::newDrawingContext(eAnchorDrawingContextType type)
{
  if (type == ANCHOR_DrawingContextTypeVulkan)
  { 
    SetupVulkan();

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.hwnd = m_hWnd;

    VkSurfaceKHR surface;
    if (vkCreateWin32SurfaceKHR(g_PixarVkInstance->GetVulkanInstance(), &createInfo, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("CRITICAL: Failed to create window surface!");
    }    
    
    ANCHOR_Rect winrect;
    getWindowBounds(winrect);

    m_vulkan_context = new ANCHOR_VulkanGPU_Surface();
    SetupVulkanWindow(m_vulkan_context, surface, winrect.getWidth(), winrect.getHeight());

    /**
     * Setup ANCHOR context. */
    ANCHOR_CHECKVERSION();
    ANCHOR::CreateContext();

    /**
     * Setup Keyboard & Gamepad controls. */
    ANCHOR_IO &io = ANCHOR::GetIO();
    io.ConfigFlags |= ANCHORConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ANCHORConfigFlags_NavEnableGamepad;

    /**
     * Setup Default Kraken theme.
     *   Themes::
     *     - ANCHOR::StyleColorsDefault()
     *     - ANCHOR::StyleColorsLight()
     *     - ANCHOR::StyleColorsDark() */
    ANCHOR::StyleColorsDefault();

    /**
     * Setup Platform/Renderer backends. */
    ANCHOR_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_PixarVkInstance->GetVulkanInstance();
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Allocator = g_Allocator;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = m_vulkan_context->ImageCount;
    init_info.CheckVkResultFn = check_vk_result;
    ANCHOR_ImplVulkan_Init(&init_info, m_vulkan_context->RenderPass);

    /**
     * Create Pixar Hydra Graphics Interface. */
    HdDriver driver;
    HgiUniquePtr hgi = HgiUniquePtr(g_PixarHydra);
    driver.name = HgiTokens->renderDriver;
    driver.driver = VtValue(hgi.get());

    /**
     * Setup Pixar Driver & Engine. */
    ANCHOR::GetPixarDriver().name = driver.name;
    ANCHOR::GetPixarDriver().driver = driver.driver;

    SetFont();
    /**
     * Upload Fonts. */
    {
      /**
       * Use any command queue. */
      VkCommandPool command_pool = m_vulkan_context->Frames[m_vulkan_context->FrameIndex].CommandPool;
      VkCommandBuffer command_buffer = m_vulkan_context->Frames[m_vulkan_context->FrameIndex].CommandBuffer;

      VkResult err = vkResetCommandPool(g_Device, command_pool, 0);
      check_vk_result(err);
      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      err = vkBeginCommandBuffer(command_buffer, &begin_info);
      check_vk_result(err);

      ANCHOR_ImplVulkan_CreateFontsTexture(command_buffer);

      VkSubmitInfo end_info = {};
      end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      end_info.commandBufferCount = 1;
      end_info.pCommandBuffers = &command_buffer;
      err = vkEndCommandBuffer(command_buffer);
      check_vk_result(err);
      err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
      check_vk_result(err);

      err = vkDeviceWaitIdle(g_Device);
      check_vk_result(err);
      ANCHOR_ImplVulkan_DestroyFontUploadObjects();
    }
  }

  /**
   * TODO: DX12 Backend
   * - Compare with the Vulkan Implementation.
   * - Will need a new Hgi::HgiDXD12 */
}

eAnchorStatus ANCHOR_WindowWin32::swapBuffers()
{
  return ANCHOR_SUCCESS;
}

void ANCHOR_WindowWin32::screenToClient(AnchorS32 inX,
                                        AnchorS32 inY,
                                        AnchorS32 &outX,
                                        AnchorS32 &outY) const
{}

void ANCHOR_WindowWin32::setTitle(const char *title)
{
  wchar_t *title_16 = alloc_utf16_from_8((char *)title, 0);
  ::SetWindowTextW(m_hWnd, (wchar_t *)title_16);
  free(title_16);
}

void ANCHOR_WindowWin32::setIcon(const char *icon)
{}

void ANCHOR_WindowWin32::clientToScreen(AnchorS32 inX,
                                        AnchorS32 inY,
                                        AnchorS32 &outX,
                                        AnchorS32 &outY) const
{}

void ANCHOR_WindowWin32::lostMouseCapture()
{
  if (m_hasMouseCaptured)
  {
    m_hasGrabMouse = false;
    m_nPressedButtons = 0;
    m_hasMouseCaptured = false;
  }
}

void ANCHOR_WindowWin32::updateMouseCapture(eAnchorMouseCaptureEventWin32 event)
{
  switch (event)
  {
    case MousePressed:
      m_nPressedButtons++;
      break;
    case MouseReleased:
      if (m_nPressedButtons)
        m_nPressedButtons--;
      break;
    case OperatorGrab:
      m_hasGrabMouse = true;
      break;
    case OperatorUngrab:
      m_hasGrabMouse = false;
      break;
  }

  if (!m_nPressedButtons && !m_hasGrabMouse && m_hasMouseCaptured)
  {
    ::ReleaseCapture();
    m_hasMouseCaptured = false;
  }
  else if ((m_nPressedButtons || m_hasGrabMouse) && !m_hasMouseCaptured)
  {
    ::SetCapture(m_hWnd);
    m_hasMouseCaptured = true;
  }
}

HCURSOR ANCHOR_WindowWin32::getStandardCursor(eAnchorStandardCursor shape) const
{
  // Convert ANCHOR cursor to Windows OEM cursor
  HANDLE cursor = NULL;
  HMODULE module = ::GetModuleHandle(0);
  AnchorU32 flags = LR_SHARED | LR_DEFAULTSIZE;
  int cx = 0, cy = 0;

  switch (shape)
  {
    case ANCHOR_StandardCursorCustom:
      if (m_customCursor)
      {
        return m_customCursor;
      }
      else
      {
        return NULL;
      }
    case ANCHOR_StandardCursorRightArrow:
      cursor = ::LoadImage(module, "arrowright_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorLeftArrow:
      cursor = ::LoadImage(module, "arrowleft_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorUpArrow:
      cursor = ::LoadImage(module, "arrowup_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorDownArrow:
      cursor = ::LoadImage(module, "arrowdown_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorVerticalSplit:
      cursor = ::LoadImage(module, "splitv_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorHorizontalSplit:
      cursor = ::LoadImage(module, "splith_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorKnife:
      cursor = ::LoadImage(module, "knife_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorEyedropper:
      cursor = ::LoadImage(module, "eyedropper_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorZoomIn:
      cursor = ::LoadImage(module, "zoomin_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorZoomOut:
      cursor = ::LoadImage(module, "zoomout_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorMove:
      cursor = ::LoadImage(module, "handopen_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorNSEWScroll:
      cursor = ::LoadImage(module, "scrollnsew_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorNSScroll:
      cursor = ::LoadImage(module, "scrollns_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorEWScroll:
      cursor = ::LoadImage(module, "scrollew_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorHelp:
      cursor = ::LoadImage(NULL, IDC_HELP, IMAGE_CURSOR, cx, cy, flags);
      break;  // Arrow and question mark
    case ANCHOR_StandardCursorWait:
      cursor = ::LoadImage(NULL, IDC_WAIT, IMAGE_CURSOR, cx, cy, flags);
      break;  // Hourglass
    case ANCHOR_StandardCursorText:
      cursor = ::LoadImage(NULL, IDC_IBEAM, IMAGE_CURSOR, cx, cy, flags);
      break;  // I-beam
    case ANCHOR_StandardCursorCrosshair:
      cursor = ::LoadImage(module, "cross_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Standard Cross
    case ANCHOR_StandardCursorCrosshairA:
      cursor = ::LoadImage(module, "crossA_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Crosshair A
    case ANCHOR_StandardCursorCrosshairB:
      cursor = ::LoadImage(module, "crossB_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Diagonal Crosshair B
    case ANCHOR_StandardCursorCrosshairC:
      cursor = ::LoadImage(module, "crossC_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Minimal Crosshair C
    case ANCHOR_StandardCursorBottomSide:
    case ANCHOR_StandardCursorUpDown:
      cursor = ::LoadImage(module, "movens_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Double-pointed arrow pointing north and south
    case ANCHOR_StandardCursorLeftSide:
    case ANCHOR_StandardCursorLeftRight:
      cursor = ::LoadImage(module, "moveew_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Double-pointed arrow pointing west and east
    case ANCHOR_StandardCursorTopSide:
      cursor = ::LoadImage(NULL, IDC_UPARROW, IMAGE_CURSOR, cx, cy, flags);
      break;  // Vertical arrow
    case ANCHOR_StandardCursorTopLeftCorner:
      cursor = ::LoadImage(NULL, IDC_SIZENWSE, IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorTopRightCorner:
      cursor = ::LoadImage(NULL, IDC_SIZENESW, IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorBottomRightCorner:
      cursor = ::LoadImage(NULL, IDC_SIZENWSE, IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorBottomLeftCorner:
      cursor = ::LoadImage(NULL, IDC_SIZENESW, IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorPencil:
      cursor = ::LoadImage(module, "pencil_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorEraser:
      cursor = ::LoadImage(module, "eraser_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;
    case ANCHOR_StandardCursorDestroy:
    case ANCHOR_StandardCursorStop:
      cursor = ::LoadImage(module, "forbidden_cursor", IMAGE_CURSOR, cx, cy, flags);
      break;  // Slashed circle
    case ANCHOR_StandardCursorDefault:
      cursor = NULL;
      break;
    default:
      return NULL;
  }

  if (cursor == NULL)
  {
    cursor = ::LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, cx, cy, flags);
  }

  return (HCURSOR)cursor;
}

eAnchorStatus ANCHOR_WindowWin32::setWindowCursorShape(eAnchorStandardCursor cursorShape)
{
  if (::GetForegroundWindow() == m_hWnd)
  {
    loadCursor(getCursorVisibility(), cursorShape);
  }

  return ANCHOR_SUCCESS;
}

void ANCHOR_WindowWin32::loadCursor(bool visible, eAnchorStandardCursor shape) const
{
  if (!visible)
  {
    while (::ShowCursor(FALSE) >= 0)
      ;
  }
  else
  {
    while (::ShowCursor(TRUE) < 0)
      ;
  }

  HCURSOR cursor = getStandardCursor(shape);
  if (cursor == NULL)
  {
    cursor = getStandardCursor(ANCHOR_StandardCursorDefault);
  }
  ::SetCursor(cursor);
}

eAnchorStatus ANCHOR_WindowWin32::setClientSize(AnchorU32 width, AnchorU32 height)
{
  return ANCHOR_SUCCESS;
}

eAnchorStatus ANCHOR_WindowWin32::setState(eAnchorWindowState state)
{
  return ANCHOR_SUCCESS;
}

eAnchorWindowState ANCHOR_WindowWin32::getState() const
{
  return ANCHOR_WindowStateFullScreen;
}

AnchorU16 ANCHOR_WindowWin32::getDPIHint()
{
  return 100;
}
