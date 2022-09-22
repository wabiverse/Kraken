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

#include "kraken/kraken.h"

#include "USD_operator.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_wm_types.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KKE_context.h"

#include "interface_intern.h"

KRAKEN_NAMESPACE_BEGIN

enum
{
  UI_ITEM_AUTO_FIXED_SIZE = 1 << 0,
  UI_ITEM_FIXED_SIZE = 1 << 1,

  UI_ITEM_BOX_ITEM = 1 << 2, /* The item is "inside" a box item */
  UI_ITEM_PROP_SEP = 1 << 3,
  UI_ITEM_INSIDE_PROP_SEP = 1 << 4,
  /* Show an icon button next to each property (to set keyframes, show status).
   * Enabled by default, depends on 'UI_ITEM_PROP_SEP'. */
  UI_ITEM_PROP_DECORATE = 1 << 5,
  UI_ITEM_PROP_DECORATE_NO_PAD = 1 << 6,
};

struct uiLayoutRoot
{
  int type;
  eWmOperatorContext opcontext;

  int emw, emh;
  int padding;

  uiMenuHandleFunc handlefunc;
  void *argv;

  const uiStyle *style;
  uiBlock *block;
  uiLayout *layout;
};

enum uiItemType
{
  ITEM_BUTTON,

  ITEM_LAYOUT_ROW,
  ITEM_LAYOUT_COLUMN,
  ITEM_LAYOUT_COLUMN_FLOW,
  ITEM_LAYOUT_ROW_FLOW,
  ITEM_LAYOUT_GRID_FLOW,
  ITEM_LAYOUT_BOX,
  ITEM_LAYOUT_ABSOLUTE,
  ITEM_LAYOUT_SPLIT,
  ITEM_LAYOUT_OVERLAP,
  ITEM_LAYOUT_RADIAL,

  ITEM_LAYOUT_ROOT
};

struct uiItem
{
  uiItemType type;
  int flag;
};

struct uiButtonItem
{
  uiItem item;
  uiBut *but;
};

struct uiLayout
{
  uiItem item;

  uiLayoutRoot *root;
  struct uiLayout *parent;
  std::vector<uiLayout *> items;

  char heading[UI_MAX_NAME_STR];

  /** Sub layout to add child items, if not the layout itself. */
  uiLayout *child_items_layout;

  int x, y, w, h;
  float scale[2];
  short space;
  bool align;
  bool active;
  bool active_default;
  bool activate_init;
  bool enabled;
  bool redalert;
  bool keepaspect;
  /** For layouts inside grid-flow, they and their items shall never have a fixed maximal size. */
  bool variable_size;
  char alignment;
  eUIEmbossType emboss;
  /** for fixed width or height to avoid UI size changes */
  float units[2];
};

static void ui_litem_init_from_parent(uiLayout *litem, uiLayout *layout, int align)
{
  litem->root = layout->root;
  litem->align = align;
  /* Children of grid-flow layout shall never have "ideal big size" returned as estimated size. */
  litem->variable_size = layout->variable_size || layout->item.type == ITEM_LAYOUT_GRID_FLOW;
  litem->active = true;
  litem->enabled = true;
  // litem->context = layout->context;
  litem->redalert = layout->redalert;
  litem->w = layout->w;
  litem->emboss = layout->emboss;
  litem->item.flag = (layout->item.flag &
                      (UI_ITEM_PROP_SEP | UI_ITEM_PROP_DECORATE | UI_ITEM_INSIDE_PROP_SEP));

  if (layout->child_items_layout) {
    layout->child_items_layout->items.push_back(litem);
    litem->parent = layout->child_items_layout;
  } else {
    layout->items.push_back(litem);
    litem->parent = layout;
  }
}

void UI_block_layout_set_current(uiBlock *block, uiLayout *layout)
{
  block->curlayout = layout;
}

/* variable button size in which direction? */
#define UI_ITEM_VARY_X 1
#define UI_ITEM_VARY_Y 2

static int ui_layout_vary_direction(uiLayout *layout)
{
  return ((ELEM(layout->root->type, UI_LAYOUT_HEADER, UI_LAYOUT_PIEMENU) ||
           (layout->alignment != UI_LAYOUT_ALIGN_EXPAND)) ?
              UI_ITEM_VARY_X :
              UI_ITEM_VARY_Y);
}

static bool ui_layout_variable_size(uiLayout *layout)
{
  /* Note that this code is probably a bit flaky, we'd probably want to know whether it's
   * variable in X and/or Y, etc. But for now it mimics previous one,
   * with addition of variable flag set for children of grid-flow layouts. */
  return ui_layout_vary_direction(layout) == UI_ITEM_VARY_X || layout->variable_size;
}

