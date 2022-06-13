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
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include "ANCHOR_api.h"

// System includes
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_MSC_VER) && _MSC_VER <= 1500
#  include <stddef.h>
#else
#  include <stdint.h>
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#  pragma warning(disable : 4996)
#  pragma warning(disable : 26451)
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#  if __has_warning("-Wunknown-warning-option")
#    pragma clang diagnostic ignored "-Wunknown-warning-option"
#  endif
#  pragma clang diagnostic ignored "-Wunknown-pragmas"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
#  pragma clang diagnostic ignored "-Wformat-security"
#  pragma clang diagnostic ignored "-Wexit-time-destructors"
#  pragma clang diagnostic ignored "-Wunused-macros"
#  pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#  pragma clang diagnostic ignored "-Wdouble-promotion"
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#  pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wpragmas"
#  pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#  pragma GCC diagnostic ignored "-Wformat-security"
#  pragma GCC diagnostic ignored "-Wdouble-promotion"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif

#ifdef _WIN32
#  define ANCHOR_NEWLINE "\r\n"
#else
#  define ANCHOR_NEWLINE "\n"
#endif

#if defined(_MSC_VER) && !defined(snprintf)
#  define snprintf _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#  define vsnprintf _vsnprintf
#endif

#ifdef _MSC_VER
#  define IM_PRId64 "I64d"
#  define IM_PRIu64 "I64u"
#else
#  define IM_PRId64 "lld"
#  define IM_PRIu64 "llu"
#endif

#define ANCHOR_MIN(A, B) (((A) < (B)) ? (A) : (B))
#define ANCHOR_MAX(A, B) (((A) >= (B)) ? (A) : (B))
#define ANCHOR_CLAMP(V, MN, MX) ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

#ifndef ANCHOR_CDECL
#  ifdef _MSC_VER
#    define ANCHOR_CDECL __cdecl
#  else
#    define ANCHOR_CDECL
#  endif
#endif

WABI_NAMESPACE_USING

// Forward Declarations
static void ShowExampleAppDocuments(bool *p_open);
static void ShowExampleAppMainMenuBar();
static void ShowExampleAppConsole(bool *p_open);
static void ShowExampleAppLog(bool *p_open);
static void ShowExampleAppLayout(bool *p_open);
static void ShowExampleAppPropertyEditor(bool *p_open);
static void ShowExampleAppLongText(bool *p_open);
static void ShowExampleAppAutoResize(bool *p_open);
static void ShowExampleAppConstrainedResize(bool *p_open);
static void ShowExampleAppSimpleOverlay(bool *p_open);
static void ShowExampleAppFullscreen(bool *p_open);
static void ShowExampleAppWindowTitles(bool *p_open);
static void ShowExampleAppCustomRendering(bool *p_open);
static void ShowExampleMenuFile();

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts
// (see docs/FONTS.md)
static void HelpMarker(const char *desc)
{
  ANCHOR::TextDisabled("(?)");
  if (ANCHOR::IsItemHovered()) {
    ANCHOR::BeginTooltip();
    ANCHOR::PushTextWrapPos(ANCHOR::GetFontSize() * 35.0f);
    ANCHOR::TextUnformatted(desc);
    ANCHOR::PopTextWrapPos();
    ANCHOR::EndTooltip();
  }
}

// Helper to display basic user controls.
void ANCHOR::ShowUserGuide()
{
  AnchorIO &io = ANCHOR::GetIO();
  ANCHOR::BulletText("Double-click on title bar to collapse window.");
  ANCHOR::BulletText(
    "Click and drag on lower corner to resize window\n"
    "(double-click to auto fit window to its contents).");
  ANCHOR::BulletText("CTRL+Click on a slider or drag box to input value as text.");
  ANCHOR::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
  if (io.FontAllowUserScaling)
    ANCHOR::BulletText("CTRL+Mouse Wheel to zoom window contents.");
  ANCHOR::BulletText("While inputing text:\n");
  ANCHOR::Indent();
  ANCHOR::BulletText("CTRL+Left/Right to word jump.");
  ANCHOR::BulletText("CTRL+A or double-click to select all.");
  ANCHOR::BulletText("CTRL+X/C/V to use clipboard cut/copy/paste.");
  ANCHOR::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
  ANCHOR::BulletText("ESCAPE to revert.");
  ANCHOR::BulletText(
    "You can apply arithmetic operators +,*,/ on numerical values.\nUse +- to subtract.");
  ANCHOR::Unindent();
  ANCHOR::BulletText("With keyboard navigation enabled:");
  ANCHOR::Indent();
  ANCHOR::BulletText("Arrow keys to navigate.");
  ANCHOR::BulletText("Space to activate a widget.");
  ANCHOR::BulletText("Return to input text into a widget.");
  ANCHOR::BulletText("Escape to deactivate a widget, close popup, exit child window.");
  ANCHOR::BulletText("Alt to jump to the menu layer of a window.");
  ANCHOR::BulletText("CTRL+Tab to select a window.");
  ANCHOR::Unindent();
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------
// - ShowDemoWindowWidgets()
// - ShowDemoWindowLayout()
// - ShowDemoWindowPopups()
// - ShowDemoWindowTables()
// - ShowDemoWindowColumns()
// - ShowDemoWindowMisc()
//-----------------------------------------------------------------------------

// We split the contents of the big ShowDemoWindow() function into smaller functions
// (because the link time of very large functions grow non-linearly)
static void ShowDemoWindowWidgets();
static void ShowDemoWindowLayout();
static void ShowDemoWindowPopups();
static void ShowDemoWindowTables();
static void ShowDemoWindowColumns();
static void ShowDemoWindowMisc();

// Demonstrate most ANCHOR features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does.
// You may then search for keywords in the code when you are interested by a specific feature.
void ANCHOR::ShowDemoWindow(bool *p_open)
{
  // Exceptionally add an extra assert here for people confused about initial ANCHOR setup
  // Most ANCHOR functions would normally just crash if the context is missing.
  ANCHOR_ASSERT(ANCHOR::GetCurrentContext() != NULL &&
                "Missing ANCHOR context. Refer to examples app!");

  // Examples Apps (accessible from the "Examples" menu)
  static bool show_app_main_menu_bar = false;
  static bool show_app_documents = false;

  static bool show_app_console = false;
  static bool show_app_log = false;
  static bool show_app_layout = false;
  static bool show_app_property_editor = false;
  static bool show_app_long_text = false;
  static bool show_app_auto_resize = false;
  static bool show_app_constrained_resize = false;
  static bool show_app_simple_overlay = false;
  static bool show_app_fullscreen = false;
  static bool show_app_window_titles = false;
  static bool show_app_custom_rendering = false;

  if (show_app_main_menu_bar)
    ShowExampleAppMainMenuBar();
  if (show_app_documents)
    ShowExampleAppDocuments(&show_app_documents);

  if (show_app_console)
    ShowExampleAppConsole(&show_app_console);
  if (show_app_log)
    ShowExampleAppLog(&show_app_log);
  if (show_app_layout)
    ShowExampleAppLayout(&show_app_layout);
  if (show_app_property_editor)
    ShowExampleAppPropertyEditor(&show_app_property_editor);
  if (show_app_long_text)
    ShowExampleAppLongText(&show_app_long_text);
  if (show_app_auto_resize)
    ShowExampleAppAutoResize(&show_app_auto_resize);
  if (show_app_constrained_resize)
    ShowExampleAppConstrainedResize(&show_app_constrained_resize);
  if (show_app_simple_overlay)
    ShowExampleAppSimpleOverlay(&show_app_simple_overlay);
  if (show_app_fullscreen)
    ShowExampleAppFullscreen(&show_app_fullscreen);
  if (show_app_window_titles)
    ShowExampleAppWindowTitles(&show_app_window_titles);
  if (show_app_custom_rendering)
    ShowExampleAppCustomRendering(&show_app_custom_rendering);

  // ANCHOR Apps (accessible from the "Tools" menu)
  static bool show_app_metrics = false;
  static bool show_app_style_editor = false;
  static bool show_app_about = false;

  if (show_app_metrics) {
    ANCHOR::ShowMetricsWindow(&show_app_metrics);
  }
  if (show_app_about) {
    ANCHOR::ShowAboutWindow(&show_app_about);
  }
  if (show_app_style_editor) {
    ANCHOR::Begin("ANCHOR Style Editor", &show_app_style_editor);
    ANCHOR::ShowStyleEditor();
    ANCHOR::End();
  }

  // Demonstrate the various window flags. Typically you would just use the default!
  static bool no_titlebar = false;
  static bool no_scrollbar = false;
  static bool no_menu = false;
  static bool no_move = false;
  static bool no_resize = false;
  static bool no_collapse = false;
  static bool no_close = false;
  static bool no_nav = false;
  static bool no_background = false;
  static bool no_bring_to_front = false;

  AnchorWindowFlags window_flags = 0;
  if (no_titlebar)
    window_flags |= AnchorWindowFlags_NoTitleBar;
  if (no_scrollbar)
    window_flags |= AnchorWindowFlags_NoScrollbar;
  if (!no_menu)
    window_flags |= AnchorWindowFlags_MenuBar;
  if (no_move)
    window_flags |= AnchorWindowFlags_NoMove;
  if (no_resize)
    window_flags |= AnchorWindowFlags_NoResize;
  if (no_collapse)
    window_flags |= AnchorWindowFlags_NoCollapse;
  if (no_nav)
    window_flags |= AnchorWindowFlags_NoNav;
  if (no_background)
    window_flags |= AnchorWindowFlags_NoBackground;
  if (no_bring_to_front)
    window_flags |= AnchorWindowFlags_NoBringToFrontOnFocus;
  if (no_close)
    p_open = NULL;  // Don't pass our bool* to Begin

  // We specify a default position/size in case there's no data in the .ini file.
  // We only do it to make the demo applications a little more welcoming, but typically this isn't
  // required.
  const AnchorViewport *main_viewport = ANCHOR::GetMainViewport();
  ANCHOR::SetNextWindowPos(
    GfVec2f(main_viewport->WorkPos[0] + 650, main_viewport->WorkPos[1] + 20),
    AnchorCond_FirstUseEver);
  ANCHOR::SetNextWindowSize(GfVec2f(550, 680), AnchorCond_FirstUseEver);

  // Main body of the Demo window starts here.
  if (!ANCHOR::Begin("Kraken on Vulkan", p_open, window_flags)) {
    // Early out if the window is collapsed, as an optimization.
    ANCHOR::End();
    return;
  }

  // Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width'
  // for details.

  // e.g. Use 2/3 of the space for widgets and 1/3 for labels (right align)
  // ANCHOR::PushItemWidth(-ANCHOR::GetWindowWidth() * 0.35f);

  // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to
  // widgets.
  ANCHOR::PushItemWidth(ANCHOR::GetFontSize() * -12);

  // Menu Bar
  if (ANCHOR::BeginMenuBar()) {
    if (ANCHOR::BeginMenu("Menu")) {
      ShowExampleMenuFile();
      ANCHOR::EndMenu();
    }
    if (ANCHOR::BeginMenu("Examples")) {
      ANCHOR::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
      ANCHOR::MenuItem("Console", NULL, &show_app_console);
      ANCHOR::MenuItem("Log", NULL, &show_app_log);
      ANCHOR::MenuItem("Simple layout", NULL, &show_app_layout);
      ANCHOR::MenuItem("Property editor", NULL, &show_app_property_editor);
      ANCHOR::MenuItem("Long text display", NULL, &show_app_long_text);
      ANCHOR::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
      ANCHOR::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
      ANCHOR::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
      ANCHOR::MenuItem("Fullscreen window", NULL, &show_app_fullscreen);
      ANCHOR::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
      ANCHOR::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
      ANCHOR::MenuItem("Documents", NULL, &show_app_documents);
      ANCHOR::EndMenu();
    }
    if (ANCHOR::BeginMenu("Tools")) {
      ANCHOR::MenuItem("Metrics/Debugger", NULL, &show_app_metrics);
      ANCHOR::MenuItem("Style Editor", NULL, &show_app_style_editor);
      ANCHOR::MenuItem("About ANCHOR", NULL, &show_app_about);
      ANCHOR::EndMenu();
    }
    ANCHOR::EndMenuBar();
  }

  ANCHOR::Text("ANCHOR says hello. (%s)", ANCHOR_VERSION);
  ANCHOR::Spacing();

  if (ANCHOR::CollapsingHeader("Help")) {
    ANCHOR::Text("ABOUT THIS DEMO:");
    ANCHOR::BulletText("Sections below are demonstrating many aspects of the library.");
    ANCHOR::BulletText("The \"Examples\" menu above leads to more demo contents.");
    ANCHOR::BulletText(
      "The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
      "and Metrics/Debugger (general purpose ANCHOR debugging tool).");
    ANCHOR::Separator();

    ANCHOR::Text("PROGRAMMER GUIDE:");
    ANCHOR::BulletText("See the ShowDemoWindow() code in ANCHOR_demo.cpp. <- you are here!");
    ANCHOR::BulletText("See comments in ANCHOR.cpp.");
    ANCHOR::BulletText("See example applications in the examples/ folder.");
    ANCHOR::BulletText("Read the FAQ at http://www.dearANCHOR.org/faq/");
    ANCHOR::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
    ANCHOR::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
    ANCHOR::Separator();

    ANCHOR::Text("USER GUIDE:");
    ANCHOR::ShowUserGuide();
  }

  if (ANCHOR::CollapsingHeader("Configuration")) {
    AnchorIO &io = ANCHOR::GetIO();

    if (ANCHOR::TreeNode("Configuration##2")) {
      ANCHOR::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard",
                            &io.ConfigFlags,
                            AnchorConfigFlags_NavEnableKeyboard);
      ANCHOR::SameLine();
      HelpMarker("Enable keyboard controls.");
      ANCHOR::CheckboxFlags("io.ConfigFlags: NavEnableGamepad",
                            &io.ConfigFlags,
                            AnchorConfigFlags_NavEnableGamepad);
      ANCHOR::SameLine();
      HelpMarker(
        "Enable gamepad controls. Require backend to set io.BackendFlags |= "
        "AnchorBackendFlags_HasGamepad.\n\nRead instructions in ANCHOR.cpp for details.");
      ANCHOR::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos",
                            &io.ConfigFlags,
                            AnchorConfigFlags_NavEnableSetMousePos);
      ANCHOR::SameLine();
      HelpMarker(
        "Instruct navigation to move the mouse cursor. See comment for "
        "AnchorConfigFlags_NavEnableSetMousePos.");
      ANCHOR::CheckboxFlags("io.ConfigFlags: NoMouse", &io.ConfigFlags, AnchorConfigFlags_NoMouse);
      if (io.ConfigFlags & AnchorConfigFlags_NoMouse) {
        // The "NoMouse" option can get us stuck with a disabled mouse! Let's provide an
        // alternative way to fix it:
        if (fmodf((float)ANCHOR::GetTime(), 0.40f) < 0.20f) {
          ANCHOR::SameLine();
          ANCHOR::Text("<<PRESS SPACE TO DISABLE>>");
        }
        if (ANCHOR::IsKeyPressed(ANCHOR::GetKeyIndex(AnchorKey_Space)))
          io.ConfigFlags &= ~AnchorConfigFlags_NoMouse;
      }
      ANCHOR::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange",
                            &io.ConfigFlags,
                            AnchorConfigFlags_NoMouseCursorChange);
      ANCHOR::SameLine();
      HelpMarker("Instruct backend to not alter mouse cursor shape and visibility.");
      ANCHOR::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
      ANCHOR::SameLine();
      HelpMarker("Enable blinking cursor (optional as some users consider it to be distracting)");
      ANCHOR::Checkbox("io.ConfigDragClickToInputText", &io.ConfigDragClickToInputText);
      ANCHOR::SameLine();
      HelpMarker(
        "Enable turning DragXXX widgets into text input with a simple mouse click-release "
        "(without moving).");
      ANCHOR::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
      ANCHOR::SameLine();
      HelpMarker(
        "Enable resizing of windows from their edges and from the lower-left corner.\nThis "
        "requires (io.BackendFlags & AnchorBackendFlags_HasMouseCursors) because it needs mouse "
        "cursor feedback.");
      ANCHOR::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly",
                       &io.ConfigWindowsMoveFromTitleBarOnly);
      ANCHOR::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
      ANCHOR::SameLine();
      HelpMarker(
        "Instruct ANCHOR to render a mouse cursor itself. Note that a mouse cursor rendered via "
        "your application GPU rendering path will feel more laggy than hardware cursor, but "
        "will be more in sync with your other visuals.\n\nSome desktop applications may use "
        "both kinds of cursors (e.g. enable software cursor only when resizing/dragging "
        "something).");
      ANCHOR::Text("Also see Style->Rendering for rendering options.");
      ANCHOR::TreePop();
      ANCHOR::Separator();
    }

    if (ANCHOR::TreeNode("Backend Flags")) {
      HelpMarker(
        "Those flags are set by the backends (ANCHOR_impl_xxx files) to specify their "
        "capabilities.\n"
        "Here we expose then as read-only fields to avoid breaking interactions with your "
        "backend.");

      // Make a local copy to avoid modifying actual backend flags.
      AnchorBackendFlags backend_flags = io.BackendFlags;
      ANCHOR::CheckboxFlags("io.BackendFlags: HasGamepad",
                            &backend_flags,
                            AnchorBackendFlags_HasGamepad);
      ANCHOR::CheckboxFlags("io.BackendFlags: HasMouseCursors",
                            &backend_flags,
                            AnchorBackendFlags_HasMouseCursors);
      ANCHOR::CheckboxFlags("io.BackendFlags: HasSetMousePos",
                            &backend_flags,
                            AnchorBackendFlags_HasSetMousePos);
      ANCHOR::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset",
                            &backend_flags,
                            AnchorBackendFlags_RendererHasVtxOffset);
      ANCHOR::TreePop();
      ANCHOR::Separator();
    }

    if (ANCHOR::TreeNode("Style")) {
      HelpMarker(
        "The same contents can be accessed in 'Tools->Style Editor' or by calling the "
        "ShowStyleEditor() function.");
      ANCHOR::ShowStyleEditor();
      ANCHOR::TreePop();
      ANCHOR::Separator();
    }

    if (ANCHOR::TreeNode("Capture/Logging")) {
      HelpMarker(
        "The logging API redirects all text output so you can easily capture the content of "
        "a window or a block. Tree nodes can be automatically expanded.\n"
        "Try opening any of the contents below in this window and then click one of the \"Log "
        "To\" button.");
      ANCHOR::LogButtons();

      HelpMarker(
        "You can also call ANCHOR::LogText() to output directly to the log without a visual "
        "output.");
      if (ANCHOR::Button("Copy \"Hello, world!\" to clipboard")) {
        ANCHOR::LogToClipboard();
        ANCHOR::LogText("Hello, world!");
        ANCHOR::LogFinish();
      }
      ANCHOR::TreePop();
    }
  }

  if (ANCHOR::CollapsingHeader("Window options")) {
    if (ANCHOR::BeginTable("split", 3)) {
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No titlebar", &no_titlebar);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No scrollbar", &no_scrollbar);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No menu", &no_menu);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No move", &no_move);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No resize", &no_resize);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No collapse", &no_collapse);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No close", &no_close);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No nav", &no_nav);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No background", &no_background);
      ANCHOR::TableNextColumn();
      ANCHOR::Checkbox("No bring to front", &no_bring_to_front);
      ANCHOR::EndTable();
    }
  }

  // All demo contents
  ShowDemoWindowWidgets();
  ShowDemoWindowLayout();
  ShowDemoWindowPopups();
  ShowDemoWindowTables();
  ShowDemoWindowMisc();

  // End of ShowDemoWindow()
  ANCHOR::PopItemWidth();
  ANCHOR::End();
}

static void ShowDemoWindowWidgets()
{
  if (!ANCHOR::CollapsingHeader("Widgets"))
    return;

  if (ANCHOR::TreeNode("Basic")) {
    static int clicked = 0;
    if (ANCHOR::Button("Button"))
      clicked++;
    if (clicked & 1) {
      ANCHOR::SameLine();
      ANCHOR::Text("Thanks for clicking me!");
    }

    static bool check = true;
    ANCHOR::Checkbox("checkbox", &check);

    static int e = 0;
    ANCHOR::RadioButton("radio a", &e, 0);
    ANCHOR::SameLine();
    ANCHOR::RadioButton("radio b", &e, 1);
    ANCHOR::SameLine();
    ANCHOR::RadioButton("radio c", &e, 2);

    // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and
    // changing style.
    for (int i = 0; i < 7; i++) {
      if (i > 0)
        ANCHOR::SameLine();
      ANCHOR::PushID(i);
      ANCHOR::PushStyleColor(AnchorCol_Button, AnchorColor::HSV(i / 7.0f, 0.6f, 0.6f).Value);
      ANCHOR::PushStyleColor(AnchorCol_ButtonHovered,
                             AnchorColor::HSV(i / 7.0f, 0.7f, 0.7f).Value);
      ANCHOR::PushStyleColor(AnchorCol_ButtonActive, AnchorColor::HSV(i / 7.0f, 0.8f, 0.8f).Value);
      ANCHOR::Button("Click");
      ANCHOR::PopStyleColor(3);
      ANCHOR::PopID();
    }

    // Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets
    // elements (otherwise a Text+SameLine+Button sequence will have the text a little too high by
    // default!) See 'Demo->Layout->Text Baseline Alignment' for details.
    ANCHOR::AlignTextToFramePadding();
    ANCHOR::Text("Hold to repeat:");
    ANCHOR::SameLine();

    // Arrow buttons with Repeater
    static int counter = 0;
    float spacing = ANCHOR::GetStyle().ItemInnerSpacing[0];
    ANCHOR::PushButtonRepeat(true);
    if (ANCHOR::ArrowButton("##left", AnchorDir_Left)) {
      counter--;
    }
    ANCHOR::SameLine(0.0f, spacing);
    if (ANCHOR::ArrowButton("##right", AnchorDir_Right)) {
      counter++;
    }
    ANCHOR::PopButtonRepeat();
    ANCHOR::SameLine();
    ANCHOR::Text("%d", counter);

    ANCHOR::Text("Hover over me");
    if (ANCHOR::IsItemHovered())
      ANCHOR::SetTooltip("I am a tooltip");

    ANCHOR::SameLine();
    ANCHOR::Text("- or me");
    if (ANCHOR::IsItemHovered()) {
      ANCHOR::BeginTooltip();
      ANCHOR::Text("I am a fancy tooltip");
      static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
      ANCHOR::PlotLines("Curve", arr, ANCHOR_ARRAYSIZE(arr));
      ANCHOR::EndTooltip();
    }

    ANCHOR::Separator();

    ANCHOR::LabelText("label", "Value");

    {
      // Using the _simplified_ one-liner Combo() api here
      // See "Combo" section for examples of how to use the more flexible BeginCombo()/EndCombo()
      // api.
      const char *items[] = {"AAAA",
                             "BBBB",
                             "CCCC",
                             "DDDD",
                             "EEEE",
                             "FFFF",
                             "GGGG",
                             "HHHH",
                             "IIIIIII",
                             "JJJJ",
                             "KKKKKKK"};
      static int item_current = 0;
      ANCHOR::Combo("combo", &item_current, items, ANCHOR_ARRAYSIZE(items));
      ANCHOR::SameLine();
      HelpMarker(
        "Using the simplified one-liner Combo API here.\nRefer to the \"Combo\" section below "
        "for an explanation of how to use the more flexible and general BeginCombo/EndCombo "
        "API.");
    }

    {
      // To wire InputText() with std::string or any other custom string type,
      // see the "Text Input > Resize Callback" section of this demo, and the
      // misc/cpp/ANCHOR_stdlib.h file.
      static char str0[128] = "Hello, world!";
      ANCHOR::InputText("input text", str0, ANCHOR_ARRAYSIZE(str0));
      ANCHOR::SameLine();
      HelpMarker(
        "USER:\n"
        "Hold SHIFT or use mouse to select text.\n"
        "CTRL+Left/Right to word jump.\n"
        "CTRL+A or double-click to select all.\n"
        "CTRL+X,CTRL+C,CTRL+V clipboard.\n"
        "CTRL+Z,CTRL+Y undo/redo.\n"
        "ESCAPE to revert.\n\n"
        "PROGRAMMER:\n"
        "You can use the AnchorInputTextFlags_CallbackResize facility if you need to wire "
        "InputText() "
        "to a dynamic string type. See misc/cpp/ANCHOR_stdlib.h for an example (this is not "
        "demonstrated "
        "in ANCHOR_demo.cpp).");

      static char str1[128] = "";
      ANCHOR::InputTextWithHint("input text (w/ hint)",
                                "enter text here",
                                str1,
                                ANCHOR_ARRAYSIZE(str1));

      static int i0 = 123;
      ANCHOR::InputInt("input int", &i0);
      ANCHOR::SameLine();
      HelpMarker(
        "You can apply arithmetic operators +,*,/ on numerical values.\n"
        "  e.g. [ 100 ], input \'*2\', result becomes [ 200 ]\n"
        "Use +- to subtract.");

      static float f0 = 0.001f;
      ANCHOR::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

      static double d0 = 999999.00000001;
      ANCHOR::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

      static float f1 = 1.e10f;
      ANCHOR::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");
      ANCHOR::SameLine();
      HelpMarker(
        "You can input value using the scientific notation,\n"
        "  e.g. \"1e+8\" becomes \"100000000\".");

      static float vec4a[4] = {0.10f, 0.20f, 0.30f, 0.44f};
      ANCHOR::InputFloat3("input float3", vec4a);
    }

    {
      static int i1 = 50, i2 = 42;
      ANCHOR::DragInt("drag int", &i1, 1);
      ANCHOR::SameLine();
      HelpMarker(
        "Click and drag to edit value.\n"
        "Hold SHIFT/ALT for faster/slower edit.\n"
        "Double-click or CTRL+click to input value.");

      ANCHOR::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%", AnchorSliderFlags_AlwaysClamp);

      static float f1 = 1.00f, f2 = 0.0067f;
      ANCHOR::DragFloat("drag float", &f1, 0.005f);
      ANCHOR::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
    }

    {
      static int i1 = 0;
      ANCHOR::SliderInt("slider int", &i1, -1, 3);
      ANCHOR::SameLine();
      HelpMarker("CTRL+click to input value.");

      static float f1 = 0.123f, f2 = 0.0f;
      ANCHOR::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
      ANCHOR::SliderFloat("slider float (log)",
                          &f2,
                          -10.0f,
                          10.0f,
                          "%.4f",
                          AnchorSliderFlags_Logarithmic);

      static float angle = 0.0f;
      ANCHOR::SliderAngle("slider angle", &angle);

      // Using the format string to display a name instead of an integer.
      // Here we completely omit '%d' from the format string, so it'll only display a name.
      // This technique can also be used with DragInt().
      enum Element
      {
        Element_Fire,
        Element_Earth,
        Element_Air,
        Element_Water,
        Element_COUNT
      };
      static int elem = Element_Fire;
      const char *elems_names[Element_COUNT] = {"Fire", "Earth", "Air", "Water"};
      const char *elem_name = (elem >= 0 && elem < Element_COUNT) ? elems_names[elem] : "Unknown";
      ANCHOR::SliderInt("slider enum", &elem, 0, Element_COUNT - 1, elem_name);
      ANCHOR::SameLine();
      HelpMarker(
        "Using the format string parameter to display a name instead of the underlying "
        "integer.");
    }

    {
      static float col1[3] = {1.0f, 0.0f, 0.2f};
      static float col2[4] = {0.4f, 0.7f, 0.0f, 0.5f};
      ANCHOR::ColorEdit3("color 1", col1);
      ANCHOR::SameLine();
      HelpMarker(
        "Click on the color square to open a color picker.\n"
        "Click and hold to use drag and drop.\n"
        "Right-click on the color square to show options.\n"
        "CTRL+click on individual component to input value.\n");

      ANCHOR::ColorEdit4("color 2", col2);
    }

    {
      // Using the _simplified_ one-liner ListBox() api here
      // See "List boxes" section for examples of how to use the more flexible
      // BeginListBox()/EndListBox() api.
      const char *items[] = {"Apple",
                             "Banana",
                             "Cherry",
                             "Kiwi",
                             "Mango",
                             "Orange",
                             "Pineapple",
                             "Strawberry",
                             "Watermelon"};
      static int item_current = 1;
      ANCHOR::ListBox("listbox", &item_current, items, ANCHOR_ARRAYSIZE(items), 4);
      ANCHOR::SameLine();
      HelpMarker(
        "Using the simplified one-liner ListBox API here.\nRefer to the \"List boxes\" section "
        "below for an explanation of how to use the more flexible and general "
        "BeginListBox/EndListBox API.");
    }

    ANCHOR::TreePop();
  }

  // Testing ANCHOROnceUponAFrame helper.
  // static ANCHOROnceUponAFrame once;
  // for (int i = 0; i < 5; i++)
  //    if (once)
  //        ANCHOR::Text("This will be displayed only once.");

  if (ANCHOR::TreeNode("Trees")) {
    if (ANCHOR::TreeNode("Basic trees")) {
      for (int i = 0; i < 5; i++) {
        // Use SetNextItemOpen() so set the default state of a node to be open. We could
        // also use TreeNodeEx() with the AnchorTreeNodeFlags_DefaultOpen flag to achieve the same
        // thing!
        if (i == 0)
          ANCHOR::SetNextItemOpen(true, AnchorCond_Once);

        if (ANCHOR::TreeNode((void *)(intptr_t)i, "Child %d", i)) {
          ANCHOR::Text("blah blah");
          ANCHOR::SameLine();
          if (ANCHOR::SmallButton("button")) {
          }
          ANCHOR::TreePop();
        }
      }
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Advanced, with Selectable nodes")) {
      HelpMarker(
        "This is a more typical looking tree with selectable nodes.\n"
        "Click to select, CTRL+Click to toggle, click on arrows or double-click to open.");
      static AnchorTreeNodeFlags base_flags = AnchorTreeNodeFlags_OpenOnArrow |
                                              AnchorTreeNodeFlags_OpenOnDoubleClick |
                                              AnchorTreeNodeFlags_SpanAvailWidth;
      static bool align_label_with_current_x_position = false;
      static bool test_drag_and_drop = false;
      ANCHOR::CheckboxFlags("AnchorTreeNodeFlags_OpenOnArrow",
                            &base_flags,
                            AnchorTreeNodeFlags_OpenOnArrow);
      ANCHOR::CheckboxFlags("AnchorTreeNodeFlags_OpenOnDoubleClick",
                            &base_flags,
                            AnchorTreeNodeFlags_OpenOnDoubleClick);
      ANCHOR::CheckboxFlags("AnchorTreeNodeFlags_SpanAvailWidth",
                            &base_flags,
                            AnchorTreeNodeFlags_SpanAvailWidth);
      ANCHOR::SameLine();
      HelpMarker(
        "Extend hit area to all available width instead of allowing more items to be laid out "
        "after the node.");
      ANCHOR::CheckboxFlags("AnchorTreeNodeFlags_SpanFullWidth",
                            &base_flags,
                            AnchorTreeNodeFlags_SpanFullWidth);
      ANCHOR::Checkbox("Align label with current X position",
                       &align_label_with_current_x_position);
      ANCHOR::Checkbox("Test tree node as drag source", &test_drag_and_drop);
      ANCHOR::Text("Hello!");
      if (align_label_with_current_x_position)
        ANCHOR::Unindent(ANCHOR::GetTreeNodeToLabelSpacing());

      // 'selection_mask' is dumb representation of what may be user-side selection state.
      //  You may retain selection state inside or outside your objects in whatever format you see
      //  fit.
      // 'node_clicked' is temporary storage of what node we have clicked to process selection at
      // the end
      /// of the loop. May be a pointer to your own node type, etc.
      static int selection_mask = (1 << 2);
      int node_clicked = -1;
      for (int i = 0; i < 6; i++) {
        // Disable the default "open on single-click behavior" + set Selected flag according to our
        // selection.
        AnchorTreeNodeFlags node_flags = base_flags;
        const bool is_selected = (selection_mask & (1 << i)) != 0;
        if (is_selected)
          node_flags |= AnchorTreeNodeFlags_Selected;
        if (i < 3) {
          // Items 0..2 are Tree Node
          bool node_open = ANCHOR::TreeNodeEx((void *)(intptr_t)i,
                                              node_flags,
                                              "Selectable Node %d",
                                              i);
          if (ANCHOR::IsItemClicked())
            node_clicked = i;
          if (test_drag_and_drop && ANCHOR::BeginDragDropSource()) {
            ANCHOR::SetDragDropPayload("_TREENODE", NULL, 0);
            ANCHOR::Text("This is a drag and drop source");
            ANCHOR::EndDragDropSource();
          }
          if (node_open) {
            ANCHOR::BulletText("Blah blah\nBlah Blah");
            ANCHOR::TreePop();
          }
        } else {
          // Items 3..5 are Tree Leaves
          // The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we
          // can use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call
          // Text().
          node_flags |= AnchorTreeNodeFlags_Leaf |
                        AnchorTreeNodeFlags_NoTreePushOnOpen;  // AnchorTreeNodeFlags_Bullet
          ANCHOR::TreeNodeEx((void *)(intptr_t)i, node_flags, "Selectable Leaf %d", i);
          if (ANCHOR::IsItemClicked())
            node_clicked = i;
          if (test_drag_and_drop && ANCHOR::BeginDragDropSource()) {
            ANCHOR::SetDragDropPayload("_TREENODE", NULL, 0);
            ANCHOR::Text("This is a drag and drop source");
            ANCHOR::EndDragDropSource();
          }
        }
      }
      if (node_clicked != -1) {
        // Update selection state
        // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
        if (ANCHOR::GetIO().KeyCtrl)
          selection_mask ^= (1 << node_clicked);  // CTRL+click to toggle
        else  // if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior
              // you want, may want to preserve selection when clicking on item that is part of the
              // selection
          selection_mask = (1 << node_clicked);  // Click to single-select
      }
      if (align_label_with_current_x_position)
        ANCHOR::Indent(ANCHOR::GetTreeNodeToLabelSpacing());
      ANCHOR::TreePop();
    }
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Collapsing Headers")) {
    static bool closable_group = true;
    ANCHOR::Checkbox("Show 2nd header", &closable_group);
    if (ANCHOR::CollapsingHeader("Header", AnchorTreeNodeFlags_None)) {
      ANCHOR::Text("IsItemHovered: %d", ANCHOR::IsItemHovered());
      for (int i = 0; i < 5; i++)
        ANCHOR::Text("Some content %d", i);
    }
    if (ANCHOR::CollapsingHeader("Header with a close button", &closable_group)) {
      ANCHOR::Text("IsItemHovered: %d", ANCHOR::IsItemHovered());
      for (int i = 0; i < 5; i++)
        ANCHOR::Text("More content %d", i);
    }
    /*
    if (ANCHOR::CollapsingHeader("Header with a bullet", AnchorTreeNodeFlags_Bullet))
        ANCHOR::Text("IsItemHovered: %d", ANCHOR::IsItemHovered());
    */
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Bullets")) {
    ANCHOR::BulletText("Bullet point 1");
    ANCHOR::BulletText("Bullet point 2\nOn multiple lines");
    if (ANCHOR::TreeNode("Tree node")) {
      ANCHOR::BulletText("Another bullet point");
      ANCHOR::TreePop();
    }
    ANCHOR::Bullet();
    ANCHOR::Text("Bullet point 3 (two calls)");
    ANCHOR::Bullet();
    ANCHOR::SmallButton("Button");
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Text")) {
    if (ANCHOR::TreeNode("Colorful Text")) {
      // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
      ANCHOR::TextColored(GfVec4f(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
      ANCHOR::TextColored(GfVec4f(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
      ANCHOR::TextDisabled("Disabled");
      ANCHOR::SameLine();
      HelpMarker("The TextDisabled color is stored in AnchorStyle.");
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Word Wrapping")) {
      // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
      ANCHOR::TextWrapped(
        "This text should automatically wrap on the edge of the window. The current "
        "implementation "
        "for text wrapping follows simple rules suitable for English and possibly other "
        "languages.");
      ANCHOR::Spacing();

      static float wrap_width = 200.0f;
      ANCHOR::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

      AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();
      for (int n = 0; n < 2; n++) {
        ANCHOR::Text("Test paragraph %d:", n);
        GfVec2f pos = ANCHOR::GetCursorScreenPos();
        GfVec2f marker_min = GfVec2f(pos[0] + wrap_width, pos[1]);
        GfVec2f marker_max = GfVec2f(pos[0] + wrap_width + 10,
                                     pos[1] + ANCHOR::GetTextLineHeight());
        ANCHOR::PushTextWrapPos(ANCHOR::GetCursorPos()[0] + wrap_width);
        if (n == 0)
          ANCHOR::Text(
            "The lazy dog is a good dog. This paragraph should fit within %.0f pixels. Testing "
            "a 1 character word. The quick brown fox jumps over the lazy dog.",
            wrap_width);
        else
          ANCHOR::Text(
            "aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");

        // Draw actual text bounding box, following by marker of our expected limit (should not
        // overlap!)
        draw_list->AddRect(ANCHOR::GetItemRectMin(),
                           ANCHOR::GetItemRectMax(),
                           ANCHOR_COL32(255, 255, 0, 255));
        draw_list->AddRectFilled(marker_min, marker_max, ANCHOR_COL32(255, 0, 255, 255));
        ANCHOR::PopTextWrapPos();
      }

      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("UTF-8 Text")) {
      // UTF-8 test with Japanese characters
      // (Needs a suitable font? Try "Google Noto" or "Arial Unicode". See docs/FONTS.md for
      // details.)
      // - From C++11 you can use the u8"my text" syntax to encode literal strings as UTF-8
      // - For earlier compiler, you may be able to encode your sources as UTF-8 (e.g. in Visual
      // Studio, you
      //   can save your source files as 'UTF-8 without signature').
      // - FOR THIS DEMO FILE ONLY, BECAUSE WE WANT TO SUPPORT OLD COMPILERS, WE ARE *NOT*
      // INCLUDING RAW UTF-8
      //   CHARACTERS IN THIS SOURCE FILE. Instead we are encoding a few strings with hexadecimal
      //   constants. Don't do this in your application! Please use u8"text in any language" in
      //   your application!
      // Note that characters values are preserved even by InputText() if the font cannot be
      // displayed, so you can safely copy & paste garbled characters into another application.
      ANCHOR::TextWrapped(
        "CJK text will only appears if the font was loaded with the appropriate CJK character "
        "ranges. "
        "Call io.Fonts->AddFontFromFileTTF() manually to load extra character ranges. "
        "Read docs/FONTS.md for details.");
      ANCHOR::Text(
        "Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 "
        "(kakikukeko)");  // Normally we would use u8"blah blah" with the proper characters
                          // directly in the string.
      ANCHOR::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
      static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
      // static char buf[32] = u8"NIHONGO"; // <- this is how you would write it with C++11, using
      // real kanjis
      ANCHOR::InputText("UTF-8 input", buf, ANCHOR_ARRAYSIZE(buf));
      ANCHOR::TreePop();
    }
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Images")) {
    AnchorIO &io = ANCHOR::GetIO();
    ANCHOR::TextWrapped(
      "Below we are displaying the font texture (which is the only texture we have access to in "
      "this demo). "
      "Use the 'AnchorTextureID' type as storage to pass pointers or identifier to your own "
      "texture data. "
      "Hover the texture for a zoomed view!");

    // Below we are displaying the font texture because it is the only texture we have access to
    // inside the demo! Remember that AnchorTextureID is just storage for whatever you want it to
    // be. It is essentially a value that will be passed to the rendering backend via the
    // AnchorDrawCmd structure. If you use one of the default ANCHOR_impl_XXXX.cpp rendering
    // backend, they all have comments at the top of their respective source file to specify what
    // they expect to be stored in AnchorTextureID, for example:
    // - The ANCHOR_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer
    // - The ANCHOR_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier, etc.
    // More:
    // - If you decided that AnchorTextureID = MyEngineTexture*, then you can pass your
    // MyEngineTexture* pointers
    //   to ANCHOR::Image(), and gather width/height through your own functions, etc.
    // - You can use ShowMetricsWindow() to inspect the draw data that are being passed to your
    // renderer,
    //   it will help you debug issues if you are confused about it.
    // - Consider using the lower-level AnchorDrawList::AddImage() API, via
    // ANCHOR::GetWindowDrawList()->AddImage().
    // - Read https://github.com/ocornut/ANCHOR/blob/master/docs/FAQ.md
    // - Read https://github.com/ocornut/ANCHOR/wiki/Image-Loading-and-Displaying-Examples
    AnchorTextureID my_tex_id = io.Fonts->TexID;
    float my_tex_w = (float)io.Fonts->TexWidth;
    float my_tex_h = (float)io.Fonts->TexHeight;
    {
      ANCHOR::Text("%.0fx%.0f", my_tex_w, my_tex_h);
      GfVec2f pos = ANCHOR::GetCursorScreenPos();
      GfVec2f uv_min = GfVec2f(0.0f, 0.0f);                  // Top-left
      GfVec2f uv_max = GfVec2f(1.0f, 1.0f);                  // Lower-right
      GfVec4f tint_col = GfVec4f(1.0f, 1.0f, 1.0f, 1.0f);    // No tint
      GfVec4f border_col = GfVec4f(1.0f, 1.0f, 1.0f, 0.5f);  // 50% opaque white
      ANCHOR::Image(my_tex_id, GfVec2f(my_tex_w, my_tex_h), uv_min, uv_max, tint_col, border_col);
      if (ANCHOR::IsItemHovered()) {
        ANCHOR::BeginTooltip();
        float region_sz = 32.0f;
        float region_x = io.MousePos[0] - pos[0] - region_sz * 0.5f;
        float region_y = io.MousePos[1] - pos[1] - region_sz * 0.5f;
        float zoom = 4.0f;
        if (region_x < 0.0f) {
          region_x = 0.0f;
        } else if (region_x > my_tex_w - region_sz) {
          region_x = my_tex_w - region_sz;
        }
        if (region_y < 0.0f) {
          region_y = 0.0f;
        } else if (region_y > my_tex_h - region_sz) {
          region_y = my_tex_h - region_sz;
        }
        ANCHOR::Text("Min: (%.2f, %.2f)", region_x, region_y);
        ANCHOR::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
        GfVec2f uv0 = GfVec2f((region_x) / my_tex_w, (region_y) / my_tex_h);
        GfVec2f uv1 = GfVec2f((region_x + region_sz) / my_tex_w,
                              (region_y + region_sz) / my_tex_h);
        ANCHOR::Image(my_tex_id,
                      GfVec2f(region_sz * zoom, region_sz * zoom),
                      uv0,
                      uv1,
                      tint_col,
                      border_col);
        ANCHOR::EndTooltip();
      }
    }
    ANCHOR::TextWrapped("And now some textured buttons..");
    static int pressed_count = 0;
    for (int i = 0; i < 8; i++) {
      ANCHOR::PushID(i);
      int frame_padding = -1 + i;            // -1 == uses default padding (style.FramePadding)
      GfVec2f size = GfVec2f(32.0f, 32.0f);  // Size of the image we want to make visible
      GfVec2f uv0 = GfVec2f(0.0f, 0.0f);     // UV coordinates for lower-left
      GfVec2f uv1 = GfVec2f(32.0f / my_tex_w,
                            32.0f / my_tex_h);  // UV coordinates for (32,32) in our texture
      GfVec4f bg_col = GfVec4f(0.0f, 0.0f, 0.0f, 1.0f);    // Black background
      GfVec4f tint_col = GfVec4f(1.0f, 1.0f, 1.0f, 1.0f);  // No tint
      if (ANCHOR::ImageButton(my_tex_id, size, uv0, uv1, frame_padding, bg_col, tint_col))
        pressed_count += 1;
      ANCHOR::PopID();
      ANCHOR::SameLine();
    }
    ANCHOR::NewLine();
    ANCHOR::Text("Pressed %d times.", pressed_count);
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Combo")) {
    // Expose flags as checkbox for the demo
    static AnchorComboFlags flags = 0;
    ANCHOR::CheckboxFlags("AnchorComboFlags_PopupAlignLeft",
                          &flags,
                          AnchorComboFlags_PopupAlignLeft);
    ANCHOR::SameLine();
    HelpMarker("Only makes a difference if the popup is larger than the combo");
    if (ANCHOR::CheckboxFlags("AnchorComboFlags_NoArrowButton",
                              &flags,
                              AnchorComboFlags_NoArrowButton))
      flags &= ~AnchorComboFlags_NoPreview;  // Clear the other flag, as we cannot combine both
    if (ANCHOR::CheckboxFlags("AnchorComboFlags_NoPreview", &flags, AnchorComboFlags_NoPreview))
      flags &= ~AnchorComboFlags_NoArrowButton;  // Clear the other flag, as we cannot combine both

    // Using the generic BeginCombo() API, you have full control over how to display the combo
    // contents. (your selection data could be an index, a pointer to the object, an id for the
    // object, a flag intrusively stored in the object itself, etc.)
    const char *items[] = {"AAAA",
                           "BBBB",
                           "CCCC",
                           "DDDD",
                           "EEEE",
                           "FFFF",
                           "GGGG",
                           "HHHH",
                           "IIII",
                           "JJJJ",
                           "KKKK",
                           "LLLLLLL",
                           "MMMM",
                           "OOOOOOO"};
    static int item_current_idx = 0;  // Here we store our selection data as an index.
    const char *combo_label = items[item_current_idx];  // Label to preview before opening the
                                                        // combo (technically it could be anything)
    if (ANCHOR::BeginCombo("combo 1", combo_label, flags)) {
      for (int n = 0; n < ANCHOR_ARRAYSIZE(items); n++) {
        const bool is_selected = (item_current_idx == n);
        if (ANCHOR::Selectable(items[n], is_selected))
          item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected)
          ANCHOR::SetItemDefaultFocus();
      }
      ANCHOR::EndCombo();
    }

    // Simplified one-liner Combo() API, using values packed in a single constant string
    static int item_current_2 = 0;
    ANCHOR::Combo("combo 2 (one-liner)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

    // Simplified one-liner Combo() using an array of const char*
    static int item_current_3 =
      -1;  // If the selection isn't within 0..count, Combo won't display a preview
    ANCHOR::Combo("combo 3 (array)", &item_current_3, items, ANCHOR_ARRAYSIZE(items));

    // Simplified one-liner Combo() using an accessor function
    struct Funcs
    {
      static bool ItemGetter(void *data, int n, const char **out_str)
      {
        *out_str = ((const char **)data)[n];
        return true;
      }
    };
    static int item_current_4 = 0;
    ANCHOR::Combo("combo 4 (function)",
                  &item_current_4,
                  &Funcs::ItemGetter,
                  items,
                  ANCHOR_ARRAYSIZE(items));

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("List boxes")) {
    // Using the generic BeginListBox() API, you have full control over how to display the combo
    // contents. (your selection data could be an index, a pointer to the object, an id for the
    // object, a flag intrusively stored in the object itself, etc.)
    const char *items[] = {"AAAA",
                           "BBBB",
                           "CCCC",
                           "DDDD",
                           "EEEE",
                           "FFFF",
                           "GGGG",
                           "HHHH",
                           "IIII",
                           "JJJJ",
                           "KKKK",
                           "LLLLLLL",
                           "MMMM",
                           "OOOOOOO"};
    static int item_current_idx = 0;  // Here we store our selection data as an index.
    if (ANCHOR::BeginListBox("listbox 1")) {
      for (int n = 0; n < ANCHOR_ARRAYSIZE(items); n++) {
        const bool is_selected = (item_current_idx == n);
        if (ANCHOR::Selectable(items[n], is_selected))
          item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected)
          ANCHOR::SetItemDefaultFocus();
      }
      ANCHOR::EndListBox();
    }

    // Custom size: use all width, 5 items tall
    ANCHOR::Text("Full-width:");
    if (ANCHOR::BeginListBox("##listbox 2",
                             GfVec2f(-FLT_MIN, 5 * ANCHOR::GetTextLineHeightWithSpacing()))) {
      for (int n = 0; n < ANCHOR_ARRAYSIZE(items); n++) {
        const bool is_selected = (item_current_idx == n);
        if (ANCHOR::Selectable(items[n], is_selected))
          item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected)
          ANCHOR::SetItemDefaultFocus();
      }
      ANCHOR::EndListBox();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Selectables")) {
    // Selectable() has 2 overloads:
    // - The one taking "bool selected" as a read-only selection information.
    //   When Selectable() has been clicked it returns true and you can alter selection state
    //   accordingly.
    // - The one taking "bool* p_selected" as a read-write selection information (convenient in
    // some cases) The earlier is more flexible, as in real application your selection may be
    // stored in many different ways and not necessarily inside a bool value (e.g. in flags within
    // objects, as an external list, etc).
    if (ANCHOR::TreeNode("Basic")) {
      static bool selection[5] = {false, true, false, false, false};
      ANCHOR::Selectable("1. I am selectable", &selection[0]);
      ANCHOR::Selectable("2. I am selectable", &selection[1]);
      ANCHOR::Text("3. I am not selectable");
      ANCHOR::Selectable("4. I am selectable", &selection[3]);
      if (ANCHOR::Selectable("5. I am double clickable",
                             selection[4],
                             AnchorSelectableFlags_AllowDoubleClick))
        if (ANCHOR::IsMouseDoubleClicked(0))
          selection[4] = !selection[4];
      ANCHOR::TreePop();
    }
    if (ANCHOR::TreeNode("Selection State: Single Selection")) {
      static int selected = -1;
      for (int n = 0; n < 5; n++) {
        char buf[32];
        sprintf(buf, "Object %d", n);
        if (ANCHOR::Selectable(buf, selected == n))
          selected = n;
      }
      ANCHOR::TreePop();
    }
    if (ANCHOR::TreeNode("Selection State: Multiple Selection")) {
      HelpMarker("Hold CTRL and click to select multiple items.");
      static bool selection[5] = {false, false, false, false, false};
      for (int n = 0; n < 5; n++) {
        char buf[32];
        sprintf(buf, "Object %d", n);
        if (ANCHOR::Selectable(buf, selection[n])) {
          if (!ANCHOR::GetIO().KeyCtrl)  // Clear selection when CTRL is not held
            memset(selection, 0, sizeof(selection));
          selection[n] ^= 1;
        }
      }
      ANCHOR::TreePop();
    }
    if (ANCHOR::TreeNode("Rendering more text into the same line")) {
      // Using the Selectable() override that takes "bool* p_selected" parameter,
      // this function toggle your bool value automatically.
      static bool selected[3] = {false, false, false};
      ANCHOR::Selectable("main.c", &selected[0]);
      ANCHOR::SameLine(300);
      ANCHOR::Text(" 2,345 bytes");
      ANCHOR::Selectable("Hello.cpp", &selected[1]);
      ANCHOR::SameLine(300);
      ANCHOR::Text("12,345 bytes");
      ANCHOR::Selectable("Hello.h", &selected[2]);
      ANCHOR::SameLine(300);
      ANCHOR::Text(" 2,345 bytes");
      ANCHOR::TreePop();
    }
    if (ANCHOR::TreeNode("In columns")) {
      static bool selected[10] = {};

      if (ANCHOR::BeginTable("split1",
                             3,
                             AnchorTableFlags_Resizable | AnchorTableFlags_NoSavedSettings)) {
        for (int i = 0; i < 10; i++) {
          char label[32];
          sprintf(label, "Item %d", i);
          ANCHOR::TableNextColumn();
          ANCHOR::Selectable(label, &selected[i]);  // FIXME-TABLE: Selection overlap
        }
        ANCHOR::EndTable();
      }
      ANCHOR::Separator();
      if (ANCHOR::BeginTable("split2",
                             3,
                             AnchorTableFlags_Resizable | AnchorTableFlags_NoSavedSettings)) {
        for (int i = 0; i < 10; i++) {
          char label[32];
          sprintf(label, "Item %d", i);
          ANCHOR::TableNextRow();
          ANCHOR::TableNextColumn();
          ANCHOR::Selectable(label, &selected[i], AnchorSelectableFlags_SpanAllColumns);
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Some other contents");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("123456");
        }
        ANCHOR::EndTable();
      }
      ANCHOR::TreePop();
    }
    if (ANCHOR::TreeNode("Grid")) {
      static char selected[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
      };

      // Add in a bit of silly fun...
      const float time = (float)ANCHOR::GetTime();
      const bool winning_state = memchr(selected, 0, sizeof(selected)) ==
                                 NULL;  // If all cells are selected...
      if (winning_state)
        ANCHOR::PushStyleVar(
          AnchorStyleVar_SelectableTextAlign,
          GfVec2f(0.5f + 0.5f * cosf(time * 2.0f), 0.5f + 0.5f * sinf(time * 3.0f)));

      for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++) {
          if (x > 0)
            ANCHOR::SameLine();
          ANCHOR::PushID(y * 4 + x);
          if (ANCHOR::Selectable("Sailor", selected[y][x] != 0, 0, GfVec2f(50, 50))) {
            // Toggle clicked cell + toggle neighbors
            selected[y][x] ^= 1;
            if (x > 0) {
              selected[y][x - 1] ^= 1;
            }
            if (x < 3) {
              selected[y][x + 1] ^= 1;
            }
            if (y > 0) {
              selected[y - 1][x] ^= 1;
            }
            if (y < 3) {
              selected[y + 1][x] ^= 1;
            }
          }
          ANCHOR::PopID();
        }

      if (winning_state)
        ANCHOR::PopStyleVar();
      ANCHOR::TreePop();
    }
    if (ANCHOR::TreeNode("Alignment")) {
      HelpMarker(
        "By default, Selectables uses style.SelectableTextAlign but it can be overridden on a "
        "per-item "
        "basis using PushStyleVar(). You'll probably want to always keep your default situation "
        "to "
        "left-align otherwise it becomes difficult to layout multiple items on a same line");
      static bool selected[3 * 3] = {true, false, true, false, true, false, true, false, true};
      for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
          GfVec2f alignment = GfVec2f((float)x / 2.0f, (float)y / 2.0f);
          char name[32];
          sprintf(name, "(%.1f,%.1f)", alignment[0], alignment[1]);
          if (x > 0)
            ANCHOR::SameLine();
          ANCHOR::PushStyleVar(AnchorStyleVar_SelectableTextAlign, alignment);
          ANCHOR::Selectable(name,
                             &selected[3 * y + x],
                             AnchorSelectableFlags_None,
                             GfVec2f(80, 80));
          ANCHOR::PopStyleVar();
        }
      }
      ANCHOR::TreePop();
    }
    ANCHOR::TreePop();
  }

  // To wire InputText() with std::string or any other custom string type,
  // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/ANCHOR_stdlib.h
  // file.
  if (ANCHOR::TreeNode("Text Input")) {
    if (ANCHOR::TreeNode("Multi-line Text Input")) {
      // Note: we are using a fixed-sized buffer for simplicity here. See
      // AnchorInputTextFlags_CallbackResize and the code in misc/cpp/ANCHOR_stdlib.h for how to
      // setup InputText() for dynamically resizing strings.
      static char text[1024 * 16] =
        "/*\n"
        " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
        " the hexadecimal encoding of one offending instruction,\n"
        " more formally, the invalid operand with locked CMPXCHG8B\n"
        " instruction bug, is a design flaw in the majority of\n"
        " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
        " processors (all in the P5 microarchitecture).\n"
        "*/\n\n"
        "label:\n"
        "\tlock cmpxchg8b eax\n";

      static AnchorInputTextFlags flags = AnchorInputTextFlags_AllowTabInput;
      HelpMarker(
        "You can use the AnchorInputTextFlags_CallbackResize facility if you need to wire "
        "InputTextMultiline() to a dynamic string type. See misc/cpp/ANCHOR_stdlib.h for an "
        "example. (This is not demonstrated in ANCHOR_demo.cpp because we don't want to include "
        "<string> in here)");
      ANCHOR::CheckboxFlags("AnchorInputTextFlags_ReadOnly",
                            &flags,
                            AnchorInputTextFlags_ReadOnly);
      ANCHOR::CheckboxFlags("AnchorInputTextFlags_AllowTabInput",
                            &flags,
                            AnchorInputTextFlags_AllowTabInput);
      ANCHOR::CheckboxFlags("AnchorInputTextFlags_CtrlEnterForNewLine",
                            &flags,
                            AnchorInputTextFlags_CtrlEnterForNewLine);
      ANCHOR::InputTextMultiline("##source",
                                 text,
                                 ANCHOR_ARRAYSIZE(text),
                                 GfVec2f(-FLT_MIN, ANCHOR::GetTextLineHeight() * 16),
                                 flags);
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Filtered Text Input")) {
      struct TextFilters
      {
        // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i'
        static int FilterANCHORLetters(AnchorInputTextCallbackData *data)
        {
          if (data->EventChar < 256 && strchr("ANCHOR", (char)data->EventChar))
            return 0;
          return 1;
        }
      };

      static char buf1[64] = "";
      ANCHOR::InputText("default", buf1, 64);
      static char buf2[64] = "";
      ANCHOR::InputText("decimal", buf2, 64, AnchorInputTextFlags_CharsDecimal);
      static char buf3[64] = "";
      ANCHOR::InputText("hexadecimal",
                        buf3,
                        64,
                        AnchorInputTextFlags_CharsHexadecimal |
                          AnchorInputTextFlags_CharsUppercase);
      static char buf4[64] = "";
      ANCHOR::InputText("uppercase", buf4, 64, AnchorInputTextFlags_CharsUppercase);
      static char buf5[64] = "";
      ANCHOR::InputText("no blank", buf5, 64, AnchorInputTextFlags_CharsNoBlank);
      static char buf6[64] = "";
      ANCHOR::InputText("\"ANCHOR\" letters",
                        buf6,
                        64,
                        AnchorInputTextFlags_CallbackCharFilter,
                        TextFilters::FilterANCHORLetters);
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Password Input")) {
      static char password[64] = "password123";
      ANCHOR::InputText("password",
                        password,
                        ANCHOR_ARRAYSIZE(password),
                        AnchorInputTextFlags_Password);
      ANCHOR::SameLine();
      HelpMarker(
        "Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
      ANCHOR::InputTextWithHint("password (w/ hint)",
                                "<password>",
                                password,
                                ANCHOR_ARRAYSIZE(password),
                                AnchorInputTextFlags_Password);
      ANCHOR::InputText("password (clear)", password, ANCHOR_ARRAYSIZE(password));
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Completion, History, Edit Callbacks")) {
      struct Funcs
      {
        static int MyCallback(AnchorInputTextCallbackData *data)
        {
          if (data->EventFlag == AnchorInputTextFlags_CallbackCompletion) {
            data->InsertChars(data->CursorPos, "..");
          } else if (data->EventFlag == AnchorInputTextFlags_CallbackHistory) {
            if (data->EventKey == AnchorKey_UpArrow) {
              data->DeleteChars(0, data->BufTextLen);
              data->InsertChars(0, "Pressed Up!");
              data->SelectAll();
            } else if (data->EventKey == AnchorKey_DownArrow) {
              data->DeleteChars(0, data->BufTextLen);
              data->InsertChars(0, "Pressed Down!");
              data->SelectAll();
            }
          } else if (data->EventFlag == AnchorInputTextFlags_CallbackEdit) {
            // Toggle casing of first character
            char c = data->Buf[0];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
              data->Buf[0] ^= 32;
            data->BufDirty = true;

            // Increment a counter
            int *p_int = (int *)data->UserData;
            *p_int = *p_int + 1;
          }
          return 0;
        }
      };
      static char buf1[64];
      ANCHOR::InputText("Completion",
                        buf1,
                        64,
                        AnchorInputTextFlags_CallbackCompletion,
                        Funcs::MyCallback);
      ANCHOR::SameLine();
      HelpMarker(
        "Here we append \"..\" each time Tab is pressed. See 'Examples>Console' for a more "
        "meaningful demonstration of using this callback.");

      static char buf2[64];
      ANCHOR::InputText("History",
                        buf2,
                        64,
                        AnchorInputTextFlags_CallbackHistory,
                        Funcs::MyCallback);
      ANCHOR::SameLine();
      HelpMarker(
        "Here we replace and select text each time Up/Down are pressed. See 'Examples>Console' "
        "for a more meaningful demonstration of using this callback.");

      static char buf3[64];
      static int edit_count = 0;
      ANCHOR::InputText("Edit",
                        buf3,
                        64,
                        AnchorInputTextFlags_CallbackEdit,
                        Funcs::MyCallback,
                        (void *)&edit_count);
      ANCHOR::SameLine();
      HelpMarker("Here we toggle the casing of the first character on every edits + count edits.");
      ANCHOR::SameLine();
      ANCHOR::Text("(%d)", edit_count);

      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Resize Callback")) {
      // To wire InputText() with std::string or any other custom string type,
      // you can use the AnchorInputTextFlags_CallbackResize flag + create a custom
      // ANCHOR::InputText() wrapper using your preferred type. See misc/cpp/ANCHOR_stdlib.h for an
      // implementation of this using std::string.
      HelpMarker(
        "Using AnchorInputTextFlags_CallbackResize to wire your custom string type to "
        "InputText().\n\n"
        "See misc/cpp/ANCHOR_stdlib.h for an implementation of this for std::string.");
      struct Funcs
      {
        static int MyResizeCallback(AnchorInputTextCallbackData *data)
        {
          if (data->EventFlag == AnchorInputTextFlags_CallbackResize) {
            AnchorVector<char> *my_str = (AnchorVector<char> *)data->UserData;
            ANCHOR_ASSERT(my_str->begin() == data->Buf);
            my_str->resize(data->BufSize);  // NB: On resizing calls, generally data->BufSize ==
                                            // data->BufTextLen + 1
            data->Buf = my_str->begin();
          }
          return 0;
        }

        // Note: Because ANCHOR:: is a namespace you would typically add your own function into the
        // namespace. For example, you code may declare a function 'ANCHOR::InputText(const char*
        // label, MyString* my_str)'
        static bool MyInputTextMultiline(const char *label,
                                         AnchorVector<char> *my_str,
                                         const GfVec2f &size = GfVec2f(0, 0),
                                         AnchorInputTextFlags flags = 0)
        {
          ANCHOR_ASSERT((flags & AnchorInputTextFlags_CallbackResize) == 0);
          return ANCHOR::InputTextMultiline(label,
                                            my_str->begin(),
                                            (size_t)my_str->size(),
                                            size,
                                            flags | AnchorInputTextFlags_CallbackResize,
                                            Funcs::MyResizeCallback,
                                            (void *)my_str);
        }
      };

      // For this demo we are using AnchorVector as a string container.
      // Note that because we need to store a terminating zero character, our size/capacity are 1
      // more than usually reported by a typical string class.
      static AnchorVector<char> my_str;
      if (my_str.empty())
        my_str.push_back(0);
      Funcs::MyInputTextMultiline("##MyStr",
                                  &my_str,
                                  GfVec2f(-FLT_MIN, ANCHOR::GetTextLineHeight() * 16));
      ANCHOR::Text("Data: %p\nSize: %d\nCapacity: %d",
                   (void *)my_str.begin(),
                   my_str.size(),
                   my_str.capacity());
      ANCHOR::TreePop();
    }

    ANCHOR::TreePop();
  }

  // Tabs
  if (ANCHOR::TreeNode("Tabs")) {
    if (ANCHOR::TreeNode("Basic")) {
      AnchorTabBarFlags tab_bar_flags = AnchorTabBarFlags_None;
      if (ANCHOR::BeginTabBar("MyTabBar", tab_bar_flags)) {
        if (ANCHOR::BeginTabItem("Avocado")) {
          ANCHOR::Text("This is the Avocado tab!\nblah blah blah blah blah");
          ANCHOR::EndTabItem();
        }
        if (ANCHOR::BeginTabItem("Broccoli")) {
          ANCHOR::Text("This is the Broccoli tab!\nblah blah blah blah blah");
          ANCHOR::EndTabItem();
        }
        if (ANCHOR::BeginTabItem("Cucumber")) {
          ANCHOR::Text("This is the Cucumber tab!\nblah blah blah blah blah");
          ANCHOR::EndTabItem();
        }
        ANCHOR::EndTabBar();
      }
      ANCHOR::Separator();
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Advanced & Close Button")) {
      // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with
      // no flags (0).
      static AnchorTabBarFlags tab_bar_flags = AnchorTabBarFlags_Reorderable;
      ANCHOR::CheckboxFlags("AnchorTabBarFlags_Reorderable",
                            &tab_bar_flags,
                            AnchorTabBarFlags_Reorderable);
      ANCHOR::CheckboxFlags("AnchorTabBarFlags_AutoSelectNewTabs",
                            &tab_bar_flags,
                            AnchorTabBarFlags_AutoSelectNewTabs);
      ANCHOR::CheckboxFlags("AnchorTabBarFlags_TabListPopupButton",
                            &tab_bar_flags,
                            AnchorTabBarFlags_TabListPopupButton);
      ANCHOR::CheckboxFlags("AnchorTabBarFlags_NoCloseWithMiddleMouseButton",
                            &tab_bar_flags,
                            AnchorTabBarFlags_NoCloseWithMiddleMouseButton);
      if ((tab_bar_flags & AnchorTabBarFlags_FittingPolicyMask_) == 0)
        tab_bar_flags |= AnchorTabBarFlags_FittingPolicyDefault_;
      if (ANCHOR::CheckboxFlags("AnchorTabBarFlags_FittingPolicyResizeDown",
                                &tab_bar_flags,
                                AnchorTabBarFlags_FittingPolicyResizeDown))
        tab_bar_flags &= ~(AnchorTabBarFlags_FittingPolicyMask_ ^
                           AnchorTabBarFlags_FittingPolicyResizeDown);
      if (ANCHOR::CheckboxFlags("AnchorTabBarFlags_FittingPolicyScroll",
                                &tab_bar_flags,
                                AnchorTabBarFlags_FittingPolicyScroll))
        tab_bar_flags &= ~(AnchorTabBarFlags_FittingPolicyMask_ ^
                           AnchorTabBarFlags_FittingPolicyScroll);

      // Tab Bar
      const char *names[4] = {"Artichoke", "Beetroot", "Celery", "Daikon"};
      static bool opened[4] = {true, true, true, true};  // Persistent user state
      for (int n = 0; n < ANCHOR_ARRAYSIZE(opened); n++) {
        if (n > 0) {
          ANCHOR::SameLine();
        }
        ANCHOR::Checkbox(names[n], &opened[n]);
      }

      // Passing a bool* to BeginTabItem() is similar to passing one to Begin():
      // the underlying bool will be set to false when the tab is closed.
      if (ANCHOR::BeginTabBar("MyTabBar", tab_bar_flags)) {
        for (int n = 0; n < ANCHOR_ARRAYSIZE(opened); n++)
          if (opened[n] && ANCHOR::BeginTabItem(names[n], &opened[n], AnchorTabItemFlags_None)) {
            ANCHOR::Text("This is the %s tab!", names[n]);
            if (n & 1)
              ANCHOR::Text("I am an odd tab.");
            ANCHOR::EndTabItem();
          }
        ANCHOR::EndTabBar();
      }
      ANCHOR::Separator();
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("TabItemButton & Leading/Trailing flags")) {
      static AnchorVector<int> active_tabs;
      static int next_tab_id = 0;
      if (next_tab_id == 0)  // Initialize with some default tabs
        for (int i = 0; i < 3; i++)
          active_tabs.push_back(next_tab_id++);

      // TabItemButton() and Leading/Trailing flags are distinct features which we will demo
      // together. (It is possible to submit regular tabs with Leading/Trailing flags, or
      // TabItemButton tabs without Leading/Trailing flags... but they tend to make more sense
      // together)
      static bool show_leading_button = true;
      static bool show_trailing_button = true;
      ANCHOR::Checkbox("Show Leading TabItemButton()", &show_leading_button);
      ANCHOR::Checkbox("Show Trailing TabItemButton()", &show_trailing_button);

      // Expose some other flags which are useful to showcase how they interact with
      // Leading/Trailing tabs
      static AnchorTabBarFlags tab_bar_flags = AnchorTabBarFlags_AutoSelectNewTabs |
                                               AnchorTabBarFlags_Reorderable |
                                               AnchorTabBarFlags_FittingPolicyResizeDown;
      ANCHOR::CheckboxFlags("AnchorTabBarFlags_TabListPopupButton",
                            &tab_bar_flags,
                            AnchorTabBarFlags_TabListPopupButton);
      if (ANCHOR::CheckboxFlags("AnchorTabBarFlags_FittingPolicyResizeDown",
                                &tab_bar_flags,
                                AnchorTabBarFlags_FittingPolicyResizeDown))
        tab_bar_flags &= ~(AnchorTabBarFlags_FittingPolicyMask_ ^
                           AnchorTabBarFlags_FittingPolicyResizeDown);
      if (ANCHOR::CheckboxFlags("AnchorTabBarFlags_FittingPolicyScroll",
                                &tab_bar_flags,
                                AnchorTabBarFlags_FittingPolicyScroll))
        tab_bar_flags &= ~(AnchorTabBarFlags_FittingPolicyMask_ ^
                           AnchorTabBarFlags_FittingPolicyScroll);

      if (ANCHOR::BeginTabBar("MyTabBar", tab_bar_flags)) {
        // Demo a Leading TabItemButton(): click the "?" button to open a menu
        if (show_leading_button)
          if (ANCHOR::TabItemButton("?",
                                    AnchorTabItemFlags_Leading | AnchorTabItemFlags_NoTooltip))
            ANCHOR::OpenPopup("MyHelpMenu");
        if (ANCHOR::BeginPopup("MyHelpMenu")) {
          ANCHOR::Selectable("Hello!");
          ANCHOR::EndPopup();
        }

        // Demo Trailing Tabs: click the "+" button to add a new tab (in your app you may want to
        // use a font icon instead of the "+") Note that we submit it before the regular tabs, but
        // because of the AnchorTabItemFlags_Trailing flag it will always appear at the end.
        if (show_trailing_button)
          if (ANCHOR::TabItemButton("+",
                                    AnchorTabItemFlags_Trailing | AnchorTabItemFlags_NoTooltip))
            active_tabs.push_back(next_tab_id++);  // Add new tab

        // Submit our regular tabs
        for (int n = 0; n < active_tabs.Size;) {
          bool open = true;
          char name[16];
          snprintf(name, ANCHOR_ARRAYSIZE(name), "%04d", active_tabs[n]);
          if (ANCHOR::BeginTabItem(name, &open, AnchorTabItemFlags_None)) {
            ANCHOR::Text("This is the %s tab!", name);
            ANCHOR::EndTabItem();
          }

          if (!open)
            active_tabs.erase(active_tabs.Data + n);
          else
            n++;
        }

        ANCHOR::EndTabBar();
      }
      ANCHOR::Separator();
      ANCHOR::TreePop();
    }
    ANCHOR::TreePop();
  }

  // Plot/Graph widgets are not very good.
  // Consider writing your own, or using a third-party one, see:
  // - ImPlot https://github.com/epezent/implot
  // - others https://github.com/ocornut/ANCHOR/wiki/Useful-Extensions
  if (ANCHOR::TreeNode("Plots Widgets")) {
    static bool animate = true;
    ANCHOR::Checkbox("Animate", &animate);

    static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
    ANCHOR::PlotLines("Frame Times", arr, ANCHOR_ARRAYSIZE(arr));

    // Fill an array of contiguous float values to plot
    // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your
    // first float and the sizeof() of your structure in the "stride" parameter.
    static float values[90] = {};
    static int values_offset = 0;
    static double refresh_time = 0.0;
    if (!animate || refresh_time == 0.0)
      refresh_time = ANCHOR::GetTime();
    while (refresh_time < ANCHOR::GetTime())  // Create data at fixed 60 Hz rate for the demo
    {
      static float phase = 0.0f;
      values[values_offset] = cosf(phase);
      values_offset = (values_offset + 1) % ANCHOR_ARRAYSIZE(values);
      phase += 0.10f * values_offset;
      refresh_time += 1.0f / 60.0f;
    }

    // Plots can display overlay texts
    // (in this example, we will display an average value)
    {
      float average = 0.0f;
      for (int n = 0; n < ANCHOR_ARRAYSIZE(values); n++)
        average += values[n];
      average /= (float)ANCHOR_ARRAYSIZE(values);
      char overlay[32];
      sprintf(overlay, "avg %f", average);
      ANCHOR::PlotLines("Lines",
                        values,
                        ANCHOR_ARRAYSIZE(values),
                        values_offset,
                        overlay,
                        -1.0f,
                        1.0f,
                        GfVec2f(0, 80.0f));
    }
    ANCHOR::PlotHistogram("Histogram",
                          arr,
                          ANCHOR_ARRAYSIZE(arr),
                          0,
                          NULL,
                          0.0f,
                          1.0f,
                          GfVec2f(0, 80.0f));

    // Use functions to generate output
    // FIXME: This is rather awkward because current plot API only pass in indices.
    // We probably want an API passing floats and user provide sample rate/count.
    struct Funcs
    {
      static float Sin(void *, int i)
      {
        return sinf(i * 0.1f);
      }
      static float Saw(void *, int i)
      {
        return (i & 1) ? 1.0f : -1.0f;
      }
    };
    static int func_type = 0, display_count = 70;
    ANCHOR::Separator();
    ANCHOR::SetNextItemWidth(ANCHOR::GetFontSize() * 8);
    ANCHOR::Combo("func", &func_type, "Sin\0Saw\0");
    ANCHOR::SameLine();
    ANCHOR::SliderInt("Sample count", &display_count, 1, 400);
    float (*func)(void *, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
    ANCHOR::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, GfVec2f(0, 80));
    ANCHOR::PlotHistogram("Histogram",
                          func,
                          NULL,
                          display_count,
                          0,
                          NULL,
                          -1.0f,
                          1.0f,
                          GfVec2f(0, 80));
    ANCHOR::Separator();

    // Animate a simple progress bar
    static float progress = 0.0f, progress_dir = 1.0f;
    if (animate) {
      progress += progress_dir * 0.4f * ANCHOR::GetIO().DeltaTime;
      if (progress >= +1.1f) {
        progress = +1.1f;
        progress_dir *= -1.0f;
      }
      if (progress <= -0.1f) {
        progress = -0.1f;
        progress_dir *= -1.0f;
      }
    }

    // Typically we would use GfVec2f(-1.0f,0.0f) or GfVec2f(-FLT_MIN,0.0f) to use all available
    // width, or GfVec2f(width,0.0f) for a specified width. GfVec2f(0.0f,0.0f) uses ItemWidth.
    ANCHOR::ProgressBar(progress, GfVec2f(0.0f, 0.0f));
    ANCHOR::SameLine(0.0f, ANCHOR::GetStyle().ItemInnerSpacing[0]);
    ANCHOR::Text("Progress Bar");

    float progress_saturated = ANCHOR_CLAMP(progress, 0.0f, 1.0f);
    char buf[32];
    sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
    ANCHOR::ProgressBar(progress, GfVec2f(0.f, 0.f), buf);
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Color/Picker Widgets")) {
    static GfVec4f color = GfVec4f(114.0f / 255.0f,
                                   144.0f / 255.0f,
                                   154.0f / 255.0f,
                                   200.0f / 255.0f);

    static bool alpha_preview = true;
    static bool alpha_half_preview = false;
    static bool drag_and_drop = true;
    static bool options_menu = true;
    static bool hdr = false;
    ANCHOR::Checkbox("With Alpha Preview", &alpha_preview);
    ANCHOR::Checkbox("With Half Alpha Preview", &alpha_half_preview);
    ANCHOR::Checkbox("With Drag and Drop", &drag_and_drop);
    ANCHOR::Checkbox("With Options Menu", &options_menu);
    ANCHOR::SameLine();
    HelpMarker("Right-click on the individual color widget to show options.");
    ANCHOR::Checkbox("With HDR", &hdr);
    ANCHOR::SameLine();
    HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");
    AnchorColorEditFlags misc_flags = (hdr ? AnchorColorEditFlags_HDR : 0) |
                                      (drag_and_drop ? 0 : AnchorColorEditFlags_NoDragDrop) |
                                      (alpha_half_preview ?
                                         AnchorColorEditFlags_AlphaPreviewHalf :
                                         (alpha_preview ? AnchorColorEditFlags_AlphaPreview : 0)) |
                                      (options_menu ? 0 : AnchorColorEditFlags_NoOptions);

    ANCHOR::Text("Color widget:");
    ANCHOR::SameLine();
    HelpMarker(
      "Click on the color square to open a color picker.\n"
      "CTRL+click on individual component to input value.\n");
    ANCHOR::ColorEdit3("MyColor##1", (float *)&color, misc_flags);

    ANCHOR::Text("Color widget HSV with Alpha:");
    ANCHOR::ColorEdit4("MyColor##2",
                       (float *)&color,
                       AnchorColorEditFlags_DisplayHSV | misc_flags);

    ANCHOR::Text("Color widget with Float Display:");
    ANCHOR::ColorEdit4("MyColor##2f", (float *)&color, AnchorColorEditFlags_Float | misc_flags);

    ANCHOR::Text("Color button with Picker:");
    ANCHOR::SameLine();
    HelpMarker(
      "With the AnchorColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\n"
      "With the AnchorColorEditFlags_NoLabel flag you can pass a non-empty label which will "
      "only "
      "be used for the tooltip and picker popup.");
    ANCHOR::ColorEdit4("MyColor##3",
                       (float *)&color,
                       AnchorColorEditFlags_NoInputs | AnchorColorEditFlags_NoLabel | misc_flags);

    ANCHOR::Text("Color button with Custom Picker Popup:");

    // Generate a default palette. The palette will persist and can be edited.
    static bool saved_palette_init = true;
    static GfVec4f saved_palette[32] = {};
    if (saved_palette_init) {
      for (int n = 0; n < ANCHOR_ARRAYSIZE(saved_palette); n++) {
        ANCHOR::ColorConvertHSVtoRGB(n / 31.0f,
                                     0.8f,
                                     0.8f,
                                     saved_palette[n][0],
                                     saved_palette[n][1],
                                     saved_palette[n][2]);
        saved_palette[n][3] = 1.0f;  // Alpha
      }
      saved_palette_init = false;
    }

    static GfVec4f backup_color;
    bool open_popup = ANCHOR::ColorButton("MyColor##3b", color, misc_flags);
    ANCHOR::SameLine(0, ANCHOR::GetStyle().ItemInnerSpacing[0]);
    open_popup |= ANCHOR::Button("Palette");
    if (open_popup) {
      ANCHOR::OpenPopup("mypicker");
      backup_color = color;
    }
    if (ANCHOR::BeginPopup("mypicker")) {
      ANCHOR::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
      ANCHOR::Separator();
      ANCHOR::ColorPicker4("##picker",
                           (float *)&color,
                           misc_flags | AnchorColorEditFlags_NoSidePreview |
                             AnchorColorEditFlags_NoSmallPreview);
      ANCHOR::SameLine();

      ANCHOR::BeginGroup();  // Lock X position
      ANCHOR::Text("Current");
      ANCHOR::ColorButton("##current",
                          color,
                          AnchorColorEditFlags_NoPicker | AnchorColorEditFlags_AlphaPreviewHalf,
                          GfVec2f(60, 40));
      ANCHOR::Text("Previous");
      if (ANCHOR::ColorButton("##previous",
                              backup_color,
                              AnchorColorEditFlags_NoPicker |
                                AnchorColorEditFlags_AlphaPreviewHalf,
                              GfVec2f(60, 40)))
        color = backup_color;
      ANCHOR::Separator();
      ANCHOR::Text("Palette");
      for (int n = 0; n < ANCHOR_ARRAYSIZE(saved_palette); n++) {
        ANCHOR::PushID(n);
        if ((n % 8) != 0)
          ANCHOR::SameLine(0.0f, ANCHOR::GetStyle().ItemSpacing[1]);

        AnchorColorEditFlags palette_button_flags = AnchorColorEditFlags_NoAlpha |
                                                    AnchorColorEditFlags_NoPicker |
                                                    AnchorColorEditFlags_NoTooltip;
        if (ANCHOR::ColorButton("##palette",
                                saved_palette[n],
                                palette_button_flags,
                                GfVec2f(20, 20)))
          color = GfVec4f(saved_palette[n][0],
                          saved_palette[n][1],
                          saved_palette[n][2],
                          color[3]);  // Preserve alpha!

        // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
        // drag source by default, unless specifying the AnchorColorEditFlags_NoDragDrop flag.
        if (ANCHOR::BeginDragDropTarget()) {
          if (const AnchorPayload *payload = ANCHOR::AcceptDragDropPayload(
                ANCHOR_PAYLOAD_TYPE_COLOR_3F))
            memcpy((float *)&saved_palette[n], payload->Data, sizeof(float) * 3);
          if (const AnchorPayload *payload = ANCHOR::AcceptDragDropPayload(
                ANCHOR_PAYLOAD_TYPE_COLOR_4F))
            memcpy((float *)&saved_palette[n], payload->Data, sizeof(float) * 4);
          ANCHOR::EndDragDropTarget();
        }

        ANCHOR::PopID();
      }
      ANCHOR::EndGroup();
      ANCHOR::EndPopup();
    }

    ANCHOR::Text("Color button only:");
    static bool no_border = false;
    ANCHOR::Checkbox("AnchorColorEditFlags_NoBorder", &no_border);
    ANCHOR::ColorButton("MyColor##3c",
                        *(GfVec4f *)&color,
                        misc_flags | (no_border ? AnchorColorEditFlags_NoBorder : 0),
                        GfVec2f(80, 80));

    ANCHOR::Text("Color picker:");
    static bool alpha = true;
    static bool alpha_bar = true;
    static bool side_preview = true;
    static bool ref_color = false;
    static GfVec4f ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
    static int display_mode = 0;
    static int picker_mode = 0;
    ANCHOR::Checkbox("With Alpha", &alpha);
    ANCHOR::Checkbox("With Alpha Bar", &alpha_bar);
    ANCHOR::Checkbox("With Side Preview", &side_preview);
    if (side_preview) {
      ANCHOR::SameLine();
      ANCHOR::Checkbox("With Ref Color", &ref_color);
      if (ref_color) {
        ANCHOR::SameLine();
        ANCHOR::ColorEdit4("##RefColor",
                           &ref_color_v[0],
                           AnchorColorEditFlags_NoInputs | misc_flags);
      }
    }
    ANCHOR::Combo("Display Mode",
                  &display_mode,
                  "Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0");
    ANCHOR::SameLine();
    HelpMarker(
      "ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, "
      "but the user can change it with a right-click.\n\nColorPicker defaults to displaying "
      "RGB+HSV+Hex "
      "if you don't specify a display mode.\n\nYou can change the defaults using "
      "SetColorEditOptions().");
    ANCHOR::Combo("Picker Mode",
                  &picker_mode,
                  "Auto/Current\0Hue bar + SV rect\0Hue wheel + SV triangle\0");
    ANCHOR::SameLine();
    HelpMarker("User can right-click the picker to change mode.");
    AnchorColorEditFlags flags = misc_flags;
    if (!alpha)
      flags |= AnchorColorEditFlags_NoAlpha;  // This is by default if you call ColorPicker3()
                                              // instead of ColorPicker4()
    if (alpha_bar)
      flags |= AnchorColorEditFlags_AlphaBar;
    if (!side_preview)
      flags |= AnchorColorEditFlags_NoSidePreview;
    if (picker_mode == 1)
      flags |= AnchorColorEditFlags_PickerHueBar;
    if (picker_mode == 2)
      flags |= AnchorColorEditFlags_PickerHueWheel;
    if (display_mode == 1)
      flags |= AnchorColorEditFlags_NoInputs;  // Disable all RGB/HSV/Hex displays
    if (display_mode == 2)
      flags |= AnchorColorEditFlags_DisplayRGB;  // Override display mode
    if (display_mode == 3)
      flags |= AnchorColorEditFlags_DisplayHSV;
    if (display_mode == 4)
      flags |= AnchorColorEditFlags_DisplayHex;
    ANCHOR::ColorPicker4("MyColor##4", (float *)&color, flags, ref_color ? &ref_color_v[0] : NULL);

    ANCHOR::Text("Set defaults in code:");
    ANCHOR::SameLine();
    HelpMarker(
      "SetColorEditOptions() is designed to allow you to set boot-time default.\n"
      "We don't have Push/Pop functions because you can force options on a per-widget basis if "
      "needed,"
      "and the user can change non-forced ones with the options menu.\nWe don't have a getter "
      "to avoid"
      "encouraging you to persistently save values that aren't forward-compatible.");
    if (ANCHOR::Button("Default: Uint8 + HSV + Hue Bar"))
      ANCHOR::SetColorEditOptions(AnchorColorEditFlags_Uint8 | AnchorColorEditFlags_DisplayHSV |
                                  AnchorColorEditFlags_PickerHueBar);
    if (ANCHOR::Button("Default: Float + HDR + Hue Wheel"))
      ANCHOR::SetColorEditOptions(AnchorColorEditFlags_Float | AnchorColorEditFlags_HDR |
                                  AnchorColorEditFlags_PickerHueWheel);

    // HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
    static GfVec4f color_hsv(0.23f, 1.0f, 1.0f, 1.0f);  // Stored as HSV!
    ANCHOR::Spacing();
    ANCHOR::Text("HSV encoded colors");
    ANCHOR::SameLine();
    HelpMarker(
      "By default, colors are given to ColorEdit and ColorPicker in RGB, but "
      "AnchorColorEditFlags_InputHSV"
      "allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. "
      "This comes with the"
      "added benefit that you can manipulate hue values with the picker even when saturation or "
      "value are zero.");
    ANCHOR::Text("Color widget with InputHSV:");
    ANCHOR::ColorEdit4("HSV shown as RGB##1",
                       (float *)&color_hsv,
                       AnchorColorEditFlags_DisplayRGB | AnchorColorEditFlags_InputHSV |
                         AnchorColorEditFlags_Float);
    ANCHOR::ColorEdit4("HSV shown as HSV##1",
                       (float *)&color_hsv,
                       AnchorColorEditFlags_DisplayHSV | AnchorColorEditFlags_InputHSV |
                         AnchorColorEditFlags_Float);
    ANCHOR::DragFloat4("Raw HSV values", (float *)&color_hsv, 0.01f, 0.0f, 1.0f);

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Drag/Slider Flags")) {
    // Demonstrate using advanced flags for DragXXX and SliderXXX functions. Note that the flags
    // are the same!
    static AnchorSliderFlags flags = AnchorSliderFlags_None;
    ANCHOR::CheckboxFlags("AnchorSliderFlags_AlwaysClamp", &flags, AnchorSliderFlags_AlwaysClamp);
    ANCHOR::SameLine();
    HelpMarker(
      "Always clamp value to min/max bounds (if any) when input manually with CTRL+Click.");
    ANCHOR::CheckboxFlags("AnchorSliderFlags_Logarithmic", &flags, AnchorSliderFlags_Logarithmic);
    ANCHOR::SameLine();
    HelpMarker("Enable logarithmic editing (more precision for small values).");
    ANCHOR::CheckboxFlags("AnchorSliderFlags_NoRoundToFormat",
                          &flags,
                          AnchorSliderFlags_NoRoundToFormat);
    ANCHOR::SameLine();
    HelpMarker(
      "Disable rounding underlying value to match precision of the format string (e.g. %.3f "
      "values are rounded to those 3 digits).");
    ANCHOR::CheckboxFlags("AnchorSliderFlags_NoInput", &flags, AnchorSliderFlags_NoInput);
    ANCHOR::SameLine();
    HelpMarker("Disable CTRL+Click or Enter key allowing to input text directly into the widget.");

    // Drags
    static float drag_f = 0.5f;
    static int drag_i = 50;
    ANCHOR::Text("Underlying float value: %f", drag_f);
    ANCHOR::DragFloat("DragFloat (0 -> 1)", &drag_f, 0.005f, 0.0f, 1.0f, "%.3f", flags);
    ANCHOR::DragFloat("DragFloat (0 -> +inf)", &drag_f, 0.005f, 0.0f, FLT_MAX, "%.3f", flags);
    ANCHOR::DragFloat("DragFloat (-inf -> 1)", &drag_f, 0.005f, -FLT_MAX, 1.0f, "%.3f", flags);
    ANCHOR::DragFloat("DragFloat (-inf -> +inf)",
                      &drag_f,
                      0.005f,
                      -FLT_MAX,
                      +FLT_MAX,
                      "%.3f",
                      flags);
    ANCHOR::DragInt("DragInt (0 -> 100)", &drag_i, 0.5f, 0, 100, "%d", flags);

    // Sliders
    static float slider_f = 0.5f;
    static int slider_i = 50;
    ANCHOR::Text("Underlying float value: %f", slider_f);
    ANCHOR::SliderFloat("SliderFloat (0 -> 1)", &slider_f, 0.0f, 1.0f, "%.3f", flags);
    ANCHOR::SliderInt("SliderInt (0 -> 100)", &slider_i, 0, 100, "%d", flags);

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Range Widgets")) {
    static float begin = 10, end = 90;
    static int begin_i = 100, end_i = 1000;
    ANCHOR::DragFloatRange2("range float",
                            &begin,
                            &end,
                            0.25f,
                            0.0f,
                            100.0f,
                            "Min: %.1f %%",
                            "Max: %.1f %%",
                            AnchorSliderFlags_AlwaysClamp);
    ANCHOR::DragIntRange2("range int",
                          &begin_i,
                          &end_i,
                          5,
                          0,
                          1000,
                          "Min: %d units",
                          "Max: %d units");
    ANCHOR::DragIntRange2("range int (no bounds)",
                          &begin_i,
                          &end_i,
                          5,
                          0,
                          0,
                          "Min: %d units",
                          "Max: %d units");
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Data Types")) {
// DragScalar/InputScalar/SliderScalar functions allow various data types
// - signed/unsigned
// - 8/16/32/64-bits
// - integer/float/double
// To avoid polluting the public API with all possible combinations, we use the AnchorDataType
// enum to pass the type, and passing all arguments by pointer. This is the reason the test code
// below creates local variables to hold "zero" "one" etc. for each types. In practice, if you
// frequently use a given type that is not covered by the normal API entry points, you can wrap it
// yourself inside a 1 line function which can take typed argument as value instead of void*, and
// then pass their address to the generic function. For example:
//   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format =
//   "%lld")
//   {
//      return SliderScalar(label, AnchorDataType_U64, value, &min, &max, format);
//   }

// Setup limits (as helper variables so we can take their address, as explained above)
// Note: SliderScalar() functions have a maximum usable range of half the natural type maximum,
// hence the /2.
#ifndef LLONG_MIN
    AnchorS64 LLONG_MIN = -9223372036854775807LL - 1;
    AnchorS64 LLONG_MAX = 9223372036854775807LL;
    AnchorU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
#endif
    const char s8_zero = 0, s8_one = 1, s8_fifty = 50, s8_min = -128, s8_max = 127;
    const AnchorU8 u8_zero = 0, u8_one = 1, u8_fifty = 50, u8_min = 0, u8_max = 255;
    const short s16_zero = 0, s16_one = 1, s16_fifty = 50, s16_min = -32768, s16_max = 32767;
    const AnchorU16 u16_zero = 0, u16_one = 1, u16_fifty = 50, u16_min = 0, u16_max = 65535;
    const AnchorS32 s32_zero = 0, s32_one = 1, s32_fifty = 50, s32_min = INT_MIN / 2,
                    s32_max = INT_MAX / 2, s32_hi_a = INT_MAX / 2 - 100, s32_hi_b = INT_MAX / 2;
    const AnchorU32 u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0, u32_max = UINT_MAX / 2,
                    u32_hi_a = UINT_MAX / 2 - 100, u32_hi_b = UINT_MAX / 2;
    const AnchorS64 s64_zero = 0, s64_one = 1, s64_fifty = 50, s64_min = LLONG_MIN / 2,
                    s64_max = LLONG_MAX / 2, s64_hi_a = LLONG_MAX / 2 - 100,
                    s64_hi_b = LLONG_MAX / 2;
    const AnchorU64 u64_zero = 0, u64_one = 1, u64_fifty = 50, u64_min = 0,
                    u64_max = ULLONG_MAX / 2, u64_hi_a = ULLONG_MAX / 2 - 100,
                    u64_hi_b = ULLONG_MAX / 2;
    const float f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f,
                f32_hi_a = +10000000000.0f;
    const double f64_zero = 0., f64_one = 1., f64_lo_a = -1000000000000000.0,
                 f64_hi_a = +1000000000000000.0;

    // State
    static char s8_v = 127;
    static AnchorU8 u8_v = 255;
    static short s16_v = 32767;
    static AnchorU16 u16_v = 65535;
    static AnchorS32 s32_v = -1;
    static AnchorU32 u32_v = (AnchorU32)-1;
    static AnchorS64 s64_v = -1;
    static AnchorU64 u64_v = (AnchorU64)-1;
    static float f32_v = 0.123f;
    static double f64_v = 90000.01234567890123456789;

    const float drag_speed = 0.2f;
    static bool drag_clamp = false;
    ANCHOR::Text("Drags:");
    ANCHOR::Checkbox("Clamp integers to 0..50", &drag_clamp);
    ANCHOR::SameLine();
    HelpMarker(
      "As with every widgets in ANCHOR, we never modify values unless there is a user "
      "interaction.\n"
      "You can override the clamping limits by using CTRL+Click to input a value.");
    ANCHOR::DragScalar("drag s8",
                       AnchorDataType_S8,
                       &s8_v,
                       drag_speed,
                       drag_clamp ? &s8_zero : NULL,
                       drag_clamp ? &s8_fifty : NULL);
    ANCHOR::DragScalar("drag u8",
                       AnchorDataType_U8,
                       &u8_v,
                       drag_speed,
                       drag_clamp ? &u8_zero : NULL,
                       drag_clamp ? &u8_fifty : NULL,
                       "%u ms");
    ANCHOR::DragScalar("drag s16",
                       AnchorDataType_S16,
                       &s16_v,
                       drag_speed,
                       drag_clamp ? &s16_zero : NULL,
                       drag_clamp ? &s16_fifty : NULL);
    ANCHOR::DragScalar("drag u16",
                       AnchorDataType_U16,
                       &u16_v,
                       drag_speed,
                       drag_clamp ? &u16_zero : NULL,
                       drag_clamp ? &u16_fifty : NULL,
                       "%u ms");
    ANCHOR::DragScalar("drag s32",
                       AnchorDataType_S32,
                       &s32_v,
                       drag_speed,
                       drag_clamp ? &s32_zero : NULL,
                       drag_clamp ? &s32_fifty : NULL);
    ANCHOR::DragScalar("drag u32",
                       AnchorDataType_U32,
                       &u32_v,
                       drag_speed,
                       drag_clamp ? &u32_zero : NULL,
                       drag_clamp ? &u32_fifty : NULL,
                       "%u ms");
    ANCHOR::DragScalar("drag s64",
                       AnchorDataType_S64,
                       &s64_v,
                       drag_speed,
                       drag_clamp ? &s64_zero : NULL,
                       drag_clamp ? &s64_fifty : NULL);
    ANCHOR::DragScalar("drag u64",
                       AnchorDataType_U64,
                       &u64_v,
                       drag_speed,
                       drag_clamp ? &u64_zero : NULL,
                       drag_clamp ? &u64_fifty : NULL);
    ANCHOR::DragScalar("drag float",
                       AnchorDataType_Float,
                       &f32_v,
                       0.005f,
                       &f32_zero,
                       &f32_one,
                       "%f");
    ANCHOR::DragScalar("drag float log",
                       AnchorDataType_Float,
                       &f32_v,
                       0.005f,
                       &f32_zero,
                       &f32_one,
                       "%f",
                       AnchorSliderFlags_Logarithmic);
    ANCHOR::DragScalar("drag double",
                       AnchorDataType_Double,
                       &f64_v,
                       0.0005f,
                       &f64_zero,
                       NULL,
                       "%.10f grams");
    ANCHOR::DragScalar("drag double log",
                       AnchorDataType_Double,
                       &f64_v,
                       0.0005f,
                       &f64_zero,
                       &f64_one,
                       "0 < %.10f < 1",
                       AnchorSliderFlags_Logarithmic);

    ANCHOR::Text("Sliders");
    ANCHOR::SliderScalar("slider s8 full", AnchorDataType_S8, &s8_v, &s8_min, &s8_max, "%d");
    ANCHOR::SliderScalar("slider u8 full", AnchorDataType_U8, &u8_v, &u8_min, &u8_max, "%u");
    ANCHOR::SliderScalar("slider s16 full", AnchorDataType_S16, &s16_v, &s16_min, &s16_max, "%d");
    ANCHOR::SliderScalar("slider u16 full", AnchorDataType_U16, &u16_v, &u16_min, &u16_max, "%u");
    ANCHOR::SliderScalar("slider s32 low",
                         AnchorDataType_S32,
                         &s32_v,
                         &s32_zero,
                         &s32_fifty,
                         "%d");
    ANCHOR::SliderScalar("slider s32 high",
                         AnchorDataType_S32,
                         &s32_v,
                         &s32_hi_a,
                         &s32_hi_b,
                         "%d");
    ANCHOR::SliderScalar("slider s32 full", AnchorDataType_S32, &s32_v, &s32_min, &s32_max, "%d");
    ANCHOR::SliderScalar("slider u32 low",
                         AnchorDataType_U32,
                         &u32_v,
                         &u32_zero,
                         &u32_fifty,
                         "%u");
    ANCHOR::SliderScalar("slider u32 high",
                         AnchorDataType_U32,
                         &u32_v,
                         &u32_hi_a,
                         &u32_hi_b,
                         "%u");
    ANCHOR::SliderScalar("slider u32 full", AnchorDataType_U32, &u32_v, &u32_min, &u32_max, "%u");
    ANCHOR::SliderScalar("slider s64 low",
                         AnchorDataType_S64,
                         &s64_v,
                         &s64_zero,
                         &s64_fifty,
                         "%" IM_PRId64);
    ANCHOR::SliderScalar("slider s64 high",
                         AnchorDataType_S64,
                         &s64_v,
                         &s64_hi_a,
                         &s64_hi_b,
                         "%" IM_PRId64);
    ANCHOR::SliderScalar("slider s64 full",
                         AnchorDataType_S64,
                         &s64_v,
                         &s64_min,
                         &s64_max,
                         "%" IM_PRId64);
    ANCHOR::SliderScalar("slider u64 low",
                         AnchorDataType_U64,
                         &u64_v,
                         &u64_zero,
                         &u64_fifty,
                         "%" IM_PRIu64 " ms");
    ANCHOR::SliderScalar("slider u64 high",
                         AnchorDataType_U64,
                         &u64_v,
                         &u64_hi_a,
                         &u64_hi_b,
                         "%" IM_PRIu64 " ms");
    ANCHOR::SliderScalar("slider u64 full",
                         AnchorDataType_U64,
                         &u64_v,
                         &u64_min,
                         &u64_max,
                         "%" IM_PRIu64 " ms");
    ANCHOR::SliderScalar("slider float low", AnchorDataType_Float, &f32_v, &f32_zero, &f32_one);
    ANCHOR::SliderScalar("slider float low log",
                         AnchorDataType_Float,
                         &f32_v,
                         &f32_zero,
                         &f32_one,
                         "%.10f",
                         AnchorSliderFlags_Logarithmic);
    ANCHOR::SliderScalar("slider float high",
                         AnchorDataType_Float,
                         &f32_v,
                         &f32_lo_a,
                         &f32_hi_a,
                         "%e");
    ANCHOR::SliderScalar("slider double low",
                         AnchorDataType_Double,
                         &f64_v,
                         &f64_zero,
                         &f64_one,
                         "%.10f grams");
    ANCHOR::SliderScalar("slider double low log",
                         AnchorDataType_Double,
                         &f64_v,
                         &f64_zero,
                         &f64_one,
                         "%.10f",
                         AnchorSliderFlags_Logarithmic);
    ANCHOR::SliderScalar("slider double high",
                         AnchorDataType_Double,
                         &f64_v,
                         &f64_lo_a,
                         &f64_hi_a,
                         "%e grams");

    ANCHOR::Text("Sliders (reverse)");
    ANCHOR::SliderScalar("slider s8 reverse", AnchorDataType_S8, &s8_v, &s8_max, &s8_min, "%d");
    ANCHOR::SliderScalar("slider u8 reverse", AnchorDataType_U8, &u8_v, &u8_max, &u8_min, "%u");
    ANCHOR::SliderScalar("slider s32 reverse",
                         AnchorDataType_S32,
                         &s32_v,
                         &s32_fifty,
                         &s32_zero,
                         "%d");
    ANCHOR::SliderScalar("slider u32 reverse",
                         AnchorDataType_U32,
                         &u32_v,
                         &u32_fifty,
                         &u32_zero,
                         "%u");
    ANCHOR::SliderScalar("slider s64 reverse",
                         AnchorDataType_S64,
                         &s64_v,
                         &s64_fifty,
                         &s64_zero,
                         "%" IM_PRId64);
    ANCHOR::SliderScalar("slider u64 reverse",
                         AnchorDataType_U64,
                         &u64_v,
                         &u64_fifty,
                         &u64_zero,
                         "%" IM_PRIu64 " ms");

    static bool inputs_step = true;
    ANCHOR::Text("Inputs");
    ANCHOR::Checkbox("Show step buttons", &inputs_step);
    ANCHOR::InputScalar("input s8",
                        AnchorDataType_S8,
                        &s8_v,
                        inputs_step ? &s8_one : NULL,
                        NULL,
                        "%d");
    ANCHOR::InputScalar("input u8",
                        AnchorDataType_U8,
                        &u8_v,
                        inputs_step ? &u8_one : NULL,
                        NULL,
                        "%u");
    ANCHOR::InputScalar("input s16",
                        AnchorDataType_S16,
                        &s16_v,
                        inputs_step ? &s16_one : NULL,
                        NULL,
                        "%d");
    ANCHOR::InputScalar("input u16",
                        AnchorDataType_U16,
                        &u16_v,
                        inputs_step ? &u16_one : NULL,
                        NULL,
                        "%u");
    ANCHOR::InputScalar("input s32",
                        AnchorDataType_S32,
                        &s32_v,
                        inputs_step ? &s32_one : NULL,
                        NULL,
                        "%d");
    ANCHOR::InputScalar("input s32 hex",
                        AnchorDataType_S32,
                        &s32_v,
                        inputs_step ? &s32_one : NULL,
                        NULL,
                        "%08X",
                        AnchorInputTextFlags_CharsHexadecimal);
    ANCHOR::InputScalar("input u32",
                        AnchorDataType_U32,
                        &u32_v,
                        inputs_step ? &u32_one : NULL,
                        NULL,
                        "%u");
    ANCHOR::InputScalar("input u32 hex",
                        AnchorDataType_U32,
                        &u32_v,
                        inputs_step ? &u32_one : NULL,
                        NULL,
                        "%08X",
                        AnchorInputTextFlags_CharsHexadecimal);
    ANCHOR::InputScalar("input s64", AnchorDataType_S64, &s64_v, inputs_step ? &s64_one : NULL);
    ANCHOR::InputScalar("input u64", AnchorDataType_U64, &u64_v, inputs_step ? &u64_one : NULL);
    ANCHOR::InputScalar("input float",
                        AnchorDataType_Float,
                        &f32_v,
                        inputs_step ? &f32_one : NULL);
    ANCHOR::InputScalar("input double",
                        AnchorDataType_Double,
                        &f64_v,
                        inputs_step ? &f64_one : NULL);

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Multi-component Widgets")) {
    static float vec4f[4] = {0.10f, 0.20f, 0.30f, 0.44f};
    static int vec4i[4] = {1, 5, 100, 255};

    ANCHOR::InputFloat2("input float2", vec4f);
    ANCHOR::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f);
    ANCHOR::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f);
    ANCHOR::InputInt2("input int2", vec4i);
    ANCHOR::DragInt2("drag int2", vec4i, 1, 0, 255);
    ANCHOR::SliderInt2("slider int2", vec4i, 0, 255);
    ANCHOR::Spacing();

    ANCHOR::InputFloat3("input float3", vec4f);
    ANCHOR::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
    ANCHOR::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f);
    ANCHOR::InputInt3("input int3", vec4i);
    ANCHOR::DragInt3("drag int3", vec4i, 1, 0, 255);
    ANCHOR::SliderInt3("slider int3", vec4i, 0, 255);
    ANCHOR::Spacing();

    ANCHOR::InputFloat4("input float4", vec4f);
    ANCHOR::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f);
    ANCHOR::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f);
    ANCHOR::InputInt4("input int4", vec4i);
    ANCHOR::DragInt4("drag int4", vec4i, 1, 0, 255);
    ANCHOR::SliderInt4("slider int4", vec4i, 0, 255);

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Vertical Sliders")) {
    const float spacing = 4;
    ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing, GfVec2f(spacing, spacing));

    static int int_value = 0;
    ANCHOR::VSliderInt("##int", GfVec2f(18, 160), &int_value, 0, 5);
    ANCHOR::SameLine();

    static float values[7] = {0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f};
    ANCHOR::PushID("set1");
    for (int i = 0; i < 7; i++) {
      if (i > 0)
        ANCHOR::SameLine();
      ANCHOR::PushID(i);
      ANCHOR::PushStyleColor(AnchorCol_FrameBg, AnchorColor::HSV(i / 7.0f, 0.5f, 0.5f).Value);
      ANCHOR::PushStyleColor(AnchorCol_FrameBgHovered,
                             AnchorColor::HSV(i / 7.0f, 0.6f, 0.5f).Value);
      ANCHOR::PushStyleColor(AnchorCol_FrameBgActive,
                             AnchorColor::HSV(i / 7.0f, 0.7f, 0.5f).Value);
      ANCHOR::PushStyleColor(AnchorCol_SliderGrab, AnchorColor::HSV(i / 7.0f, 0.9f, 0.9f).Value);
      ANCHOR::VSliderFloat("##v", GfVec2f(18, 160), &values[i], 0.0f, 1.0f, "");
      if (ANCHOR::IsItemActive() || ANCHOR::IsItemHovered())
        ANCHOR::SetTooltip("%.3f", values[i]);
      ANCHOR::PopStyleColor(4);
      ANCHOR::PopID();
    }
    ANCHOR::PopID();

    ANCHOR::SameLine();
    ANCHOR::PushID("set2");
    static float values2[4] = {0.20f, 0.80f, 0.40f, 0.25f};
    const int rows = 3;
    const GfVec2f small_slider_size(18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
    for (int nx = 0; nx < 4; nx++) {
      if (nx > 0)
        ANCHOR::SameLine();
      ANCHOR::BeginGroup();
      for (int ny = 0; ny < rows; ny++) {
        ANCHOR::PushID(nx * rows + ny);
        ANCHOR::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
        if (ANCHOR::IsItemActive() || ANCHOR::IsItemHovered())
          ANCHOR::SetTooltip("%.3f", values2[nx]);
        ANCHOR::PopID();
      }
      ANCHOR::EndGroup();
    }
    ANCHOR::PopID();

    ANCHOR::SameLine();
    ANCHOR::PushID("set3");
    for (int i = 0; i < 4; i++) {
      if (i > 0)
        ANCHOR::SameLine();
      ANCHOR::PushID(i);
      ANCHOR::PushStyleVar(AnchorStyleVar_GrabMinSize, 40);
      ANCHOR::VSliderFloat("##v", GfVec2f(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
      ANCHOR::PopStyleVar();
      ANCHOR::PopID();
    }
    ANCHOR::PopID();
    ANCHOR::PopStyleVar();
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Drag and Drop")) {
    if (ANCHOR::TreeNode("Drag and drop in standard widgets")) {
      // ColorEdit widgets automatically act as drag source and drag target.
      // They are using standardized payload strings ANCHOR_PAYLOAD_TYPE_COLOR_3F and
      // ANCHOR_PAYLOAD_TYPE_COLOR_4F to allow your own widgets to use colors in their drag and
      // drop interaction. Also see 'Demo->Widgets->Color/Picker Widgets->Palette' demo.
      HelpMarker("You can drag from the color squares.");
      static float col1[3] = {1.0f, 0.0f, 0.2f};
      static float col2[4] = {0.4f, 0.7f, 0.0f, 0.5f};
      ANCHOR::ColorEdit3("color 1", col1);
      ANCHOR::ColorEdit4("color 2", col2);
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Drag and drop to copy/swap items")) {
      enum Mode
      {
        Mode_Copy,
        Mode_Move,
        Mode_Swap
      };
      static int mode = 0;
      if (ANCHOR::RadioButton("Copy", mode == Mode_Copy)) {
        mode = Mode_Copy;
      }
      ANCHOR::SameLine();
      if (ANCHOR::RadioButton("Move", mode == Mode_Move)) {
        mode = Mode_Move;
      }
      ANCHOR::SameLine();
      if (ANCHOR::RadioButton("Swap", mode == Mode_Swap)) {
        mode = Mode_Swap;
      }
      static const char *names[9] =
        {"Bobby", "Beatrice", "Betty", "Brianna", "Barry", "Bernard", "Bibi", "Blaine", "Bryn"};
      for (int n = 0; n < ANCHOR_ARRAYSIZE(names); n++) {
        ANCHOR::PushID(n);
        if ((n % 3) != 0)
          ANCHOR::SameLine();
        ANCHOR::Button(names[n], GfVec2f(60, 60));

        // Our buttons are both drag sources and drag targets here!
        if (ANCHOR::BeginDragDropSource(AnchorDragDropFlags_None)) {
          // Set payload to carry the index of our item (could be anything)
          ANCHOR::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));

          // Display preview (could be anything, e.g. when dragging an image we could decide to
          // display the filename and a small preview of the image, etc.)
          if (mode == Mode_Copy) {
            ANCHOR::Text("Copy %s", names[n]);
          }
          if (mode == Mode_Move) {
            ANCHOR::Text("Move %s", names[n]);
          }
          if (mode == Mode_Swap) {
            ANCHOR::Text("Swap %s", names[n]);
          }
          ANCHOR::EndDragDropSource();
        }
        if (ANCHOR::BeginDragDropTarget()) {
          if (const AnchorPayload *payload = ANCHOR::AcceptDragDropPayload("DND_DEMO_CELL")) {
            ANCHOR_ASSERT(payload->DataSize == sizeof(int));
            int payload_n = *(const int *)payload->Data;
            if (mode == Mode_Copy) {
              names[n] = names[payload_n];
            }
            if (mode == Mode_Move) {
              names[n] = names[payload_n];
              names[payload_n] = "";
            }
            if (mode == Mode_Swap) {
              const char *tmp = names[n];
              names[n] = names[payload_n];
              names[payload_n] = tmp;
            }
          }
          ANCHOR::EndDragDropTarget();
        }
        ANCHOR::PopID();
      }
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Drag to reorder items (simple)")) {
      // Simple reordering
      HelpMarker(
        "We don't use the drag and drop api at all here! "
        "Instead we query when the item is held but not hovered, and order items accordingly.");
      static const char *item_names[] = {"Item One",
                                         "Item Two",
                                         "Item Three",
                                         "Item Four",
                                         "Item Five"};
      for (int n = 0; n < ANCHOR_ARRAYSIZE(item_names); n++) {
        const char *item = item_names[n];
        ANCHOR::Selectable(item);

        if (ANCHOR::IsItemActive() && !ANCHOR::IsItemHovered()) {
          int n_next = n + (ANCHOR::GetMouseDragDelta(0)[1] < 0.f ? -1 : 1);
          if (n_next >= 0 && n_next < ANCHOR_ARRAYSIZE(item_names)) {
            item_names[n] = item_names[n_next];
            item_names[n_next] = item;
            ANCHOR::ResetMouseDragDelta();
          }
        }
      }
      ANCHOR::TreePop();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Querying Status (Edited/Active/Focused/Hovered etc.)")) {
    // Select an item type
    const char *item_names[] = {"Text",
                                "Button",
                                "Button (w/ repeat)",
                                "Checkbox",
                                "SliderFloat",
                                "InputText",
                                "InputFloat",
                                "InputFloat3",
                                "ColorEdit4",
                                "MenuItem",
                                "TreeNode",
                                "TreeNode (w/ double-click)",
                                "Combo",
                                "ListBox"};
    static int item_type = 1;
    ANCHOR::Combo("Item Type",
                  &item_type,
                  item_names,
                  ANCHOR_ARRAYSIZE(item_names),
                  ANCHOR_ARRAYSIZE(item_names));
    ANCHOR::SameLine();
    HelpMarker(
      "Testing how various types of items are interacting with the IsItemXXX functions. Note "
      "that the bool return value of most ANCHOR function is generally equivalent to calling "
      "ANCHOR::IsItemHovered().");

    // Submit selected item item so we can query their status in the code following it.
    bool ret = false;
    static bool b = false;
    static float col4f[4] = {1.0f, 0.5, 0.0f, 1.0f};
    static char str[16] = {};
    if (item_type == 0) {
      ANCHOR::Text("ITEM: Text");
    }  // Testing text items with no identifier/interaction
    if (item_type == 1) {
      ret = ANCHOR::Button("ITEM: Button");
    }  // Testing button
    if (item_type == 2) {
      ANCHOR::PushButtonRepeat(true);
      ret = ANCHOR::Button("ITEM: Button");
      ANCHOR::PopButtonRepeat();
    }  // Testing button (with repeater)
    if (item_type == 3) {
      ret = ANCHOR::Checkbox("ITEM: Checkbox", &b);
    }  // Testing checkbox
    if (item_type == 4) {
      ret = ANCHOR::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f);
    }  // Testing basic item
    if (item_type == 5) {
      ret = ANCHOR::InputText("ITEM: InputText", &str[0], ANCHOR_ARRAYSIZE(str));
    }  // Testing input text (which handles tabbing)
    if (item_type == 6) {
      ret = ANCHOR::InputFloat("ITEM: InputFloat", col4f, 1.0f);
    }  // Testing +/- buttons on scalar input
    if (item_type == 7) {
      ret = ANCHOR::InputFloat3("ITEM: InputFloat3", col4f);
    }  // Testing multi-component items (IsItemXXX flags are reported merged)
    if (item_type == 8) {
      ret = ANCHOR::ColorEdit4("ITEM: ColorEdit4", col4f);
    }  // Testing multi-component items (IsItemXXX flags are reported merged)
    if (item_type == 9) {
      ret = ANCHOR::MenuItem("ITEM: MenuItem");
    }  // Testing menu item (they use AnchorButtonFlags_PressedOnRelease button policy)
    if (item_type == 10) {
      ret = ANCHOR::TreeNode("ITEM: TreeNode");
      if (ret)
        ANCHOR::TreePop();
    }  // Testing tree node
    if (item_type == 11) {
      ret = ANCHOR::TreeNodeEx("ITEM: TreeNode w/ AnchorTreeNodeFlags_OpenOnDoubleClick",
                               AnchorTreeNodeFlags_OpenOnDoubleClick |
                                 AnchorTreeNodeFlags_NoTreePushOnOpen);
    }  // Testing tree node with AnchorButtonFlags_PressedOnDoubleClick button policy.
    if (item_type == 12) {
      const char *items[] = {"Apple", "Banana", "Cherry", "Kiwi"};
      static int current = 1;
      ret = ANCHOR::Combo("ITEM: Combo", &current, items, ANCHOR_ARRAYSIZE(items));
    }
    if (item_type == 13) {
      const char *items[] = {"Apple", "Banana", "Cherry", "Kiwi"};
      static int current = 1;
      ret = ANCHOR::ListBox("ITEM: ListBox",
                            &current,
                            items,
                            ANCHOR_ARRAYSIZE(items),
                            ANCHOR_ARRAYSIZE(items));
    }

    // Display the values of IsItemHovered() and other common item state functions.
    // Note that the AnchorHoveredFlags_XXX flags can be combined.
    // Because BulletText is an item itself and that would affect the output of IsItemXXX
    // functions, we query every state in a single call to avoid storing them and to simplify the
    // code.
    ANCHOR::BulletText(
      "Return value = %d\n"
      "IsItemFocused() = %d\n"
      "IsItemHovered() = %d\n"
      "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
      "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
      "IsItemHovered(_AllowWhenOverlapped) = %d\n"
      "IsItemHovered(_RectOnly) = %d\n"
      "IsItemActive() = %d\n"
      "IsItemEdited() = %d\n"
      "IsItemActivated() = %d\n"
      "IsItemDeactivated() = %d\n"
      "IsItemDeactivatedAfterEdit() = %d\n"
      "IsItemVisible() = %d\n"
      "IsItemClicked() = %d\n"
      "IsItemToggledOpen() = %d\n"
      "GetItemRectMin() = (%.1f, %.1f)\n"
      "GetItemRectMax() = (%.1f, %.1f)\n"
      "GetItemRectSize() = (%.1f, %.1f)",
      ret,
      ANCHOR::IsItemFocused(),
      ANCHOR::IsItemHovered(),
      ANCHOR::IsItemHovered(AnchorHoveredFlags_AllowWhenBlockedByPopup),
      ANCHOR::IsItemHovered(AnchorHoveredFlags_AllowWhenBlockedByActiveItem),
      ANCHOR::IsItemHovered(AnchorHoveredFlags_AllowWhenOverlapped),
      ANCHOR::IsItemHovered(AnchorHoveredFlags_RectOnly),
      ANCHOR::IsItemActive(),
      ANCHOR::IsItemEdited(),
      ANCHOR::IsItemActivated(),
      ANCHOR::IsItemDeactivated(),
      ANCHOR::IsItemDeactivatedAfterEdit(),
      ANCHOR::IsItemVisible(),
      ANCHOR::IsItemClicked(),
      ANCHOR::IsItemToggledOpen(),
      ANCHOR::GetItemRectMin()[0],
      ANCHOR::GetItemRectMin()[1],
      ANCHOR::GetItemRectMax()[0],
      ANCHOR::GetItemRectMax()[1],
      ANCHOR::GetItemRectSize()[0],
      ANCHOR::GetItemRectSize()[1]);

    static bool embed_all_inside_a_child_window = false;
    ANCHOR::Checkbox("Embed everything inside a child window (for additional testing)",
                     &embed_all_inside_a_child_window);
    if (embed_all_inside_a_child_window)
      ANCHOR::BeginChild("outer_child", GfVec2f(0, ANCHOR::GetFontSize() * 20.0f), true);

    // Testing IsWindowFocused() function with its various flags.
    // Note that the AnchorFocusedFlags_XXX flags can be combined.
    ANCHOR::BulletText(
      "IsWindowFocused() = %d\n"
      "IsWindowFocused(_ChildWindows) = %d\n"
      "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
      "IsWindowFocused(_RootWindow) = %d\n"
      "IsWindowFocused(_AnyWindow) = %d\n",
      ANCHOR::IsWindowFocused(),
      ANCHOR::IsWindowFocused(AnchorFocusedFlags_ChildWindows),
      ANCHOR::IsWindowFocused(AnchorFocusedFlags_ChildWindows | AnchorFocusedFlags_RootWindow),
      ANCHOR::IsWindowFocused(AnchorFocusedFlags_RootWindow),
      ANCHOR::IsWindowFocused(AnchorFocusedFlags_AnyWindow));

    // Testing IsWindowHovered() function with its various flags.
    // Note that the AnchorHoveredFlags_XXX flags can be combined.
    ANCHOR::BulletText(
      "IsWindowHovered() = %d\n"
      "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
      "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
      "IsWindowHovered(_ChildWindows) = %d\n"
      "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
      "IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
      "IsWindowHovered(_RootWindow) = %d\n"
      "IsWindowHovered(_AnyWindow) = %d\n",
      ANCHOR::IsWindowHovered(),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_AllowWhenBlockedByPopup),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_AllowWhenBlockedByActiveItem),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_ChildWindows),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_ChildWindows | AnchorHoveredFlags_RootWindow),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_ChildWindows |
                              AnchorHoveredFlags_AllowWhenBlockedByPopup),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_RootWindow),
      ANCHOR::IsWindowHovered(AnchorHoveredFlags_AnyWindow));

    ANCHOR::BeginChild("child", GfVec2f(0, 50), true);
    ANCHOR::Text("This is another child window for testing the _ChildWindows flag.");
    ANCHOR::EndChild();
    if (embed_all_inside_a_child_window)
      ANCHOR::EndChild();

    static char unused_str[] =
      "This widget is only here to be able to tab-out of the widgets above.";
    ANCHOR::InputText("unused",
                      unused_str,
                      ANCHOR_ARRAYSIZE(unused_str),
                      AnchorInputTextFlags_ReadOnly);

    // Calling IsItemHovered() after begin returns the hovered status of the title bar.
    // This is useful in particular if you want to create a context menu associated to the title
    // bar of a window.
    static bool test_window = false;
    ANCHOR::Checkbox("Hovered/Active tests after Begin() for title bar testing", &test_window);
    if (test_window) {
      ANCHOR::Begin("Title bar Hovered/Active tests", &test_window);
      if (ANCHOR::BeginPopupContextItem())  // <-- This is using IsItemHovered()
      {
        if (ANCHOR::MenuItem("Close")) {
          test_window = false;
        }
        ANCHOR::EndPopup();
      }
      ANCHOR::Text(
        "IsItemHovered() after begin = %d (== is title bar hovered)\n"
        "IsItemActive() after begin = %d (== is window being clicked/moved)\n",
        ANCHOR::IsItemHovered(),
        ANCHOR::IsItemActive());
      ANCHOR::End();
    }

    ANCHOR::TreePop();
  }
}

static void ShowDemoWindowLayout()
{
  if (!ANCHOR::CollapsingHeader("Layout & Scrolling"))
    return;

  if (ANCHOR::TreeNode("Child windows")) {
    HelpMarker(
      "Use child windows to begin into a self-contained independent scrolling/clipping regions "
      "within a host window.");
    static bool disable_mouse_wheel = false;
    static bool disable_menu = false;
    ANCHOR::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
    ANCHOR::Checkbox("Disable Menu", &disable_menu);

    // Child 1: no border, enable horizontal scrollbar
    {
      AnchorWindowFlags window_flags = AnchorWindowFlags_HorizontalScrollbar;
      if (disable_mouse_wheel)
        window_flags |= AnchorWindowFlags_NoScrollWithMouse;
      ANCHOR::BeginChild("ChildL",
                         GfVec2f(ANCHOR::GetWindowContentRegionWidth() * 0.5f, 260),
                         false,
                         window_flags);
      for (int i = 0; i < 100; i++)
        ANCHOR::Text("%04d: scrollable region", i);
      ANCHOR::EndChild();
    }

    ANCHOR::SameLine();

    // Child 2: rounded border
    {
      AnchorWindowFlags window_flags = AnchorWindowFlags_None;
      if (disable_mouse_wheel)
        window_flags |= AnchorWindowFlags_NoScrollWithMouse;
      if (!disable_menu)
        window_flags |= AnchorWindowFlags_MenuBar;
      ANCHOR::PushStyleVar(AnchorStyleVar_ChildRounding, 5.0f);
      ANCHOR::BeginChild("ChildR", GfVec2f(0, 260), true, window_flags);
      if (!disable_menu && ANCHOR::BeginMenuBar()) {
        if (ANCHOR::BeginMenu("Menu")) {
          ShowExampleMenuFile();
          ANCHOR::EndMenu();
        }
        ANCHOR::EndMenuBar();
      }
      if (ANCHOR::BeginTable("split",
                             2,
                             AnchorTableFlags_Resizable | AnchorTableFlags_NoSavedSettings)) {
        for (int i = 0; i < 100; i++) {
          char buf[32];
          sprintf(buf, "%03d", i);
          ANCHOR::TableNextColumn();
          ANCHOR::Button(buf, GfVec2f(-FLT_MIN, 0.0f));
        }
        ANCHOR::EndTable();
      }
      ANCHOR::EndChild();
      ANCHOR::PopStyleVar();
    }

    ANCHOR::Separator();

    // Demonstrate a few extra things
    // - Changing AnchorCol_ChildBg (which is transparent black in default styles)
    // - Using SetCursorPos() to position child window (the child window is an item from the POV of
    // parent window)
    //   You can also call SetNextWindowPos() to position the child window. The parent window will
    //   effectively layout from this position.
    // - Using ANCHOR::GetItemRectMin/Max() to query the "item" state (because the child window is
    // an item from
    //   the POV of the parent window). See 'Demo->Querying Status (Active/Focused/Hovered etc.)'
    //   for details.
    {
      static int offset_x = 0;
      ANCHOR::SetNextItemWidth(ANCHOR::GetFontSize() * 8);
      ANCHOR::DragInt("Offset X", &offset_x, 1.0f, -1000, 1000);

      ANCHOR::SetCursorPosX(ANCHOR::GetCursorPosX() + (float)offset_x);
      ANCHOR::PushStyleColor(AnchorCol_ChildBg, ANCHOR_COL32(255, 0, 0, 100));
      ANCHOR::BeginChild("Red", GfVec2f(200, 100), true, AnchorWindowFlags_None);
      for (int n = 0; n < 50; n++)
        ANCHOR::Text("Some test %d", n);
      ANCHOR::EndChild();
      bool child_is_hovered = ANCHOR::IsItemHovered();
      GfVec2f child_rect_min = ANCHOR::GetItemRectMin();
      GfVec2f child_rect_max = ANCHOR::GetItemRectMax();
      ANCHOR::PopStyleColor();
      ANCHOR::Text("Hovered: %d", child_is_hovered);
      ANCHOR::Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)",
                   child_rect_min[0],
                   child_rect_min[1],
                   child_rect_max[0],
                   child_rect_max[1]);
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Widgets Width")) {
    static float f = 0.0f;
    static bool show_indented_items = true;
    ANCHOR::Checkbox("Show indented items", &show_indented_items);

    // Use SetNextItemWidth() to set the width of a single upcoming item.
    // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
    // In real code use you'll probably want to choose width values that are proportional to your
    // font size e.g. Using '20.0f * GetFontSize()' as width instead of '200.0f', etc.

    ANCHOR::Text("SetNextItemWidth/PushItemWidth(100)");
    ANCHOR::SameLine();
    HelpMarker("Fixed width.");
    ANCHOR::PushItemWidth(100);
    ANCHOR::DragFloat("float##1b", &f);
    if (show_indented_items) {
      ANCHOR::Indent();
      ANCHOR::DragFloat("float (indented)##1b", &f);
      ANCHOR::Unindent();
    }
    ANCHOR::PopItemWidth();

    ANCHOR::Text("SetNextItemWidth/PushItemWidth(-100)");
    ANCHOR::SameLine();
    HelpMarker("Align to right edge minus 100");
    ANCHOR::PushItemWidth(-100);
    ANCHOR::DragFloat("float##2a", &f);
    if (show_indented_items) {
      ANCHOR::Indent();
      ANCHOR::DragFloat("float (indented)##2b", &f);
      ANCHOR::Unindent();
    }
    ANCHOR::PopItemWidth();

    ANCHOR::Text("SetNextItemWidth/PushItemWidth(GetContentRegionAvail()[0] * 0.5f)");
    ANCHOR::SameLine();
    HelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
    ANCHOR::PushItemWidth(ANCHOR::GetContentRegionAvail()[0] * 0.5f);
    ANCHOR::DragFloat("float##3a", &f);
    if (show_indented_items) {
      ANCHOR::Indent();
      ANCHOR::DragFloat("float (indented)##3b", &f);
      ANCHOR::Unindent();
    }
    ANCHOR::PopItemWidth();

    ANCHOR::Text("SetNextItemWidth/PushItemWidth(-GetContentRegionAvail()[0] * 0.5f)");
    ANCHOR::SameLine();
    HelpMarker("Align to right edge minus half");
    ANCHOR::PushItemWidth(-ANCHOR::GetContentRegionAvail()[0] * 0.5f);
    ANCHOR::DragFloat("float##4a", &f);
    if (show_indented_items) {
      ANCHOR::Indent();
      ANCHOR::DragFloat("float (indented)##4b", &f);
      ANCHOR::Unindent();
    }
    ANCHOR::PopItemWidth();

    // Demonstrate using PushItemWidth to surround three items.
    // Calling SetNextItemWidth() before each of them would have the same effect.
    ANCHOR::Text("SetNextItemWidth/PushItemWidth(-FLT_MIN)");
    ANCHOR::SameLine();
    HelpMarker("Align to right edge");
    ANCHOR::PushItemWidth(-FLT_MIN);
    ANCHOR::DragFloat("##float5a", &f);
    if (show_indented_items) {
      ANCHOR::Indent();
      ANCHOR::DragFloat("float (indented)##5b", &f);
      ANCHOR::Unindent();
    }
    ANCHOR::PopItemWidth();

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Basic Horizontal Layout")) {
    ANCHOR::TextWrapped(
      "(Use ANCHOR::SameLine() to keep adding items to the right of the preceding item)");

    // Text
    ANCHOR::Text("Two items: Hello");
    ANCHOR::SameLine();
    ANCHOR::TextColored(GfVec4f(1, 1, 0, 1), "Sailor");

    // Adjust spacing
    ANCHOR::Text("More spacing: Hello");
    ANCHOR::SameLine(0, 20);
    ANCHOR::TextColored(GfVec4f(1, 1, 0, 1), "Sailor");

    // Button
    ANCHOR::AlignTextToFramePadding();
    ANCHOR::Text("Normal buttons");
    ANCHOR::SameLine();
    ANCHOR::Button("Banana");
    ANCHOR::SameLine();
    ANCHOR::Button("Apple");
    ANCHOR::SameLine();
    ANCHOR::Button("Corniflower");

    // Button
    ANCHOR::Text("Small buttons");
    ANCHOR::SameLine();
    ANCHOR::SmallButton("Like this one");
    ANCHOR::SameLine();
    ANCHOR::Text("can fit within a text block.");

    // Aligned to arbitrary position. Easy/cheap column.
    ANCHOR::Text("Aligned");
    ANCHOR::SameLine(150);
    ANCHOR::Text("x=150");
    ANCHOR::SameLine(300);
    ANCHOR::Text("x=300");
    ANCHOR::Text("Aligned");
    ANCHOR::SameLine(150);
    ANCHOR::SmallButton("x=150");
    ANCHOR::SameLine(300);
    ANCHOR::SmallButton("x=300");

    // Checkbox
    static bool c1 = false, c2 = false, c3 = false, c4 = false;
    ANCHOR::Checkbox("My", &c1);
    ANCHOR::SameLine();
    ANCHOR::Checkbox("Tailor", &c2);
    ANCHOR::SameLine();
    ANCHOR::Checkbox("Is", &c3);
    ANCHOR::SameLine();
    ANCHOR::Checkbox("Rich", &c4);

    // Various
    static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
    ANCHOR::PushItemWidth(80);
    const char *items[] = {"AAAA", "BBBB", "CCCC", "DDDD"};
    static int item = -1;
    ANCHOR::Combo("Combo", &item, items, ANCHOR_ARRAYSIZE(items));
    ANCHOR::SameLine();
    ANCHOR::SliderFloat("X", &f0, 0.0f, 5.0f);
    ANCHOR::SameLine();
    ANCHOR::SliderFloat("Y", &f1, 0.0f, 5.0f);
    ANCHOR::SameLine();
    ANCHOR::SliderFloat("Z", &f2, 0.0f, 5.0f);
    ANCHOR::PopItemWidth();

    ANCHOR::PushItemWidth(80);
    ANCHOR::Text("Lists:");
    static int selection[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
      if (i > 0)
        ANCHOR::SameLine();
      ANCHOR::PushID(i);
      ANCHOR::ListBox("", &selection[i], items, ANCHOR_ARRAYSIZE(items));
      ANCHOR::PopID();
      // if (ANCHOR::IsItemHovered()) ANCHOR::SetTooltip("ListBox %d hovered", i);
    }
    ANCHOR::PopItemWidth();

    // Dummy
    GfVec2f button_sz(40, 40);
    ANCHOR::Button("A", button_sz);
    ANCHOR::SameLine();
    ANCHOR::Dummy(button_sz);
    ANCHOR::SameLine();
    ANCHOR::Button("B", button_sz);

    // Manually wrapping
    // (we should eventually provide this as an automatic layout feature, but for now you can do it
    // manually)
    ANCHOR::Text("Manually wrapping:");
    AnchorStyle &style = ANCHOR::GetStyle();
    int buttons_count = 20;
    float window_visible_x2 = ANCHOR::GetWindowPos()[0] + ANCHOR::GetWindowContentRegionMax()[0];
    for (int n = 0; n < buttons_count; n++) {
      ANCHOR::PushID(n);
      ANCHOR::Button("Box", button_sz);
      float last_button_x2 = ANCHOR::GetItemRectMax()[0];
      float next_button_x2 = last_button_x2 + style.ItemSpacing[0] +
                             button_sz[0];  // Expected position if next button was on same line
      if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
        ANCHOR::SameLine();
      ANCHOR::PopID();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Groups")) {
    HelpMarker(
      "BeginGroup() basically locks the horizontal position for new line. "
      "EndGroup() bundles the whole group so that you can use \"item\" functions such as "
      "IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.");
    ANCHOR::BeginGroup();
    {
      ANCHOR::BeginGroup();
      ANCHOR::Button("AAA");
      ANCHOR::SameLine();
      ANCHOR::Button("BBB");
      ANCHOR::SameLine();
      ANCHOR::BeginGroup();
      ANCHOR::Button("CCC");
      ANCHOR::Button("DDD");
      ANCHOR::EndGroup();
      ANCHOR::SameLine();
      ANCHOR::Button("EEE");
      ANCHOR::EndGroup();
      if (ANCHOR::IsItemHovered())
        ANCHOR::SetTooltip("First group hovered");
    }
    // Capture the group size and create widgets using the same size
    GfVec2f size = ANCHOR::GetItemRectSize();
    const float values[5] = {0.5f, 0.20f, 0.80f, 0.60f, 0.25f};
    ANCHOR::PlotHistogram("##values", values, ANCHOR_ARRAYSIZE(values), 0, NULL, 0.0f, 1.0f, size);

    ANCHOR::Button("ACTION",
                   GfVec2f((size[0] - ANCHOR::GetStyle().ItemSpacing[0]) * 0.5f, size[1]));
    ANCHOR::SameLine();
    ANCHOR::Button("REACTION",
                   GfVec2f((size[0] - ANCHOR::GetStyle().ItemSpacing[0]) * 0.5f, size[1]));
    ANCHOR::EndGroup();
    ANCHOR::SameLine();

    ANCHOR::Button("LEVERAGE\nBUZZWORD", size);
    ANCHOR::SameLine();

    if (ANCHOR::BeginListBox("List", size)) {
      ANCHOR::Selectable("Selected", true);
      ANCHOR::Selectable("Not Selected", false);
      ANCHOR::EndListBox();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Text Baseline Alignment")) {
    {
      ANCHOR::BulletText("Text baseline:");
      ANCHOR::SameLine();
      HelpMarker(
        "This is testing the vertical alignment that gets applied on text to keep it aligned "
        "with widgets. "
        "Lines only composed of text or \"small\" widgets use less vertical space than lines "
        "with framed widgets.");
      ANCHOR::Indent();

      ANCHOR::Text("KO Blahblah");
      ANCHOR::SameLine();
      ANCHOR::Button("Some framed item");
      ANCHOR::SameLine();
      HelpMarker("Baseline of button will look misaligned with text..");

      // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming
      // widgets. (because we don't know what's coming after the Text() statement, we need to move
      // the text baseline down by FramePadding[1] ahead of time)
      ANCHOR::AlignTextToFramePadding();
      ANCHOR::Text("OK Blahblah");
      ANCHOR::SameLine();
      ANCHOR::Button("Some framed item");
      ANCHOR::SameLine();
      HelpMarker(
        "We call AlignTextToFramePadding() to vertically align the text baseline by "
        "+FramePadding[1]");

      // SmallButton() uses the same vertical padding as Text
      ANCHOR::Button("TEST##1");
      ANCHOR::SameLine();
      ANCHOR::Text("TEST");
      ANCHOR::SameLine();
      ANCHOR::SmallButton("TEST##2");

      // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming
      // widgets.
      ANCHOR::AlignTextToFramePadding();
      ANCHOR::Text("Text aligned to framed item");
      ANCHOR::SameLine();
      ANCHOR::Button("Item##1");
      ANCHOR::SameLine();
      ANCHOR::Text("Item");
      ANCHOR::SameLine();
      ANCHOR::SmallButton("Item##2");
      ANCHOR::SameLine();
      ANCHOR::Button("Item##3");

      ANCHOR::Unindent();
    }

    ANCHOR::Spacing();

    {
      ANCHOR::BulletText("Multi-line text:");
      ANCHOR::Indent();
      ANCHOR::Text("One\nTwo\nThree");
      ANCHOR::SameLine();
      ANCHOR::Text("Hello\nWorld");
      ANCHOR::SameLine();
      ANCHOR::Text("Banana");

      ANCHOR::Text("Banana");
      ANCHOR::SameLine();
      ANCHOR::Text("Hello\nWorld");
      ANCHOR::SameLine();
      ANCHOR::Text("One\nTwo\nThree");

      ANCHOR::Button("HOP##1");
      ANCHOR::SameLine();
      ANCHOR::Text("Banana");
      ANCHOR::SameLine();
      ANCHOR::Text("Hello\nWorld");
      ANCHOR::SameLine();
      ANCHOR::Text("Banana");

      ANCHOR::Button("HOP##2");
      ANCHOR::SameLine();
      ANCHOR::Text("Hello\nWorld");
      ANCHOR::SameLine();
      ANCHOR::Text("Banana");
      ANCHOR::Unindent();
    }

    ANCHOR::Spacing();

    {
      ANCHOR::BulletText("Misc items:");
      ANCHOR::Indent();

      // SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of
      // previous Button.
      ANCHOR::Button("80x80", GfVec2f(80, 80));
      ANCHOR::SameLine();
      ANCHOR::Button("50x50", GfVec2f(50, 50));
      ANCHOR::SameLine();
      ANCHOR::Button("Button()");
      ANCHOR::SameLine();
      ANCHOR::SmallButton("SmallButton()");

      // Tree
      const float spacing = ANCHOR::GetStyle().ItemInnerSpacing[0];
      ANCHOR::Button("Button##1");
      ANCHOR::SameLine(0.0f, spacing);
      if (ANCHOR::TreeNode("Node##1")) {
        // Placeholder tree data
        for (int i = 0; i < 6; i++)
          ANCHOR::BulletText("Item %d..", i);
        ANCHOR::TreePop();
      }

      // Vertically align text node a bit lower so it'll be vertically centered with upcoming
      // widget. Otherwise you can use SmallButton() (smaller fit).
      ANCHOR::AlignTextToFramePadding();

      // Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we
      // add other contents below the node.
      bool node_open = ANCHOR::TreeNode("Node##2");
      ANCHOR::SameLine(0.0f, spacing);
      ANCHOR::Button("Button##2");
      if (node_open) {
        // Placeholder tree data
        for (int i = 0; i < 6; i++)
          ANCHOR::BulletText("Item %d..", i);
        ANCHOR::TreePop();
      }

      // Bullet
      ANCHOR::Button("Button##3");
      ANCHOR::SameLine(0.0f, spacing);
      ANCHOR::BulletText("Bullet text");

      ANCHOR::AlignTextToFramePadding();
      ANCHOR::BulletText("Node");
      ANCHOR::SameLine(0.0f, spacing);
      ANCHOR::Button("Button##4");
      ANCHOR::Unindent();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Scrolling")) {
    // Vertical scroll functions
    HelpMarker(
      "Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given vertical position.");

    static int track_item = 50;
    static bool enable_track = true;
    static bool enable_extra_decorations = false;
    static float scroll_to_off_px = 0.0f;
    static float scroll_to_pos_px = 200.0f;

    ANCHOR::Checkbox("Decoration", &enable_extra_decorations);

    ANCHOR::Checkbox("Track", &enable_track);
    ANCHOR::PushItemWidth(100);
    ANCHOR::SameLine(140);
    enable_track |= ANCHOR::DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");

    bool scroll_to_off = ANCHOR::Button("Scroll Offset");
    ANCHOR::SameLine(140);
    scroll_to_off |= ANCHOR::DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");

    bool scroll_to_pos = ANCHOR::Button("Scroll To Pos");
    ANCHOR::SameLine(140);
    scroll_to_pos |=
      ANCHOR::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
    ANCHOR::PopItemWidth();

    if (scroll_to_off || scroll_to_pos)
      enable_track = false;

    AnchorStyle &style = ANCHOR::GetStyle();
    float child_w = (ANCHOR::GetContentRegionAvail()[0] - 4 * style.ItemSpacing[0]) / 5;
    if (child_w < 1.0f)
      child_w = 1.0f;
    ANCHOR::PushID("##VerticalScrolling");
    for (int i = 0; i < 5; i++) {
      if (i > 0)
        ANCHOR::SameLine();
      ANCHOR::BeginGroup();
      const char *names[] = {"Top", "25%", "Center", "75%", "Bottom"};
      ANCHOR::TextUnformatted(names[i]);

      const AnchorWindowFlags child_flags = enable_extra_decorations ? AnchorWindowFlags_MenuBar :
                                                                       0;
      const ANCHOR_ID child_id = ANCHOR::GetID((void *)(intptr_t)i);
      const bool child_is_visible = ANCHOR::BeginChild(child_id,
                                                       GfVec2f(child_w, 200.0f),
                                                       true,
                                                       child_flags);
      if (ANCHOR::BeginMenuBar()) {
        ANCHOR::TextUnformatted("abc");
        ANCHOR::EndMenuBar();
      }
      if (scroll_to_off)
        ANCHOR::SetScrollY(scroll_to_off_px);
      if (scroll_to_pos)
        ANCHOR::SetScrollFromPosY(ANCHOR::GetCursorStartPos()[1] + scroll_to_pos_px, i * 0.25f);
      if (child_is_visible)  // Avoid calling SetScrollHereY when running with culled items
      {
        for (int item = 0; item < 100; item++) {
          if (enable_track && item == track_item) {
            ANCHOR::TextColored(GfVec4f(1, 1, 0, 1), "Item %d", item);
            ANCHOR::SetScrollHereY(i * 0.25f);  // 0.0f:top, 0.5f:center, 1.0f:bottom
          } else {
            ANCHOR::Text("Item %d", item);
          }
        }
      }
      float scroll_y = ANCHOR::GetScrollY();
      float scroll_max_y = ANCHOR::GetScrollMaxY();
      ANCHOR::EndChild();
      ANCHOR::Text("%.0f/%.0f", scroll_y, scroll_max_y);
      ANCHOR::EndGroup();
    }
    ANCHOR::PopID();

    // Horizontal scroll functions
    ANCHOR::Spacing();
    HelpMarker(
      "Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\n"
      "Because the clipping rectangle of most window hides half worth of WindowPadding on the "
      "left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the "
      "equivalent SetScrollFromPosY(+1) wouldn't.");
    ANCHOR::PushID("##HorizontalScrolling");
    for (int i = 0; i < 5; i++) {
      float child_height = ANCHOR::GetTextLineHeight() + style.ScrollbarSize +
                           style.WindowPadding[1] * 2.0f;
      AnchorWindowFlags child_flags = AnchorWindowFlags_HorizontalScrollbar |
                                      (enable_extra_decorations ?
                                         AnchorWindowFlags_AlwaysVerticalScrollbar :
                                         0);
      ANCHOR_ID child_id = ANCHOR::GetID((void *)(intptr_t)i);
      bool child_is_visible = ANCHOR::BeginChild(child_id,
                                                 GfVec2f(-100, child_height),
                                                 true,
                                                 child_flags);
      if (scroll_to_off)
        ANCHOR::SetScrollX(scroll_to_off_px);
      if (scroll_to_pos)
        ANCHOR::SetScrollFromPosX(ANCHOR::GetCursorStartPos()[0] + scroll_to_pos_px, i * 0.25f);
      if (child_is_visible)  // Avoid calling SetScrollHereY when running with culled items
      {
        for (int item = 0; item < 100; item++) {
          if (item > 0)
            ANCHOR::SameLine();
          if (enable_track && item == track_item) {
            ANCHOR::TextColored(GfVec4f(1, 1, 0, 1), "Item %d", item);
            ANCHOR::SetScrollHereX(i * 0.25f);  // 0.0f:left, 0.5f:center, 1.0f:right
          } else {
            ANCHOR::Text("Item %d", item);
          }
        }
      }
      float scroll_x = ANCHOR::GetScrollX();
      float scroll_max_x = ANCHOR::GetScrollMaxX();
      ANCHOR::EndChild();
      ANCHOR::SameLine();
      const char *names[] = {"Left", "25%", "Center", "75%", "Right"};
      ANCHOR::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
      ANCHOR::Spacing();
    }
    ANCHOR::PopID();

    // Miscellaneous Horizontal Scrolling Demo
    HelpMarker(
      "Horizontal scrolling for a window is enabled via the "
      "AnchorWindowFlags_HorizontalScrollbar flag.\n\n"
      "You may want to also explicitly specify content width by using "
      "SetNextWindowContentWidth() before Begin().");
    static int lines = 7;
    ANCHOR::SliderInt("Lines", &lines, 1, 15);
    ANCHOR::PushStyleVar(AnchorStyleVar_FrameRounding, 3.0f);
    ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(2.0f, 1.0f));
    GfVec2f scrolling_child_size = GfVec2f(0, ANCHOR::GetFrameHeightWithSpacing() * 7 + 30);
    ANCHOR::BeginChild("scrolling",
                       scrolling_child_size,
                       true,
                       AnchorWindowFlags_HorizontalScrollbar);
    for (int line = 0; line < lines; line++) {
      // Display random stuff. For the sake of this trivial demo we are using basic Button() +
      // SameLine() If you want to create your own time line for a real application you may be
      // better off manipulating the cursor position yourself, aka using
      // SetCursorPos/SetCursorScreenPos to position the widgets yourself. You may also want to use
      // the lower-level AnchorDrawList API.
      int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
      for (int n = 0; n < num_buttons; n++) {
        if (n > 0)
          ANCHOR::SameLine();
        ANCHOR::PushID(n + line * 1000);
        char num_buf[16];
        sprintf(num_buf, "%d", n);
        const char *label = (!(n % 15)) ? "FizzBuzz" :
                            (!(n % 3))  ? "Fizz" :
                            (!(n % 5))  ? "Buzz" :
                                          num_buf;
        float hue = n * 0.05f;
        ANCHOR::PushStyleColor(AnchorCol_Button, AnchorColor::HSV(hue, 0.6f, 0.6f).Value);
        ANCHOR::PushStyleColor(AnchorCol_ButtonHovered, AnchorColor::HSV(hue, 0.7f, 0.7f).Value);
        ANCHOR::PushStyleColor(AnchorCol_ButtonActive, AnchorColor::HSV(hue, 0.8f, 0.8f).Value);
        ANCHOR::Button(label, GfVec2f(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
        ANCHOR::PopStyleColor(3);
        ANCHOR::PopID();
      }
    }
    float scroll_x = ANCHOR::GetScrollX();
    float scroll_max_x = ANCHOR::GetScrollMaxX();
    ANCHOR::EndChild();
    ANCHOR::PopStyleVar(2);
    float scroll_x_delta = 0.0f;
    ANCHOR::SmallButton("<<");
    if (ANCHOR::IsItemActive())
      scroll_x_delta = -ANCHOR::GetIO().DeltaTime * 1000.0f;
    ANCHOR::SameLine();
    ANCHOR::Text("Scroll from code");
    ANCHOR::SameLine();
    ANCHOR::SmallButton(">>");
    if (ANCHOR::IsItemActive())
      scroll_x_delta = +ANCHOR::GetIO().DeltaTime * 1000.0f;
    ANCHOR::SameLine();
    ANCHOR::Text("%.0f/%.0f", scroll_x, scroll_max_x);
    if (scroll_x_delta != 0.0f) {
      // Demonstrate a trick: you can use Begin to set yourself in the context of another window
      // (here we are already out of your child window)
      ANCHOR::BeginChild("scrolling");
      ANCHOR::SetScrollX(ANCHOR::GetScrollX() + scroll_x_delta);
      ANCHOR::EndChild();
    }
    ANCHOR::Spacing();

    static bool show_horizontal_contents_size_demo_window = false;
    ANCHOR::Checkbox("Show Horizontal contents size demo window",
                     &show_horizontal_contents_size_demo_window);

    if (show_horizontal_contents_size_demo_window) {
      static bool show_h_scrollbar = true;
      static bool show_button = true;
      static bool show_tree_nodes = true;
      static bool show_text_wrapped = false;
      static bool show_columns = true;
      static bool show_tab_bar = true;
      static bool show_child = false;
      static bool explicit_content_size = false;
      static float contents_size_x = 300.0f;
      if (explicit_content_size)
        ANCHOR::SetNextWindowContentSize(GfVec2f(contents_size_x, 0.0f));
      ANCHOR::Begin("Horizontal contents size demo window",
                    &show_horizontal_contents_size_demo_window,
                    show_h_scrollbar ? AnchorWindowFlags_HorizontalScrollbar : 0);
      ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing, GfVec2f(2, 0));
      ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(2, 0));
      HelpMarker(
        "Test of different widgets react and impact the work rectangle growing when horizontal "
        "scrolling is enabled.\n\nUse 'Metrics->Tools->Show windows rectangles' to visualize "
        "rectangles.");
      ANCHOR::Checkbox("H-scrollbar", &show_h_scrollbar);
      ANCHOR::Checkbox("Button",
                       &show_button);  // Will grow contents size (unless explicitly overwritten)
      ANCHOR::Checkbox(
        "Tree nodes",
        &show_tree_nodes);  // Will grow contents size and display highlight over full width
      ANCHOR::Checkbox("Text wrapped", &show_text_wrapped);  // Will grow and use contents size
      ANCHOR::Checkbox("Columns", &show_columns);            // Will use contents size
      ANCHOR::Checkbox("Tab bar", &show_tab_bar);            // Will use contents size
      ANCHOR::Checkbox("Child", &show_child);                // Will grow and use contents size
      ANCHOR::Checkbox("Explicit content size", &explicit_content_size);
      ANCHOR::Text("Scroll %.1f/%.1f %.1f/%.1f",
                   ANCHOR::GetScrollX(),
                   ANCHOR::GetScrollMaxX(),
                   ANCHOR::GetScrollY(),
                   ANCHOR::GetScrollMaxY());
      if (explicit_content_size) {
        ANCHOR::SameLine();
        ANCHOR::SetNextItemWidth(100);
        ANCHOR::DragFloat("##csx", &contents_size_x);
        GfVec2f p = ANCHOR::GetCursorScreenPos();
        ANCHOR::GetWindowDrawList()->AddRectFilled(p,
                                                   GfVec2f(p[0] + 10, p[1] + 10),
                                                   ANCHOR_COL32_WHITE);
        ANCHOR::GetWindowDrawList()->AddRectFilled(GfVec2f(p[0] + contents_size_x - 10, p[1]),
                                                   GfVec2f(p[0] + contents_size_x, p[1] + 10),
                                                   ANCHOR_COL32_WHITE);
        ANCHOR::Dummy(GfVec2f(0, 10));
      }
      ANCHOR::PopStyleVar(2);
      ANCHOR::Separator();
      if (show_button) {
        ANCHOR::Button("this is a 300-wide button", GfVec2f(300, 0));
      }
      if (show_tree_nodes) {
        bool open = true;
        if (ANCHOR::TreeNode("this is a tree node")) {
          if (ANCHOR::TreeNode("another one of those tree node...")) {
            ANCHOR::Text("Some tree contents");
            ANCHOR::TreePop();
          }
          ANCHOR::TreePop();
        }
        ANCHOR::CollapsingHeader("CollapsingHeader", &open);
      }
      if (show_text_wrapped) {
        ANCHOR::TextWrapped(
          "This text should automatically wrap on the edge of the work rectangle.");
      }
      if (show_columns) {
        ANCHOR::Text("Tables:");
        if (ANCHOR::BeginTable("table", 4, AnchorTableFlags_Borders)) {
          for (int n = 0; n < 4; n++) {
            ANCHOR::TableNextColumn();
            ANCHOR::Text("Width %.2f", ANCHOR::GetContentRegionAvail()[0]);
          }
          ANCHOR::EndTable();
        }
        ANCHOR::Text("Columns:");
        ANCHOR::Columns(4);
        for (int n = 0; n < 4; n++) {
          ANCHOR::Text("Width %.2f", ANCHOR::GetColumnWidth());
          ANCHOR::NextColumn();
        }
        ANCHOR::Columns(1);
      }
      if (show_tab_bar && ANCHOR::BeginTabBar("Hello")) {
        if (ANCHOR::BeginTabItem("OneOneOne")) {
          ANCHOR::EndTabItem();
        }
        if (ANCHOR::BeginTabItem("TwoTwoTwo")) {
          ANCHOR::EndTabItem();
        }
        if (ANCHOR::BeginTabItem("ThreeThreeThree")) {
          ANCHOR::EndTabItem();
        }
        if (ANCHOR::BeginTabItem("FourFourFour")) {
          ANCHOR::EndTabItem();
        }
        ANCHOR::EndTabBar();
      }
      if (show_child) {
        ANCHOR::BeginChild("child", GfVec2f(0, 0), true);
        ANCHOR::EndChild();
      }
      ANCHOR::End();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Clipping")) {
    static GfVec2f size(100.0f, 100.0f);
    static GfVec2f offset(30.0f, 30.0f);
    ANCHOR::DragFloat2("size", (float *)&size, 0.5f, 1.0f, 200.0f, "%.0f");
    ANCHOR::TextWrapped("(Click and drag to scroll)");

    for (int n = 0; n < 3; n++) {
      if (n > 0)
        ANCHOR::SameLine();
      ANCHOR::PushID(n);
      ANCHOR::BeginGroup();  // Lock X position

      ANCHOR::InvisibleButton("##empty", size);
      if (ANCHOR::IsItemActive() && ANCHOR::IsMouseDragging(AnchorMouseButton_Left)) {
        offset[0] += ANCHOR::GetIO().MouseDelta[0];
        offset[1] += ANCHOR::GetIO().MouseDelta[1];
      }
      const GfVec2f p0 = ANCHOR::GetItemRectMin();
      const GfVec2f p1 = ANCHOR::GetItemRectMax();
      const char *text_str = "Line 1 hello\nLine 2 clip me!";
      const GfVec2f text_pos = GfVec2f(p0[0] + offset[0], p0[1] + offset[1]);
      AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();

      switch (n) {
        case 0:
          HelpMarker(
            "Using ANCHOR::PushClipRect():\n"
            "Will alter ANCHOR hit-testing logic + AnchorDrawList rendering.\n"
            "(use this if you want your clipping rectangle to affect interactions)");
          ANCHOR::PushClipRect(p0, p1, true);
          draw_list->AddRectFilled(p0, p1, ANCHOR_COL32(90, 90, 120, 255));
          draw_list->AddText(text_pos, ANCHOR_COL32_WHITE, text_str);
          ANCHOR::PopClipRect();
          break;
        case 1:
          HelpMarker(
            "Using AnchorDrawList::PushClipRect():\n"
            "Will alter AnchorDrawList rendering only.\n"
            "(use this as a shortcut if you are only using AnchorDrawList calls)");
          draw_list->PushClipRect(p0, p1, true);
          draw_list->AddRectFilled(p0, p1, ANCHOR_COL32(90, 90, 120, 255));
          draw_list->AddText(text_pos, ANCHOR_COL32_WHITE, text_str);
          draw_list->PopClipRect();
          break;
        case 2:
          HelpMarker(
            "Using AnchorDrawList::AddText() with a fine ClipRect:\n"
            "Will alter only this specific AnchorDrawList::AddText() rendering.\n"
            "(this is often used internally to avoid altering the clipping rectangle and "
            "minimize draw calls)");
          GfVec4f clip_rect(p0[0],
                            p0[1],
                            p1[0],
                            p1[1]);  // AddText() takes a GfVec4f* here so let's convert.
          draw_list->AddRectFilled(p0, p1, ANCHOR_COL32(90, 90, 120, 255));
          draw_list->AddText(ANCHOR::GetFont(),
                             ANCHOR::GetFontSize(),
                             text_pos,
                             ANCHOR_COL32_WHITE,
                             text_str,
                             NULL,
                             0.0f,
                             &clip_rect);
          break;
      }
      ANCHOR::EndGroup();
      ANCHOR::PopID();
    }

    ANCHOR::TreePop();
  }
}

static void ShowDemoWindowPopups()
{
  if (!ANCHOR::CollapsingHeader("Popups & Modal windows"))
    return;

  // The properties of popups windows are:
  // - They block normal mouse hovering detection outside them. (*)
  // - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
  // - Their visibility state (~bool) is held internally by ANCHOR instead of being held by the
  // programmer as
  //   we are used to with regular Begin() calls. User can manipulate the visibility state by
  //   calling OpenPopup().
  // (*) One can use IsItemHovered(AnchorHoveredFlags_AllowWhenBlockedByPopup) to bypass it and
  // detect hovering even
  //     when normally blocked by a popup.
  // Those three properties are connected. The library needs to hold their visibility state BECAUSE
  // it can close popups at any time.

  // Typical use for regular windows:
  //   bool my_tool_is_active = false; if (ANCHOR::Button("Open")) my_tool_is_active = true; [...]
  //   if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
  // Typical use for popups:
  //   if (ANCHOR::Button("Open")) ANCHOR::OpenPopup("MyPopup"); if (ANCHOR::BeginPopup("MyPopup")
  //   { [...] EndPopup(); }

  // With popups we have to go through a library call (here OpenPopup) to manipulate the visibility
  // state. This may be a bit confusing at first but it should quickly make sense. Follow on the
  // examples below.

  if (ANCHOR::TreeNode("Popups")) {
    ANCHOR::TextWrapped(
      "When a popup is active, it inhibits interacting with windows that are behind the popup. "
      "Clicking outside the popup closes it.");

    static int selected_fish = -1;
    const char *names[] = {"Bream", "Haddock", "Mackerel", "Pollock", "Tilefish"};
    static bool toggles[] = {true, false, false, false, false};

    // Simple selection popup (if you want to show the current selection inside the Button itself,
    // you may want to build a string using the "###" operator to preserve a constant ID with a
    // variable label)
    if (ANCHOR::Button("Select.."))
      ANCHOR::OpenPopup("my_select_popup");
    ANCHOR::SameLine();
    ANCHOR::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
    if (ANCHOR::BeginPopup("my_select_popup")) {
      ANCHOR::Text("Aquarium");
      ANCHOR::Separator();
      for (int i = 0; i < ANCHOR_ARRAYSIZE(names); i++)
        if (ANCHOR::Selectable(names[i]))
          selected_fish = i;
      ANCHOR::EndPopup();
    }

    // Showing a menu with toggles
    if (ANCHOR::Button("Toggle.."))
      ANCHOR::OpenPopup("my_toggle_popup");
    if (ANCHOR::BeginPopup("my_toggle_popup")) {
      for (int i = 0; i < ANCHOR_ARRAYSIZE(names); i++)
        ANCHOR::MenuItem(names[i], "", &toggles[i]);
      if (ANCHOR::BeginMenu("Sub-menu")) {
        ANCHOR::MenuItem("Click me");
        ANCHOR::EndMenu();
      }

      ANCHOR::Separator();
      ANCHOR::Text("Tooltip here");
      if (ANCHOR::IsItemHovered())
        ANCHOR::SetTooltip("I am a tooltip over a popup");

      if (ANCHOR::Button("Stacked Popup"))
        ANCHOR::OpenPopup("another popup");
      if (ANCHOR::BeginPopup("another popup")) {
        for (int i = 0; i < ANCHOR_ARRAYSIZE(names); i++)
          ANCHOR::MenuItem(names[i], "", &toggles[i]);
        if (ANCHOR::BeginMenu("Sub-menu")) {
          ANCHOR::MenuItem("Click me");
          if (ANCHOR::Button("Stacked Popup"))
            ANCHOR::OpenPopup("another popup");
          if (ANCHOR::BeginPopup("another popup")) {
            ANCHOR::Text("I am the last one here.");
            ANCHOR::EndPopup();
          }
          ANCHOR::EndMenu();
        }
        ANCHOR::EndPopup();
      }
      ANCHOR::EndPopup();
    }

    // Call the more complete ShowExampleMenuFile which we use in various places of this demo
    if (ANCHOR::Button("File Menu.."))
      ANCHOR::OpenPopup("my_file_popup");
    if (ANCHOR::BeginPopup("my_file_popup")) {
      ShowExampleMenuFile();
      ANCHOR::EndPopup();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Context menus")) {
    HelpMarker(
      "\"Context\" functions are simple helpers to associate a Popup to a given Item or Window "
      "identifier.");

    // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially
    // doing:
    //     if (id == 0)
    //         id = GetItemID(); // Use last item id
    //     if (IsItemHovered() && IsMouseReleased(AnchorMouseButton_Right))
    //         OpenPopup(id);
    //     return BeginPopup(id);
    // For advanced advanced uses you may want to replicate and customize this code.
    // See more details in BeginPopupContextItem().

    // Example 1
    // When used after an item that has an ID (e.g. Button), we can skip providing an ID to
    // BeginPopupContextItem(), and BeginPopupContextItem() will use the last item ID as the popup
    // ID.
    {
      const char *names[5] = {"Label1", "Label2", "Label3", "Label4", "Label5"};
      for (int n = 0; n < 5; n++) {
        ANCHOR::Selectable(names[n]);
        if (ANCHOR::BeginPopupContextItem())  // <-- use last item id as popup id
        {
          ANCHOR::Text("This a popup for \"%s\"!", names[n]);
          if (ANCHOR::Button("Close"))
            ANCHOR::CloseCurrentPopup();
          ANCHOR::EndPopup();
        }
        if (ANCHOR::IsItemHovered())
          ANCHOR::SetTooltip("Right-click to open popup");
      }
    }

    // Example 2
    // Popup on a Text() element which doesn't have an identifier: we need to provide an identifier
    // to BeginPopupContextItem(). Using an explicit identifier is also convenient if you want to
    // activate the popups from different locations.
    {
      HelpMarker("Text() elements don't have stable identifiers so we need to provide one.");
      static float value = 0.5f;
      ANCHOR::Text("Value = %.3f <-- (1) right-click this value", value);
      if (ANCHOR::BeginPopupContextItem("my popup")) {
        if (ANCHOR::Selectable("Set to zero"))
          value = 0.0f;
        if (ANCHOR::Selectable("Set to PI"))
          value = 3.1415f;
        ANCHOR::SetNextItemWidth(-FLT_MIN);
        ANCHOR::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
        ANCHOR::EndPopup();
      }

      // We can also use OpenPopupOnItemClick() to toggle the visibility of a given popup.
      // Here we make it that right-clicking this other text element opens the same popup as above.
      // The popup itself will be submitted by the code above.
      ANCHOR::Text("(2) Or right-click this text");
      ANCHOR::OpenPopupOnItemClick("my popup", AnchorPopupFlags_MouseButtonRight);

      // Back to square one: manually open the same popup.
      if (ANCHOR::Button("(3) Or click this button"))
        ANCHOR::OpenPopup("my popup");
    }

    // Example 3
    // When using BeginPopupContextItem() with an implicit identifier (NULL == use last item ID),
    // we need to make sure your item identifier is stable.
    // In this example we showcase altering the item label while preserving its identifier, using
    // the ### operator (see FAQ).
    {
      HelpMarker(
        "Showcase using a popup ID linked to item ID, with the item having a changing label + "
        "stable ID using the ### operator.");
      static char name[32] = "Label1";
      char buf[64];
      sprintf(buf,
              "Button: %s###Button",
              name);  // ### operator override ID ignoring the preceding label
      ANCHOR::Button(buf);
      if (ANCHOR::BeginPopupContextItem()) {
        ANCHOR::Text("Edit name:");
        ANCHOR::InputText("##edit", name, ANCHOR_ARRAYSIZE(name));
        if (ANCHOR::Button("Close"))
          ANCHOR::CloseCurrentPopup();
        ANCHOR::EndPopup();
      }
      ANCHOR::SameLine();
      ANCHOR::Text("(<-- right-click here)");
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Modals")) {
    ANCHOR::TextWrapped(
      "Modal windows are like popups but the user cannot close them by clicking outside.");

    if (ANCHOR::Button("Delete.."))
      ANCHOR::OpenPopup("Delete?");

    // Always center this window when appearing
    GfVec2f center = ANCHOR::GetMainViewport()->GetCenter();
    ANCHOR::SetNextWindowPos(center, AnchorCond_Appearing, GfVec2f(0.5f, 0.5f));

    if (ANCHOR::BeginPopupModal("Delete?", NULL, AnchorWindowFlags_AlwaysAutoResize)) {
      ANCHOR::Text(
        "All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
      ANCHOR::Separator();

      // static int unused_i = 0;
      // ANCHOR::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

      static bool dont_ask_me_next_time = false;
      ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(0, 0));
      ANCHOR::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
      ANCHOR::PopStyleVar();

      if (ANCHOR::Button("OK", GfVec2f(120, 0))) {
        ANCHOR::CloseCurrentPopup();
      }
      ANCHOR::SetItemDefaultFocus();
      ANCHOR::SameLine();
      if (ANCHOR::Button("Cancel", GfVec2f(120, 0))) {
        ANCHOR::CloseCurrentPopup();
      }
      ANCHOR::EndPopup();
    }

    if (ANCHOR::Button("Stacked modals.."))
      ANCHOR::OpenPopup("Stacked 1");
    if (ANCHOR::BeginPopupModal("Stacked 1", NULL, AnchorWindowFlags_MenuBar)) {
      if (ANCHOR::BeginMenuBar()) {
        if (ANCHOR::BeginMenu("File")) {
          if (ANCHOR::MenuItem("Some menu item")) {
          }
          ANCHOR::EndMenu();
        }
        ANCHOR::EndMenuBar();
      }
      ANCHOR::Text(
        "Hello from Stacked The First\nUsing style.Colors[AnchorCol_ModalWindowDimBg] behind "
        "it.");

      // Testing behavior of widgets stacking their own regular popups over the modal.
      static int item = 1;
      static float color[4] = {0.4f, 0.7f, 0.0f, 0.5f};
      ANCHOR::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
      ANCHOR::ColorEdit4("color", color);

      if (ANCHOR::Button("Add another modal.."))
        ANCHOR::OpenPopup("Stacked 2");

      // Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close
      // button which will close the popup. Note that the visibility state of popups is owned by
      // ANCHOR, so the input value of the bool actually doesn't matter here.
      bool unused_open = true;
      if (ANCHOR::BeginPopupModal("Stacked 2", &unused_open)) {
        ANCHOR::Text("Hello from Stacked The Second!");
        if (ANCHOR::Button("Close"))
          ANCHOR::CloseCurrentPopup();
        ANCHOR::EndPopup();
      }

      if (ANCHOR::Button("Close"))
        ANCHOR::CloseCurrentPopup();
      ANCHOR::EndPopup();
    }

    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Menus inside a regular window")) {
    ANCHOR::TextWrapped(
      "Below we are testing adding menu items to a regular window. It's rather unusual but "
      "should work!");
    ANCHOR::Separator();

    // Note: As a quirk in this very specific example, we want to differentiate the parent of this
    // menu from the parent of the various popup menus above. To do so we are encloding the items
    // in a PushID()/PopID() block to make them two different menusets. If we don't, opening any
    // popup above and hovering our menu here would open it. This is because once a menu is active,
    // we allow to switch to a sibling menu by just hovering on it, which is the desired behavior
    // for regular menus.
    ANCHOR::PushID("foo");
    ANCHOR::MenuItem("Menu item", "CTRL+M");
    if (ANCHOR::BeginMenu("Menu inside a regular window")) {
      ShowExampleMenuFile();
      ANCHOR::EndMenu();
    }
    ANCHOR::PopID();
    ANCHOR::Separator();
    ANCHOR::TreePop();
  }
}

// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate AnchorVector<MyItem> template if this structure if
// defined inside the demo function)
namespace
{
  // We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in
  // the sorting code. This identifier will be passed down into AnchorTableSortSpec::ColumnUserID.
  // But it is possible to omit the user id parameter of TableSetupColumn() and just use the column
  // index instead! (AnchorTableSortSpec::ColumnIndex) If you don't use sorting, you will generally
  // never care about giving column an ID!
  enum MyItemColumnID
  {
    MyItemColumnID_ID,
    MyItemColumnID_Name,
    MyItemColumnID_Action,
    MyItemColumnID_Quantity,
    MyItemColumnID_Description
  };

  struct MyItem
  {
    int ID;
    const char *Name;
    int Quantity;

    // We have a problem which is affecting _only this demo_ and should not affect your code:
    // As we don't rely on std:: or other third-party library to compile ANCHOR, we only have
    // reliable access to qsort(), however qsort doesn't allow passing user data to comparing
    // function. As a workaround, we are storing the sort specs in a static/global for the
    // comparing function to access. In your own use case you would probably pass the sort specs to
    // your sorting/comparing functions directly and not use a global. We could technically call
    // ANCHOR::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is
    // called very often by the sorting algorithm it would be a little wasteful.
    static const AnchorTableSortSpecs *s_current_sort_specs;

    // Compare function to be used by qsort()
    static int ANCHOR_CDECL CompareWithSortSpecs(const void *lhs, const void *rhs)
    {
      const MyItem *a = (const MyItem *)lhs;
      const MyItem *b = (const MyItem *)rhs;
      for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
        // Here we identify columns using the ColumnUserID value that we ourselves passed to
        // TableSetupColumn() We could also choose to identify columns based on their index
        // (sort_spec->ColumnIndex), which is simpler!
        const AnchorTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
        int delta = 0;
        switch (sort_spec->ColumnUserID) {
          case MyItemColumnID_ID:
            delta = (a->ID - b->ID);
            break;
          case MyItemColumnID_Name:
            delta = (strcmp(a->Name, b->Name));
            break;
          case MyItemColumnID_Quantity:
            delta = (a->Quantity - b->Quantity);
            break;
          case MyItemColumnID_Description:
            delta = (strcmp(a->Name, b->Name));
            break;
          default:
            ANCHOR_ASSERT(0);
            break;
        }
        if (delta > 0)
          return (sort_spec->SortDirection == AnchorSortDirection_Ascending) ? +1 : -1;
        if (delta < 0)
          return (sort_spec->SortDirection == AnchorSortDirection_Ascending) ? -1 : +1;
      }

      // qsort() is instable so always return a way to differenciate items.
      // Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name
      // compare if it wasn't already part of the sort specs.
      return (a->ID - b->ID);
    }
  };
  const AnchorTableSortSpecs *MyItem::s_current_sort_specs = NULL;
}  // namespace

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
  AnchorStyle &style = ANCHOR::GetStyle();
  ANCHOR::PushStyleVar(
    AnchorStyleVar_FramePadding,
    GfVec2f(style.FramePadding[0], (float)(int)(style.FramePadding[1] * 0.60f)));
  ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing,
                       GfVec2f(style.ItemSpacing[0], (float)(int)(style.ItemSpacing[1] * 0.60f)));
}

static void PopStyleCompact()
{
  ANCHOR::PopStyleVar(2);
}

// Show a combo box with a choice of sizing policies
static void EditTableSizingFlags(AnchorTableFlags *p_flags)
{
  struct EnumDesc
  {
    AnchorTableFlags Value;
    const char *Name;
    const char *Tooltip;
  };
  static const EnumDesc policies[] = {
    {AnchorTableFlags_None,
     "Default",                            "Use default sizing policy:\n- AnchorTableFlags_SizingFixedFit if ScrollX is on or if "
     "host window has AnchorWindowFlags_AlwaysAutoResize.\n- "
     "AnchorTableFlags_SizingStretchSame otherwise."                                                        },
    {AnchorTableFlags_SizingFixedFit,
     "AnchorTableFlags_SizingFixedFit",    "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching "
     "contents width."                                                              },
    {AnchorTableFlags_SizingFixedSame,
     "AnchorTableFlags_SizingFixedSame",   "Columns are all the same width, matching the maximum contents width.\nImplicitly disable "
     "AnchorTableFlags_Resizable and enable AnchorTableFlags_NoKeepColumnsVisible."},
    {AnchorTableFlags_SizingStretchProp,
     "AnchorTableFlags_SizingStretchProp", "Columns default to _WidthStretch with weights proportional to their widths."   },
    {AnchorTableFlags_SizingStretchSame,
     "AnchorTableFlags_SizingStretchSame", "Columns default to _WidthStretch with same weights."                           }
  };
  int idx;
  for (idx = 0; idx < ANCHOR_ARRAYSIZE(policies); idx++)
    if (policies[idx].Value == (*p_flags & AnchorTableFlags_SizingMask_))
      break;
  const char *preview_text = (idx < ANCHOR_ARRAYSIZE(policies)) ?
                               policies[idx].Name + (idx > 0 ? strlen("AnchorTableFlags") : 0) :
                               "";
  if (ANCHOR::BeginCombo("Sizing Policy", preview_text)) {
    for (int n = 0; n < ANCHOR_ARRAYSIZE(policies); n++)
      if (ANCHOR::Selectable(policies[n].Name, idx == n))
        *p_flags = (*p_flags & ~AnchorTableFlags_SizingMask_) | policies[n].Value;
    ANCHOR::EndCombo();
  }
  ANCHOR::SameLine();
  ANCHOR::TextDisabled("(?)");
  if (ANCHOR::IsItemHovered()) {
    ANCHOR::BeginTooltip();
    ANCHOR::PushTextWrapPos(ANCHOR::GetFontSize() * 50.0f);
    for (int m = 0; m < ANCHOR_ARRAYSIZE(policies); m++) {
      ANCHOR::Separator();
      ANCHOR::Text("%s:", policies[m].Name);
      ANCHOR::Separator();
      ANCHOR::SetCursorPosX(ANCHOR::GetCursorPosX() + ANCHOR::GetStyle().IndentSpacing * 0.5f);
      ANCHOR::TextUnformatted(policies[m].Tooltip);
    }
    ANCHOR::PopTextWrapPos();
    ANCHOR::EndTooltip();
  }
}

static void EditTableColumnsFlags(AnchorTableColumnFlags *p_flags)
{
  ANCHOR::CheckboxFlags("_DefaultHide", p_flags, AnchorTableColumnFlags_DefaultHide);
  ANCHOR::CheckboxFlags("_DefaultSort", p_flags, AnchorTableColumnFlags_DefaultSort);
  if (ANCHOR::CheckboxFlags("_WidthStretch", p_flags, AnchorTableColumnFlags_WidthStretch))
    *p_flags &= ~(AnchorTableColumnFlags_WidthMask_ ^ AnchorTableColumnFlags_WidthStretch);
  if (ANCHOR::CheckboxFlags("_WidthFixed", p_flags, AnchorTableColumnFlags_WidthFixed))
    *p_flags &= ~(AnchorTableColumnFlags_WidthMask_ ^ AnchorTableColumnFlags_WidthFixed);
  ANCHOR::CheckboxFlags("_NoResize", p_flags, AnchorTableColumnFlags_NoResize);
  ANCHOR::CheckboxFlags("_NoReorder", p_flags, AnchorTableColumnFlags_NoReorder);
  ANCHOR::CheckboxFlags("_NoHide", p_flags, AnchorTableColumnFlags_NoHide);
  ANCHOR::CheckboxFlags("_NoClip", p_flags, AnchorTableColumnFlags_NoClip);
  ANCHOR::CheckboxFlags("_NoSort", p_flags, AnchorTableColumnFlags_NoSort);
  ANCHOR::CheckboxFlags("_NoSortAscending", p_flags, AnchorTableColumnFlags_NoSortAscending);
  ANCHOR::CheckboxFlags("_NoSortDescending", p_flags, AnchorTableColumnFlags_NoSortDescending);
  ANCHOR::CheckboxFlags("_NoHeaderWidth", p_flags, AnchorTableColumnFlags_NoHeaderWidth);
  ANCHOR::CheckboxFlags("_PreferSortAscending",
                        p_flags,
                        AnchorTableColumnFlags_PreferSortAscending);
  ANCHOR::CheckboxFlags("_PreferSortDescending",
                        p_flags,
                        AnchorTableColumnFlags_PreferSortDescending);
  ANCHOR::CheckboxFlags("_IndentEnable", p_flags, AnchorTableColumnFlags_IndentEnable);
  ANCHOR::SameLine();
  HelpMarker("Default for column 0");
  ANCHOR::CheckboxFlags("_IndentDisable", p_flags, AnchorTableColumnFlags_IndentDisable);
  ANCHOR::SameLine();
  HelpMarker("Default for column >0");
}

static void ShowTableColumnsStatusFlags(AnchorTableColumnFlags flags)
{
  ANCHOR::CheckboxFlags("_IsEnabled", &flags, AnchorTableColumnFlags_IsEnabled);
  ANCHOR::CheckboxFlags("_IsVisible", &flags, AnchorTableColumnFlags_IsVisible);
  ANCHOR::CheckboxFlags("_IsSorted", &flags, AnchorTableColumnFlags_IsSorted);
  ANCHOR::CheckboxFlags("_IsHovered", &flags, AnchorTableColumnFlags_IsHovered);
}

static void ShowDemoWindowTables()
{
  // ANCHOR::SetNextItemOpen(true, AnchorCond_Once);
  if (!ANCHOR::CollapsingHeader("Tables & Columns"))
    return;

  // Using those as a base value to create width/height that are factor of the size of our font
  const float TEXT_BASE_WIDTH = ANCHOR::CalcTextSize("A")[0];
  const float TEXT_BASE_HEIGHT = ANCHOR::GetTextLineHeightWithSpacing();

  ANCHOR::PushID("Tables");

  int open_action = -1;
  if (ANCHOR::Button("Open all"))
    open_action = 1;
  ANCHOR::SameLine();
  if (ANCHOR::Button("Close all"))
    open_action = 0;
  ANCHOR::SameLine();

  // Options
  static bool disable_indent = false;
  ANCHOR::Checkbox("Disable tree indentation", &disable_indent);
  ANCHOR::SameLine();
  HelpMarker("Disable the indenting of tree nodes so demo tables can use the full window width.");
  ANCHOR::Separator();
  if (disable_indent)
    ANCHOR::PushStyleVar(AnchorStyleVar_IndentSpacing, 0.0f);

  // About Styling of tables
  // Most settings are configured on a per-table basis via the flags passed to BeginTable() and
  // TableSetupColumns APIs. There are however a few settings that a shared and part of the
  // AnchorStyle structure:
  //   style.CellPadding                          // Padding within each cell
  //   style.Colors[AnchorCol_TableHeaderBg]       // Table header background
  //   style.Colors[AnchorCol_TableBorderStrong]   // Table outer and header borders
  //   style.Colors[AnchorCol_TableBorderLight]    // Table inner borders
  //   style.Colors[AnchorCol_TableRowBg]          // Table row background when
  //   AnchorTableFlags_RowBg is enabled (even rows) style.Colors[AnchorCol_TableRowBgAlt] //
  //   Table row background when AnchorTableFlags_RowBg is enabled (odds rows)

  // Demos
  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Basic")) {
    // Here we will showcase three different ways to output a table.
    // They are very simple variations of a same thing!

    // [Method 1] Using TableNextRow() to create a new row, and TableSetColumnIndex() to select the
    // column. In many situations, this is the most flexible and easy to use pattern.
    HelpMarker(
      "Using TableNextRow() + calling TableSetColumnIndex() _before_ each cell, in a loop.");
    if (ANCHOR::BeginTable("table1", 3)) {
      for (int row = 0; row < 4; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Row %d Column %d", row, column);
        }
      }
      ANCHOR::EndTable();
    }

    // [Method 2] Using TableNextColumn() called multiple times, instead of using a for loop +
    // TableSetColumnIndex(). This is generally more convenient when you have code manually
    // submitting the contents of each columns.
    HelpMarker("Using TableNextRow() + calling TableNextColumn() _before_ each cell, manually.");
    if (ANCHOR::BeginTable("table2", 3)) {
      for (int row = 0; row < 4; row++) {
        ANCHOR::TableNextRow();
        ANCHOR::TableNextColumn();
        ANCHOR::Text("Row %d", row);
        ANCHOR::TableNextColumn();
        ANCHOR::Text("Some contents");
        ANCHOR::TableNextColumn();
        ANCHOR::Text("123.456");
      }
      ANCHOR::EndTable();
    }

    // [Method 3] We call TableNextColumn() _before_ each cell. We never call TableNextRow(),
    // as TableNextColumn() will automatically wrap around and create new roes as needed.
    // This is generally more convenient when your cells all contains the same type of data.
    HelpMarker(
      "Only using TableNextColumn(), which tends to be convenient for tables where every cells "
      "contains the same type of contents.\n"
      "This is also more similar to the old NextColumn() function of the Columns API, and "
      "provided to facilitate the Columns->Tables API transition.");
    if (ANCHOR::BeginTable("table3", 3)) {
      for (int item = 0; item < 14; item++) {
        ANCHOR::TableNextColumn();
        ANCHOR::Text("Item %d", item);
      }
      ANCHOR::EndTable();
    }

    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Borders, background")) {
    // Expose a few Borders related flags interactively
    enum ContentsType
    {
      CT_Text,
      CT_FillButton
    };
    static AnchorTableFlags flags = AnchorTableFlags_Borders | AnchorTableFlags_RowBg;
    static bool display_headers = false;
    static int contents_type = CT_Text;

    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_RowBg", &flags, AnchorTableFlags_RowBg);
    ANCHOR::CheckboxFlags("AnchorTableFlags_Borders", &flags, AnchorTableFlags_Borders);
    ANCHOR::SameLine();
    HelpMarker(
      "AnchorTableFlags_Borders\n = AnchorTableFlags_BordersInnerV\n | "
      "AnchorTableFlags_BordersOuterV\n | AnchorTableFlags_BordersInnerV\n | "
      "AnchorTableFlags_BordersOuterH");
    ANCHOR::Indent();

    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersH", &flags, AnchorTableFlags_BordersH);
    ANCHOR::Indent();
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuterH",
                          &flags,
                          AnchorTableFlags_BordersOuterH);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInnerH",
                          &flags,
                          AnchorTableFlags_BordersInnerH);
    ANCHOR::Unindent();

    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersV", &flags, AnchorTableFlags_BordersV);
    ANCHOR::Indent();
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuterV",
                          &flags,
                          AnchorTableFlags_BordersOuterV);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInnerV",
                          &flags,
                          AnchorTableFlags_BordersInnerV);
    ANCHOR::Unindent();

    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuter", &flags, AnchorTableFlags_BordersOuter);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInner", &flags, AnchorTableFlags_BordersInner);
    ANCHOR::Unindent();

    ANCHOR::AlignTextToFramePadding();
    ANCHOR::Text("Cell contents:");
    ANCHOR::SameLine();
    ANCHOR::RadioButton("Text", &contents_type, CT_Text);
    ANCHOR::SameLine();
    ANCHOR::RadioButton("FillButton", &contents_type, CT_FillButton);
    ANCHOR::Checkbox("Display headers", &display_headers);
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoBordersInBody",
                          &flags,
                          AnchorTableFlags_NoBordersInBody);
    ANCHOR::SameLine();
    HelpMarker("Disable vertical borders in columns Body (borders will always appears in Headers");
    PopStyleCompact();

    if (ANCHOR::BeginTable("table1", 3, flags)) {
      // Display headers so we can inspect their interaction with borders.
      // (Headers are not the main purpose of this section of the demo, so we are not elaborating
      // on them too much. See other sections for details)
      if (display_headers) {
        ANCHOR::TableSetupColumn("One");
        ANCHOR::TableSetupColumn("Two");
        ANCHOR::TableSetupColumn("Three");
        ANCHOR::TableHeadersRow();
      }

      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          char buf[32];
          sprintf(buf, "Hello %d,%d", column, row);
          if (contents_type == CT_Text)
            ANCHOR::TextUnformatted(buf);
          else if (contents_type)
            ANCHOR::Button(buf, GfVec2f(-FLT_MIN, 0.0f));
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Resizable, stretch")) {
    // By default, if we don't enable ScrollX the sizing policy for each columns is "Stretch"
    // Each columns maintain a sizing weight, and they will occupy all available width.
    static AnchorTableFlags flags = AnchorTableFlags_SizingStretchSame |
                                    AnchorTableFlags_Resizable | AnchorTableFlags_BordersOuter |
                                    AnchorTableFlags_BordersV | AnchorTableFlags_ContextMenuInBody;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags, AnchorTableFlags_Resizable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersV", &flags, AnchorTableFlags_BordersV);
    ANCHOR::SameLine();
    HelpMarker(
      "Using the _Resizable flag automatically enables the _BordersInnerV flag as well, this is "
      "why the resize borders are still showing when unchecking this.");
    PopStyleCompact();

    if (ANCHOR::BeginTable("table1", 3, flags)) {
      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Hello %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Resizable, fixed")) {
    // Here we use AnchorTableFlags_SizingFixedFit (even though _ScrollX is not set)
    // So columns will adopt the "Fixed" policy and will maintain a fixed width regardless of the
    // whole available width (unless table is small) If there is not enough available width to fit
    // all columns, they will however be resized down.
    // FIXME-TABLE: Providing a stretch-on-init would make sense especially for tables which don't
    // have saved settings
    HelpMarker(
      "Using _Resizable + _SizingFixedFit flags.\n"
      "Fixed-width columns generally makes more sense if you want to use horizontal "
      "scrolling.\n\n"
      "Double-click a column border to auto-fit the column to its contents.");
    PushStyleCompact();
    static AnchorTableFlags flags = AnchorTableFlags_SizingFixedFit | AnchorTableFlags_Resizable |
                                    AnchorTableFlags_BordersOuter | AnchorTableFlags_BordersV |
                                    AnchorTableFlags_ContextMenuInBody;
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoHostExtendX",
                          &flags,
                          AnchorTableFlags_NoHostExtendX);
    PopStyleCompact();

    if (ANCHOR::BeginTable("table1", 3, flags)) {
      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Hello %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Resizable, mixed")) {
    HelpMarker(
      "Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n"
      "When combining Fixed and Stretch columns, generally you only want one, maybe two "
      "trailing columns to use _WidthStretch.");
    static AnchorTableFlags flags = AnchorTableFlags_SizingFixedFit | AnchorTableFlags_RowBg |
                                    AnchorTableFlags_Borders | AnchorTableFlags_Resizable |
                                    AnchorTableFlags_Reorderable | AnchorTableFlags_Hideable;

    if (ANCHOR::BeginTable("table1", 3, flags)) {
      ANCHOR::TableSetupColumn("AAA", AnchorTableColumnFlags_WidthFixed);
      ANCHOR::TableSetupColumn("BBB", AnchorTableColumnFlags_WidthFixed);
      ANCHOR::TableSetupColumn("CCC", AnchorTableColumnFlags_WidthStretch);
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("%s %d,%d", (column == 2) ? "Stretch" : "Fixed", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    if (ANCHOR::BeginTable("table2", 6, flags)) {
      ANCHOR::TableSetupColumn("AAA", AnchorTableColumnFlags_WidthFixed);
      ANCHOR::TableSetupColumn("BBB", AnchorTableColumnFlags_WidthFixed);
      ANCHOR::TableSetupColumn("CCC",
                               AnchorTableColumnFlags_WidthFixed |
                                 AnchorTableColumnFlags_DefaultHide);
      ANCHOR::TableSetupColumn("DDD", AnchorTableColumnFlags_WidthStretch);
      ANCHOR::TableSetupColumn("EEE", AnchorTableColumnFlags_WidthStretch);
      ANCHOR::TableSetupColumn("FFF",
                               AnchorTableColumnFlags_WidthStretch |
                                 AnchorTableColumnFlags_DefaultHide);
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 6; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("%s %d,%d", (column >= 3) ? "Stretch" : "Fixed", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Reorderable, hideable, with headers")) {
    HelpMarker(
      "Click and drag column headers to reorder columns.\n\n"
      "Right-click on a header to open a context menu.");
    static AnchorTableFlags flags = AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                    AnchorTableFlags_Hideable | AnchorTableFlags_BordersOuter |
                                    AnchorTableFlags_BordersV;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags, AnchorTableFlags_Resizable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_Reorderable", &flags, AnchorTableFlags_Reorderable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_Hideable", &flags, AnchorTableFlags_Hideable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoBordersInBody",
                          &flags,
                          AnchorTableFlags_NoBordersInBody);
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoBordersInBodyUntilResize",
                          &flags,
                          AnchorTableFlags_NoBordersInBodyUntilResize);
    ANCHOR::SameLine();
    HelpMarker(
      "Disable vertical borders in columns Body until hovered for resize (borders will always "
      "appears in Headers)");
    PopStyleCompact();

    if (ANCHOR::BeginTable("table1", 3, flags)) {
      // Submit columns name with TableSetupColumn() and call TableHeadersRow() to create a row
      // with a header in each column. (Later we will show how TableSetupColumn() has other uses,
      // optional flags, sizing weight etc.)
      ANCHOR::TableSetupColumn("One");
      ANCHOR::TableSetupColumn("Two");
      ANCHOR::TableSetupColumn("Three");
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 6; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Hello %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }

    // Use outer_size[0] == 0.0f instead of default to make the table as tight as possible (only
    // valid when no scrolling and no stretch column)
    if (ANCHOR::BeginTable("table2",
                           3,
                           flags | AnchorTableFlags_SizingFixedFit,
                           GfVec2f(0.0f, 0.0f))) {
      ANCHOR::TableSetupColumn("One");
      ANCHOR::TableSetupColumn("Two");
      ANCHOR::TableSetupColumn("Three");
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 6; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Fixed %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Padding")) {
    // First example: showcase use of padding flags and effect of BorderOuterV/BorderInnerV on X
    // padding. We don't expose BorderOuterH/BorderInnerH here because they have no effect on X
    // padding.
    HelpMarker(
      "We often want outer padding activated when any using features which makes the edges of a "
      "column visible:\n"
      "e.g.:\n"
      "- BorderOuterV\n"
      "- any form of row selection\n"
      "Because of this, activating BorderOuterV sets the default to PadOuterX. Using PadOuterX "
      "or NoPadOuterX you can override the default.\n\n"
      "Actual padding values are using style.CellPadding.\n\n"
      "In this demo we don't show horizontal borders to emphasis how they don't affect default "
      "horizontal padding.");

    static AnchorTableFlags flags1 = AnchorTableFlags_BordersV;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_PadOuterX", &flags1, AnchorTableFlags_PadOuterX);
    ANCHOR::SameLine();
    HelpMarker("Enable outer-most padding (default if AnchorTableFlags_BordersOuterV is set)");
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoPadOuterX", &flags1, AnchorTableFlags_NoPadOuterX);
    ANCHOR::SameLine();
    HelpMarker(
      "Disable outer-most padding (default if AnchorTableFlags_BordersOuterV is not set)");
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoPadInnerX", &flags1, AnchorTableFlags_NoPadInnerX);
    ANCHOR::SameLine();
    HelpMarker(
      "Disable inner padding between columns (double inner padding if BordersOuterV is on, "
      "single inner padding if BordersOuterV is off)");
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuterV",
                          &flags1,
                          AnchorTableFlags_BordersOuterV);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInnerV",
                          &flags1,
                          AnchorTableFlags_BordersInnerV);
    static bool show_headers = false;
    ANCHOR::Checkbox("show_headers", &show_headers);
    PopStyleCompact();

    if (ANCHOR::BeginTable("table_padding", 3, flags1)) {
      if (show_headers) {
        ANCHOR::TableSetupColumn("One");
        ANCHOR::TableSetupColumn("Two");
        ANCHOR::TableSetupColumn("Three");
        ANCHOR::TableHeadersRow();
      }

      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          if (row == 0) {
            ANCHOR::Text("Avail %.2f", ANCHOR::GetContentRegionAvail()[0]);
          } else {
            char buf[32];
            sprintf(buf, "Hello %d,%d", column, row);
            ANCHOR::Button(buf, GfVec2f(-FLT_MIN, 0.0f));
          }
          // if (ANCHOR::TableGetColumnFlags() & AnchorTableColumnFlags_IsHovered)
          //    ANCHOR::TableSetBgColor(AnchorTableBGTarget_CellBg, ANCHOR_COL32(0, 100, 0, 255));
        }
      }
      ANCHOR::EndTable();
    }

    // Second example: set style.CellPadding to (0.0) or a custom value.
    // FIXME-TABLE: Vertical border effectively not displayed the same way as horizontal one...
    HelpMarker("Setting style.CellPadding to (0,0) or a custom value.");
    static AnchorTableFlags flags2 = AnchorTableFlags_Borders | AnchorTableFlags_RowBg;
    static GfVec2f cell_padding(0.0f, 0.0f);
    static bool show_widget_frame_bg = true;

    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Borders", &flags2, AnchorTableFlags_Borders);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersH", &flags2, AnchorTableFlags_BordersH);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersV", &flags2, AnchorTableFlags_BordersV);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInner", &flags2, AnchorTableFlags_BordersInner);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuter", &flags2, AnchorTableFlags_BordersOuter);
    ANCHOR::CheckboxFlags("AnchorTableFlags_RowBg", &flags2, AnchorTableFlags_RowBg);
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags2, AnchorTableFlags_Resizable);
    ANCHOR::Checkbox("show_widget_frame_bg", &show_widget_frame_bg);
    ANCHOR::SliderFloat2("CellPadding", &cell_padding[0], 0.0f, 10.0f, "%.0f");
    PopStyleCompact();

    ANCHOR::PushStyleVar(AnchorStyleVar_CellPadding, cell_padding);
    if (ANCHOR::BeginTable("table_padding_2", 3, flags2)) {
      static char text_bufs[3 * 5][16];  // Mini text storage for 3x5 cells
      static bool init = true;
      if (!show_widget_frame_bg)
        ANCHOR::PushStyleColor(AnchorCol_FrameBg, 0);
      for (int cell = 0; cell < 3 * 5; cell++) {
        ANCHOR::TableNextColumn();
        if (init)
          strcpy(text_bufs[cell], "edit me");
        ANCHOR::SetNextItemWidth(-FLT_MIN);
        ANCHOR::PushID(cell);
        ANCHOR::InputText("##cell", text_bufs[cell], ANCHOR_ARRAYSIZE(text_bufs[cell]));
        ANCHOR::PopID();
      }
      if (!show_widget_frame_bg)
        ANCHOR::PopStyleColor();
      init = false;
      ANCHOR::EndTable();
    }
    ANCHOR::PopStyleVar();

    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Sizing policies")) {
    static AnchorTableFlags flags1 = AnchorTableFlags_BordersV | AnchorTableFlags_BordersOuterH |
                                     AnchorTableFlags_RowBg | AnchorTableFlags_ContextMenuInBody;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags1, AnchorTableFlags_Resizable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoHostExtendX",
                          &flags1,
                          AnchorTableFlags_NoHostExtendX);
    PopStyleCompact();

    static AnchorTableFlags sizing_policy_flags[4] = {AnchorTableFlags_SizingFixedFit,
                                                      AnchorTableFlags_SizingFixedSame,
                                                      AnchorTableFlags_SizingStretchProp,
                                                      AnchorTableFlags_SizingStretchSame};
    for (int table_n = 0; table_n < 4; table_n++) {
      ANCHOR::PushID(table_n);
      ANCHOR::SetNextItemWidth(TEXT_BASE_WIDTH * 30);
      EditTableSizingFlags(&sizing_policy_flags[table_n]);

      // To make it easier to understand the different sizing policy,
      // For each policy: we display one table where the columns have equal contents width, and one
      // where the columns have different contents width.
      if (ANCHOR::BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1)) {
        for (int row = 0; row < 3; row++) {
          ANCHOR::TableNextRow();
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Oh dear");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Oh dear");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Oh dear");
        }
        ANCHOR::EndTable();
      }
      if (ANCHOR::BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1)) {
        for (int row = 0; row < 3; row++) {
          ANCHOR::TableNextRow();
          ANCHOR::TableNextColumn();
          ANCHOR::Text("AAAA");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("BBBBBBBB");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("CCCCCCCCCCCC");
        }
        ANCHOR::EndTable();
      }
      ANCHOR::PopID();
    }

    ANCHOR::Spacing();
    ANCHOR::TextUnformatted("Advanced");
    ANCHOR::SameLine();
    HelpMarker(
      "This section allows you to interact and see the effect of various sizing policies "
      "depending on whether Scroll is enabled and the contents of your columns.");

    enum ContentsType
    {
      CT_ShowWidth,
      CT_ShortText,
      CT_LongText,
      CT_Button,
      CT_FillButton,
      CT_InputText
    };
    static AnchorTableFlags flags = AnchorTableFlags_ScrollY | AnchorTableFlags_Borders |
                                    AnchorTableFlags_RowBg | AnchorTableFlags_Resizable;
    static int contents_type = CT_ShowWidth;
    static int column_count = 3;

    PushStyleCompact();
    ANCHOR::PushID("Advanced");
    ANCHOR::PushItemWidth(TEXT_BASE_WIDTH * 30);
    EditTableSizingFlags(&flags);
    ANCHOR::Combo("Contents",
                  &contents_type,
                  "Show width\0Short Text\0Long Text\0Button\0Fill Button\0InputText\0");
    if (contents_type == CT_FillButton) {
      ANCHOR::SameLine();
      HelpMarker(
        "Be mindful that using right-alignment (e.g. size[0] = -FLT_MIN) creates a feedback "
        "loop where contents width can feed into auto-column width can feed into contents "
        "width.");
    }
    ANCHOR::DragInt("Columns", &column_count, 0.1f, 1, 64, "%d", AnchorSliderFlags_AlwaysClamp);
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags, AnchorTableFlags_Resizable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_PreciseWidths",
                          &flags,
                          AnchorTableFlags_PreciseWidths);
    ANCHOR::SameLine();
    HelpMarker(
      "Disable distributing remainder width to stretched columns (width allocation on a "
      "100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). "
      "With larger number of columns, resizing will appear to be less smooth.");
    ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollX", &flags, AnchorTableFlags_ScrollX);
    ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollY", &flags, AnchorTableFlags_ScrollY);
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoClip", &flags, AnchorTableFlags_NoClip);
    ANCHOR::PopItemWidth();
    ANCHOR::PopID();
    PopStyleCompact();

    if (ANCHOR::BeginTable("table2", column_count, flags, GfVec2f(0.0f, TEXT_BASE_HEIGHT * 7))) {
      for (int cell = 0; cell < 10 * column_count; cell++) {
        ANCHOR::TableNextColumn();
        int column = ANCHOR::TableGetColumnIndex();
        int row = ANCHOR::TableGetRowIndex();

        ANCHOR::PushID(cell);
        char label[32];
        static char text_buf[32] = "";
        sprintf(label, "Hello %d,%d", column, row);
        switch (contents_type) {
          case CT_ShortText:
            ANCHOR::TextUnformatted(label);
            break;
          case CT_LongText:
            ANCHOR::Text("Some %s text %d,%d\nOver two lines..",
                         column == 0 ? "long" : "longeeer",
                         column,
                         row);
            break;
          case CT_ShowWidth:
            ANCHOR::Text("W: %.1f", ANCHOR::GetContentRegionAvail()[0]);
            break;
          case CT_Button:
            ANCHOR::Button(label);
            break;
          case CT_FillButton:
            ANCHOR::Button(label, GfVec2f(-FLT_MIN, 0.0f));
            break;
          case CT_InputText:
            ANCHOR::SetNextItemWidth(-FLT_MIN);
            ANCHOR::InputText("##", text_buf, ANCHOR_ARRAYSIZE(text_buf));
            break;
        }
        ANCHOR::PopID();
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Vertical scrolling, with clipping")) {
    HelpMarker(
      "Here we activate ScrollY, which will create a child window container to allow hosting "
      "scrollable contents.\n\nWe also demonstrate using AnchorListClipper to virtualize the "
      "submission of many items.");
    static AnchorTableFlags flags = AnchorTableFlags_ScrollY | AnchorTableFlags_RowBg |
                                    AnchorTableFlags_BordersOuter | AnchorTableFlags_BordersV |
                                    AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                    AnchorTableFlags_Hideable;

    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollY", &flags, AnchorTableFlags_ScrollY);
    PopStyleCompact();

    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    GfVec2f outer_size = GfVec2f(0.0f, TEXT_BASE_HEIGHT * 8);
    if (ANCHOR::BeginTable("table_scrolly", 3, flags, outer_size)) {
      ANCHOR::TableSetupScrollFreeze(0, 1);  // Make top row always visible
      ANCHOR::TableSetupColumn("One", AnchorTableColumnFlags_None);
      ANCHOR::TableSetupColumn("Two", AnchorTableColumnFlags_None);
      ANCHOR::TableSetupColumn("Three", AnchorTableColumnFlags_None);
      ANCHOR::TableHeadersRow();

      // Demonstrate using clipper for large vertical lists
      AnchorListClipper clipper;
      clipper.Begin(1000);
      while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
          ANCHOR::TableNextRow();
          for (int column = 0; column < 3; column++) {
            ANCHOR::TableSetColumnIndex(column);
            ANCHOR::Text("Hello %d,%d", column, row);
          }
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Horizontal scrolling")) {
    HelpMarker(
      "When ScrollX is enabled, the default sizing policy becomes "
      "AnchorTableFlags_SizingFixedFit, "
      "as automatically stretching columns doesn't make much sense with horizontal "
      "scrolling.\n\n"
      "Also note that as of the current version, you will almost always want to enable ScrollY "
      "along with ScrollX,"
      "because the container window won't automatically extend vertically to fix contents (this "
      "may be improved in future versions).");
    static AnchorTableFlags flags = AnchorTableFlags_ScrollX | AnchorTableFlags_ScrollY |
                                    AnchorTableFlags_RowBg | AnchorTableFlags_BordersOuter |
                                    AnchorTableFlags_BordersV | AnchorTableFlags_Resizable |
                                    AnchorTableFlags_Reorderable | AnchorTableFlags_Hideable;
    static int freeze_cols = 1;
    static int freeze_rows = 1;

    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags, AnchorTableFlags_Resizable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollX", &flags, AnchorTableFlags_ScrollX);
    ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollY", &flags, AnchorTableFlags_ScrollY);
    ANCHOR::SetNextItemWidth(ANCHOR::GetFrameHeight());
    ANCHOR::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, AnchorSliderFlags_NoInput);
    ANCHOR::SetNextItemWidth(ANCHOR::GetFrameHeight());
    ANCHOR::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, AnchorSliderFlags_NoInput);
    PopStyleCompact();

    // When using ScrollX or ScrollY we need to specify a size for our table container!
    // Otherwise by default the table will fit all available space, like a BeginChild() call.
    GfVec2f outer_size = GfVec2f(0.0f, TEXT_BASE_HEIGHT * 8);
    if (ANCHOR::BeginTable("table_scrollx", 7, flags, outer_size)) {
      ANCHOR::TableSetupScrollFreeze(freeze_cols, freeze_rows);
      ANCHOR::TableSetupColumn(
        "Line #",
        AnchorTableColumnFlags_NoHide);  // Make the first column not hideable to
                                         // match our use of TableSetupScrollFreeze()
      ANCHOR::TableSetupColumn("One");
      ANCHOR::TableSetupColumn("Two");
      ANCHOR::TableSetupColumn("Three");
      ANCHOR::TableSetupColumn("Four");
      ANCHOR::TableSetupColumn("Five");
      ANCHOR::TableSetupColumn("Six");
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 20; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 7; column++) {
          // Both TableNextColumn() and TableSetColumnIndex() return true when a column is visible
          // or performing width measurement. Because here we know that:
          // - A) all our columns are contributing the same to row height
          // - B) column 0 is always visible,
          // We only always submit this one column and can skip others.
          // More advanced per-column clipping behaviors may benefit from polling the status flags
          // via TableGetColumnFlags().
          if (!ANCHOR::TableSetColumnIndex(column) && column > 0)
            continue;
          if (column == 0)
            ANCHOR::Text("Line %d", row);
          else
            ANCHOR::Text("Hello world %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }

    ANCHOR::Spacing();
    ANCHOR::TextUnformatted("Stretch + ScrollX");
    ANCHOR::SameLine();
    HelpMarker(
      "Showcase using Stretch columns + ScrollX together: "
      "this is rather unusual and only makes sense when specifying an 'inner_width' for the "
      "table!\n"
      "Without an explicit value, inner_width is == outer_size[0] and therefore using Stretch "
      "columns + ScrollX together doesn't make sense.");
    static AnchorTableFlags flags2 = AnchorTableFlags_SizingStretchSame |
                                     AnchorTableFlags_ScrollX | AnchorTableFlags_ScrollY |
                                     AnchorTableFlags_BordersOuter | AnchorTableFlags_RowBg |
                                     AnchorTableFlags_ContextMenuInBody;
    static float inner_width = 1000.0f;
    PushStyleCompact();
    ANCHOR::PushID("flags3");
    ANCHOR::PushItemWidth(TEXT_BASE_WIDTH * 30);
    ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollX", &flags2, AnchorTableFlags_ScrollX);
    ANCHOR::DragFloat("inner_width", &inner_width, 1.0f, 0.0f, FLT_MAX, "%.1f");
    ANCHOR::PopItemWidth();
    ANCHOR::PopID();
    PopStyleCompact();
    if (ANCHOR::BeginTable("table2", 7, flags2, outer_size, inner_width)) {
      for (int cell = 0; cell < 20 * 7; cell++) {
        ANCHOR::TableNextColumn();
        ANCHOR::Text("Hello world %d,%d",
                     ANCHOR::TableGetColumnIndex(),
                     ANCHOR::TableGetRowIndex());
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Columns flags")) {
    // Create a first table just to show all the options/flags we want to make visible in our
    // example!
    const int column_count = 3;
    const char *column_names[column_count] = {"One", "Two", "Three"};
    static AnchorTableColumnFlags column_flags[column_count] = {
      AnchorTableColumnFlags_DefaultSort,
      AnchorTableColumnFlags_None,
      AnchorTableColumnFlags_DefaultHide};
    static AnchorTableColumnFlags column_flags_out[column_count] = {
      0,
      0,
      0};  // Output from TableGetColumnFlags()

    if (ANCHOR::BeginTable("table_columns_flags_checkboxes",
                           column_count,
                           AnchorTableFlags_None)) {
      PushStyleCompact();
      for (int column = 0; column < column_count; column++) {
        ANCHOR::TableNextColumn();
        ANCHOR::PushID(column);
        ANCHOR::AlignTextToFramePadding();  // FIXME-TABLE: Workaround for wrong text baseline
                                            // propagation
        ANCHOR::Text("'%s'", column_names[column]);
        ANCHOR::Spacing();
        ANCHOR::Text("Input flags:");
        EditTableColumnsFlags(&column_flags[column]);
        ANCHOR::Spacing();
        ANCHOR::Text("Output flags:");
        ShowTableColumnsStatusFlags(column_flags_out[column]);
        ANCHOR::PopID();
      }
      PopStyleCompact();
      ANCHOR::EndTable();
    }

    // Create the real table we care about for the example!
    // We use a scrolling table to be able to showcase the difference between the _IsEnabled and
    // _IsVisible flags above, otherwise in a non-scrolling table columns are always visible
    // (unless using AnchorTableFlags_NoKeepColumnsVisible + resizing the parent window down)
    const AnchorTableFlags flags = AnchorTableFlags_SizingFixedFit | AnchorTableFlags_ScrollX |
                                   AnchorTableFlags_ScrollY | AnchorTableFlags_RowBg |
                                   AnchorTableFlags_BordersOuter | AnchorTableFlags_BordersV |
                                   AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                   AnchorTableFlags_Hideable | AnchorTableFlags_Sortable;
    GfVec2f outer_size = GfVec2f(0.0f, TEXT_BASE_HEIGHT * 9);
    if (ANCHOR::BeginTable("table_columns_flags", column_count, flags, outer_size)) {
      for (int column = 0; column < column_count; column++)
        ANCHOR::TableSetupColumn(column_names[column], column_flags[column]);
      ANCHOR::TableHeadersRow();
      for (int column = 0; column < column_count; column++)
        column_flags_out[column] = ANCHOR::TableGetColumnFlags(column);
      float indent_step = (float)((int)TEXT_BASE_WIDTH / 2);
      for (int row = 0; row < 8; row++) {
        ANCHOR::Indent(indent_step);  // Add some indentation to demonstrate usage of per-column
                                      // IndentEnable/IndentDisable flags.
        ANCHOR::TableNextRow();
        for (int column = 0; column < column_count; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("%s %s",
                       (column == 0) ? "Indented" : "Hello",
                       ANCHOR::TableGetColumnName(column));
        }
      }
      ANCHOR::Unindent(indent_step * 8.0f);

      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Columns widths")) {
    HelpMarker("Using TableSetupColumn() to setup default width.");

    static AnchorTableFlags flags1 = AnchorTableFlags_Borders |
                                     AnchorTableFlags_NoBordersInBodyUntilResize;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags1, AnchorTableFlags_Resizable);
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoBordersInBodyUntilResize",
                          &flags1,
                          AnchorTableFlags_NoBordersInBodyUntilResize);
    PopStyleCompact();
    if (ANCHOR::BeginTable("table1", 3, flags1)) {
      // We could also set AnchorTableFlags_SizingFixedFit on the table and all columns will
      // default to AnchorTableColumnFlags_WidthFixed.
      ANCHOR::TableSetupColumn("one",
                               AnchorTableColumnFlags_WidthFixed,
                               100.0f);  // Default to 100.0f
      ANCHOR::TableSetupColumn("two",
                               AnchorTableColumnFlags_WidthFixed,
                               200.0f);                                      // Default to 200.0f
      ANCHOR::TableSetupColumn("three", AnchorTableColumnFlags_WidthFixed);  // Default to auto
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 4; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableSetColumnIndex(column);
          if (row == 0)
            ANCHOR::Text("(w: %5.1f)", ANCHOR::GetContentRegionAvail()[0]);
          else
            ANCHOR::Text("Hello %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }

    HelpMarker(
      "Using TableSetupColumn() to setup explicit width.\n\nUnless _NoKeepColumnsVisible is "
      "set, fixed columns with set width may still be shrunk down if there's not enough space "
      "in the host.");

    static AnchorTableFlags flags2 = AnchorTableFlags_None;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoKeepColumnsVisible",
                          &flags2,
                          AnchorTableFlags_NoKeepColumnsVisible);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInnerV",
                          &flags2,
                          AnchorTableFlags_BordersInnerV);
    ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuterV",
                          &flags2,
                          AnchorTableFlags_BordersOuterV);
    PopStyleCompact();
    if (ANCHOR::BeginTable("table2", 4, flags2)) {
      // We could also set AnchorTableFlags_SizingFixedFit on the table and all columns will
      // default to AnchorTableColumnFlags_WidthFixed.
      ANCHOR::TableSetupColumn("", AnchorTableColumnFlags_WidthFixed, 100.0f);
      ANCHOR::TableSetupColumn("", AnchorTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
      ANCHOR::TableSetupColumn("", AnchorTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 30.0f);
      ANCHOR::TableSetupColumn("", AnchorTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 15.0f);
      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 4; column++) {
          ANCHOR::TableSetColumnIndex(column);
          if (row == 0)
            ANCHOR::Text("(w: %5.1f)", ANCHOR::GetContentRegionAvail()[0]);
          else
            ANCHOR::Text("Hello %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Nested tables")) {
    HelpMarker("This demonstrate embedding a table into another table cell.");

    if (ANCHOR::BeginTable("table_nested1",
                           2,
                           AnchorTableFlags_Borders | AnchorTableFlags_Resizable |
                             AnchorTableFlags_Reorderable | AnchorTableFlags_Hideable)) {
      ANCHOR::TableSetupColumn("A0");
      ANCHOR::TableSetupColumn("A1");
      ANCHOR::TableHeadersRow();

      ANCHOR::TableNextColumn();
      ANCHOR::Text("A0 Row 0");
      {
        float rows_height = TEXT_BASE_HEIGHT * 2;
        if (ANCHOR::BeginTable("table_nested2",
                               2,
                               AnchorTableFlags_Borders | AnchorTableFlags_Resizable |
                                 AnchorTableFlags_Reorderable | AnchorTableFlags_Hideable)) {
          ANCHOR::TableSetupColumn("B0");
          ANCHOR::TableSetupColumn("B1");
          ANCHOR::TableHeadersRow();

          ANCHOR::TableNextRow(AnchorTableRowFlags_None, rows_height);
          ANCHOR::TableNextColumn();
          ANCHOR::Text("B0 Row 0");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("B1 Row 0");
          ANCHOR::TableNextRow(AnchorTableRowFlags_None, rows_height);
          ANCHOR::TableNextColumn();
          ANCHOR::Text("B0 Row 1");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("B1 Row 1");

          ANCHOR::EndTable();
        }
      }
      ANCHOR::TableNextColumn();
      ANCHOR::Text("A1 Row 0");
      ANCHOR::TableNextColumn();
      ANCHOR::Text("A0 Row 1");
      ANCHOR::TableNextColumn();
      ANCHOR::Text("A1 Row 1");
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Row height")) {
    HelpMarker(
      "You can pass a 'min_row_height' to TableNextRow().\n\nRows are padded with "
      "'style.CellPadding[1]' on top and bottom, so effectively the minimum row height will "
      "always be >= 'style.CellPadding[1] * 2.0f'.\n\nWe cannot honor a _maximum_ row height as "
      "that would requires a unique clipping rectangle per row.");
    if (ANCHOR::BeginTable("table_row_height",
                           1,
                           AnchorTableFlags_BordersOuter | AnchorTableFlags_BordersInnerV)) {
      for (int row = 0; row < 10; row++) {
        float min_row_height = (float)(int)(TEXT_BASE_HEIGHT * 0.30f * row);
        ANCHOR::TableNextRow(AnchorTableRowFlags_None, min_row_height);
        ANCHOR::TableNextColumn();
        ANCHOR::Text("min_row_height = %.2f", min_row_height);
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Outer size")) {
    // Showcasing use of AnchorTableFlags_NoHostExtendX and AnchorTableFlags_NoHostExtendY
    // Important to that note how the two flags have slightly different behaviors!
    ANCHOR::Text("Using NoHostExtendX and NoHostExtendY:");
    PushStyleCompact();
    static AnchorTableFlags flags = AnchorTableFlags_Borders | AnchorTableFlags_Resizable |
                                    AnchorTableFlags_ContextMenuInBody | AnchorTableFlags_RowBg |
                                    AnchorTableFlags_SizingFixedFit |
                                    AnchorTableFlags_NoHostExtendX;
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoHostExtendX",
                          &flags,
                          AnchorTableFlags_NoHostExtendX);
    ANCHOR::SameLine();
    HelpMarker(
      "Make outer width auto-fit to columns, overriding outer_size[0] value.\n\nOnly available "
      "when ScrollX/ScrollY are disabled and Stretch columns are not used.");
    ANCHOR::CheckboxFlags("AnchorTableFlags_NoHostExtendY",
                          &flags,
                          AnchorTableFlags_NoHostExtendY);
    ANCHOR::SameLine();
    HelpMarker(
      "Make outer height stop exactly at outer_size[1] (prevent auto-extending table past the "
      "limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the limit will "
      "be clipped and not visible.");
    PopStyleCompact();

    GfVec2f outer_size = GfVec2f(0.0f, TEXT_BASE_HEIGHT * 5.5f);
    if (ANCHOR::BeginTable("table1", 3, flags, outer_size)) {
      for (int row = 0; row < 10; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Cell %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::SameLine();
    ANCHOR::Text("Hello!");

    ANCHOR::Spacing();

    ANCHOR::Text("Using explicit size:");
    if (ANCHOR::BeginTable("table2",
                           3,
                           AnchorTableFlags_Borders | AnchorTableFlags_RowBg,
                           GfVec2f(TEXT_BASE_WIDTH * 30, 0.0f))) {
      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Cell %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::SameLine();
    if (ANCHOR::BeginTable("table3",
                           3,
                           AnchorTableFlags_Borders | AnchorTableFlags_RowBg,
                           GfVec2f(TEXT_BASE_WIDTH * 30, 0.0f))) {
      for (int row = 0; row < 3; row++) {
        ANCHOR::TableNextRow(0, TEXT_BASE_HEIGHT * 1.5f);
        for (int column = 0; column < 3; column++) {
          ANCHOR::TableNextColumn();
          ANCHOR::Text("Cell %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }

    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Background color")) {
    static AnchorTableFlags flags = AnchorTableFlags_RowBg;
    static int row_bg_type = 1;
    static int row_bg_target = 1;
    static int cell_bg_type = 1;

    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_Borders", &flags, AnchorTableFlags_Borders);
    ANCHOR::CheckboxFlags("AnchorTableFlags_RowBg", &flags, AnchorTableFlags_RowBg);
    ANCHOR::SameLine();
    HelpMarker(
      "AnchorTableFlags_RowBg automatically sets RowBg0 to alternative colors pulled from the "
      "Style.");
    ANCHOR::Combo("row bg type", (int *)&row_bg_type, "None\0Red\0Gradient\0");
    ANCHOR::Combo("row bg target", (int *)&row_bg_target, "RowBg0\0RowBg1\0");
    ANCHOR::SameLine();
    HelpMarker(
      "Target RowBg0 to override the alternating odd/even colors,\nTarget RowBg1 to blend with "
      "them.");
    ANCHOR::Combo("cell bg type", (int *)&cell_bg_type, "None\0Blue\0");
    ANCHOR::SameLine();
    HelpMarker("We are colorizing cells to B1->C2 here.");
    ANCHOR_ASSERT(row_bg_type >= 0 && row_bg_type <= 2);
    ANCHOR_ASSERT(row_bg_target >= 0 && row_bg_target <= 1);
    ANCHOR_ASSERT(cell_bg_type >= 0 && cell_bg_type <= 1);
    PopStyleCompact();

    if (ANCHOR::BeginTable("table1", 5, flags)) {
      for (int row = 0; row < 6; row++) {
        ANCHOR::TableNextRow();

        // Demonstrate setting a row background color with
        // 'ANCHOR::TableSetBgColor(AnchorTableBGTarget_RowBgX, ...)' We use a transparent color
        // so we can see the one behind in case our target is RowBg1 and RowBg0 was already
        // targeted by the AnchorTableFlags_RowBg flag.
        if (row_bg_type != 0) {
          AnchorU32 row_bg_color = ANCHOR::GetColorU32(
            row_bg_type == 1 ?
              GfVec4f(0.7f, 0.3f, 0.3f, 0.65f) :
              GfVec4f(0.2f + row * 0.1f, 0.2f, 0.2f, 0.65f));  // Flat or Gradient?
          ANCHOR::TableSetBgColor(AnchorTableBGTarget_RowBg0 + row_bg_target, row_bg_color);
        }

        // Fill cells
        for (int column = 0; column < 5; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("%c%c", 'A' + row, '0' + column);

          // Change background of Cells B1->C2
          // Demonstrate setting a cell background color with
          // 'ANCHOR::TableSetBgColor(AnchorTableBGTarget_CellBg, ...)' (the CellBg color will be
          // blended over the RowBg and ColumnBg colors) We can also pass a column number as a
          // third parameter to TableSetBgColor() and do this outside the column loop.
          if (row >= 1 && row <= 2 && column >= 1 && column <= 2 && cell_bg_type == 1) {
            AnchorU32 cell_bg_color = ANCHOR::GetColorU32(GfVec4f(0.3f, 0.3f, 0.7f, 0.65f));
            ANCHOR::TableSetBgColor(AnchorTableBGTarget_CellBg, cell_bg_color);
          }
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Tree view")) {
    static AnchorTableFlags flags = AnchorTableFlags_BordersV | AnchorTableFlags_BordersOuterH |
                                    AnchorTableFlags_Resizable | AnchorTableFlags_RowBg |
                                    AnchorTableFlags_NoBordersInBody;

    if (ANCHOR::BeginTable("3ways", 3, flags)) {
      // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed
      // when ScrollX is On
      ANCHOR::TableSetupColumn("Name", AnchorTableColumnFlags_NoHide);
      ANCHOR::TableSetupColumn("Size", AnchorTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
      ANCHOR::TableSetupColumn("Type", AnchorTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
      ANCHOR::TableHeadersRow();

      // Simple storage to output a dummy file-system.
      struct MyTreeNode
      {
        const char *Name;
        const char *Type;
        int Size;
        int ChildIdx;
        int ChildCount;
        static void DisplayNode(const MyTreeNode *node, const MyTreeNode *all_nodes)
        {
          ANCHOR::TableNextRow();
          ANCHOR::TableNextColumn();
          const bool is_folder = (node->ChildCount > 0);
          if (is_folder) {
            bool open = ANCHOR::TreeNodeEx(node->Name, AnchorTreeNodeFlags_SpanFullWidth);
            ANCHOR::TableNextColumn();
            ANCHOR::TextDisabled("--");
            ANCHOR::TableNextColumn();
            ANCHOR::TextUnformatted(node->Type);
            if (open) {
              for (int child_n = 0; child_n < node->ChildCount; child_n++)
                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
              ANCHOR::TreePop();
            }
          } else {
            ANCHOR::TreeNodeEx(node->Name,
                               AnchorTreeNodeFlags_Leaf | AnchorTreeNodeFlags_Bullet |
                                 AnchorTreeNodeFlags_NoTreePushOnOpen |
                                 AnchorTreeNodeFlags_SpanFullWidth);
            ANCHOR::TableNextColumn();
            ANCHOR::Text("%d", node->Size);
            ANCHOR::TableNextColumn();
            ANCHOR::TextUnformatted(node->Type);
          }
        }
      };
      static const MyTreeNode nodes[] = {
        {"Root",                          "Folder",      -1,     1,  3 }, // 0
        {"Music",                         "Folder",      -1,     4,  2 }, // 1
        {"Textures",                      "Folder",      -1,     6,  3 }, // 2
        {"desktop.ini",                   "System file", 1024,   -1, -1}, // 3
        {"File1_a.wav",                   "Audio file",  123000, -1, -1}, // 4
        {"File1_b.wav",                   "Audio file",  456000, -1, -1}, // 5
        {"Image001.png",                  "Image file",  203128, -1, -1}, // 6
        {"Copy of Image001.png",          "Image file",  203256, -1, -1}, // 7
        {"Copy of Image001 (Final2).png", "Image file",  203512, -1, -1}, // 8
      };

      MyTreeNode::DisplayNode(&nodes[0], nodes);

      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Item width")) {
    HelpMarker(
      "Showcase using PushItemWidth() and how it is preserved on a per-column basis.\n\n"
      "Note that on auto-resizing non-resizable fixed columns, querying the content width for "
      "e.g. right-alignment doesn't make sense.");
    if (ANCHOR::BeginTable("table_item_width", 3, AnchorTableFlags_Borders)) {
      ANCHOR::TableSetupColumn("small");
      ANCHOR::TableSetupColumn("half");
      ANCHOR::TableSetupColumn("right-align");
      ANCHOR::TableHeadersRow();

      for (int row = 0; row < 3; row++) {
        ANCHOR::TableNextRow();
        if (row == 0) {
          // Setup ItemWidth once (instead of setting up every time, which is also possible but
          // less efficient)
          ANCHOR::TableSetColumnIndex(0);
          ANCHOR::PushItemWidth(TEXT_BASE_WIDTH * 3.0f);  // Small
          ANCHOR::TableSetColumnIndex(1);
          ANCHOR::PushItemWidth(-ANCHOR::GetContentRegionAvail()[0] * 0.5f);
          ANCHOR::TableSetColumnIndex(2);
          ANCHOR::PushItemWidth(-FLT_MIN);  // Right-aligned
        }

        // Draw our contents
        static float dummy_f = 0.0f;
        ANCHOR::PushID(row);
        ANCHOR::TableSetColumnIndex(0);
        ANCHOR::SliderFloat("float0", &dummy_f, 0.0f, 1.0f);
        ANCHOR::TableSetColumnIndex(1);
        ANCHOR::SliderFloat("float1", &dummy_f, 0.0f, 1.0f);
        ANCHOR::TableSetColumnIndex(2);
        ANCHOR::SliderFloat("float2", &dummy_f, 0.0f, 1.0f);
        ANCHOR::PopID();
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  // Demonstrate using TableHeader() calls instead of TableHeadersRow()
  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Custom headers")) {
    const int COLUMNS_COUNT = 3;
    if (ANCHOR::BeginTable("table_custom_headers",
                           COLUMNS_COUNT,
                           AnchorTableFlags_Borders | AnchorTableFlags_Reorderable |
                             AnchorTableFlags_Hideable)) {
      ANCHOR::TableSetupColumn("Apricot");
      ANCHOR::TableSetupColumn("Banana");
      ANCHOR::TableSetupColumn("Cherry");

      // Dummy entire-column selection storage
      // FIXME: It would be nice to actually demonstrate full-featured selection using those
      // checkbox.
      static bool column_selected[3] = {};

      // Instead of calling TableHeadersRow() we'll submit custom headers ourselves
      ANCHOR::TableNextRow(AnchorTableRowFlags_Headers);
      for (int column = 0; column < COLUMNS_COUNT; column++) {
        ANCHOR::TableSetColumnIndex(column);
        const char *column_name = ANCHOR::TableGetColumnName(
          column);  // Retrieve name passed to TableSetupColumn()
        ANCHOR::PushID(column);
        ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(0, 0));
        ANCHOR::Checkbox("##checkall", &column_selected[column]);
        ANCHOR::PopStyleVar();
        ANCHOR::SameLine(0.0f, ANCHOR::GetStyle().ItemInnerSpacing[0]);
        ANCHOR::TableHeader(column_name);
        ANCHOR::PopID();
      }

      for (int row = 0; row < 5; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < 3; column++) {
          char buf[32];
          sprintf(buf, "Cell %d,%d", column, row);
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Selectable(buf, column_selected[column]);
        }
      }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  // Demonstrate creating custom context menus inside columns, while playing it nice with context
  // menus provided by TableHeadersRow()/TableHeader()
  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Context menus")) {
    HelpMarker(
      "By default, right-clicking over a TableHeadersRow()/TableHeader() line will open the "
      "default context-menu.\nUsing AnchorTableFlags_ContextMenuInBody we also allow "
      "right-clicking over columns body.");
    static AnchorTableFlags flags1 = AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                     AnchorTableFlags_Hideable | AnchorTableFlags_Borders |
                                     AnchorTableFlags_ContextMenuInBody;

    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_ContextMenuInBody",
                          &flags1,
                          AnchorTableFlags_ContextMenuInBody);
    PopStyleCompact();

    // Context Menus: first example
    // [1.1] Right-click on the TableHeadersRow() line to open the default table context menu.
    // [1.2] Right-click in columns also open the default table context menu (if
    // AnchorTableFlags_ContextMenuInBody is set)
    const int COLUMNS_COUNT = 3;
    if (ANCHOR::BeginTable("table_context_menu", COLUMNS_COUNT, flags1)) {
      ANCHOR::TableSetupColumn("One");
      ANCHOR::TableSetupColumn("Two");
      ANCHOR::TableSetupColumn("Three");

      // [1.1]] Right-click on the TableHeadersRow() line to open the default table context menu.
      ANCHOR::TableHeadersRow();

      // Submit dummy contents
      for (int row = 0; row < 4; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < COLUMNS_COUNT; column++) {
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Cell %d,%d", column, row);
        }
      }
      ANCHOR::EndTable();
    }

    // Context Menus: second example
    // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
    // [2.2] Right-click on the ".." to open a custom popup
    // [2.3] Right-click in columns to open another custom popup
    HelpMarker(
      "Demonstrate mixing table context menu (over header), item context button (over button) "
      "and custom per-colum context menu (over column body).");
    AnchorTableFlags flags2 = AnchorTableFlags_Resizable | AnchorTableFlags_SizingFixedFit |
                              AnchorTableFlags_Reorderable | AnchorTableFlags_Hideable |
                              AnchorTableFlags_Borders;
    if (ANCHOR::BeginTable("table_context_menu_2", COLUMNS_COUNT, flags2)) {
      ANCHOR::TableSetupColumn("One");
      ANCHOR::TableSetupColumn("Two");
      ANCHOR::TableSetupColumn("Three");

      // [2.1] Right-click on the TableHeadersRow() line to open the default table context menu.
      ANCHOR::TableHeadersRow();
      for (int row = 0; row < 4; row++) {
        ANCHOR::TableNextRow();
        for (int column = 0; column < COLUMNS_COUNT; column++) {
          // Submit dummy contents
          ANCHOR::TableSetColumnIndex(column);
          ANCHOR::Text("Cell %d,%d", column, row);
          ANCHOR::SameLine();

          // [2.2] Right-click on the ".." to open a custom popup
          ANCHOR::PushID(row * COLUMNS_COUNT + column);
          ANCHOR::SmallButton("..");
          if (ANCHOR::BeginPopupContextItem()) {
            ANCHOR::Text("This is the popup for Button(\"..\") in Cell %d,%d", column, row);
            if (ANCHOR::Button("Close"))
              ANCHOR::CloseCurrentPopup();
            ANCHOR::EndPopup();
          }
          ANCHOR::PopID();
        }
      }

      // [2.3] Right-click anywhere in columns to open another custom popup
      // (instead of testing for !IsAnyItemHovered() we could also call OpenPopup() with
      // AnchorPopupFlags_NoOpenOverExistingPopup to manage popup priority as the popups triggers,
      // here "are we hovering a column" are overlapping)
      int hovered_column = -1;
      for (int column = 0; column < COLUMNS_COUNT + 1; column++) {
        ANCHOR::PushID(column);
        if (ANCHOR::TableGetColumnFlags(column) & AnchorTableColumnFlags_IsHovered)
          hovered_column = column;
        if (hovered_column == column && !ANCHOR::IsAnyItemHovered() && ANCHOR::IsMouseReleased(1))
          ANCHOR::OpenPopup("MyPopup");
        if (ANCHOR::BeginPopup("MyPopup")) {
          if (column == COLUMNS_COUNT)
            ANCHOR::Text("This is a custom popup for unused space after the last column.");
          else
            ANCHOR::Text("This is a custom popup for Column %d", column);
          if (ANCHOR::Button("Close"))
            ANCHOR::CloseCurrentPopup();
          ANCHOR::EndPopup();
        }
        ANCHOR::PopID();
      }

      ANCHOR::EndTable();
      ANCHOR::Text("Hovered column: %d", hovered_column);
    }
    ANCHOR::TreePop();
  }

  // Demonstrate creating multiple tables with the same ID
  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Synced instances")) {
    HelpMarker(
      "Multiple tables with the same identifier will share their settings, width, visibility, "
      "order etc.");
    for (int n = 0; n < 3; n++) {
      char buf[32];
      sprintf(buf, "Synced Table %d", n);
      bool open = ANCHOR::CollapsingHeader(buf, AnchorTreeNodeFlags_DefaultOpen);
      if (open && ANCHOR::BeginTable("Table",
                                     3,
                                     AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                       AnchorTableFlags_Hideable | AnchorTableFlags_Borders |
                                       AnchorTableFlags_SizingFixedFit |
                                       AnchorTableFlags_NoSavedSettings)) {
        ANCHOR::TableSetupColumn("One");
        ANCHOR::TableSetupColumn("Two");
        ANCHOR::TableSetupColumn("Three");
        ANCHOR::TableHeadersRow();
        for (int cell = 0; cell < 9; cell++) {
          ANCHOR::TableNextColumn();
          ANCHOR::Text("this cell %d", cell);
        }
        ANCHOR::EndTable();
      }
    }
    ANCHOR::TreePop();
  }

  // Demonstrate using Sorting facilities
  // This is a simplified version of the "Advanced" example, where we mostly focus on the code
  // necessary to handle sorting. Note that the "Advanced" example also showcase manually
  // triggering a sort (e.g. if item quantities have been modified)
  static const char *template_items_names[] = {"Banana",
                                               "Apple",
                                               "Cherry",
                                               "Watermelon",
                                               "Grapefruit",
                                               "Strawberry",
                                               "Mango",
                                               "Kiwi",
                                               "Orange",
                                               "Pineapple",
                                               "Blueberry",
                                               "Plum",
                                               "Coconut",
                                               "Pear",
                                               "Apricot"};
  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Sorting")) {
    // Create item list
    static AnchorVector<MyItem> items;
    if (items.Size == 0) {
      items.resize(50, MyItem());
      for (int n = 0; n < items.Size; n++) {
        const int template_n = n % ANCHOR_ARRAYSIZE(template_items_names);
        MyItem &item = items[n];
        item.ID = n;
        item.Name = template_items_names[template_n];
        item.Quantity = (n * n - n) % 20;  // Assign default quantities
      }
    }

    // Options
    static AnchorTableFlags flags = AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                    AnchorTableFlags_Hideable | AnchorTableFlags_Sortable |
                                    AnchorTableFlags_SortMulti | AnchorTableFlags_RowBg |
                                    AnchorTableFlags_BordersOuter | AnchorTableFlags_BordersV |
                                    AnchorTableFlags_NoBordersInBody | AnchorTableFlags_ScrollY;
    PushStyleCompact();
    ANCHOR::CheckboxFlags("AnchorTableFlags_SortMulti", &flags, AnchorTableFlags_SortMulti);
    ANCHOR::SameLine();
    HelpMarker(
      "When sorting is enabled: hold shift when clicking headers to sort on multiple column. "
      "TableGetSortSpecs() may return specs where (SpecsCount > 1).");
    ANCHOR::CheckboxFlags("AnchorTableFlags_SortTristate", &flags, AnchorTableFlags_SortTristate);
    ANCHOR::SameLine();
    HelpMarker(
      "When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() "
      "may return specs where (SpecsCount == 0).");
    PopStyleCompact();

    if (ANCHOR::BeginTable("table_sorting",
                           4,
                           flags,
                           GfVec2f(0.0f, TEXT_BASE_HEIGHT * 15),
                           0.0f)) {
      // Declare columns
      // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be
      // stored in the sort specifications. This is so our sort function can identify a column
      // given our own identifier. We could also identify them based on their index! Demonstrate
      // using a mixture of flags among available sort-related flags:
      // - AnchorTableColumnFlags_DefaultSort
      // - AnchorTableColumnFlags_NoSort / AnchorTableColumnFlags_NoSortAscending /
      // AnchorTableColumnFlags_NoSortDescending
      // - AnchorTableColumnFlags_PreferSortAscending /
      // AnchorTableColumnFlags_PreferSortDescending
      ANCHOR::TableSetupColumn("ID",
                               AnchorTableColumnFlags_DefaultSort |
                                 AnchorTableColumnFlags_WidthFixed,
                               0.0f,
                               MyItemColumnID_ID);
      ANCHOR::TableSetupColumn("Name",
                               AnchorTableColumnFlags_WidthFixed,
                               0.0f,
                               MyItemColumnID_Name);
      ANCHOR::TableSetupColumn("Action",
                               AnchorTableColumnFlags_NoSort | AnchorTableColumnFlags_WidthFixed,
                               0.0f,
                               MyItemColumnID_Action);
      ANCHOR::TableSetupColumn("Quantity",
                               AnchorTableColumnFlags_PreferSortDescending |
                                 AnchorTableColumnFlags_WidthStretch,
                               0.0f,
                               MyItemColumnID_Quantity);
      ANCHOR::TableSetupScrollFreeze(0, 1);  // Make row always visible
      ANCHOR::TableHeadersRow();

      // Sort our data if sort specs have been changed!
      if (AnchorTableSortSpecs *sorts_specs = ANCHOR::TableGetSortSpecs())
        if (sorts_specs->SpecsDirty) {
          MyItem::s_current_sort_specs =
            sorts_specs;  // Store in variable accessible by the sort function.
          if (items.Size > 1)
            qsort(&items[0], (size_t)items.Size, sizeof(items[0]), MyItem::CompareWithSortSpecs);
          MyItem::s_current_sort_specs = NULL;
          sorts_specs->SpecsDirty = false;
        }

      // Demonstrate using clipper for large vertical lists
      AnchorListClipper clipper;
      clipper.Begin(items.Size);
      while (clipper.Step())
        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
          // Display a data item
          MyItem *item = &items[row_n];
          ANCHOR::PushID(item->ID);
          ANCHOR::TableNextRow();
          ANCHOR::TableNextColumn();
          ANCHOR::Text("%04d", item->ID);
          ANCHOR::TableNextColumn();
          ANCHOR::TextUnformatted(item->Name);
          ANCHOR::TableNextColumn();
          ANCHOR::SmallButton("None");
          ANCHOR::TableNextColumn();
          ANCHOR::Text("%d", item->Quantity);
          ANCHOR::PopID();
        }
      ANCHOR::EndTable();
    }
    ANCHOR::TreePop();
  }

  // In this example we'll expose most table flags and settings.
  // For specific flags and settings refer to the corresponding section for more detailed
  // explanation. This section is mostly useful to experiment with combining certain flags or
  // settings with each others.
  // ANCHOR::SetNextItemOpen(true, AnchorCond_Once); // [DEBUG]
  if (open_action != -1)
    ANCHOR::SetNextItemOpen(open_action != 0);
  if (ANCHOR::TreeNode("Advanced")) {
    static AnchorTableFlags flags = AnchorTableFlags_Resizable | AnchorTableFlags_Reorderable |
                                    AnchorTableFlags_Hideable | AnchorTableFlags_Sortable |
                                    AnchorTableFlags_SortMulti | AnchorTableFlags_RowBg |
                                    AnchorTableFlags_Borders | AnchorTableFlags_NoBordersInBody |
                                    AnchorTableFlags_ScrollX | AnchorTableFlags_ScrollY |
                                    AnchorTableFlags_SizingFixedFit;

    enum ContentsType
    {
      CT_Text,
      CT_Button,
      CT_SmallButton,
      CT_FillButton,
      CT_Selectable,
      CT_SelectableSpanRow
    };
    static int contents_type = CT_SelectableSpanRow;
    const char *contents_type_names[] =
      {"Text", "Button", "SmallButton", "FillButton", "Selectable", "Selectable (span row)"};
    static int freeze_cols = 1;
    static int freeze_rows = 1;
    static int items_count = ANCHOR_ARRAYSIZE(template_items_names) * 2;
    static GfVec2f outer_size_value = GfVec2f(0.0f, TEXT_BASE_HEIGHT * 12);
    static float row_min_height = 0.0f;           // Auto
    static float inner_width_with_scroll = 0.0f;  // Auto-extend
    static bool outer_size_enabled = true;
    static bool show_headers = true;
    static bool show_wrapped_text = false;
    // static AnchorTextFilter filter;
    // ANCHOR::SetNextItemOpen(true, AnchorCond_Once); // FIXME-TABLE: Enabling this results in
    // initial clipped first pass on table which tend to affects column sizing
    if (ANCHOR::TreeNode("Options")) {
      // Make the UI compact because there are so many fields
      PushStyleCompact();
      ANCHOR::PushItemWidth(TEXT_BASE_WIDTH * 28.0f);

      if (ANCHOR::TreeNodeEx("Features:", AnchorTreeNodeFlags_DefaultOpen)) {
        ANCHOR::CheckboxFlags("AnchorTableFlags_Resizable", &flags, AnchorTableFlags_Resizable);
        ANCHOR::CheckboxFlags("AnchorTableFlags_Reorderable",
                              &flags,
                              AnchorTableFlags_Reorderable);
        ANCHOR::CheckboxFlags("AnchorTableFlags_Hideable", &flags, AnchorTableFlags_Hideable);
        ANCHOR::CheckboxFlags("AnchorTableFlags_Sortable", &flags, AnchorTableFlags_Sortable);
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoSavedSettings",
                              &flags,
                              AnchorTableFlags_NoSavedSettings);
        ANCHOR::CheckboxFlags("AnchorTableFlags_ContextMenuInBody",
                              &flags,
                              AnchorTableFlags_ContextMenuInBody);
        ANCHOR::TreePop();
      }

      if (ANCHOR::TreeNodeEx("Decorations:", AnchorTreeNodeFlags_DefaultOpen)) {
        ANCHOR::CheckboxFlags("AnchorTableFlags_RowBg", &flags, AnchorTableFlags_RowBg);
        ANCHOR::CheckboxFlags("AnchorTableFlags_BordersV", &flags, AnchorTableFlags_BordersV);
        ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuterV",
                              &flags,
                              AnchorTableFlags_BordersOuterV);
        ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInnerV",
                              &flags,
                              AnchorTableFlags_BordersInnerV);
        ANCHOR::CheckboxFlags("AnchorTableFlags_BordersH", &flags, AnchorTableFlags_BordersH);
        ANCHOR::CheckboxFlags("AnchorTableFlags_BordersOuterH",
                              &flags,
                              AnchorTableFlags_BordersOuterH);
        ANCHOR::CheckboxFlags("AnchorTableFlags_BordersInnerH",
                              &flags,
                              AnchorTableFlags_BordersInnerH);
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoBordersInBody",
                              &flags,
                              AnchorTableFlags_NoBordersInBody);
        ANCHOR::SameLine();
        HelpMarker(
          "Disable vertical borders in columns Body (borders will always appears in Headers");
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoBordersInBodyUntilResize",
                              &flags,
                              AnchorTableFlags_NoBordersInBodyUntilResize);
        ANCHOR::SameLine();
        HelpMarker(
          "Disable vertical borders in columns Body until hovered for resize (borders will "
          "always appears in Headers)");
        ANCHOR::TreePop();
      }

      if (ANCHOR::TreeNodeEx("Sizing:", AnchorTreeNodeFlags_DefaultOpen)) {
        EditTableSizingFlags(&flags);
        ANCHOR::SameLine();
        HelpMarker(
          "In the Advanced demo we override the policy of each column so those table-wide "
          "settings have less effect that typical.");
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoHostExtendX",
                              &flags,
                              AnchorTableFlags_NoHostExtendX);
        ANCHOR::SameLine();
        HelpMarker(
          "Make outer width auto-fit to columns, overriding outer_size[0] value.\n\nOnly "
          "available when ScrollX/ScrollY are disabled and Stretch columns are not used.");
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoHostExtendY",
                              &flags,
                              AnchorTableFlags_NoHostExtendY);
        ANCHOR::SameLine();
        HelpMarker(
          "Make outer height stop exactly at outer_size[1] (prevent auto-extending table past "
          "the limit).\n\nOnly available when ScrollX/ScrollY are disabled. Data below the "
          "limit will be clipped and not visible.");
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoKeepColumnsVisible",
                              &flags,
                              AnchorTableFlags_NoKeepColumnsVisible);
        ANCHOR::SameLine();
        HelpMarker("Only available if ScrollX is disabled.");
        ANCHOR::CheckboxFlags("AnchorTableFlags_PreciseWidths",
                              &flags,
                              AnchorTableFlags_PreciseWidths);
        ANCHOR::SameLine();
        HelpMarker(
          "Disable distributing remainder width to stretched columns (width allocation on a "
          "100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: "
          "33,33,33). With larger number of columns, resizing will appear to be less smooth.");
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoClip", &flags, AnchorTableFlags_NoClip);
        ANCHOR::SameLine();
        HelpMarker(
          "Disable clipping rectangle for every individual columns (reduce draw command count, "
          "items will be able to overflow into other columns). Generally incompatible with "
          "ScrollFreeze options.");
        ANCHOR::TreePop();
      }

      if (ANCHOR::TreeNodeEx("Padding:", AnchorTreeNodeFlags_DefaultOpen)) {
        ANCHOR::CheckboxFlags("AnchorTableFlags_PadOuterX", &flags, AnchorTableFlags_PadOuterX);
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoPadOuterX",
                              &flags,
                              AnchorTableFlags_NoPadOuterX);
        ANCHOR::CheckboxFlags("AnchorTableFlags_NoPadInnerX",
                              &flags,
                              AnchorTableFlags_NoPadInnerX);
        ANCHOR::TreePop();
      }

      if (ANCHOR::TreeNodeEx("Scrolling:", AnchorTreeNodeFlags_DefaultOpen)) {
        ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollX", &flags, AnchorTableFlags_ScrollX);
        ANCHOR::SameLine();
        ANCHOR::SetNextItemWidth(ANCHOR::GetFrameHeight());
        ANCHOR::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, AnchorSliderFlags_NoInput);
        ANCHOR::CheckboxFlags("AnchorTableFlags_ScrollY", &flags, AnchorTableFlags_ScrollY);
        ANCHOR::SameLine();
        ANCHOR::SetNextItemWidth(ANCHOR::GetFrameHeight());
        ANCHOR::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, AnchorSliderFlags_NoInput);
        ANCHOR::TreePop();
      }

      if (ANCHOR::TreeNodeEx("Sorting:", AnchorTreeNodeFlags_DefaultOpen)) {
        ANCHOR::CheckboxFlags("AnchorTableFlags_SortMulti", &flags, AnchorTableFlags_SortMulti);
        ANCHOR::SameLine();
        HelpMarker(
          "When sorting is enabled: hold shift when clicking headers to sort on multiple "
          "column. TableGetSortSpecs() may return specs where (SpecsCount > 1).");
        ANCHOR::CheckboxFlags("AnchorTableFlags_SortTristate",
                              &flags,
                              AnchorTableFlags_SortTristate);
        ANCHOR::SameLine();
        HelpMarker(
          "When sorting is enabled: allow no sorting, disable default sorting. "
          "TableGetSortSpecs() may return specs where (SpecsCount == 0).");
        ANCHOR::TreePop();
      }

      if (ANCHOR::TreeNodeEx("Other:", AnchorTreeNodeFlags_DefaultOpen)) {
        ANCHOR::Checkbox("show_headers", &show_headers);
        ANCHOR::Checkbox("show_wrapped_text", &show_wrapped_text);

        ANCHOR::DragFloat2("##OuterSize", &outer_size_value[0]);
        ANCHOR::SameLine(0.0f, ANCHOR::GetStyle().ItemInnerSpacing[0]);
        ANCHOR::Checkbox("outer_size", &outer_size_enabled);
        ANCHOR::SameLine();
        HelpMarker(
          "If scrolling is disabled (ScrollX and ScrollY not set):\n"
          "- The table is output directly in the parent window.\n"
          "- OuterSize[0] < 0.0f will right-align the table.\n"
          "- OuterSize[0] = 0.0f will narrow fit the table unless there are any Stretch "
          "column.\n"
          "- OuterSize[1] then becomes the minimum size for the table, which will extend "
          "vertically if there are more rows (unless NoHostExtendY is set).");

        // From a user point of view we will tend to use 'inner_width' differently depending on
        // whether our table is embedding scrolling. To facilitate toying with this demo we will
        // actually pass 0.0f to the BeginTable() when ScrollX is disabled.
        ANCHOR::DragFloat("inner_width (when ScrollX active)",
                          &inner_width_with_scroll,
                          1.0f,
                          0.0f,
                          FLT_MAX);

        ANCHOR::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);
        ANCHOR::SameLine();
        HelpMarker("Specify height of the Selectable item.");

        ANCHOR::DragInt("items_count", &items_count, 0.1f, 0, 9999);
        ANCHOR::Combo("items_type (first column)",
                      &contents_type,
                      contents_type_names,
                      ANCHOR_ARRAYSIZE(contents_type_names));
        // filter.Draw("filter");
        ANCHOR::TreePop();
      }

      ANCHOR::PopItemWidth();
      PopStyleCompact();
      ANCHOR::Spacing();
      ANCHOR::TreePop();
    }

    // Update item list if we changed the number of items
    static AnchorVector<MyItem> items;
    static AnchorVector<int> selection;
    static bool items_need_sort = false;
    if (items.Size != items_count) {
      items.resize(items_count, MyItem());
      for (int n = 0; n < items_count; n++) {
        const int template_n = n % ANCHOR_ARRAYSIZE(template_items_names);
        MyItem &item = items[n];
        item.ID = n;
        item.Name = template_items_names[template_n];
        item.Quantity = (template_n == 3) ? 10 :
                        (template_n == 4) ? 20 :
                                            0;  // Assign default quantities
      }
    }

    const AnchorDrawList *parent_draw_list = ANCHOR::GetWindowDrawList();
    const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
    GfVec2f table_scroll_cur, table_scroll_max;    // For debug display
    const AnchorDrawList *table_draw_list = NULL;  // "

    // Submit table
    const float inner_width_to_use = (flags & AnchorTableFlags_ScrollX) ? inner_width_with_scroll :
                                                                          0.0f;
    if (ANCHOR::BeginTable("table_advanced",
                           6,
                           flags,
                           outer_size_enabled ? outer_size_value : GfVec2f(0, 0),
                           inner_width_to_use)) {
      // Declare columns
      // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be
      // stored in the sort specifications. This is so our sort function can identify a column
      // given our own identifier. We could also identify them based on their index!
      ANCHOR::TableSetupColumn("ID",
                               AnchorTableColumnFlags_DefaultSort |
                                 AnchorTableColumnFlags_WidthFixed | AnchorTableColumnFlags_NoHide,
                               0.0f,
                               MyItemColumnID_ID);
      ANCHOR::TableSetupColumn("Name",
                               AnchorTableColumnFlags_WidthFixed,
                               0.0f,
                               MyItemColumnID_Name);
      ANCHOR::TableSetupColumn("Action",
                               AnchorTableColumnFlags_NoSort | AnchorTableColumnFlags_WidthFixed,
                               0.0f,
                               MyItemColumnID_Action);
      ANCHOR::TableSetupColumn("Quantity",
                               AnchorTableColumnFlags_PreferSortDescending,
                               0.0f,
                               MyItemColumnID_Quantity);
      ANCHOR::TableSetupColumn(
        "Description",
        (flags & AnchorTableFlags_NoHostExtendX) ? 0 : AnchorTableColumnFlags_WidthStretch,
        0.0f,
        MyItemColumnID_Description);
      ANCHOR::TableSetupColumn("Hidden",
                               AnchorTableColumnFlags_DefaultHide | AnchorTableColumnFlags_NoSort);
      ANCHOR::TableSetupScrollFreeze(freeze_cols, freeze_rows);

      // Sort our data if sort specs have been changed!
      AnchorTableSortSpecs *sorts_specs = ANCHOR::TableGetSortSpecs();
      if (sorts_specs && sorts_specs->SpecsDirty)
        items_need_sort = true;
      if (sorts_specs && items_need_sort && items.Size > 1) {
        MyItem::s_current_sort_specs =
          sorts_specs;  // Store in variable accessible by the sort function.
        qsort(&items[0], (size_t)items.Size, sizeof(items[0]), MyItem::CompareWithSortSpecs);
        MyItem::s_current_sort_specs = NULL;
        sorts_specs->SpecsDirty = false;
      }
      items_need_sort = false;

      // Take note of whether we are currently sorting based on the Quantity field,
      // we will use this to trigger sorting when we know the data of this column has been
      // modified.
      const bool sorts_specs_using_quantity = (ANCHOR::TableGetColumnFlags(3) &
                                               AnchorTableColumnFlags_IsSorted) != 0;

      // Show headers
      if (show_headers)
        ANCHOR::TableHeadersRow();

      // Show data
      // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
      ANCHOR::PushButtonRepeat(true);
#if 1
      // Demonstrate using clipper for large vertical lists
      AnchorListClipper clipper;
      clipper.Begin(items.Size);
      while (clipper.Step()) {
        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
      // Without clipper
      {
        for (int row_n = 0; row_n < items.Size; row_n++)
#endif
        {
          MyItem *item = &items[row_n];
          // if (!filter.PassFilter(item->Name))
          //    continue;

          const bool item_is_selected = selection.contains(item->ID);
          ANCHOR::PushID(item->ID);
          ANCHOR::TableNextRow(AnchorTableRowFlags_None, row_min_height);

          // For the demo purpose we can select among different type of items submitted in the
          // first column
          ANCHOR::TableSetColumnIndex(0);
          char label[32];
          sprintf(label, "%04d", item->ID);
          if (contents_type == CT_Text)
            ANCHOR::TextUnformatted(label);
          else if (contents_type == CT_Button)
            ANCHOR::Button(label);
          else if (contents_type == CT_SmallButton)
            ANCHOR::SmallButton(label);
          else if (contents_type == CT_FillButton)
            ANCHOR::Button(label, GfVec2f(-FLT_MIN, 0.0f));
          else if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow) {
            AnchorSelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ?
                                                       AnchorSelectableFlags_SpanAllColumns |
                                                         AnchorSelectableFlags_AllowItemOverlap :
                                                       AnchorSelectableFlags_None;
            if (ANCHOR::Selectable(label,
                                   item_is_selected,
                                   selectable_flags,
                                   GfVec2f(0, row_min_height))) {
              if (ANCHOR::GetIO().KeyCtrl) {
                if (item_is_selected)
                  selection.find_erase_unsorted(item->ID);
                else
                  selection.push_back(item->ID);
              } else {
                selection.clear();
                selection.push_back(item->ID);
              }
            }
          }

          if (ANCHOR::TableSetColumnIndex(1))
            ANCHOR::TextUnformatted(item->Name);

          // Here we demonstrate marking our data set as needing to be sorted again if we modified
          // a quantity, and we are currently sorting on the column showing the Quantity. To avoid
          // triggering a sort while holding the button, we only trigger it when the button has
          // been released. You will probably need a more advanced system in your code if you want
          // to automatically sort when a specific entry changes.
          if (ANCHOR::TableSetColumnIndex(2)) {
            if (ANCHOR::SmallButton("Chop")) {
              item->Quantity += 1;
            }
            if (sorts_specs_using_quantity && ANCHOR::IsItemDeactivated()) {
              items_need_sort = true;
            }
            ANCHOR::SameLine();
            if (ANCHOR::SmallButton("Eat")) {
              item->Quantity -= 1;
            }
            if (sorts_specs_using_quantity && ANCHOR::IsItemDeactivated()) {
              items_need_sort = true;
            }
          }

          if (ANCHOR::TableSetColumnIndex(3))
            ANCHOR::Text("%d", item->Quantity);

          ANCHOR::TableSetColumnIndex(4);
          if (show_wrapped_text)
            ANCHOR::TextWrapped("Lorem ipsum dolor sit amet");
          else
            ANCHOR::Text("Lorem ipsum dolor sit amet");

          if (ANCHOR::TableSetColumnIndex(5))
            ANCHOR::Text("1234");

          ANCHOR::PopID();
        }
      }
      ANCHOR::PopButtonRepeat();

      // Store some info to display debug details below
      table_scroll_cur = GfVec2f(ANCHOR::GetScrollX(), ANCHOR::GetScrollY());
      table_scroll_max = GfVec2f(ANCHOR::GetScrollMaxX(), ANCHOR::GetScrollMaxY());
      table_draw_list = ANCHOR::GetWindowDrawList();
      ANCHOR::EndTable();
    }
    static bool show_debug_details = false;
    ANCHOR::Checkbox("Debug details", &show_debug_details);
    if (show_debug_details && table_draw_list) {
      ANCHOR::SameLine(0.0f, 0.0f);
      const int table_draw_list_draw_cmd_count = table_draw_list->CmdBuffer.Size;
      if (table_draw_list == parent_draw_list)
        ANCHOR::Text(": DrawCmd: +%d (in same window)",
                     table_draw_list_draw_cmd_count - parent_draw_list_draw_cmd_count);
      else
        ANCHOR::Text(": DrawCmd: +%d (in child window), Scroll: (%.f/%.f) (%.f/%.f)",
                     table_draw_list_draw_cmd_count - 1,
                     table_scroll_cur[0],
                     table_scroll_max[0],
                     table_scroll_cur[1],
                     table_scroll_max[1]);
    }
    ANCHOR::TreePop();
  }

  ANCHOR::PopID();

  ShowDemoWindowColumns();

  if (disable_indent)
    ANCHOR::PopStyleVar();
}

// Demonstrate old/legacy Columns API!
// [2020: Columns are under-featured and not maintained. Prefer using the more flexible and
// powerful BeginTable() API!]
static void ShowDemoWindowColumns()
{
  bool open = ANCHOR::TreeNode("Legacy Columns API");
  ANCHOR::SameLine();
  HelpMarker(
    "Columns() is an old API! Prefer using the more flexible and powerful BeginTable() API!");
  if (!open)
    return;

  // Basic columns
  if (ANCHOR::TreeNode("Basic")) {
    ANCHOR::Text("Without border:");
    ANCHOR::Columns(3, "mycolumns3", false);  // 3-ways, no border
    ANCHOR::Separator();
    for (int n = 0; n < 14; n++) {
      char label[32];
      sprintf(label, "Item %d", n);
      if (ANCHOR::Selectable(label)) {
      }
      // if (ANCHOR::Button(label, GfVec2f(-FLT_MIN,0.0f))) {}
      ANCHOR::NextColumn();
    }
    ANCHOR::Columns(1);
    ANCHOR::Separator();

    ANCHOR::Text("With border:");
    ANCHOR::Columns(4, "mycolumns");  // 4-ways, with border
    ANCHOR::Separator();
    ANCHOR::Text("ID");
    ANCHOR::NextColumn();
    ANCHOR::Text("Name");
    ANCHOR::NextColumn();
    ANCHOR::Text("Path");
    ANCHOR::NextColumn();
    ANCHOR::Text("Hovered");
    ANCHOR::NextColumn();
    ANCHOR::Separator();
    const char *names[3] = {"One", "Two", "Three"};
    const char *paths[3] = {"/path/one", "/path/two", "/path/three"};
    static int selected = -1;
    for (int i = 0; i < 3; i++) {
      char label[32];
      sprintf(label, "%04d", i);
      if (ANCHOR::Selectable(label, selected == i, AnchorSelectableFlags_SpanAllColumns))
        selected = i;
      bool hovered = ANCHOR::IsItemHovered();
      ANCHOR::NextColumn();
      ANCHOR::Text(names[i]);
      ANCHOR::NextColumn();
      ANCHOR::Text(paths[i]);
      ANCHOR::NextColumn();
      ANCHOR::Text("%d", hovered);
      ANCHOR::NextColumn();
    }
    ANCHOR::Columns(1);
    ANCHOR::Separator();
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Borders")) {
    // NB: Future columns API should allow automatic horizontal borders.
    static bool h_borders = true;
    static bool v_borders = true;
    static int columns_count = 4;
    const int lines_count = 3;
    ANCHOR::SetNextItemWidth(ANCHOR::GetFontSize() * 8);
    ANCHOR::DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
    if (columns_count < 2)
      columns_count = 2;
    ANCHOR::SameLine();
    ANCHOR::Checkbox("horizontal", &h_borders);
    ANCHOR::SameLine();
    ANCHOR::Checkbox("vertical", &v_borders);
    ANCHOR::Columns(columns_count, NULL, v_borders);
    for (int i = 0; i < columns_count * lines_count; i++) {
      if (h_borders && ANCHOR::GetColumnIndex() == 0)
        ANCHOR::Separator();
      ANCHOR::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
      ANCHOR::Text("Width %.2f", ANCHOR::GetColumnWidth());
      ANCHOR::Text("Avail %.2f", ANCHOR::GetContentRegionAvail()[0]);
      ANCHOR::Text("Offset %.2f", ANCHOR::GetColumnOffset());
      ANCHOR::Text("Long text that is likely to clip");
      ANCHOR::Button("Button", GfVec2f(-FLT_MIN, 0.0f));
      ANCHOR::NextColumn();
    }
    ANCHOR::Columns(1);
    if (h_borders)
      ANCHOR::Separator();
    ANCHOR::TreePop();
  }

  // Create multiple items in a same cell before switching to next column
  if (ANCHOR::TreeNode("Mixed items")) {
    ANCHOR::Columns(3, "mixed");
    ANCHOR::Separator();

    ANCHOR::Text("Hello");
    ANCHOR::Button("Banana");
    ANCHOR::NextColumn();

    ANCHOR::Text("ANCHOR");
    ANCHOR::Button("Apple");
    static float foo = 1.0f;
    ANCHOR::InputFloat("red", &foo, 0.05f, 0, "%.3f");
    ANCHOR::Text("An extra line here.");
    ANCHOR::NextColumn();

    ANCHOR::Text("Sailor");
    ANCHOR::Button("Corniflower");
    static float bar = 1.0f;
    ANCHOR::InputFloat("blue", &bar, 0.05f, 0, "%.3f");
    ANCHOR::NextColumn();

    if (ANCHOR::CollapsingHeader("Category A")) {
      ANCHOR::Text("Blah blah blah");
    }
    ANCHOR::NextColumn();
    if (ANCHOR::CollapsingHeader("Category B")) {
      ANCHOR::Text("Blah blah blah");
    }
    ANCHOR::NextColumn();
    if (ANCHOR::CollapsingHeader("Category C")) {
      ANCHOR::Text("Blah blah blah");
    }
    ANCHOR::NextColumn();
    ANCHOR::Columns(1);
    ANCHOR::Separator();
    ANCHOR::TreePop();
  }

  // Word wrapping
  if (ANCHOR::TreeNode("Word-wrapping")) {
    ANCHOR::Columns(2, "word-wrapping");
    ANCHOR::Separator();
    ANCHOR::TextWrapped("The quick brown fox jumps over the lazy dog.");
    ANCHOR::TextWrapped("Hello Left");
    ANCHOR::NextColumn();
    ANCHOR::TextWrapped("The quick brown fox jumps over the lazy dog.");
    ANCHOR::TextWrapped("Hello Right");
    ANCHOR::Columns(1);
    ANCHOR::Separator();
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Horizontal Scrolling")) {
    ANCHOR::SetNextWindowContentSize(GfVec2f(1500.0f, 0.0f));
    GfVec2f child_size = GfVec2f(0, ANCHOR::GetFontSize() * 20.0f);
    ANCHOR::BeginChild("##ScrollingRegion",
                       child_size,
                       false,
                       AnchorWindowFlags_HorizontalScrollbar);
    ANCHOR::Columns(10);

    // Also demonstrate using clipper for large vertical lists
    int ITEMS_COUNT = 2000;
    AnchorListClipper clipper;
    clipper.Begin(ITEMS_COUNT);
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        for (int j = 0; j < 10; j++) {
          ANCHOR::Text("Line %d Column %d...", i, j);
          ANCHOR::NextColumn();
        }
    }
    ANCHOR::Columns(1);
    ANCHOR::EndChild();
    ANCHOR::TreePop();
  }

  if (ANCHOR::TreeNode("Tree")) {
    ANCHOR::Columns(2, "tree", true);
    for (int x = 0; x < 3; x++) {
      bool open1 = ANCHOR::TreeNode((void *)(intptr_t)x, "Node%d", x);
      ANCHOR::NextColumn();
      ANCHOR::Text("Node contents");
      ANCHOR::NextColumn();
      if (open1) {
        for (int y = 0; y < 3; y++) {
          bool open2 = ANCHOR::TreeNode((void *)(intptr_t)y, "Node%d.%d", x, y);
          ANCHOR::NextColumn();
          ANCHOR::Text("Node contents");
          if (open2) {
            ANCHOR::Text("Even more contents");
            if (ANCHOR::TreeNode("Tree in column")) {
              ANCHOR::Text("The quick brown fox jumps over the lazy dog");
              ANCHOR::TreePop();
            }
          }
          ANCHOR::NextColumn();
          if (open2)
            ANCHOR::TreePop();
        }
        ANCHOR::TreePop();
      }
    }
    ANCHOR::Columns(1);
    ANCHOR::TreePop();
  }

  ANCHOR::TreePop();
}

static void ShowDemoWindowMisc()
{
  if (ANCHOR::CollapsingHeader("Filtering")) {
    // Helper class to easy setup a text filter.
    // You may want to implement a more feature-full filtering scheme in your own application.
    static AnchorTextFilter filter;
    ANCHOR::Text(
      "Filter usage:\n"
      "  \"\"         display all lines\n"
      "  \"xxx\"      display lines containing \"xxx\"\n"
      "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
      "  \"-xxx\"     hide lines containing \"xxx\"");
    filter.Draw();
    const char *lines[] =
      {"aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world"};
    for (int i = 0; i < ANCHOR_ARRAYSIZE(lines); i++)
      if (filter.PassFilter(lines[i]))
        ANCHOR::BulletText("%s", lines[i]);
  }

  if (ANCHOR::CollapsingHeader("Inputs, Navigation & Focus")) {
    AnchorIO &io = ANCHOR::GetIO();

    // Display AnchorIO output flags
    ANCHOR::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
    ANCHOR::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
    ANCHOR::Text("WantTextInput: %d", io.WantTextInput);
    ANCHOR::Text("WantSetMousePos: %d", io.WantSetMousePos);
    ANCHOR::Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);

    // Display Mouse state
    if (ANCHOR::TreeNode("Mouse State")) {
      if (ANCHOR::IsMousePosValid())
        ANCHOR::Text("Mouse pos: (%g, %g)", io.MousePos[0], io.MousePos[1]);
      else
        ANCHOR::Text("Mouse pos: <INVALID>");
      ANCHOR::Text("Mouse delta: (%g, %g)", io.MouseDelta[0], io.MouseDelta[1]);
      ANCHOR::Text("Mouse down:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.MouseDown); i++)
        if (ANCHOR::IsMouseDown(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
        }
      ANCHOR::Text("Mouse clicked:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.MouseDown); i++)
        if (ANCHOR::IsMouseClicked(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("b%d", i);
        }
      ANCHOR::Text("Mouse dblclick:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.MouseDown); i++)
        if (ANCHOR::IsMouseDoubleClicked(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("b%d", i);
        }
      ANCHOR::Text("Mouse released:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.MouseDown); i++)
        if (ANCHOR::IsMouseReleased(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("b%d", i);
        }
      ANCHOR::Text("Mouse wheel: %.1f", io.MouseWheel);
      ANCHOR::Text("Pen Pressure: %.1f", io.PenPressure);  // Note: currently unused
      ANCHOR::TreePop();
    }

    // Display Keyboard/Mouse state
    if (ANCHOR::TreeNode("Keyboard & Navigation State")) {
      ANCHOR::Text("Keys down:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.KeysDown); i++)
        if (ANCHOR::IsKeyDown(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("%d (0x%X) (%.02f secs)", i, i, io.KeysDownDuration[i]);
        }
      ANCHOR::Text("Keys pressed:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.KeysDown); i++)
        if (ANCHOR::IsKeyPressed(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("%d (0x%X)", i, i);
        }
      ANCHOR::Text("Keys release:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.KeysDown); i++)
        if (ANCHOR::IsKeyReleased(i)) {
          ANCHOR::SameLine();
          ANCHOR::Text("%d (0x%X)", i, i);
        }
      ANCHOR::Text("Keys mods: %s%s%s%s",
                   io.KeyCtrl ? "CTRL " : "",
                   io.KeyShift ? "SHIFT " : "",
                   io.KeyAlt ? "ALT " : "",
                   io.KeySuper ? "SUPER " : "");
      ANCHOR::Text("Chars queue:");
      for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
        AnchorWChar c = io.InputQueueCharacters[i];
        ANCHOR::SameLine();
        ANCHOR::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c);
      }  // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.

      ANCHOR::Text("NavInputs down:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.NavInputs); i++)
        if (io.NavInputs[i] > 0.0f) {
          ANCHOR::SameLine();
          ANCHOR::Text("[%d] %.2f (%.02f secs)", i, io.NavInputs[i], io.NavInputsDownDuration[i]);
        }
      ANCHOR::Text("NavInputs pressed:");
      for (int i = 0; i < ANCHOR_ARRAYSIZE(io.NavInputs); i++)
        if (io.NavInputsDownDuration[i] == 0.0f) {
          ANCHOR::SameLine();
          ANCHOR::Text("[%d]", i);
        }

      ANCHOR::Button("Hovering me sets the\nkeyboard capture flag");
      if (ANCHOR::IsItemHovered())
        ANCHOR::CaptureKeyboardFromApp(true);
      ANCHOR::SameLine();
      ANCHOR::Button("Holding me clears the\nthe keyboard capture flag");
      if (ANCHOR::IsItemActive())
        ANCHOR::CaptureKeyboardFromApp(false);
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Tabbing")) {
      ANCHOR::Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
      static char buf[32] = "hello";
      ANCHOR::InputText("1", buf, ANCHOR_ARRAYSIZE(buf));
      ANCHOR::InputText("2", buf, ANCHOR_ARRAYSIZE(buf));
      ANCHOR::InputText("3", buf, ANCHOR_ARRAYSIZE(buf));
      ANCHOR::PushAllowKeyboardFocus(false);
      ANCHOR::InputText("4 (tab skip)", buf, ANCHOR_ARRAYSIZE(buf));
      ANCHOR::SameLine();
      HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
      ANCHOR::PopAllowKeyboardFocus();
      ANCHOR::InputText("5", buf, ANCHOR_ARRAYSIZE(buf));
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Focus from code")) {
      bool focus_1 = ANCHOR::Button("Focus on 1");
      ANCHOR::SameLine();
      bool focus_2 = ANCHOR::Button("Focus on 2");
      ANCHOR::SameLine();
      bool focus_3 = ANCHOR::Button("Focus on 3");
      int has_focus = 0;
      static char buf[128] = "click on a button to set focus";

      if (focus_1)
        ANCHOR::SetKeyboardFocusHere();
      ANCHOR::InputText("1", buf, ANCHOR_ARRAYSIZE(buf));
      if (ANCHOR::IsItemActive())
        has_focus = 1;

      if (focus_2)
        ANCHOR::SetKeyboardFocusHere();
      ANCHOR::InputText("2", buf, ANCHOR_ARRAYSIZE(buf));
      if (ANCHOR::IsItemActive())
        has_focus = 2;

      ANCHOR::PushAllowKeyboardFocus(false);
      if (focus_3)
        ANCHOR::SetKeyboardFocusHere();
      ANCHOR::InputText("3 (tab skip)", buf, ANCHOR_ARRAYSIZE(buf));
      if (ANCHOR::IsItemActive())
        has_focus = 3;
      ANCHOR::SameLine();
      HelpMarker("Item won't be cycled through when using TAB or Shift+Tab.");
      ANCHOR::PopAllowKeyboardFocus();

      if (has_focus)
        ANCHOR::Text("Item with focus: %d", has_focus);
      else
        ANCHOR::Text("Item with focus: <none>");

      // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
      static float f3[3] = {0.0f, 0.0f, 0.0f};
      int focus_ahead = -1;
      if (ANCHOR::Button("Focus on X")) {
        focus_ahead = 0;
      }
      ANCHOR::SameLine();
      if (ANCHOR::Button("Focus on Y")) {
        focus_ahead = 1;
      }
      ANCHOR::SameLine();
      if (ANCHOR::Button("Focus on Z")) {
        focus_ahead = 2;
      }
      if (focus_ahead != -1)
        ANCHOR::SetKeyboardFocusHere(focus_ahead);
      ANCHOR::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

      ANCHOR::TextWrapped(
        "NB: Cursor & selection are preserved when refocusing last used item in code.");
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Dragging")) {
      ANCHOR::TextWrapped(
        "You can use ANCHOR::GetMouseDragDelta(0) to query for the dragged amount on any "
        "widget.");
      for (int button = 0; button < 3; button++) {
        ANCHOR::Text("IsMouseDragging(%d):", button);
        ANCHOR::Text("  w/ default threshold: %d,", ANCHOR::IsMouseDragging(button));
        ANCHOR::Text("  w/ zero threshold: %d,", ANCHOR::IsMouseDragging(button, 0.0f));
        ANCHOR::Text("  w/ large threshold: %d,", ANCHOR::IsMouseDragging(button, 20.0f));
      }

      ANCHOR::Button("Drag Me");
      if (ANCHOR::IsItemActive())
        ANCHOR::GetForegroundDrawList()->AddLine(
          io.MouseClickedPos[0],
          io.MousePos,
          ANCHOR::GetColorU32(AnchorCol_Button),
          4.0f);  // Draw a line between the button and the mouse cursor

      // Drag operations gets "unlocked" when the mouse has moved past a certain threshold
      // (the default threshold is stored in io.MouseDragThreshold). You can request a lower or
      // higher threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta().
      GfVec2f value_raw = ANCHOR::GetMouseDragDelta(0, 0.0f);
      GfVec2f value_with_lock_threshold = ANCHOR::GetMouseDragDelta(0);
      GfVec2f mouse_delta = io.MouseDelta;
      ANCHOR::Text("GetMouseDragDelta(0):");
      ANCHOR::Text("  w/ default threshold: (%.1f, %.1f)",
                   value_with_lock_threshold[0],
                   value_with_lock_threshold[1]);
      ANCHOR::Text("  w/ zero threshold: (%.1f, %.1f)", value_raw[0], value_raw[1]);
      ANCHOR::Text("io.MouseDelta: (%.1f, %.1f)", mouse_delta[0], mouse_delta[1]);
      ANCHOR::TreePop();
    }

    if (ANCHOR::TreeNode("Mouse cursors")) {
      const char *mouse_cursors_names[] = {"Arrow",
                                           "TextInput",
                                           "ResizeAll",
                                           "ResizeNS",
                                           "ResizeEW",
                                           "ResizeNESW",
                                           "ResizeNWSE",
                                           "Hand",
                                           "NotAllowed"};
      ANCHOR_ASSERT(ANCHOR_ARRAYSIZE(mouse_cursors_names) == ANCHOR_StandardCursorNumCursors);

      AnchorMouseCursor current = ANCHOR::GetMouseCursor();
      ANCHOR::Text("Current mouse cursor = %d: %s", current, mouse_cursors_names[current]);
      ANCHOR::Text("Hover to see mouse cursors:");
      ANCHOR::SameLine();
      HelpMarker(
        "Your application can render a different mouse cursor based on what "
        "ANCHOR::GetMouseCursor() returns. "
        "If software cursor rendering (io.MouseDrawCursor) is set ANCHOR will draw the right "
        "cursor for you, "
        "otherwise your backend needs to handle it.");
      for (int i = 0; i < ANCHOR_StandardCursorNumCursors; i++) {
        char label[32];
        sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
        ANCHOR::Bullet();
        ANCHOR::Selectable(label, false);
        if (ANCHOR::IsItemHovered())
          ANCHOR::SetMouseCursor(i);
      }
      ANCHOR::TreePop();
    }
  }
}

void ANCHOR::ShowAboutWindow(bool *p_open)
{
  if (!ANCHOR::Begin("About ANCHOR", p_open, AnchorWindowFlags_AlwaysAutoResize)) {
    ANCHOR::End();
    return;
  }
  ANCHOR::Text("ANCHOR %s", ANCHOR::GetVersion());
  ANCHOR::Separator();
  ANCHOR::Text("By Omar Cornut and all ANCHOR contributors.");
  ANCHOR::Text("ANCHOR is licensed under the MIT License, see LICENSE for more information.");

  static bool show_config_info = false;
  ANCHOR::Checkbox("Config/Build Information", &show_config_info);
  if (show_config_info) {
    AnchorIO &io = ANCHOR::GetIO();
    AnchorStyle &style = ANCHOR::GetStyle();

    bool copy_to_clipboard = ANCHOR::Button("Copy to clipboard");
    GfVec2f child_size = GfVec2f(0, ANCHOR::GetTextLineHeightWithSpacing() * 18);
    ANCHOR::BeginChildFrame(ANCHOR::GetID("cfg_infos"), child_size, AnchorWindowFlags_NoMove);
    if (copy_to_clipboard) {
      ANCHOR::LogToClipboard();
      ANCHOR::LogText("```\n");  // Back quotes will make text appears without formatting when
                                 // pasting on GitHub
    }

    ANCHOR::Text("ANCHOR %s (%d)", ANCHOR_VERSION, ANCHOR_VERSION_NUM);
    ANCHOR::Separator();
    ANCHOR::Text("sizeof(size_t): %d, sizeof(AnchorDrawIdx): %d, sizeof(AnchorDrawVert): %d",
                 (int)sizeof(size_t),
                 (int)sizeof(AnchorDrawIdx),
                 (int)sizeof(AnchorDrawVert));
    ANCHOR::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef ANCHOR_DISABLE_OBSOLETE_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_WIN32_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_DEFAULT_FORMAT_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_DEFAULT_MATH_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_FILE_FUNCTIONS
    ANCHOR::Text("define: ANCHOR_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef ANCHOR_DISABLE_DEFAULT_ALLOCATORS
    ANCHOR::Text("define: ANCHOR_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef ANCHOR_USE_BGRA_PACKED_COLOR
    ANCHOR::Text("define: ANCHOR_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
    ANCHOR::Text("define: _WIN32");
#endif
#ifdef _WIN64
    ANCHOR::Text("define: _WIN64");
#endif
#ifdef __linux__
    ANCHOR::Text("define: __linux__");
#endif
#ifdef __APPLE__
    ANCHOR::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
    ANCHOR::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
    ANCHOR::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
    ANCHOR::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
    ANCHOR::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
    ANCHOR::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
    ANCHOR::Text("define: __clang_version__=%s", __clang_version__);
#endif
    ANCHOR::Separator();
    ANCHOR::Text("io.BackendPlatformName: %s",
                 io.BackendPlatformName ? io.BackendPlatformName : "NULL");
    ANCHOR::Text("io.BackendRendererName: %s",
                 io.BackendRendererName ? io.BackendRendererName : "NULL");
    ANCHOR::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
    if (io.ConfigFlags & AnchorConfigFlags_NavEnableKeyboard)
      ANCHOR::Text(" NavEnableKeyboard");
    if (io.ConfigFlags & AnchorConfigFlags_NavEnableGamepad)
      ANCHOR::Text(" NavEnableGamepad");
    if (io.ConfigFlags & AnchorConfigFlags_NavEnableSetMousePos)
      ANCHOR::Text(" NavEnableSetMousePos");
    if (io.ConfigFlags & AnchorConfigFlags_NavNoCaptureKeyboard)
      ANCHOR::Text(" NavNoCaptureKeyboard");
    if (io.ConfigFlags & AnchorConfigFlags_NoMouse)
      ANCHOR::Text(" NoMouse");
    if (io.ConfigFlags & AnchorConfigFlags_NoMouseCursorChange)
      ANCHOR::Text(" NoMouseCursorChange");
    if (io.MouseDrawCursor)
      ANCHOR::Text("io.MouseDrawCursor");
    if (io.ConfigMacOSXBehaviors)
      ANCHOR::Text("io.ConfigMacOSXBehaviors");
    if (io.ConfigInputTextCursorBlink)
      ANCHOR::Text("io.ConfigInputTextCursorBlink");
    if (io.ConfigWindowsResizeFromEdges)
      ANCHOR::Text("io.ConfigWindowsResizeFromEdges");
    if (io.ConfigWindowsMoveFromTitleBarOnly)
      ANCHOR::Text("io.ConfigWindowsMoveFromTitleBarOnly");
    if (io.ConfigMemoryCompactTimer >= 0.0f)
      ANCHOR::Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
    ANCHOR::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
    if (io.BackendFlags & AnchorBackendFlags_HasGamepad)
      ANCHOR::Text(" HasGamepad");
    if (io.BackendFlags & AnchorBackendFlags_HasMouseCursors)
      ANCHOR::Text(" HasMouseCursors");
    if (io.BackendFlags & AnchorBackendFlags_HasSetMousePos)
      ANCHOR::Text(" HasSetMousePos");
    if (io.BackendFlags & AnchorBackendFlags_RendererHasVtxOffset)
      ANCHOR::Text(" RendererHasVtxOffset");
    ANCHOR::Separator();
    ANCHOR::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d",
                 io.Fonts->Fonts.Size,
                 io.Fonts->Flags,
                 io.Fonts->TexWidth,
                 io.Fonts->TexHeight);
    ANCHOR::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize[0], io.DisplaySize[1]);
    ANCHOR::Text("io.DisplayFramebufferScale: %.2f,%.2f",
                 io.DisplayFramebufferScale[0],
                 io.DisplayFramebufferScale[1]);
    ANCHOR::Separator();
    ANCHOR::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding[0], style.WindowPadding[1]);
    ANCHOR::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
    ANCHOR::Text("style.FramePadding: %.2f,%.2f", style.FramePadding[0], style.FramePadding[1]);
    ANCHOR::Text("style.FrameRounding: %.2f", style.FrameRounding);
    ANCHOR::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
    ANCHOR::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing[0], style.ItemSpacing[1]);
    ANCHOR::Text("style.ItemInnerSpacing: %.2f,%.2f",
                 style.ItemInnerSpacing[0],
                 style.ItemInnerSpacing[1]);

    if (copy_to_clipboard) {
      ANCHOR::LogText("\n```\n");
      ANCHOR::LogFinish();
    }
    ANCHOR::EndChildFrame();
  }
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------
// - ShowFontSelector()
// - ShowStyleSelector()
// - ShowStyleEditor()
//-----------------------------------------------------------------------------

// Forward declare ShowFontAtlas() which isn't worth putting in public API yet
namespace ANCHOR
{
  ANCHOR_API void ShowFontAtlas(AnchorFontAtlas *atlas);
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is more the more flexible one.
void ANCHOR::ShowFontSelector(const char *label)
{
  AnchorIO &io = ANCHOR::GetIO();
  AnchorFont *font_current = ANCHOR::GetFont();
  if (ANCHOR::BeginCombo(label, font_current->GetDebugName())) {
    for (int n = 0; n < io.Fonts->Fonts.Size; n++) {
      AnchorFont *font = io.Fonts->Fonts[n];
      ANCHOR::PushID((void *)font);
      if (ANCHOR::Selectable(font->GetDebugName(), font == font_current))
        io.FontDefault = font;
      ANCHOR::PopID();
    }
    ANCHOR::EndCombo();
  }
  ANCHOR::SameLine();
  HelpMarker(
    "- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
    "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
    "- Read FAQ and docs/FONTS.md for more details.\n"
    "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling "
    "NewFrame().");
}

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced
// options. Here we use the simplified Combo() api that packs items into a single literal string.
// Useful for quick combo boxes where the choices are known locally.
bool ANCHOR::ShowStyleSelector(const char *label)
{
  static int style_idx = -1;
  if (ANCHOR::Combo(label, &style_idx, "Default\0Dark\0Light\0")) {
    switch (style_idx) {
      case 0:
        ANCHOR::StyleColorsDefault();
        break;
      case 1:
        ANCHOR::StyleColorsDark();
        break;
      case 2:
        ANCHOR::StyleColorsLight();
        break;
    }
    return true;
  }
  return false;
}

void ANCHOR::ShowStyleEditor(AnchorStyle *ref)
{
  // You can pass in a reference AnchorStyle structure to compare to, revert to and save to
  // (without a reference style pointer, we will use one compared locally as a reference)
  AnchorStyle &style = ANCHOR::GetStyle();
  static AnchorStyle ref_saved_style;

  // Default to using internal storage as reference
  static bool init = true;
  if (init && ref == NULL)
    ref_saved_style = style;
  init = false;
  if (ref == NULL)
    ref = &ref_saved_style;

  ANCHOR::PushItemWidth(ANCHOR::GetWindowWidth() * 0.50f);

  if (ANCHOR::ShowStyleSelector("Colors##Selector"))
    ref_saved_style = style;
  ANCHOR::ShowFontSelector("Fonts##Selector");

  // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f
  // or 1.0f)
  if (ANCHOR::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
    style.GrabRounding =
      style.FrameRounding;  // Make GrabRounding always the same value as FrameRounding
  {
    bool border = (style.WindowBorderSize > 0.0f);
    if (ANCHOR::Checkbox("WindowBorder", &border)) {
      style.WindowBorderSize = border ? 1.0f : 0.0f;
    }
  }
  ANCHOR::SameLine();
  {
    bool border = (style.FrameBorderSize > 0.0f);
    if (ANCHOR::Checkbox("FrameBorder", &border)) {
      style.FrameBorderSize = border ? 1.0f : 0.0f;
    }
  }
  ANCHOR::SameLine();
  {
    bool border = (style.PopupBorderSize > 0.0f);
    if (ANCHOR::Checkbox("PopupBorder", &border)) {
      style.PopupBorderSize = border ? 1.0f : 0.0f;
    }
  }

  // Save/Revert button
  if (ANCHOR::Button("Save Ref"))
    *ref = ref_saved_style = style;
  ANCHOR::SameLine();
  if (ANCHOR::Button("Revert Ref"))
    style = *ref;
  ANCHOR::SameLine();
  HelpMarker(
    "Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
    "Use \"Export\" below to save them somewhere.");

  ANCHOR::Separator();

  if (ANCHOR::BeginTabBar("##tabs", AnchorTabBarFlags_None)) {
    if (ANCHOR::BeginTabItem("Sizes")) {
      ANCHOR::Text("Main");
      ANCHOR::SliderFloat2("WindowPadding", (float *)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
      ANCHOR::SliderFloat2("FramePadding", (float *)&style.FramePadding, 0.0f, 20.0f, "%.0f");
      ANCHOR::SliderFloat2("CellPadding", (float *)&style.CellPadding, 0.0f, 20.0f, "%.0f");
      ANCHOR::SliderFloat2("ItemSpacing", (float *)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
      ANCHOR::SliderFloat2("ItemInnerSpacing",
                           (float *)&style.ItemInnerSpacing,
                           0.0f,
                           20.0f,
                           "%.0f");
      ANCHOR::SliderFloat2("TouchExtraPadding",
                           (float *)&style.TouchExtraPadding,
                           0.0f,
                           10.0f,
                           "%.0f");
      ANCHOR::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
      ANCHOR::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
      ANCHOR::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
      ANCHOR::Text("Borders");
      ANCHOR::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
      ANCHOR::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
      ANCHOR::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
      ANCHOR::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
      ANCHOR::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
      ANCHOR::Text("Rounding");
      ANCHOR::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
      ANCHOR::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
      ANCHOR::Text("Alignment");
      ANCHOR::SliderFloat2("WindowTitleAlign",
                           (float *)&style.WindowTitleAlign,
                           0.0f,
                           1.0f,
                           "%.2f");
      int window_menu_button_position = style.WindowMenuButtonPosition + 1;
      if (ANCHOR::Combo("WindowMenuButtonPosition",
                        (int *)&window_menu_button_position,
                        "None\0Left\0Right\0"))
        style.WindowMenuButtonPosition = window_menu_button_position - 1;
      ANCHOR::Combo("ColorButtonPosition", (int *)&style.ColorButtonPosition, "Left\0Right\0");
      ANCHOR::SliderFloat2("ButtonTextAlign", (float *)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
      ANCHOR::SameLine();
      HelpMarker("Alignment applies when a button is larger than its text content.");
      ANCHOR::SliderFloat2("SelectableTextAlign",
                           (float *)&style.SelectableTextAlign,
                           0.0f,
                           1.0f,
                           "%.2f");
      ANCHOR::SameLine();
      HelpMarker("Alignment applies when a selectable is larger than its text content.");
      ANCHOR::Text("Safe Area Padding");
      ANCHOR::SameLine();
      HelpMarker(
        "Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not "
        "been configured).");
      ANCHOR::SliderFloat2("DisplaySafeAreaPadding",
                           (float *)&style.DisplaySafeAreaPadding,
                           0.0f,
                           30.0f,
                           "%.0f");
      ANCHOR::EndTabItem();
    }

    if (ANCHOR::BeginTabItem("Colors")) {
      static int output_dest = 0;
      static bool output_only_modified = true;
      if (ANCHOR::Button("Export")) {
        if (output_dest == 0)
          ANCHOR::LogToClipboard();
        else
          ANCHOR::LogToTTY();
        ANCHOR::LogText("GfVec4f* colors = ANCHOR::GetStyle().Colors;" ANCHOR_NEWLINE);
        for (int i = 0; i < AnchorCol_COUNT; i++) {
          const GfVec4f &col = style.Colors[i];
          const char *name = ANCHOR::GetStyleColorName(i);
          if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(GfVec4f)) != 0)
            ANCHOR::LogText(
              "colors[AnchorCol_%s]%*s= GfVec4f(%.2ff, %.2ff, %.2ff, %.2ff);" ANCHOR_NEWLINE,
              name,
              23 - (int)strlen(name),
              "",
              col[0],
              col[1],
              col[2],
              col[3]);
        }
        ANCHOR::LogFinish();
      }
      ANCHOR::SameLine();
      ANCHOR::SetNextItemWidth(120);
      ANCHOR::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
      ANCHOR::SameLine();
      ANCHOR::Checkbox("Only Modified Colors", &output_only_modified);

      static AnchorTextFilter filter;
      filter.Draw("Filter colors", ANCHOR::GetFontSize() * 16);

      static AnchorColorEditFlags alpha_flags = 0;
      if (ANCHOR::RadioButton("Opaque", alpha_flags == AnchorColorEditFlags_None)) {
        alpha_flags = AnchorColorEditFlags_None;
      }
      ANCHOR::SameLine();
      if (ANCHOR::RadioButton("Alpha", alpha_flags == AnchorColorEditFlags_AlphaPreview)) {
        alpha_flags = AnchorColorEditFlags_AlphaPreview;
      }
      ANCHOR::SameLine();
      if (ANCHOR::RadioButton("Both", alpha_flags == AnchorColorEditFlags_AlphaPreviewHalf)) {
        alpha_flags = AnchorColorEditFlags_AlphaPreviewHalf;
      }
      ANCHOR::SameLine();
      HelpMarker(
        "In the color list:\n"
        "Left-click on color square to open color picker,\n"
        "Right-click to open edit options menu.");

      ANCHOR::BeginChild("##colors",
                         GfVec2f(0, 0),
                         true,
                         AnchorWindowFlags_AlwaysVerticalScrollbar |
                           AnchorWindowFlags_AlwaysHorizontalScrollbar |
                           AnchorWindowFlags_NavFlattened);
      ANCHOR::PushItemWidth(-160);
      for (int i = 0; i < AnchorCol_COUNT; i++) {
        const char *name = ANCHOR::GetStyleColorName(i);
        if (!filter.PassFilter(name))
          continue;
        ANCHOR::PushID(i);
        ANCHOR::ColorEdit4("##color",
                           (float *)&style.Colors[i],
                           AnchorColorEditFlags_AlphaBar | alpha_flags);
        if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(GfVec4f)) != 0) {
          // Tips: in a real user application, you may want to merge and use an icon font into the
          // main font, so instead of "Save"/"Revert" you'd use icons! Read the FAQ and
          // docs/FONTS.md about using icon fonts. It's really easy and super convenient!
          ANCHOR::SameLine(0.0f, style.ItemInnerSpacing[0]);
          if (ANCHOR::Button("Save")) {
            ref->Colors[i] = style.Colors[i];
          }
          ANCHOR::SameLine(0.0f, style.ItemInnerSpacing[0]);
          if (ANCHOR::Button("Revert")) {
            style.Colors[i] = ref->Colors[i];
          }
        }
        ANCHOR::SameLine(0.0f, style.ItemInnerSpacing[0]);
        ANCHOR::TextUnformatted(name);
        ANCHOR::PopID();
      }
      ANCHOR::PopItemWidth();
      ANCHOR::EndChild();

      ANCHOR::EndTabItem();
    }

    if (ANCHOR::BeginTabItem("Fonts")) {
      AnchorIO &io = ANCHOR::GetIO();
      AnchorFontAtlas *atlas = io.Fonts;
      HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
      ANCHOR::ShowFontAtlas(atlas);

      // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
      // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click
      // text to get out of bounds).
      const float MIN_SCALE = 0.3f;
      const float MAX_SCALE = 2.0f;
      HelpMarker(
        "Those are old settings provided for convenience.\n"
        "However, the _correct_ way of scaling your UI is currently to reload your font at the "
        "designed size, "
        "rebuild the font atlas, and call style.ScaleAllSizes() on a reference AnchorStyle "
        "structure.\n"
        "Using those settings here will give you poor quality results.");
      static float window_scale = 1.0f;
      ANCHOR::PushItemWidth(ANCHOR::GetFontSize() * 8);
      if (ANCHOR::DragFloat("window scale",
                            &window_scale,
                            0.005f,
                            MIN_SCALE,
                            MAX_SCALE,
                            "%.2f",
                            AnchorSliderFlags_AlwaysClamp))  // Scale only this window
        ANCHOR::SetWindowFontScale(window_scale);
      ANCHOR::DragFloat("global scale",
                        &io.FontGlobalScale,
                        0.005f,
                        MIN_SCALE,
                        MAX_SCALE,
                        "%.2f",
                        AnchorSliderFlags_AlwaysClamp);  // Scale everything
      ANCHOR::PopItemWidth();

      ANCHOR::EndTabItem();
    }

    if (ANCHOR::BeginTabItem("Rendering")) {
      ANCHOR::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
      ANCHOR::SameLine();
      HelpMarker(
        "When disabling anti-aliasing lines, you'll probably want to disable borders in your "
        "style as well.");

      ANCHOR::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
      ANCHOR::SameLine();
      HelpMarker(
        "Faster lines using texture data. Require backend to render with bilinear filtering "
        "(not point/nearest filtering).");

      ANCHOR::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
      ANCHOR::PushItemWidth(ANCHOR::GetFontSize() * 8);
      ANCHOR::DragFloat("Curve Tessellation Tolerance",
                        &style.CurveTessellationTol,
                        0.02f,
                        0.10f,
                        10.0f,
                        "%.2f");
      if (style.CurveTessellationTol < 0.10f)
        style.CurveTessellationTol = 0.10f;

      // When editing the "Circle Segment Max Error" value, draw a preview of its effect on
      // auto-tessellated circles.
      ANCHOR::DragFloat("Circle Tessellation Max Error",
                        &style.CircleTessellationMaxError,
                        0.005f,
                        0.10f,
                        5.0f,
                        "%.2f",
                        AnchorSliderFlags_AlwaysClamp);
      if (ANCHOR::IsItemActive()) {
        ANCHOR::SetNextWindowPos(ANCHOR::GetCursorScreenPos());
        ANCHOR::BeginTooltip();
        ANCHOR::TextUnformatted("(R = radius, N = number of segments)");
        ANCHOR::Spacing();
        AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();
        const float min_widget_width = ANCHOR::CalcTextSize("N: MMM\nR: MMM")[0];
        for (int n = 0; n < 8; n++) {
          const float RAD_MIN = 5.0f;
          const float RAD_MAX = 70.0f;
          const float rad = RAD_MIN + (RAD_MAX - RAD_MIN) * (float)n / (8.0f - 1.0f);

          ANCHOR::BeginGroup();

          ANCHOR::Text("R: %.f\nN: %d", rad, draw_list->_CalcCircleAutoSegmentCount(rad));

          const float canvas_width = ANCHOR_MAX(min_widget_width, rad * 2.0f);
          const float offset_x = floorf(canvas_width * 0.5f);
          const float offset_y = floorf(RAD_MAX);

          const GfVec2f p1 = ANCHOR::GetCursorScreenPos();
          draw_list->AddCircle(GfVec2f(p1[0] + offset_x, p1[1] + offset_y),
                               rad,
                               ANCHOR::GetColorU32(AnchorCol_Text));
          ANCHOR::Dummy(GfVec2f(canvas_width, RAD_MAX * 2));

          /*
          const GfVec2f p2 = ANCHOR::GetCursorScreenPos();
          draw_list->AddCircleFilled(GfVec2f(p2[0] + offset_x, p2[1] + offset_y), rad,
          ANCHOR::GetColorU32(AnchorCol_Text)); ANCHOR::Dummy(GfVec2f(canvas_width, RAD_MAX * 2));
          */

          ANCHOR::EndGroup();
          ANCHOR::SameLine();
        }
        ANCHOR::EndTooltip();
      }
      ANCHOR::SameLine();
      HelpMarker(
        "When drawing circle primitives with \"num_segments == 0\" tesselation will be "
        "calculated automatically.");

      ANCHOR::DragFloat("Global Alpha",
                        &style.Alpha,
                        0.005f,
                        0.20f,
                        1.0f,
                        "%.2f");  // Not exposing zero here so user doesn't "lose" the UI (zero
                                  // alpha clips all widgets). But application code could have a
                                  // toggle to switch between zero and non-zero.
      ANCHOR::PopItemWidth();

      ANCHOR::EndTabItem();
    }

    ANCHOR::EndTabBar();
  }

  ANCHOR::PopItemWidth();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowExampleAppMainMenuBar()
// - ShowExampleMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the AnchorWindowFlags_MenuBar
// flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport +
// call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar()
{
  if (ANCHOR::BeginMainMenuBar()) {
    if (ANCHOR::BeginMenu("File")) {
      ShowExampleMenuFile();
      ANCHOR::EndMenu();
    }
    if (ANCHOR::BeginMenu("Edit")) {
      if (ANCHOR::MenuItem("Undo", "CTRL+Z")) {
      }
      if (ANCHOR::MenuItem("Redo", "CTRL+Y", false, false)) {
      }  // Disabled item
      ANCHOR::Separator();
      if (ANCHOR::MenuItem("Cut", "CTRL+X")) {
      }
      if (ANCHOR::MenuItem("Copy", "CTRL+C")) {
      }
      if (ANCHOR::MenuItem("Paste", "CTRL+V")) {
      }
      ANCHOR::EndMenu();
    }
    ANCHOR::EndMainMenuBar();
  }
}

// Note that shortcuts are currently provided for display only
// (future version will add explicit flags to BeginMenu() to request processing shortcuts)
static void ShowExampleMenuFile()
{
  ANCHOR::MenuItem("(demo menu)", NULL, false, false);
  if (ANCHOR::MenuItem("New")) {
  }
  if (ANCHOR::MenuItem("Open", "Ctrl+O")) {
  }
  if (ANCHOR::BeginMenu("Open Recent")) {
    ANCHOR::MenuItem("fish_hat.c");
    ANCHOR::MenuItem("fish_hat.inl");
    ANCHOR::MenuItem("fish_hat.h");
    if (ANCHOR::BeginMenu("More..")) {
      ANCHOR::MenuItem("Hello");
      ANCHOR::MenuItem("Sailor");
      if (ANCHOR::BeginMenu("Recurse..")) {
        ShowExampleMenuFile();
        ANCHOR::EndMenu();
      }
      ANCHOR::EndMenu();
    }
    ANCHOR::EndMenu();
  }
  if (ANCHOR::MenuItem("Save", "Ctrl+S")) {
  }
  if (ANCHOR::MenuItem("Save As..")) {
  }

  ANCHOR::Separator();
  if (ANCHOR::BeginMenu("Options")) {
    static bool enabled = true;
    ANCHOR::MenuItem("Enabled", "", &enabled);
    ANCHOR::BeginChild("child", GfVec2f(0, 60), true);
    for (int i = 0; i < 10; i++)
      ANCHOR::Text("Scrolling Text %d", i);
    ANCHOR::EndChild();
    static float f = 0.5f;
    static int n = 0;
    ANCHOR::SliderFloat("Value", &f, 0.0f, 1.0f);
    ANCHOR::InputFloat("Input", &f, 0.1f);
    ANCHOR::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
    ANCHOR::EndMenu();
  }

  if (ANCHOR::BeginMenu("Colors")) {
    float sz = ANCHOR::GetTextLineHeight();
    for (int i = 0; i < AnchorCol_COUNT; i++) {
      const char *name = ANCHOR::GetStyleColorName((AnchorCol)i);
      GfVec2f p = ANCHOR::GetCursorScreenPos();
      ANCHOR::GetWindowDrawList()->AddRectFilled(p,
                                                 GfVec2f(p[0] + sz, p[1] + sz),
                                                 ANCHOR::GetColorU32((AnchorCol)i));
      ANCHOR::Dummy(GfVec2f(sz, sz));
      ANCHOR::SameLine();
      ANCHOR::MenuItem(name);
    }
    ANCHOR::EndMenu();
  }

  // Here we demonstrate appending again to the "Options" menu (which we already created above)
  // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options")
  // twice. In a real code-base using it would make senses to use this feature from very different
  // code locations.
  if (ANCHOR::BeginMenu("Options"))  // <-- Append!
  {
    static bool b = true;
    ANCHOR::Checkbox("SomeOption", &b);
    ANCHOR::EndMenu();
  }

  if (ANCHOR::BeginMenu("Disabled", false))  // Disabled
  {
    ANCHOR_ASSERT(0);
  }
  if (ANCHOR::MenuItem("Checked", NULL, true)) {
  }
  if (ANCHOR::MenuItem("Quit", "Alt+F4")) {
  }
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, we are using a more C++ like approach of declaring a class to hold both
// data and functions.
struct ExampleAppConsole
{
  char InputBuf[256];
  AnchorVector<char *> Items;
  AnchorVector<const char *> Commands;
  AnchorVector<char *> History;
  int HistoryPos;  // -1: new line, 0..History.Size-1 browsing history.
  AnchorTextFilter Filter;
  bool AutoScroll;
  bool ScrollToBottom;

  ExampleAppConsole()
  {
    ClearLog();
    memset(InputBuf, 0, sizeof(InputBuf));
    HistoryPos = -1;

    // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display
    // multiple matches.
    Commands.push_back("HELP");
    Commands.push_back("HISTORY");
    Commands.push_back("CLEAR");
    Commands.push_back("CLASSIFY");
    AutoScroll = true;
    ScrollToBottom = false;
    AddLog("Welcome to ANCHOR!");
  }
  ~ExampleAppConsole()
  {
    ClearLog();
    for (int i = 0; i < History.Size; i++)
      free(History[i]);
  }

  // Portable helpers
  static int Stricmp(const char *s1, const char *s2)
  {
    int d;
    while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
      s1++;
      s2++;
    }
    return d;
  }
  static int Strnicmp(const char *s1, const char *s2, int n)
  {
    int d = 0;
    while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
      s1++;
      s2++;
      n--;
    }
    return d;
  }
  static char *Strdup(const char *s)
  {
    ANCHOR_ASSERT(s);
    size_t len = strlen(s) + 1;
    void *buf = malloc(len);
    ANCHOR_ASSERT(buf);
    return (char *)memcpy(buf, (const void *)s, len);
  }
  static void Strtrim(char *s)
  {
    char *str_end = s + strlen(s);
    while (str_end > s && str_end[-1] == ' ')
      str_end--;
    *str_end = 0;
  }

  void ClearLog()
  {
    for (int i = 0; i < Items.Size; i++)
      free(Items[i]);
    Items.clear();
  }

  void AddLog(const char *fmt, ...) ANCHOR_FMTARGS(2)
  {
    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, ANCHOR_ARRAYSIZE(buf), fmt, args);
    buf[ANCHOR_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    Items.push_back(Strdup(buf));
  }

  void Draw(const char *title, bool *p_open)
  {
    ANCHOR::SetNextWindowSize(GfVec2f(520, 600), AnchorCond_FirstUseEver);
    if (!ANCHOR::Begin(title, p_open)) {
      ANCHOR::End();
      return;
    }

    // As a specific feature guaranteed by the library, after calling Begin() the last Item
    // represent the title bar. So e.g. IsItemHovered() will return true when hovering the title
    // bar. Here we create a context menu only available from the title bar.
    if (ANCHOR::BeginPopupContextItem()) {
      if (ANCHOR::MenuItem("Close Console"))
        *p_open = false;
      ANCHOR::EndPopup();
    }

    ANCHOR::TextWrapped(
      "This example implements a console with basic coloring, completion (TAB key) and history "
      "(Up/Down keys). A more elaborate "
      "implementation may want to store entries along with extra data such as timestamp, "
      "emitter, etc.");
    ANCHOR::TextWrapped("Enter 'HELP' for help.");

    // TODO: display items starting from the bottom

    if (ANCHOR::SmallButton("Add Debug Text")) {
      AddLog("%d some text", Items.Size);
      AddLog("some more text");
      AddLog("display very important message here!");
    }
    ANCHOR::SameLine();
    if (ANCHOR::SmallButton("Add Debug Error")) {
      AddLog("[error] something went wrong");
    }
    ANCHOR::SameLine();
    if (ANCHOR::SmallButton("Clear")) {
      ClearLog();
    }
    ANCHOR::SameLine();
    bool copy_to_clipboard = ANCHOR::SmallButton("Copy");
    // static float t = 0.0f; if (ANCHOR::GetTime() - t > 0.02f) { t = ANCHOR::GetTime();
    // AddLog("Spam %f", t); }

    ANCHOR::Separator();

    // Options menu
    if (ANCHOR::BeginPopup("Options")) {
      ANCHOR::Checkbox("Auto-scroll", &AutoScroll);
      ANCHOR::EndPopup();
    }

    // Options, Filter
    if (ANCHOR::Button("Options"))
      ANCHOR::OpenPopup("Options");
    ANCHOR::SameLine();
    Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    ANCHOR::Separator();

    // Reserve enough left-over height for 1 separator + 1 input text
    const float footer_height_to_reserve = ANCHOR::GetStyle().ItemSpacing[1] +
                                           ANCHOR::GetFrameHeightWithSpacing();
    ANCHOR::BeginChild("ScrollingRegion",
                       GfVec2f(0, -footer_height_to_reserve),
                       false,
                       AnchorWindowFlags_HorizontalScrollbar);
    if (ANCHOR::BeginPopupContextWindow()) {
      if (ANCHOR::Selectable("Clear"))
        ClearLog();
      ANCHOR::EndPopup();
    }

    // Display every line as a separate entry so we can change their color or add custom widgets.
    // If you only want raw text you can use ANCHOR::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient and may require
    // user-side clipping to only process visible items. The clipper will automatically measure the
    // height of your first item and then "seek" to display only items in the visible area. To use
    // the clipper we can replace your standard loop:
    //      for (int i = 0; i < Items.Size; i++)
    //   With:
    //      AnchorListClipper clipper;
    //      clipper.Begin(Items.Size);
    //      while (clipper.Step())
    //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    // - That your items are evenly spaced (same height)
    // - That you have cheap random access to your elements (you can access them given their index,
    //   without processing all the ones before)
    // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access'
    // property. We would need random-access on the post-filtered list. A typical application
    // wanting coarse clipping and filtering may want to pre-compute an array of indices or offsets
    // of items that passed the filtering test, recomputing this array when user changes the
    // filter, and appending newly elements as they are inserted. This is left as a task to the
    // user until we can manage to improve this example code! If your items are of variable height:
    // - Split them into same height items would be simpler and facilitate random-seeking into your
    // list.
    // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your
    // items.
    ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing, GfVec2f(4, 1));  // Tighten spacing
    if (copy_to_clipboard)
      ANCHOR::LogToClipboard();
    for (int i = 0; i < Items.Size; i++) {
      const char *item = Items[i];
      if (!Filter.PassFilter(item))
        continue;

      // Normally you would store more information in your item than just a string.
      // (e.g. make Items[] an array of structure, store color/type etc.)
      GfVec4f color;
      bool has_color = false;
      if (strstr(item, "[error]")) {
        color = GfVec4f(1.0f, 0.4f, 0.4f, 1.0f);
        has_color = true;
      } else if (strncmp(item, "# ", 2) == 0) {
        color = GfVec4f(1.0f, 0.8f, 0.6f, 1.0f);
        has_color = true;
      }
      if (has_color)
        ANCHOR::PushStyleColor(AnchorCol_Text, color);
      ANCHOR::TextUnformatted(item);
      if (has_color)
        ANCHOR::PopStyleColor();
    }
    if (copy_to_clipboard)
      ANCHOR::LogFinish();

    if (ScrollToBottom || (AutoScroll && ANCHOR::GetScrollY() >= ANCHOR::GetScrollMaxY()))
      ANCHOR::SetScrollHereY(1.0f);
    ScrollToBottom = false;

    ANCHOR::PopStyleVar();
    ANCHOR::EndChild();
    ANCHOR::Separator();

    // Command-line
    bool reclaim_focus = false;
    AnchorInputTextFlags input_text_flags = AnchorInputTextFlags_EnterReturnsTrue |
                                            AnchorInputTextFlags_CallbackCompletion |
                                            AnchorInputTextFlags_CallbackHistory;
    if (ANCHOR::InputText("Input",
                          InputBuf,
                          ANCHOR_ARRAYSIZE(InputBuf),
                          input_text_flags,
                          &TextEditCallbackStub,
                          (void *)this)) {
      char *s = InputBuf;
      Strtrim(s);
      if (s[0])
        ExecCommand(s);
      strcpy(s, "");
      reclaim_focus = true;
    }

    // Auto-focus on window apparition
    ANCHOR::SetItemDefaultFocus();
    if (reclaim_focus)
      ANCHOR::SetKeyboardFocusHere(-1);  // Auto focus previous widget

    ANCHOR::End();
  }

  void ExecCommand(const char *command_line)
  {
    AddLog("# %s\n", command_line);

    // Insert into history. First find match and delete it so it can be pushed to the back.
    // This isn't trying to be smart or optimal.
    HistoryPos = -1;
    for (int i = History.Size - 1; i >= 0; i--)
      if (Stricmp(History[i], command_line) == 0) {
        free(History[i]);
        History.erase(History.begin() + i);
        break;
      }
    History.push_back(Strdup(command_line));

    // Process command
    if (Stricmp(command_line, "CLEAR") == 0) {
      ClearLog();
    } else if (Stricmp(command_line, "HELP") == 0) {
      AddLog("Commands:");
      for (int i = 0; i < Commands.Size; i++)
        AddLog("- %s", Commands[i]);
    } else if (Stricmp(command_line, "HISTORY") == 0) {
      int first = History.Size - 10;
      for (int i = first > 0 ? first : 0; i < History.Size; i++)
        AddLog("%3d: %s\n", i, History[i]);
    } else {
      AddLog("Unknown command: '%s'\n", command_line);
    }

    // On command input, we scroll to bottom even if AutoScroll==false
    ScrollToBottom = true;
  }

  // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
  static int TextEditCallbackStub(AnchorInputTextCallbackData *data)
  {
    ExampleAppConsole *console = (ExampleAppConsole *)data->UserData;
    return console->TextEditCallback(data);
  }

  int TextEditCallback(AnchorInputTextCallbackData *data)
  {
    // AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart,
    // data->SelectionEnd);
    switch (data->EventFlag) {
      case AnchorInputTextFlags_CallbackCompletion: {
        // Example of TEXT COMPLETION

        // Locate beginning of current word
        const char *word_end = data->Buf + data->CursorPos;
        const char *word_start = word_end;
        while (word_start > data->Buf) {
          const char c = word_start[-1];
          if (c == ' ' || c == '\t' || c == ',' || c == ';')
            break;
          word_start--;
        }

        // Build a list of candidates
        AnchorVector<const char *> candidates;
        for (int i = 0; i < Commands.Size; i++)
          if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
            candidates.push_back(Commands[i]);

        if (candidates.Size == 0) {
          // No match
          AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
        } else if (candidates.Size == 1) {
          // Single match. Delete the beginning of the word and replace it entirely so we've got
          // nice casing.
          data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
          data->InsertChars(data->CursorPos, candidates[0]);
          data->InsertChars(data->CursorPos, " ");
        } else {
          // Multiple matches. Complete as much as we can..
          // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as
          // matches.
          int match_len = (int)(word_end - word_start);
          for (;;) {
            int c = 0;
            bool all_candidates_matches = true;
            for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
              if (i == 0)
                c = toupper(candidates[i][match_len]);
              else if (c == 0 || c != toupper(candidates[i][match_len]))
                all_candidates_matches = false;
            if (!all_candidates_matches)
              break;
            match_len++;
          }

          if (match_len > 0) {
            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
          }

          // List matches
          AddLog("Possible matches:\n");
          for (int i = 0; i < candidates.Size; i++)
            AddLog("- %s\n", candidates[i]);
        }

        break;
      }
      case AnchorInputTextFlags_CallbackHistory: {
        // Example of HISTORY
        const int prev_history_pos = HistoryPos;
        if (data->EventKey == AnchorKey_UpArrow) {
          if (HistoryPos == -1)
            HistoryPos = History.Size - 1;
          else if (HistoryPos > 0)
            HistoryPos--;
        } else if (data->EventKey == AnchorKey_DownArrow) {
          if (HistoryPos != -1)
            if (++HistoryPos >= History.Size)
              HistoryPos = -1;
        }

        // A better implementation would preserve the data on the current input line along with
        // cursor position.
        if (prev_history_pos != HistoryPos) {
          const char *history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
          data->DeleteChars(0, data->BufTextLen);
          data->InsertChars(0, history_str);
        }
      }
    }
    return 0;
  }
};

static void ShowExampleAppConsole(bool *p_open)
{
  static ExampleAppConsole console;
  console.Draw("Example: Console", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
  AnchorTextBuffer Buf;
  AnchorTextFilter Filter;
  AnchorVector<int> LineOffsets;  // Index to lines offset. We maintain this with AddLog() calls.
  bool AutoScroll;                // Keep scrolling if already at the bottom.

  ExampleAppLog()
  {
    AutoScroll = true;
    Clear();
  }

  void Clear()
  {
    Buf.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
  }

  void AddLog(const char *fmt, ...) ANCHOR_FMTARGS(2)
  {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n')
        LineOffsets.push_back(old_size + 1);
  }

  void Draw(const char *title, bool *p_open = NULL)
  {
    if (!ANCHOR::Begin(title, p_open)) {
      ANCHOR::End();
      return;
    }

    // Options menu
    if (ANCHOR::BeginPopup("Options")) {
      ANCHOR::Checkbox("Auto-scroll", &AutoScroll);
      ANCHOR::EndPopup();
    }

    // Main window
    if (ANCHOR::Button("Options"))
      ANCHOR::OpenPopup("Options");
    ANCHOR::SameLine();
    bool clear = ANCHOR::Button("Clear");
    ANCHOR::SameLine();
    bool copy = ANCHOR::Button("Copy");
    ANCHOR::SameLine();
    Filter.Draw("Filter", -100.0f);

    ANCHOR::Separator();
    ANCHOR::BeginChild("scrolling", GfVec2f(0, 0), false, AnchorWindowFlags_HorizontalScrollbar);

    if (clear)
      Clear();
    if (copy)
      ANCHOR::LogToClipboard();

    ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing, GfVec2f(0, 0));
    const char *buf = Buf.begin();
    const char *buf_end = Buf.end();
    if (Filter.IsActive()) {
      // In this example we don't use the clipper when Filter is enabled.
      // This is because we don't have a random access on the result on our filter.
      // A real application processing logs with ten of thousands of entries may want to store the
      // result of search/filter.. especially if the filtering function is not trivial (e.g.
      // reg-exp).
      for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
        const char *line_start = buf + LineOffsets[line_no];
        const char *line_end = (line_no + 1 < LineOffsets.Size) ?
                                 (buf + LineOffsets[line_no + 1] - 1) :
                                 buf_end;
        if (Filter.PassFilter(line_start, line_end))
          ANCHOR::TextUnformatted(line_start, line_end);
      }
    } else {
      // The simplest and easy way to display the entire buffer:
      //   ANCHOR::TextUnformatted(buf_begin, buf_end);
      // And it'll just work. TextUnformatted() has specialization for large blob of text and will
      // fast-forward to skip non-visible lines. Here we instead demonstrate using the clipper to
      // only process lines that are within the visible area. If you have tens of thousands of
      // items and their processing cost is non-negligible, coarse clipping them on your side is
      // recommended. Using AnchorListClipper requires
      // - A) random access into your data
      // - B) items all being the  same height,
      // both of which we can handle since we an array pointing to the beginning of each line of
      // text. When using the filter (in the block of code above) we don't have random access into
      // the data to display anymore, which is why we don't use the clipper. Storing or skimming
      // through the search result would make it possible (and would be recommended if you want to
      // search through tens of thousands of entries).
      AnchorListClipper clipper;
      clipper.Begin(LineOffsets.Size);
      while (clipper.Step()) {
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
          const char *line_start = buf + LineOffsets[line_no];
          const char *line_end = (line_no + 1 < LineOffsets.Size) ?
                                   (buf + LineOffsets[line_no + 1] - 1) :
                                   buf_end;
          ANCHOR::TextUnformatted(line_start, line_end);
        }
      }
      clipper.End();
    }
    ANCHOR::PopStyleVar();

    if (AutoScroll && ANCHOR::GetScrollY() >= ANCHOR::GetScrollMaxY())
      ANCHOR::SetScrollHereY(1.0f);

    ANCHOR::EndChild();
    ANCHOR::End();
  }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool *p_open)
{
  static ExampleAppLog log;

  // For the demo: add a debug button _BEFORE_ the normal log window contents
  // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to
  // the _same_ window. Most of the contents of the window will be added by the log.Draw() call.
  ANCHOR::SetNextWindowSize(GfVec2f(500, 400), AnchorCond_FirstUseEver);
  ANCHOR::Begin("Example: Log", p_open);
  if (ANCHOR::SmallButton("[Debug] Add 5 entries")) {
    static int counter = 0;
    const char *categories[3] = {"info", "warn", "error"};
    const char *words[] = {"Bumfuzzled",
                           "Cattywampus",
                           "Snickersnee",
                           "Abibliophobia",
                           "Absquatulate",
                           "Nincompoop",
                           "Pauciloquent"};
    for (int n = 0; n < 5; n++) {
      const char *category = categories[counter % ANCHOR_ARRAYSIZE(categories)];
      const char *word = words[counter % ANCHOR_ARRAYSIZE(words)];
      log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                 ANCHOR::GetFrameCount(),
                 category,
                 ANCHOR::GetTime(),
                 word);
      counter++;
    }
  }
  ANCHOR::End();

  // Actually call in the regular Log helper (which will Begin() into the same window as we just
  // did)
  log.Draw("Example: Log", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool *p_open)
{
  ANCHOR::SetNextWindowSize(GfVec2f(500, 440), AnchorCond_FirstUseEver);
  if (ANCHOR::Begin("Example: Simple layout", p_open, AnchorWindowFlags_MenuBar)) {
    if (ANCHOR::BeginMenuBar()) {
      if (ANCHOR::BeginMenu("File")) {
        if (ANCHOR::MenuItem("Close"))
          *p_open = false;
        ANCHOR::EndMenu();
      }
      ANCHOR::EndMenuBar();
    }

    // Left
    static int selected = 0;
    {
      ANCHOR::BeginChild("left pane", GfVec2f(150, 0), true);
      for (int i = 0; i < 100; i++) {
        char label[128];
        sprintf(label, "MyObject %d", i);
        if (ANCHOR::Selectable(label, selected == i))
          selected = i;
      }
      ANCHOR::EndChild();
    }
    ANCHOR::SameLine();

    // Right
    {
      ANCHOR::BeginGroup();
      ANCHOR::BeginChild(
        "item view",
        GfVec2f(0, -ANCHOR::GetFrameHeightWithSpacing()));  // Leave room for 1 line below us
      ANCHOR::Text("MyObject: %d", selected);
      ANCHOR::Separator();
      if (ANCHOR::BeginTabBar("##Tabs", AnchorTabBarFlags_None)) {
        if (ANCHOR::BeginTabItem("Description")) {
          ANCHOR::TextWrapped(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
            "incididunt ut labore et dolore magna aliqua. ");
          ANCHOR::EndTabItem();
        }
        if (ANCHOR::BeginTabItem("Details")) {
          ANCHOR::Text("ID: 0123456789");
          ANCHOR::EndTabItem();
        }
        ANCHOR::EndTabBar();
      }
      ANCHOR::EndChild();
      if (ANCHOR::Button("Revert")) {
      }
      ANCHOR::SameLine();
      if (ANCHOR::Button("Save")) {
      }
      ANCHOR::EndGroup();
    }
  }
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
//-----------------------------------------------------------------------------

static void ShowPlaceholderObject(const char *prefix, int uid)
{
  // Use object uid as identifier. Most commonly you could also use the object pointer as a base
  // ID.
  ANCHOR::PushID(uid);

  // Text and Tree nodes are less high than framed widgets, using AlignTextToFramePadding() we add
  // vertical spacing to make the tree lines equal high.
  ANCHOR::TableNextRow();
  ANCHOR::TableSetColumnIndex(0);
  ANCHOR::AlignTextToFramePadding();
  bool node_open = ANCHOR::TreeNode("Object", "%s_%u", prefix, uid);
  ANCHOR::TableSetColumnIndex(1);
  ANCHOR::Text("my sailor is rich");

  if (node_open) {
    static float placeholder_members[8] = {0.0f, 0.0f, 1.0f, 3.1416f, 100.0f, 999.0f};
    for (int i = 0; i < 8; i++) {
      ANCHOR::PushID(i);  // Use field index as identifier.
      if (i < 2) {
        ShowPlaceholderObject("Child", 424242);
      } else {
        // Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
        ANCHOR::TableNextRow();
        ANCHOR::TableSetColumnIndex(0);
        ANCHOR::AlignTextToFramePadding();
        AnchorTreeNodeFlags flags = AnchorTreeNodeFlags_Leaf |
                                    AnchorTreeNodeFlags_NoTreePushOnOpen |
                                    AnchorTreeNodeFlags_Bullet;
        ANCHOR::TreeNodeEx("Field", flags, "Field_%d", i);

        ANCHOR::TableSetColumnIndex(1);
        ANCHOR::SetNextItemWidth(-FLT_MIN);
        if (i >= 5)
          ANCHOR::InputFloat("##value", &placeholder_members[i], 1.0f);
        else
          ANCHOR::DragFloat("##value", &placeholder_members[i], 0.01f);
        ANCHOR::NextColumn();
      }
      ANCHOR::PopID();
    }
    ANCHOR::TreePop();
  }
  ANCHOR::PopID();
}

// Demonstrate create a simple property editor.
static void ShowExampleAppPropertyEditor(bool *p_open)
{
  ANCHOR::SetNextWindowSize(GfVec2f(430, 450), AnchorCond_FirstUseEver);
  if (!ANCHOR::Begin("Example: Property editor", p_open)) {
    ANCHOR::End();
    return;
  }

  HelpMarker(
    "This example shows how you may implement a property editor using two columns.\n"
    "All objects/fields data are dummies here.\n"
    "Remember that in many simple cases, you can use ANCHOR::SameLine(xxx) to position\n"
    "your cursor horizontally instead of using the Columns() API.");

  ANCHOR::PushStyleVar(AnchorStyleVar_FramePadding, GfVec2f(2, 2));
  if (ANCHOR::BeginTable("split", 2, AnchorTableFlags_BordersOuter | AnchorTableFlags_Resizable)) {
    // Iterate placeholder objects (all the same data)
    for (int obj_i = 0; obj_i < 4; obj_i++) {
      ShowPlaceholderObject("Object", obj_i);
      // ANCHOR::Separator();
    }
    ANCHOR::EndTable();
  }
  ANCHOR::PopStyleVar();
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool *p_open)
{
  ANCHOR::SetNextWindowSize(GfVec2f(520, 600), AnchorCond_FirstUseEver);
  if (!ANCHOR::Begin("Example: Long text display", p_open)) {
    ANCHOR::End();
    return;
  }

  static int test_type = 0;
  static AnchorTextBuffer log;
  static int lines = 0;
  ANCHOR::Text("Printing unusually long amount of text.");
  ANCHOR::Combo("Test type",
                &test_type,
                "Single call to TextUnformatted()\0"
                "Multiple calls to Text(), clipped\0"
                "Multiple calls to Text(), not clipped (slow)\0");
  ANCHOR::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
  if (ANCHOR::Button("Clear")) {
    log.clear();
    lines = 0;
  }
  ANCHOR::SameLine();
  if (ANCHOR::Button("Add 1000 lines")) {
    for (int i = 0; i < 1000; i++)
      log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines + i);
    lines += 1000;
  }
  ANCHOR::BeginChild("Log");
  switch (test_type) {
    case 0:
      // Single call to TextUnformatted() with a big buffer
      ANCHOR::TextUnformatted(log.begin(), log.end());
      break;
    case 1: {
      // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the
      // AnchorListClipper helper.
      ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing, GfVec2f(0, 0));
      AnchorListClipper clipper;
      clipper.Begin(lines);
      while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
          ANCHOR::Text("%i The quick brown fox jumps over the lazy dog", i);
      ANCHOR::PopStyleVar();
      break;
    }
    case 2:
      // Multiple calls to Text(), not clipped (slow)
      ANCHOR::PushStyleVar(AnchorStyleVar_ItemSpacing, GfVec2f(0, 0));
      for (int i = 0; i < lines; i++)
        ANCHOR::Text("%i The quick brown fox jumps over the lazy dog", i);
      ANCHOR::PopStyleVar();
      break;
  }
  ANCHOR::EndChild();
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool *p_open)
{
  if (!ANCHOR::Begin("Example: Auto-resizing window",
                     p_open,
                     AnchorWindowFlags_AlwaysAutoResize)) {
    ANCHOR::End();
    return;
  }

  static int lines = 10;
  ANCHOR::TextUnformatted(
    "Window will resize every-frame to the size of its content.\n"
    "Note that you probably don't want to query the window size to\n"
    "output your content because that would create a feedback loop.");
  ANCHOR::SliderInt("Number of lines", &lines, 1, 20);
  for (int i = 0; i < lines; i++)
    ANCHOR::Text("%*sThis is line %d",
                 i * 4,
                 "",
                 i);  // Pad with space to extend size horizontally
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window with custom resize constraints.
static void ShowExampleAppConstrainedResize(bool *p_open)
{
  struct CustomConstraints
  {
    // Helper functions to demonstrate programmatic constraints
    static void Square(AnchorSizeCallbackData *data)
    {
      data->DesiredSize[0] = data->DesiredSize[1] = ANCHOR_MAX(data->DesiredSize[0],
                                                               data->DesiredSize[1]);
    }
    static void Step(AnchorSizeCallbackData *data)
    {
      float step = (float)(int)(intptr_t)data->UserData;
      data->DesiredSize = GfVec2f((int)(data->DesiredSize[0] / step + 0.5f) * step,
                                  (int)(data->DesiredSize[1] / step + 0.5f) * step);
    }
  };

  const char *test_desc[] = {
    "Resize vertical only",
    "Resize horizontal only",
    "Width > 100, Height > 100",
    "Width 400-500",
    "Height 400-500",
    "Custom: Always Square",
    "Custom: Fixed Steps (100)",
  };

  static bool auto_resize = false;
  static int type = 0;
  static int display_lines = 10;
  if (type == 0)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(-1, 0), GfVec2f(-1, FLT_MAX));  // Vertical only
  if (type == 1)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(0, -1), GfVec2f(FLT_MAX, -1));  // Horizontal only
  if (type == 2)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(100, 100),
                                         GfVec2f(FLT_MAX, FLT_MAX));  // Width > 100, Height > 100
  if (type == 3)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(400, -1), GfVec2f(500, -1));  // Width 400-500
  if (type == 4)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(-1, 400), GfVec2f(-1, 500));  // Height 400-500
  if (type == 5)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(0, 0),
                                         GfVec2f(FLT_MAX, FLT_MAX),
                                         CustomConstraints::Square);  // Always Square
  if (type == 6)
    ANCHOR::SetNextWindowSizeConstraints(GfVec2f(0, 0),
                                         GfVec2f(FLT_MAX, FLT_MAX),
                                         CustomConstraints::Step,
                                         (void *)(intptr_t)100);  // Fixed Step

  AnchorWindowFlags flags = auto_resize ? AnchorWindowFlags_AlwaysAutoResize : 0;
  if (ANCHOR::Begin("Example: Constrained Resize", p_open, flags)) {
    if (ANCHOR::Button("200x200")) {
      ANCHOR::SetWindowSize(GfVec2f(200, 200));
    }
    ANCHOR::SameLine();
    if (ANCHOR::Button("500x500")) {
      ANCHOR::SetWindowSize(GfVec2f(500, 500));
    }
    ANCHOR::SameLine();
    if (ANCHOR::Button("800x200")) {
      ANCHOR::SetWindowSize(GfVec2f(800, 200));
    }
    ANCHOR::SetNextItemWidth(200);
    ANCHOR::Combo("Constraint", &type, test_desc, ANCHOR_ARRAYSIZE(test_desc));
    ANCHOR::SetNextItemWidth(200);
    ANCHOR::DragInt("Lines", &display_lines, 0.2f, 1, 100);
    ANCHOR::Checkbox("Auto-resize", &auto_resize);
    for (int i = 0; i < display_lines; i++)
      ANCHOR::Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
  }
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool *p_open)
{
  const float PAD = 10.0f;
  static int corner = 0;
  AnchorIO &io = ANCHOR::GetIO();
  AnchorWindowFlags window_flags = AnchorWindowFlags_NoDecoration |
                                   AnchorWindowFlags_AlwaysAutoResize |
                                   AnchorWindowFlags_NoSavedSettings |
                                   AnchorWindowFlags_NoFocusOnAppearing | AnchorWindowFlags_NoNav;
  if (corner != -1) {
    const AnchorViewport *viewport = ANCHOR::GetMainViewport();
    GfVec2f work_pos = viewport->WorkPos;  // Use work area to avoid menu-bar/task-bar, if any!
    GfVec2f work_size = viewport->WorkSize;
    GfVec2f window_pos, window_pos_pivot;
    window_pos[0] = (corner & 1) ? (work_pos[0] + work_size[0] - PAD) : (work_pos[0] + PAD);
    window_pos[1] = (corner & 2) ? (work_pos[1] + work_size[1] - PAD) : (work_pos[1] + PAD);
    window_pos_pivot[0] = (corner & 1) ? 1.0f : 0.0f;
    window_pos_pivot[1] = (corner & 2) ? 1.0f : 0.0f;
    ANCHOR::SetNextWindowPos(window_pos, AnchorCond_Always, window_pos_pivot);
    window_flags |= AnchorWindowFlags_NoMove;
  }
  ANCHOR::SetNextWindowBgAlpha(0.35f);  // Transparent background
  if (ANCHOR::Begin("Example: Simple overlay", p_open, window_flags)) {
    ANCHOR::Text(
      "Simple overlay\n"
      "in the corner of the screen.\n"
      "(right-click to change position)");
    ANCHOR::Separator();
    if (ANCHOR::IsMousePosValid())
      ANCHOR::Text("Mouse Position: (%.1f,%.1f)", io.MousePos[0], io.MousePos[1]);
    else
      ANCHOR::Text("Mouse Position: <invalid>");
    if (ANCHOR::BeginPopupContextWindow()) {
      if (ANCHOR::MenuItem("Custom", NULL, corner == -1))
        corner = -1;
      if (ANCHOR::MenuItem("Top-left", NULL, corner == 0))
        corner = 0;
      if (ANCHOR::MenuItem("Top-right", NULL, corner == 1))
        corner = 1;
      if (ANCHOR::MenuItem("Bottom-left", NULL, corner == 2))
        corner = 2;
      if (ANCHOR::MenuItem("Bottom-right", NULL, corner == 3))
        corner = 3;
      if (p_open && ANCHOR::MenuItem("Close"))
        *p_open = false;
      ANCHOR::EndPopup();
    }
  }
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Fullscreen window / ShowExampleAppFullscreen()
//-----------------------------------------------------------------------------

// Demonstrate creating a window covering the entire screen/viewport
static void ShowExampleAppFullscreen(bool *p_open)
{
  static bool use_work_area = true;
  static AnchorWindowFlags flags = AnchorWindowFlags_NoDecoration | AnchorWindowFlags_NoMove |
                                   AnchorWindowFlags_NoResize | AnchorWindowFlags_NoSavedSettings;

  // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars
  // etc.) Based on your use case you may want one of the other.
  const AnchorViewport *viewport = ANCHOR::GetMainViewport();
  ANCHOR::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
  ANCHOR::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

  if (ANCHOR::Begin("Example: Fullscreen window", p_open, flags)) {
    ANCHOR::Checkbox("Use work area instead of main area", &use_work_area);
    ANCHOR::SameLine();
    HelpMarker(
      "Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the "
      "main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the "
      "difference.");

    ANCHOR::CheckboxFlags("AnchorWindowFlags_NoBackground",
                          &flags,
                          AnchorWindowFlags_NoBackground);
    ANCHOR::CheckboxFlags("AnchorWindowFlags_NoDecoration",
                          &flags,
                          AnchorWindowFlags_NoDecoration);
    ANCHOR::Indent();
    ANCHOR::CheckboxFlags("AnchorWindowFlags_NoTitleBar", &flags, AnchorWindowFlags_NoTitleBar);
    ANCHOR::CheckboxFlags("AnchorWindowFlags_NoCollapse", &flags, AnchorWindowFlags_NoCollapse);
    ANCHOR::CheckboxFlags("AnchorWindowFlags_NoScrollbar", &flags, AnchorWindowFlags_NoScrollbar);
    ANCHOR::Unindent();

    if (p_open && ANCHOR::Button("Close this window"))
      *p_open = false;
  }
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate using "##" and "###" in identifiers to manipulate ID generation.
// This apply to all regular items as well.
// Read FAQ section "How can I have multiple widgets with the same label?" for details.
static void ShowExampleAppWindowTitles(bool *)
{
  const AnchorViewport *viewport = ANCHOR::GetMainViewport();
  const GfVec2f base_pos = viewport->Pos;

  // By default, Windows are uniquely identified by their title.
  // You can use the "##" and "###" markers to manipulate the display/ID.

  // Using "##" to display same title but have unique identifier.
  ANCHOR::SetNextWindowPos(GfVec2f(base_pos[0] + 100, base_pos[1] + 100), AnchorCond_FirstUseEver);
  ANCHOR::Begin("Same title as another window##1");
  ANCHOR::Text(
    "This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
  ANCHOR::End();

  ANCHOR::SetNextWindowPos(GfVec2f(base_pos[0] + 100, base_pos[1] + 200), AnchorCond_FirstUseEver);
  ANCHOR::Begin("Same title as another window##2");
  ANCHOR::Text(
    "This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
  ANCHOR::End();

  // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
  char buf[128];
  sprintf(buf,
          "Animated title %c %d###AnimatedTitle",
          "|/-\\"[(int)(ANCHOR::GetTime() / 0.25f) & 3],
          ANCHOR::GetFrameCount());
  ANCHOR::SetNextWindowPos(GfVec2f(base_pos[0] + 100, base_pos[1] + 300), AnchorCond_FirstUseEver);
  ANCHOR::Begin(buf);
  ANCHOR::Text("This window has a changing title.");
  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using AnchorDrawList API /
// ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Demonstrate using the low-level AnchorDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool *p_open)
{
  if (!ANCHOR::Begin("Example: Custom rendering", p_open)) {
    ANCHOR::End();
    return;
  }

  // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types
  // and benefit of overloaded operators, etc. Define IM_VEC2_CLASS_EXTRA in ANCHOR_config.h to
  // create implicit conversions between your types and GfVec2f/GfVec4f. ANCHOR defines overloaded
  // operators but they are internal to ANCHOR.cpp and not exposed outside (to avoid messing with
  // your types) In this example we are not using the maths operators!

  if (ANCHOR::BeginTabBar("##TabBar")) {
    if (ANCHOR::BeginTabItem("Primitives")) {
      ANCHOR::PushItemWidth(-ANCHOR::GetFontSize() * 15);
      AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();

      // Draw gradients
      // (note that those are currently exacerbating our sRGB/Linear issues)
      // Calling ANCHOR::GetColorU32() multiplies the given colors by the current Style Alpha, but
      // you may pass the ANCHOR_COL32() directly as well..
      ANCHOR::Text("Gradients");
      GfVec2f gradient_size = GfVec2f(ANCHOR::CalcItemWidth(), ANCHOR::GetFrameHeight());
      {
        GfVec2f p0 = ANCHOR::GetCursorScreenPos();
        GfVec2f p1 = GfVec2f(p0[0] + gradient_size[0], p0[1] + gradient_size[1]);
        AnchorU32 col_a = ANCHOR::GetColorU32(ANCHOR_COL32(0, 0, 0, 255));
        AnchorU32 col_b = ANCHOR::GetColorU32(ANCHOR_COL32(255, 255, 255, 255));
        draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
        ANCHOR::InvisibleButton("##gradient1", gradient_size);
      }
      {
        GfVec2f p0 = ANCHOR::GetCursorScreenPos();
        GfVec2f p1 = GfVec2f(p0[0] + gradient_size[0], p0[1] + gradient_size[1]);
        AnchorU32 col_a = ANCHOR::GetColorU32(ANCHOR_COL32(0, 255, 0, 255));
        AnchorU32 col_b = ANCHOR::GetColorU32(ANCHOR_COL32(255, 0, 0, 255));
        draw_list->AddRectFilledMultiColor(p0, p1, col_a, col_b, col_b, col_a);
        ANCHOR::InvisibleButton("##gradient2", gradient_size);
      }

      // Draw a bunch of primitives
      ANCHOR::Text("All primitives");
      static float sz = 36.0f;
      static float thickness = 3.0f;
      static int ngon_sides = 6;
      static bool circle_segments_override = false;
      static int circle_segments_override_v = 12;
      static bool curve_segments_override = false;
      static int curve_segments_override_v = 8;
      static GfVec4f colf = GfVec4f(1.0f, 1.0f, 0.4f, 1.0f);
      ANCHOR::DragFloat("Size", &sz, 0.2f, 2.0f, 100.0f, "%.0f");
      ANCHOR::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
      ANCHOR::SliderInt("N-gon sides", &ngon_sides, 3, 12);
      ANCHOR::Checkbox("##circlesegmentoverride", &circle_segments_override);
      ANCHOR::SameLine(0.0f, ANCHOR::GetStyle().ItemInnerSpacing[0]);
      circle_segments_override |= ANCHOR::SliderInt("Circle segments override",
                                                    &circle_segments_override_v,
                                                    3,
                                                    40);
      ANCHOR::Checkbox("##curvessegmentoverride", &curve_segments_override);
      ANCHOR::SameLine(0.0f, ANCHOR::GetStyle().ItemInnerSpacing[0]);
      curve_segments_override |= ANCHOR::SliderInt("Curves segments override",
                                                   &curve_segments_override_v,
                                                   3,
                                                   40);
      ANCHOR::ColorEdit4("Color", &colf[0]);

      const GfVec2f p = ANCHOR::GetCursorScreenPos();
      const AnchorU32 col = AnchorColor(colf);
      const float spacing = 10.0f;
      const AnchorDrawFlags corners_tl_br = AnchorDrawFlags_RoundCornersTopLeft |
                                            AnchorDrawFlags_RoundCornersBottomRight;
      const float rounding = sz / 5.0f;
      const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
      const int curve_segments = curve_segments_override ? curve_segments_override_v : 0;
      float x = p[0] + 4.0f;
      float y = p[1] + 4.0f;
      for (int n = 0; n < 2; n++) {
        // First line uses a thickness of 1.0f, second line uses the configurable thickness
        float th = (n == 0) ? 1.0f : thickness;
        draw_list->AddNgon(GfVec2f(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides, th);
        x += sz + spacing;  // N-gon
        draw_list->AddCircle(GfVec2f(x + sz * 0.5f, y + sz * 0.5f),
                             sz * 0.5f,
                             col,
                             circle_segments,
                             th);
        x += sz + spacing;  // Circle
        draw_list
          ->AddRect(GfVec2f(x, y), GfVec2f(x + sz, y + sz), col, 0.0f, AnchorDrawFlags_None, th);
        x += sz + spacing;  // Square
        draw_list->AddRect(GfVec2f(x, y),
                           GfVec2f(x + sz, y + sz),
                           col,
                           rounding,
                           AnchorDrawFlags_None,
                           th);
        x += sz + spacing;  // Square with all rounded corners
        draw_list
          ->AddRect(GfVec2f(x, y), GfVec2f(x + sz, y + sz), col, rounding, corners_tl_br, th);
        x += sz + spacing;  // Square with two rounded corners
        draw_list->AddTriangle(GfVec2f(x + sz * 0.5f, y),
                               GfVec2f(x + sz, y + sz - 0.5f),
                               GfVec2f(x, y + sz - 0.5f),
                               col,
                               th);
        x += sz + spacing;  // Triangle
        // draw_list->AddTriangle(GfVec2f(x+sz*0.2f,y), GfVec2f(x, y+sz-0.5f), GfVec2f(x+sz*0.4f,
        // y+sz-0.5f), col, th);x+= sz*0.4f + spacing; // Thin triangle
        draw_list->AddLine(GfVec2f(x, y), GfVec2f(x + sz, y), col, th);
        x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
        draw_list->AddLine(GfVec2f(x, y), GfVec2f(x, y + sz), col, th);
        x += spacing;  // Vertical line (note: drawing a filled rectangle will be faster!)
        draw_list->AddLine(GfVec2f(x, y), GfVec2f(x + sz, y + sz), col, th);
        x += sz + spacing;  // Diagonal line

        // Quadratic Bezier Curve (3 control points)
        GfVec2f cp3[3] = {GfVec2f(x, y + sz * 0.6f),
                          GfVec2f(x + sz * 0.5f, y - sz * 0.4f),
                          GfVec2f(x + sz, y + sz)};
        draw_list->AddBezierQuadratic(cp3[0], cp3[1], cp3[2], col, th, curve_segments);
        x += sz + spacing;

        // Cubic Bezier Curve (4 control points)
        GfVec2f cp4[4] = {GfVec2f(x, y),
                          GfVec2f(x + sz * 1.3f, y + sz * 0.3f),
                          GfVec2f(x + sz - sz * 1.3f, y + sz - sz * 0.3f),
                          GfVec2f(x + sz, y + sz)};
        draw_list->AddBezierCubic(cp4[0], cp4[1], cp4[2], cp4[3], col, th, curve_segments);

        x = p[0] + 4;
        y += sz + spacing;
      }
      draw_list->AddNgonFilled(GfVec2f(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides);
      x += sz + spacing;  // N-gon
      draw_list->AddCircleFilled(GfVec2f(x + sz * 0.5f, y + sz * 0.5f),
                                 sz * 0.5f,
                                 col,
                                 circle_segments);
      x += sz + spacing;  // Circle
      draw_list->AddRectFilled(GfVec2f(x, y), GfVec2f(x + sz, y + sz), col);
      x += sz + spacing;  // Square
      draw_list->AddRectFilled(GfVec2f(x, y), GfVec2f(x + sz, y + sz), col, 10.0f);
      x += sz + spacing;  // Square with all rounded corners
      draw_list->AddRectFilled(GfVec2f(x, y), GfVec2f(x + sz, y + sz), col, 10.0f, corners_tl_br);
      x += sz + spacing;  // Square with two rounded corners
      draw_list->AddTriangleFilled(GfVec2f(x + sz * 0.5f, y),
                                   GfVec2f(x + sz, y + sz - 0.5f),
                                   GfVec2f(x, y + sz - 0.5f),
                                   col);
      x += sz + spacing;  // Triangle
      // draw_list->AddTriangleFilled(GfVec2f(x+sz*0.2f,y), GfVec2f(x, y+sz-0.5f),
      // GfVec2f(x+sz*0.4f, y+sz-0.5f), col); x += sz*0.4f + spacing; // Thin triangle
      draw_list->AddRectFilled(GfVec2f(x, y), GfVec2f(x + sz, y + thickness), col);
      x += sz +
           spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
      draw_list->AddRectFilled(GfVec2f(x, y), GfVec2f(x + thickness, y + sz), col);
      x += spacing *
           2.0f;  // Vertical line (faster than AddLine, but only handle integer thickness)
      draw_list->AddRectFilled(GfVec2f(x, y), GfVec2f(x + 1, y + 1), col);
      x += sz;  // Pixel (faster than AddLine)
      draw_list->AddRectFilledMultiColor(GfVec2f(x, y),
                                         GfVec2f(x + sz, y + sz),
                                         ANCHOR_COL32(0, 0, 0, 255),
                                         ANCHOR_COL32(255, 0, 0, 255),
                                         ANCHOR_COL32(255, 255, 0, 255),
                                         ANCHOR_COL32(0, 255, 0, 255));

      ANCHOR::Dummy(GfVec2f((sz + spacing) * 10.2f, (sz + spacing) * 3.0f));
      ANCHOR::PopItemWidth();
      ANCHOR::EndTabItem();
    }

    if (ANCHOR::BeginTabItem("Canvas")) {
      static AnchorVector<GfVec2f> points;
      static GfVec2f scrolling(0.0f, 0.0f);
      static bool opt_enable_grid = true;
      static bool opt_enable_context_menu = true;
      static bool adding_line = false;

      ANCHOR::Checkbox("Enable grid", &opt_enable_grid);
      ANCHOR::Checkbox("Enable context menu", &opt_enable_context_menu);
      ANCHOR::Text(
        "Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");

      // Typically you would use a BeginChild()/EndChild() pair to benefit from a clipping region +
      // own scrolling. Here we demonstrate that this can be replaced by simple offsetting + custom
      // drawing + PushClipRect/PopClipRect() calls. To use a child window instead we could use,
      // e.g:
      //      ANCHOR::PushStyleVar(AnchorStyleVar_WindowPadding, GfVec2f(0, 0));      // Disable
      //      padding ANCHOR::PushStyleColor(AnchorCol_ChildBg, ANCHOR_COL32(50, 50, 50, 255)); //
      //      Set a background color ANCHOR::BeginChild("canvas", GfVec2f(0.0f, 0.0f), true,
      //      AnchorWindowFlags_NoMove); ANCHOR::PopStyleColor(); ANCHOR::PopStyleVar();
      //      [...]
      //      ANCHOR::EndChild();

      // Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2)
      // allows us to use IsItemHovered()/IsItemActive()
      GfVec2f canvas_p0 =
        ANCHOR::GetCursorScreenPos();  // AnchorDrawList API uses screen coordinates!
      GfVec2f canvas_sz = ANCHOR::GetContentRegionAvail();  // Resize canvas to what's available
      if (canvas_sz[0] < 50.0f)
        canvas_sz[0] = 50.0f;
      if (canvas_sz[1] < 50.0f)
        canvas_sz[1] = 50.0f;
      GfVec2f canvas_p1 = GfVec2f(canvas_p0[0] + canvas_sz[0], canvas_p0[1] + canvas_sz[1]);

      // Draw border and background color
      AnchorIO &io = ANCHOR::GetIO();
      AnchorDrawList *draw_list = ANCHOR::GetWindowDrawList();
      draw_list->AddRectFilled(canvas_p0, canvas_p1, ANCHOR_COL32(50, 50, 50, 255));
      draw_list->AddRect(canvas_p0, canvas_p1, ANCHOR_COL32(255, 255, 255, 255));

      // This will catch our interactions
      ANCHOR::InvisibleButton("canvas",
                              canvas_sz,
                              AnchorButtonFlags_MouseButtonLeft |
                                AnchorButtonFlags_MouseButtonRight);
      const bool is_hovered = ANCHOR::IsItemHovered();  // Hovered
      const bool is_active = ANCHOR::IsItemActive();    // Held
      const GfVec2f origin(canvas_p0[0] + scrolling[0],
                           canvas_p0[1] + scrolling[1]);  // Lock scrolled origin
      const GfVec2f mouse_pos_in_canvas(io.MousePos[0] - origin[0], io.MousePos[1] - origin[1]);

      // Add first and second point
      if (is_hovered && !adding_line && ANCHOR::IsMouseClicked(AnchorMouseButton_Left)) {
        points.push_back(mouse_pos_in_canvas);
        points.push_back(mouse_pos_in_canvas);
        adding_line = true;
      }
      if (adding_line) {
        points.back() = mouse_pos_in_canvas;
        if (!ANCHOR::IsMouseDown(AnchorMouseButton_Left))
          adding_line = false;
      }

      // Pan (we use a zero mouse threshold when there's no context menu)
      // You may decide to make that threshold dynamic based on whether the mouse is hovering
      // something etc.
      const float mouse_threshold_for_pan = opt_enable_context_menu ? -1.0f : 0.0f;
      if (is_active && ANCHOR::IsMouseDragging(AnchorMouseButton_Right, mouse_threshold_for_pan)) {
        scrolling[0] += io.MouseDelta[0];
        scrolling[1] += io.MouseDelta[1];
      }

      // Context menu (under default mouse threshold)
      GfVec2f drag_delta = ANCHOR::GetMouseDragDelta(AnchorMouseButton_Right);
      if (opt_enable_context_menu && ANCHOR::IsMouseReleased(AnchorMouseButton_Right) &&
          drag_delta[0] == 0.0f && drag_delta[1] == 0.0f)
        ANCHOR::OpenPopupOnItemClick("context");
      if (ANCHOR::BeginPopup("context")) {
        if (adding_line)
          points.resize(points.size() - 2);
        adding_line = false;
        if (ANCHOR::MenuItem("Remove one", NULL, false, points.Size > 0)) {
          points.resize(points.size() - 2);
        }
        if (ANCHOR::MenuItem("Remove all", NULL, false, points.Size > 0)) {
          points.clear();
        }
        ANCHOR::EndPopup();
      }

      // Draw grid + all lines in the canvas
      draw_list->PushClipRect(canvas_p0, canvas_p1, true);
      if (opt_enable_grid) {
        const float GRID_STEP = 64.0f;
        for (float x = fmodf(scrolling[0], GRID_STEP); x < canvas_sz[0]; x += GRID_STEP)
          draw_list->AddLine(GfVec2f(canvas_p0[0] + x, canvas_p0[1]),
                             GfVec2f(canvas_p0[0] + x, canvas_p1[1]),
                             ANCHOR_COL32(200, 200, 200, 40));
        for (float y = fmodf(scrolling[1], GRID_STEP); y < canvas_sz[1]; y += GRID_STEP)
          draw_list->AddLine(GfVec2f(canvas_p0[0], canvas_p0[1] + y),
                             GfVec2f(canvas_p1[0], canvas_p0[1] + y),
                             ANCHOR_COL32(200, 200, 200, 40));
      }
      for (int n = 0; n < points.Size; n += 2)
        draw_list->AddLine(GfVec2f(origin[0] + points[n][0], origin[1] + points[n][1]),
                           GfVec2f(origin[0] + points[n + 1][0], origin[1] + points[n + 1][1]),
                           ANCHOR_COL32(255, 255, 0, 255),
                           2.0f);
      draw_list->PopClipRect();

      ANCHOR::EndTabItem();
    }

    if (ANCHOR::BeginTabItem("BG/FG draw lists")) {
      static bool draw_bg = true;
      static bool draw_fg = true;
      ANCHOR::Checkbox("Draw in Background draw list", &draw_bg);
      ANCHOR::SameLine();
      HelpMarker("The Background draw list will be rendered below every ANCHOR windows.");
      ANCHOR::Checkbox("Draw in Foreground draw list", &draw_fg);
      ANCHOR::SameLine();
      HelpMarker("The Foreground draw list will be rendered over every ANCHOR windows.");
      GfVec2f window_pos = ANCHOR::GetWindowPos();
      GfVec2f window_size = ANCHOR::GetWindowSize();
      GfVec2f window_center = GfVec2f(window_pos[0] + window_size[0] * 0.5f,
                                      window_pos[1] + window_size[1] * 0.5f);
      if (draw_bg)
        ANCHOR::GetBackgroundDrawList()->AddCircle(window_center,
                                                   window_size[0] * 0.6f,
                                                   ANCHOR_COL32(255, 0, 0, 200),
                                                   0,
                                                   10 + 4);
      if (draw_fg)
        ANCHOR::GetForegroundDrawList()->AddCircle(window_center,
                                                   window_size[1] * 0.6f,
                                                   ANCHOR_COL32(0, 255, 0, 200),
                                                   0,
                                                   10);
      ANCHOR::EndTabItem();
    }

    ANCHOR::EndTabBar();
  }

  ANCHOR::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
//-----------------------------------------------------------------------------

// Simplified structure to mimic a Document model
struct MyDocument
{
  const char *Name;  // Document title
  bool Open;  // Set when open (we keep an array of all available documents to simplify demo code!)
  bool OpenPrev;   // Copy of Open from last update.
  bool Dirty;      // Set when the document has been modified
  bool WantClose;  // Set when the document
  GfVec4f Color;   // An arbitrary variable associated to the document

  MyDocument(const char *name,
             bool open = true,
             const GfVec4f &color = GfVec4f(1.0f, 1.0f, 1.0f, 1.0f))
  {
    Name = name;
    Open = OpenPrev = open;
    Dirty = false;
    WantClose = false;
    Color = color;
  }
  void DoOpen()
  {
    Open = true;
  }
  void DoQueueClose()
  {
    WantClose = true;
  }
  void DoForceClose()
  {
    Open = false;
    Dirty = false;
  }
  void DoSave()
  {
    Dirty = false;
  }

  // Display placeholder contents for the Document
  static void DisplayContents(MyDocument *doc)
  {
    ANCHOR::PushID(doc);
    ANCHOR::Text("Document \"%s\"", doc->Name);
    ANCHOR::PushStyleColor(AnchorCol_Text, doc->Color);
    ANCHOR::TextWrapped(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
      "incididunt ut labore et dolore magna aliqua.");
    ANCHOR::PopStyleColor();
    if (ANCHOR::Button("Modify", GfVec2f(100, 0)))
      doc->Dirty = true;
    ANCHOR::SameLine();
    if (ANCHOR::Button("Save", GfVec2f(100, 0)))
      doc->DoSave();
    ANCHOR::ColorEdit3(
      "color",
      &doc->Color[0]);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
    ANCHOR::PopID();
  }

  // Display context menu for the Document
  static void DisplayContextMenu(MyDocument *doc)
  {
    if (!ANCHOR::BeginPopupContextItem())
      return;

    char buf[256];
    sprintf(buf, "Save %s", doc->Name);
    if (ANCHOR::MenuItem(buf, "CTRL+S", false, doc->Open))
      doc->DoSave();
    if (ANCHOR::MenuItem("Close", "CTRL+W", false, doc->Open))
      doc->DoQueueClose();
    ANCHOR::EndPopup();
  }
};

struct ExampleAppDocuments
{
  AnchorVector<MyDocument> Documents;

  ExampleAppDocuments()
  {
    Documents.push_back(MyDocument("Lettuce", true, GfVec4f(0.4f, 0.8f, 0.4f, 1.0f)));
    Documents.push_back(MyDocument("Eggplant", true, GfVec4f(0.8f, 0.5f, 1.0f, 1.0f)));
    Documents.push_back(MyDocument("Carrot", true, GfVec4f(1.0f, 0.8f, 0.5f, 1.0f)));
    Documents.push_back(MyDocument("Tomato", false, GfVec4f(1.0f, 0.3f, 0.4f, 1.0f)));
    Documents.push_back(MyDocument("A Rather Long Title", false));
    Documents.push_back(MyDocument("Some Document", false));
  }
};

// [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab
// interface. If a tab has been closed programmatically (aka closed from another source such as the
// Checkbox() in the demo, as opposed to clicking on the regular tab closing button) and stops
// being submitted, it will take a frame for the tab bar to notice its absence. During this frame
// there will be a gap in the tab bar, and if the tab that has disappeared was the selected one,
// the tab bar will report no selected tab during the frame. This will effectively give the
// impression of a flicker for one frame. We call SetTabItemClosed() to manually notify the Tab Bar
// or Docking system of removed tabs to avoid this glitch. Note that this completely optional, and
// only affect tab bars with the AnchorTabBarFlags_Reorderable flag.
static void NotifyOfDocumentsClosedElsewhere(ExampleAppDocuments &app)
{
  for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
    MyDocument *doc = &app.Documents[doc_n];
    if (!doc->Open && doc->OpenPrev)
      ANCHOR::SetTabItemClosed(doc->Name);
    doc->OpenPrev = doc->Open;
  }
}

void ShowExampleAppDocuments(bool *p_open)
{
  static ExampleAppDocuments app;

  // Options
  static bool opt_reorderable = true;
  static AnchorTabBarFlags opt_fitting_flags = AnchorTabBarFlags_FittingPolicyDefault_;

  bool window_contents_visible = ANCHOR::Begin("Example: Documents",
                                               p_open,
                                               AnchorWindowFlags_MenuBar);
  if (!window_contents_visible) {
    ANCHOR::End();
    return;
  }

  // Menu
  if (ANCHOR::BeginMenuBar()) {
    if (ANCHOR::BeginMenu("File")) {
      int open_count = 0;
      for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
        open_count += app.Documents[doc_n].Open ? 1 : 0;

      if (ANCHOR::BeginMenu("Open", open_count < app.Documents.Size)) {
        for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
          MyDocument *doc = &app.Documents[doc_n];
          if (!doc->Open)
            if (ANCHOR::MenuItem(doc->Name))
              doc->DoOpen();
        }
        ANCHOR::EndMenu();
      }
      if (ANCHOR::MenuItem("Close All Documents", NULL, false, open_count > 0))
        for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
          app.Documents[doc_n].DoQueueClose();
      if (ANCHOR::MenuItem("Exit", "Alt+F4")) {
      }
      ANCHOR::EndMenu();
    }
    ANCHOR::EndMenuBar();
  }

  // [Debug] List documents with one checkbox for each
  for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
    MyDocument *doc = &app.Documents[doc_n];
    if (doc_n > 0)
      ANCHOR::SameLine();
    ANCHOR::PushID(doc);
    if (ANCHOR::Checkbox(doc->Name, &doc->Open))
      if (!doc->Open)
        doc->DoForceClose();
    ANCHOR::PopID();
  }

  ANCHOR::Separator();

  // Submit Tab Bar and Tabs
  {
    AnchorTabBarFlags tab_bar_flags = (opt_fitting_flags) |
                                      (opt_reorderable ? AnchorTabBarFlags_Reorderable : 0);
    if (ANCHOR::BeginTabBar("##tabs", tab_bar_flags)) {
      if (opt_reorderable)
        NotifyOfDocumentsClosedElsewhere(app);

      // [DEBUG] Stress tests
      // if ((ANCHOR::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG]
      // Automatically show/hide a tab. Test various interactions e.g. dragging with this on. if
      // (ANCHOR::GetIO().KeyCtrl) ANCHOR::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test
      // SetTabItemSelected(), probably not very useful as-is anyway..

      // Submit Tabs
      for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
        MyDocument *doc = &app.Documents[doc_n];
        if (!doc->Open)
          continue;

        AnchorTabItemFlags tab_flags = (doc->Dirty ? AnchorTabItemFlags_UnsavedDocument : 0);
        bool visible = ANCHOR::BeginTabItem(doc->Name, &doc->Open, tab_flags);

        // Cancel attempt to close when unsaved add to save queue so we can display a popup.
        if (!doc->Open && doc->Dirty) {
          doc->Open = true;
          doc->DoQueueClose();
        }

        MyDocument::DisplayContextMenu(doc);
        if (visible) {
          MyDocument::DisplayContents(doc);
          ANCHOR::EndTabItem();
        }
      }

      ANCHOR::EndTabBar();
    }
  }

  // Update closing queue
  static AnchorVector<MyDocument *> close_queue;
  if (close_queue.empty()) {
    // Close queue is locked once we started a popup
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++) {
      MyDocument *doc = &app.Documents[doc_n];
      if (doc->WantClose) {
        doc->WantClose = false;
        close_queue.push_back(doc);
      }
    }
  }

  // Display closing confirmation UI
  if (!close_queue.empty()) {
    int close_queue_unsaved_documents = 0;
    for (int n = 0; n < close_queue.Size; n++)
      if (close_queue[n]->Dirty)
        close_queue_unsaved_documents++;

    if (close_queue_unsaved_documents == 0) {
      // Close documents when all are unsaved
      for (int n = 0; n < close_queue.Size; n++)
        close_queue[n]->DoForceClose();
      close_queue.clear();
    } else {
      if (!ANCHOR::IsPopupOpen("Save?"))
        ANCHOR::OpenPopup("Save?");
      if (ANCHOR::BeginPopupModal("Save?", NULL, AnchorWindowFlags_AlwaysAutoResize)) {
        ANCHOR::Text("Save change to the following items?");
        float item_height = ANCHOR::GetTextLineHeightWithSpacing();
        if (ANCHOR::BeginChildFrame(ANCHOR::GetID("frame"),
                                    GfVec2f(-FLT_MIN, 6.25f * item_height))) {
          for (int n = 0; n < close_queue.Size; n++)
            if (close_queue[n]->Dirty)
              ANCHOR::Text("%s", close_queue[n]->Name);
          ANCHOR::EndChildFrame();
        }

        GfVec2f button_size(ANCHOR::GetFontSize() * 7.0f, 0.0f);
        if (ANCHOR::Button("Yes", button_size)) {
          for (int n = 0; n < close_queue.Size; n++) {
            if (close_queue[n]->Dirty)
              close_queue[n]->DoSave();
            close_queue[n]->DoForceClose();
          }
          close_queue.clear();
          ANCHOR::CloseCurrentPopup();
        }
        ANCHOR::SameLine();
        if (ANCHOR::Button("No", button_size)) {
          for (int n = 0; n < close_queue.Size; n++)
            close_queue[n]->DoForceClose();
          close_queue.clear();
          ANCHOR::CloseCurrentPopup();
        }
        ANCHOR::SameLine();
        if (ANCHOR::Button("Cancel", button_size)) {
          close_queue.clear();
          ANCHOR::CloseCurrentPopup();
        }
        ANCHOR::EndPopup();
      }
    }
  }

  ANCHOR::End();
}
