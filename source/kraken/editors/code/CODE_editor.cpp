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
 * Editors.
 * Tools for Artists.
 */

#include "ANCHOR_api.h"
#include "ED_code.h"

#include <wabi/base/tf/stringUtils.h>

#include "zep.h"
#include <mutex>

using namespace Zep;
WABI_NAMESPACE_BEGIN

void ED_code_run(bool *show)
{
  static edCode editor;

  static bool show_new_file = false;

  ANCHOR_WindowFlags flags;

  flags |= ANCHOR_WindowFlags_MenuBar;

  if (!ANCHOR::Begin("Code Editor", show, flags))
  {
    /** Early out if window is collapsed. */
    ANCHOR::End();
    return;
  }

  if (ANCHOR::BeginMenuBar())
  {
    if (ANCHOR::BeginMenu("File"))
    {
      ANCHOR::MenuItem("New File", NULL, &show_new_file);
      ANCHOR::EndMenu();
    }
    ANCHOR::EndMenuBar();
  }

  static std::once_flag init;
  std::call_once(init, []() {
    editor.reset(new ZepEditor_ANCHOR(ZepPath("file.txt"), NVec2f(1.0f)));
    editor->InitWithFileOrDir("edit_me.txt");
  });

  GfVec2f min = ANCHOR::GetWindowContentRegionMin();
  GfVec2f max = ANCHOR::GetWindowContentRegionMax();

  min[0] += ANCHOR::GetItemRectMin()[0];
  min[1] += ANCHOR::GetItemRectMin()[1];
  max[0] += ANCHOR::GetItemRectMax()[0];
  max[1] += ANCHOR::GetItemRectMax()[1];

  editor->HandleInput();
  editor->SetDisplayRegion(NVec2f(min[0], min[1]), NVec2f(max[0], max[0]));
  editor->Display();

  ANCHOR::End();
}

WABI_NAMESPACE_END