/**
 * Factors to apply to #UI_UNIT_X when calculating button width.
 * This is used when the layout is a varying size, see #ui_layout_variable_size.
 */
struct uiTextIconPadFactor
{
  float text;
  float icon;
  float icon_only;
};

/**
 * This adds over an icons width of padding even when no icon is used,
 * this is done because most buttons need additional space (drop-down chevron for example).
 * menus and labels use much smaller `text` values compared to this default.
 *
 * \note It may seem odd that the icon only adds 0.25
 * but taking margins into account its fine,
 * except for #ui_text_pad_compact where a bit more margin is required.
 */
static const struct uiTextIconPadFactor ui_text_pad_default = {
  .text = 1.50f,
  .icon = 0.25f,
  .icon_only = 0.0f,
};

/** #ui_text_pad_default scaled down. */
static const struct uiTextIconPadFactor ui_text_pad_compact = {
  .text = 1.25f,
  .icon = 0.35f,
  .icon_only = 0.0f,
};

/** Least amount of padding not to clip the text or icon. */
static const struct uiTextIconPadFactor ui_text_pad_none = {
  .text = 0.25f,
  .icon = 1.50f,
  .icon_only = 0.0f,
};

/**
 * Estimated size of text + icon.
 */
static int ui_text_icon_width_ex(uiLayout *layout,
                                 const char *name,
                                 int icon,
                                 const struct uiTextIconPadFactor *pad_factor)
{
  const int unit_x = UI_UNIT_X * (layout->scale[0] ? layout->scale[0] : 1.0f);

  /* When there is no text, always behave as if this is an icon-only button
   * since it's not useful to return empty space. */
  if (icon && !name[0]) {
    return unit_x * (1.0f + pad_factor->icon_only);
  }

  if (ui_layout_variable_size(layout)) {
    if (!icon && !name[0]) {
      return unit_x * (1.0f + pad_factor->icon_only);
    }

    if (layout->alignment != UI_LAYOUT_ALIGN_EXPAND) {
      layout->item.flag |= UI_ITEM_FIXED_SIZE;
    }

    float margin = pad_factor->text;
    if (icon) {
      margin += pad_factor->icon;
    }

    const float aspect = layout->root->block->aspect;
    const uiFontStyle *fstyle = UI_FSTYLE_WIDGET;
    return UI_fontstyle_string_width_with_block_aspect(fstyle, name, aspect) +
           (int)ceilf(unit_x * margin);
  }
  return unit_x * 10;
}

/* label item */
static uiBut *uiItemL_(uiLayout *layout, const char *name, int icon)
{
  uiBlock *block = layout->root->block;

  UI_block_layout_set_current(block, layout);
  UI_block_new_button_group(block, (uiButtonGroupFlag)0);

  if (!name) {
    name = "";
  }
  if (layout->root->type == UI_LAYOUT_MENU && !icon) {
    icon = ICON_BLANK1;
  }

  const int w = ui_text_icon_width_ex(layout, name, icon, &ui_text_pad_none);
  uiBut *but;
  if (icon && name[0]) {
    but = uiDefIconTextBut(block,
                           UI_BTYPE_LABEL,
                           0,
                           icon,
                           name,
                           0,
                           0,
                           w,
                           UI_UNIT_Y,
                           NULL,
                           0.0,
                           0.0,
                           0,
                           0,
                           NULL);
  } else if (icon) {
    but =
      uiDefIconBut(block, UI_BTYPE_LABEL, 0, icon, 0, 0, w, UI_UNIT_Y, NULL, 0.0, 0.0, 0, 0, NULL);
  } else {
    but = uiDefBut(block, UI_BTYPE_LABEL, 0, name, 0, 0, w, UI_UNIT_Y, NULL, 0.0, 0.0, 0, 0, NULL);
  }

  /* to compensate for string size padding in ui_text_icon_width,
   * make text aligned right if the layout is aligned right.
   */
  if (uiLayoutGetAlignment(layout) == UI_LAYOUT_ALIGN_RIGHT) {
    but->drawflag &= ~UI_BUT_TEXT_LEFT; /* default, needs to be unset */
    but->drawflag |= UI_BUT_TEXT_RIGHT;
  }

  /* Mark as a label inside a list-box. */
  if (block->flag & UI_BLOCK_LIST_ITEM) {
    but->flag |= UI_BUT_LIST_ITEM;
  }

  if (layout->redalert) {
    UI_but_flag_enable(but, UI_BUT_REDALERT);
  }

  return but;
}

