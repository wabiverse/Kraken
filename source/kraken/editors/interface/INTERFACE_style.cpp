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

#include "USD_area.h"
#include "USD_operator.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_wm_types.h"

#include "WM_draw.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"

#include "interface_intern.h"

KRAKEN_NAMESPACE_BEGIN

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

static uiStyle *ui_style_new(const std::vector<uiStyle *> &styles, const char *name, short uifont_id)
{
  uiStyle *style = new uiStyle();

  styles.push_back(style);

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

const uiStyle *UI_style_get(void)
{
#if 0
  uiStyle *style = nullptr;
  /* offset is two struct uiStyle pointers */
  style = KLI_findstring(&U.uistyles, "Unifont Style", sizeof(style) * 2);
  return (style != nullptr) ? style : U.uistyles.first;
#else
  return static_cast<const uiStyle *>(UI_STYLES_LIST.back());
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

KRAKEN_NAMESPACE_END