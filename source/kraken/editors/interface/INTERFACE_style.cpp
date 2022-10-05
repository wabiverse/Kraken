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
 * Editors.
 * Tools for Artists.
 */

#include <vector>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "MEM_guardedalloc.h"

#include "USD_wm_types.h"
#include "USD_operator.h"
#include "USD_area.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_userdef_types.h"

#include "KLI_listbase.h"
#include "KLI_rect.h"
#include "KLI_string.h"
#include "KLI_utildefines.h"

#include "KRF_api.h"

#include "WM_draw.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_global.h"

#include "interface_intern.h"

/* style + theme + layout-engine = UI */

/**
 * This is a complete set of layout rules, the 'state' of the Layout
 * Engine. Multiple styles are possible, defined via C or Python. Styles
 * get a name, and will typically get activated per region type, like
 * "Header", or "Listview" or "Toolbar". Properties of Style definitions
 * are:
 *
 * - default column properties, internal spacing, aligning, min/max width
 * - button alignment rules (for groups)
 * - label placement rules
 * - internal labeling or external labeling default
 * - default minimum widths for buttons/labels (in amount of characters)
 * - font types, styles and relative sizes for Panel titles, labels, etc.
 */

/* ********************************************** */

static uiStyle *ui_style_new(ListBase *styles, const char *name, short uifont_id)
{
  uiStyle *style = MEM_cnew<uiStyle>(__func__);

  KLI_addtail(styles, style);
  KLI_strncpy(style->name, name, MAX_STYLE_NAME);

  style->panelzoom = 1.0; /* unused */

  style->paneltitle.uifont_id = uifont_id;
  style->paneltitle.points = UI_DEFAULT_TITLE_POINTS;
  style->paneltitle.shadow = 3;
  style->paneltitle.shadx = 0;
  style->paneltitle.shady = -1;
  style->paneltitle.shadowalpha = 0.5f;
  style->paneltitle.shadowcolor = 0.0f;

  style->grouplabel.uifont_id = uifont_id;
  style->grouplabel.points = UI_DEFAULT_TITLE_POINTS;
  style->grouplabel.shadow = 3;
  style->grouplabel.shadx = 0;
  style->grouplabel.shady = -1;
  style->grouplabel.shadowalpha = 0.5f;
  style->grouplabel.shadowcolor = 0.0f;

  style->widgetlabel.uifont_id = uifont_id;
  style->widgetlabel.points = UI_DEFAULT_TEXT_POINTS;
  style->widgetlabel.shadow = 3;
  style->widgetlabel.shadx = 0;
  style->widgetlabel.shady = -1;
  style->widgetlabel.shadowalpha = 0.5f;
  style->widgetlabel.shadowcolor = 0.0f;

  style->widget.uifont_id = uifont_id;
  style->widget.points = UI_DEFAULT_TEXT_POINTS;
  style->widget.shadow = 1;
  style->widget.shady = -1;
  style->widget.shadowalpha = 0.5f;
  style->widget.shadowcolor = 0.0f;

  style->columnspace = 8;
  style->templatespace = 5;
  style->boxspace = 5;
  style->buttonspacex = 8;
  style->buttonspacey = 2;
  style->panelspace = 8;
  style->panelouter = 4;

  return style;
}

static uiFont *uifont_to_krfont(int id)
{
  uiFont *font = static_cast<uiFont *>(U.uifonts.first);

  for (; font; font = font->next) {
    if (font->uifont_id == id) {
      return font;
    }
  }
  return static_cast<uiFont *>(U.uifonts.first);
}

const uiStyle *UI_style_get(void)
{
#if 0
  uiStyle *style = nullptr;
  /* offset is two struct uiStyle pointers */
  style = KLI_findstring(&U.uistyles, "Unifont Style", sizeof(style) * 2);
  return (style != nullptr) ? style : U.uistyles.first;
#else
  return static_cast<const uiStyle *>(U.uistyles.first);
#endif
}

const uiStyle *UI_style_get_dpi(void)
{
  const uiStyle *style = UI_style_get();
  static uiStyle _style;

  _style = *style;

  _style.paneltitle.shadx = (short)(UI_DPI_FAC * _style.paneltitle.shadx);
  _style.paneltitle.shady = (short)(UI_DPI_FAC * _style.paneltitle.shady);
  _style.grouplabel.shadx = (short)(UI_DPI_FAC * _style.grouplabel.shadx);
  _style.grouplabel.shady = (short)(UI_DPI_FAC * _style.grouplabel.shady);
  _style.widgetlabel.shadx = (short)(UI_DPI_FAC * _style.widgetlabel.shadx);
  _style.widgetlabel.shady = (short)(UI_DPI_FAC * _style.widgetlabel.shady);

  _style.columnspace = (short)(UI_DPI_FAC * _style.columnspace);
  _style.templatespace = (short)(UI_DPI_FAC * _style.templatespace);
  _style.boxspace = (short)(UI_DPI_FAC * _style.boxspace);
  _style.buttonspacex = (short)(UI_DPI_FAC * _style.buttonspacex);
  _style.buttonspacey = (short)(UI_DPI_FAC * _style.buttonspacey);
  _style.panelspace = (short)(UI_DPI_FAC * _style.panelspace);
  _style.panelouter = (short)(UI_DPI_FAC * _style.panelouter);

  return &_style;
}