void uiItemL(uiLayout *layout, const char *name, int icon)
{
  uiItemL_(layout, name, icon);
}

uiLayout *uiLayoutRow(uiLayout *layout, bool align)
{
  uiLayout *litem = new uiLayout();
  ui_litem_init_from_parent(litem, layout, align);

  litem->item.type = ITEM_LAYOUT_ROW;
  litem->space = (align) ? 0 : layout->root->style->buttonspacex;

  UI_block_layout_set_current(layout->root->block, litem);

  return litem;
}

void UI_menutype_draw(kContext *C, MenuType *mt, struct uiLayout *layout)
{
  Menu menu = {
    .layout = layout,
    .type = mt,
  };

  // if (layout->context) {
  //   CTX_store_set(C, layout->context);
  // }

  mt->draw(C, &menu);

  // if (layout->context) {
  //   CTX_store_set(C, NULL);
  // }
}

static bool ui_layout_has_panel_label(const uiLayout *layout, const PanelType *pt)
{
  for (auto &sublayout : layout->items) {
    if (sublayout->item.type == ITEM_BUTTON) {
      uiButtonItem *bitem = (uiButtonItem *)&sublayout->item;
      if (!(bitem->but->flag & UI_HIDDEN) && STREQ(bitem->but->str, pt->label)) {
        return true;
      }
    } else {
      uiLayout *litem = (uiLayout *)sublayout;
      if (ui_layout_has_panel_label(litem, pt)) {
        return true;
      }
    }
  }

  return false;
}

static void ui_paneltype_draw_impl(kContext *C, PanelType *pt, uiLayout *layout, bool show_header)
{
  Panel *panel = new Panel();
  panel->type = pt;
  panel->flag = PNL_POPOVER;

  uiLayout *last_item = nullptr;
  if (!layout->items.empty()) {
    last_item = layout->items.back();
  }

  /* Draw main panel. */
  if (show_header) {
    uiLayout *row = uiLayoutRow(layout, false);
    if (pt->draw_header) {
      panel->layout = row;
      pt->draw_header(C, panel);
      panel->layout = NULL;
    }

    /* draw_header() is often used to add a checkbox to the header. If we add the label like below
     * the label is disconnected from the checkbox, adding a weird looking gap. As workaround, let
     * the checkbox add the label instead. */
    if (!ui_layout_has_panel_label(row, pt)) {
      uiItemL(row, CTX_IFACE_(pt->translation_context, pt->label), ICON_NONE);
    }
  }

  panel->layout = layout;
  pt->draw(C, panel);
  panel->layout = NULL;
  KLI_assert(panel->runtime.custom_data_ptr == NULL);

  delete panel;

  /* Draw child panels. */
  for(auto &child_pt : pt->children)
  {
    if (child_pt->poll == NULL || child_pt->poll(C, child_pt)) {
      /* Add space if something was added to the layout. */
      if (last_item != layout->items.back()) {
        uiItemS(layout);
        last_item = layout->items.back();
      }

      uiLayout *col = uiLayoutColumn(layout, false);
      ui_paneltype_draw_impl(C, child_pt, col, true);
    }
  }
}

void UI_paneltype_draw(kContext *C, PanelType *pt, uiLayout *layout)
{
  // if (layout->context) {
  //   CTX_store_set(C, layout->context);
  // }

  ui_paneltype_draw_impl(C, pt, layout, false);

  // if (layout->context) {
  //   CTX_store_set(C, NULL);
  // }
}

void ui_item_menutype_func(kContext *C, uiLayout *layout, void *arg_mt)
{
  MenuType *mt = (MenuType *)arg_mt;

  UI_menutype_draw(C, mt, layout);

  /* Menus are created flipped (from event handling point of view). */
  layout->root->block->flag ^= UI_BLOCK_IS_FLIP;
}

void ui_item_paneltype_func(kContext *C, uiLayout *layout, void *arg_pt)
{
  PanelType *pt = (PanelType *)arg_pt;
  UI_paneltype_draw(C, pt, layout);

  /* Panels are created flipped (from event handling POV). */
  layout->root->block->flag ^= UI_BLOCK_IS_FLIP;
}

PanelType *UI_but_paneltype_get(uiBut *but)
{
  if (but->menu_create_func == ui_item_paneltype_func) {
    return (PanelType *)but->poin;
  }
  return NULL;
}

WABI_NAMESPACE_END