/* ************** init exit ************************ */

void uiStyleInit(void)
{
  const uiStyle *style = static_cast<uiStyle *>(U.uistyles.first);

  /* Recover from uninitialized DPI. */
  if (U.dpi == 0) {
    U.dpi = 72;
  }
  CLAMP(U.dpi, 48, 144);

  /* Needed so that custom fonts are always first. */
  KRF_unload_all();

  uiFont *font_first = static_cast<uiFont *>(U.uifonts.first);

  /* default builtin */
  if (font_first == nullptr) {
    font_first = MEM_cnew<uiFont>(__func__);
    KLI_addtail(&U.uifonts, font_first);
  }

  if (U.font_path_ui[0]) {
    KLI_strncpy(font_first->filepath, U.font_path_ui, sizeof(font_first->filepath));
    font_first->uifont_id = UIFONT_CUSTOM1;
  } else {
    KLI_strncpy(font_first->filepath, "default", sizeof(font_first->filepath));
    font_first->uifont_id = UIFONT_DEFAULT;
  }

  LISTBASE_FOREACH(uiFont *, font, &U.uifonts)
  {
    const bool unique = false;

    if (font->uifont_id == UIFONT_DEFAULT) {
      font->blf_id = KRF_load_default(unique);
    } else {
      font->blf_id = KRF_load(font->filepath);
      if (font->blf_id == -1) {
        font->blf_id = KRF_load_default(unique);
      }
    }

    KRF_default_set(font->blf_id);

    if (font->blf_id == -1) {
      if (G.debug & G_DEBUG) {
        printf("%s: error, no fonts available\n", __func__);
      }
    }
  }

  if (style == nullptr) {
    style = ui_style_new(&U.uistyles, "Default Style", UIFONT_DEFAULT);
  }

  KRF_cache_flush_set_fn(UI_widgetbase_draw_cache_flush);

  KRF_default_size(style->widgetlabel.points);

  /* XXX, this should be moved into a style,
   * but for now best only load the monospaced font once. */
  KLI_assert(krf_mono_font == -1);
  /* Use unique font loading to avoid thread safety issues with mono font
   * used for render metadata stamp in threads. */
  if (U.font_path_ui_mono[0]) {
    krf_mono_font = KRF_load_unique(U.font_path_ui_mono);
  }
  if (krf_mono_font == -1) {
    const bool unique = true;
    krf_mono_font = KRF_load_mono_default(unique);
  }

  /* Set default flags based on UI preferences (not render fonts) */
  {
    const int flag_disable = (KRF_MONOCHROME | KRF_HINTING_NONE | KRF_HINTING_SLIGHT |
                              KRF_HINTING_FULL);
    int flag_enable = 0;

    if (U.text_render & USER_TEXT_HINTING_NONE) {
      flag_enable |= KRF_HINTING_NONE;
    } else if (U.text_render & USER_TEXT_HINTING_SLIGHT) {
      flag_enable |= KRF_HINTING_SLIGHT;
    } else if (U.text_render & USER_TEXT_HINTING_FULL) {
      flag_enable |= KRF_HINTING_FULL;
    }

    if (U.text_render & USER_TEXT_DISABLE_AA) {
      flag_enable |= KRF_MONOCHROME;
    }

    LISTBASE_FOREACH(uiFont *, font, &U.uifonts)
    {
      if (font->blf_id != -1) {
        KRF_disable(font->blf_id, flag_disable);
        KRF_enable(font->blf_id, flag_enable);
      }
    }
    if (krf_mono_font != -1) {
      KRF_disable(krf_mono_font, flag_disable);
      KRF_enable(krf_mono_font, flag_enable);
    }
  }

  /**
   * Second for rendering else we get threading problems,
   *
   * @note This isn't good that the render font depends on the preferences,
   * keep for now though, since without this there is no way to display many unicode chars.
   */
  if (krf_mono_font_render == -1) {
    const bool unique = true;
    krf_mono_font_render = KRF_load_mono_default(unique);
  }

  /* Load the fallback fonts last. */
  KRF_load_font_stack();
}

int UI_fontstyle_string_width(const uiFontStyle *fs, const char *str)
{
  UI_fontstyle_set(fs);
  return int(KRF_width(fs->uifont_id, str, KRF_DRAW_STR_DUMMY_MAX));
}

int UI_fontstyle_string_width_with_block_aspect(const uiFontStyle *fs,
                                                const char *str,
                                                const float aspect)
{
  uiFontStyle fs_buf;
  if (aspect != 1.0f) {
    fs_buf = *fs;
    ui_fontscale(&fs_buf.points, aspect);
    fs = &fs_buf;
  }

  int width = UI_fontstyle_string_width(fs, str);

  if (aspect != 1.0f) {
    /* While in most cases rounding up isn't important, it can make a difference
     * with small fonts (3px or less), zooming out in the node-editor for e.g. */
    width = int(ceilf(width * aspect));
  }
  return width;
}

void UI_fontstyle_set(const uiFontStyle *fs)
{
  uiFont *font = uifont_to_krfont(fs->uifont_id);

  KRF_size(font->blf_id, fs->points * U.dpi_fac);
}