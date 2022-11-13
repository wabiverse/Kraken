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

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "USD_space_types.h"
#include "USD_wm_types.h"
#include "USD_view2d.h"

#include "ED_screen.h"

#include "KLI_listbase.h"
#include "KLI_math.h"
#include "KLI_string.h"
// #include "KLI_string_search.h"
#include "KLI_utildefines.h"

#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_lib_id.h"
#include "KKE_report.h"

#include "MEM_guardedalloc.h"

#include "LUXO_access.h"
#include "LUXO_define.h"

#include "UI_interface.h"
#include "UI_interface_icons.h"
#include "UI_resources.h"
#include "UI_view2d.h"

#include "interface_intern.h"

uiBut *uiDefAutoButR(uiBlock *block,
                     KrakenPRIM *ptr,
                     KrakenPROP *prop,
                     int index,
                     const char *name,
                     int icon,
                     int x,
                     int y,
                     int width,
                     int height)
{
  uiBut *but = nullptr;

  switch (LUXO_prop_type(prop)) {
    case PROP_BOOLEAN: {
      if (prop->GetTypeName().IsArray() && index == -1) {
        return nullptr;
      }

      if (icon && name && name[0] == '\0') {
        but = uiDefIconButR_prop(block,
                                 UI_BTYPE_ICON_TOGGLE,
                                 0,
                                 icon,
                                 x,
                                 y,
                                 width,
                                 height,
                                 ptr,
                                 prop,
                                 index,
                                 0,
                                 0,
                                 -1,
                                 -1,
                                 nullptr);
      } else if (icon) {
        but = uiDefIconTextButR_prop(block,
                                     UI_BTYPE_ICON_TOGGLE,
                                     0,
                                     icon,
                                     name,
                                     x,
                                     y,
                                     width,
                                     height,
                                     ptr,
                                     prop,
                                     index,
                                     0,
                                     0,
                                     -1,
                                     -1,
                                     nullptr);
      } else {
        but = uiDefButR_prop(block,
                             UI_BTYPE_CHECKBOX,
                             0,
                             name,
                             x,
                             y,
                             width,
                             height,
                             ptr,
                             prop,
                             index,
                             0,
                             0,
                             -1,
                             -1,
                             nullptr);
      }
      break;
    }
    case PROP_INT:
    case PROP_FLOAT: {
      if (prop->GetTypeName().IsArray() && index == -1) {
        if (ELEM(LUXO_prop_subtype(prop), PROP_COLOR, PROP_COLOR_GAMMA)) {
          but = uiDefButR_prop(block,
                               UI_BTYPE_COLOR,
                               0,
                               name,
                               x,
                               y,
                               width,
                               height,
                               ptr,
                               prop,
                               -1,
                               0,
                               0,
                               0,
                               0,
                               nullptr);
        } else {
          return nullptr;
        }
      } else if (LUXO_prop_subtype(prop) == PROP_PERCENTAGE ||
                 LUXO_prop_subtype(prop) == PROP_FACTOR) {
        but = uiDefButR_prop(block,
                             UI_BTYPE_NUM_SLIDER,
                             0,
                             name,
                             x,
                             y,
                             width,
                             height,
                             ptr,
                             prop,
                             index,
                             0,
                             0,
                             -1,
                             -1,
                             nullptr);
      } else {
        but = uiDefButR_prop(block,
                             UI_BTYPE_NUM,
                             0,
                             name,
                             x,
                             y,
                             width,
                             height,
                             ptr,
                             prop,
                             index,
                             0,
                             0,
                             0,
                             0,
                             nullptr);
      }

      if (prop->flag & PROP_TEXTEDIT_UPDATE) {
        UI_but_flag_enable(but, UI_BUT_TEXTEDIT_UPDATE);
      }
      break;
    }
    case PROP_ENUM:
      if (icon && name && name[0] == '\0') {
        but = uiDefIconButR_prop(block,
                                 UI_BTYPE_MENU,
                                 0,
                                 icon,
                                 x,
                                 y,
                                 width,
                                 height,
                                 ptr,
                                 prop,
                                 index,
                                 0,
                                 0,
                                 -1,
                                 -1,
                                 nullptr);
      } else if (icon) {
        but = uiDefIconTextButR_prop(block,
                                     UI_BTYPE_MENU,
                                     0,
                                     icon,
                                     nullptr,
                                     x,
                                     y,
                                     width,
                                     height,
                                     ptr,
                                     prop,
                                     index,
                                     0,
                                     0,
                                     -1,
                                     -1,
                                     nullptr);
      } else {
        but = uiDefButR_prop(block,
                             UI_BTYPE_MENU,
                             0,
                             name,
                             x,
                             y,
                             width,
                             height,
                             ptr,
                             prop,
                             index,
                             0,
                             0,
                             -1,
                             -1,
                             nullptr);
      }
      break;
    case PROP_STRING:
      if (icon && name && name[0] == '\0') {
        but = uiDefIconButR_prop(block,
                                 UI_BTYPE_TEXT,
                                 0,
                                 icon,
                                 x,
                                 y,
                                 width,
                                 height,
                                 ptr,
                                 prop,
                                 index,
                                 0,
                                 0,
                                 -1,
                                 -1,
                                 nullptr);
      } else if (icon) {
        but = uiDefIconTextButR_prop(block,
                                     UI_BTYPE_TEXT,
                                     0,
                                     icon,
                                     name,
                                     x,
                                     y,
                                     width,
                                     height,
                                     ptr,
                                     prop,
                                     index,
                                     0,
                                     0,
                                     -1,
                                     -1,
                                     nullptr);
      } else {
        but = uiDefButR_prop(block,
                             UI_BTYPE_TEXT,
                             0,
                             name,
                             x,
                             y,
                             width,
                             height,
                             ptr,
                             prop,
                             index,
                             0,
                             0,
                             -1,
                             -1,
                             nullptr);
      }

      if (prop->flag & PROP_TEXTEDIT_UPDATE) {
        /* TEXTEDIT_UPDATE is usually used for search buttons. For these we also want
         * the 'x' icon to clear search string, so setting VALUE_CLEAR flag, too. */
        UI_but_flag_enable(but, UI_BUT_TEXTEDIT_UPDATE | UI_BUT_VALUE_CLEAR);
      }
      break;
    case PROP_POINTER: {
      if (icon == 0) {
        const KrakenPRIM *pptr = MEM_new<KrakenPRIM>(__func__, prop->GetPrim());
        icon = LUXO_prim_ui_icon(pptr->type ? pptr->type : nullptr);
      }
      if (icon == ICON_DOT) {
        icon = 0;
      }

      but = uiDefIconTextButR_prop(block,
                                   UI_BTYPE_SEARCH_MENU,
                                   0,
                                   icon,
                                   name,
                                   x,
                                   y,
                                   width,
                                   height,
                                   ptr,
                                   prop,
                                   index,
                                   0,
                                   0,
                                   -1,
                                   -1,
                                   nullptr);
      ui_but_add_search(but, ptr, prop, nullptr, nullptr, false);
      break;
    }
    case PROP_COLLECTION: {
      char text[256];
      KLI_snprintf(text,
                   sizeof(text),
                   IFACE_("%d items"),
                   (int)prop->GetPrim().GetChildrenNames().size());
      but = uiDefBut(block,
                     UI_BTYPE_LABEL,
                     0,
                     text,
                     x,
                     y,
                     width,
                     height,
                     nullptr,
                     0,
                     0,
                     0,
                     0,
                     nullptr);
      UI_but_flag_enable(but, UI_BUT_DISABLED);
      break;
    }
    default:
      but = nullptr;
      break;
  }

  return but;
}

void uiDefAutoButsArrayR(uiBlock *block,
                         KrakenPRIM *ptr,
                         KrakenPROP *prop,
                         const int icon,
                         const int x,
                         const int y,
                         const int tot_width,
                         const int height)
{
  const int len = (int)prop->GetTypeName().GetDimensions().size;
  if (len == 0) {
    return;
  }

  const int item_width = tot_width / len;

  UI_block_align_begin(block);
  for (int i = 0; i < len; i++) {
    uiDefAutoButR(block, ptr, prop, i, "", icon, x + i * item_width, y, item_width, height);
  }
  UI_block_align_end(block);
}

eAutoPropButsReturn uiDefAutoButsPRIM(uiLayout *layout,
                                      KrakenPRIM *ptr,
                                      bool (*check_prop)(KrakenPRIM *ptr,
                                                         KrakenPROP *prop,
                                                         void *user_data),
                                      void *user_data,
                                      KrakenPROP *prop_activate_init,
                                      const eButLabelAlign label_align,
                                      const bool compact)
{
  eAutoPropButsReturn return_info = UI_PROP_BUTS_NONE_ADDED;
  uiLayout *col;
  const char *name;

  /**
   * Build the ui button properties for this prim. */
  for (auto &prop : ptr->props) {
    const int flag = prop->flag;

    if (flag & PROP_HIDDEN) {
      continue;
    }
    if (check_prop && check_prop(ptr, prop, user_data) == 0) {
      return_info |= UI_PROP_BUTS_ANY_FAILED_CHECK;
      continue;
    }

    const PropertyType type = prop->type;
    switch (label_align) {
      case UI_BUT_LABEL_ALIGN_COLUMN:
      case UI_BUT_LABEL_ALIGN_SPLIT_COLUMN: {
        const bool is_boolean = (type == PROP_BOOLEAN && !prop->GetTypeName().IsArray());

        name = prop->GetDisplayName().c_str();

        if (label_align == UI_BUT_LABEL_ALIGN_COLUMN) {
          col = uiLayoutColumn(layout, true);

          if (!is_boolean) {
            uiItemL(col, name, ICON_NONE);
          }
        } else {
          KLI_assert(label_align == UI_BUT_LABEL_ALIGN_SPLIT_COLUMN);
          col = uiLayoutColumn(layout, true);
          /* Let uiItemFullR() create the split layout. */
          uiLayoutSetPropSep(col, true);
        }

        break;
      }
      case UI_BUT_LABEL_ALIGN_NONE:
      default:
        col = layout;
        name = nullptr; /* no smart label alignment, show default name with button */
        break;
    }

    /* Only buttons that can be edited as text. */
    const bool use_activate_init = ((prop == prop_activate_init) &&
                                    ELEM(type, PROP_STRING, PROP_INT, PROP_FLOAT));

    if (use_activate_init) {
      uiLayoutSetActivateInit(col, true);
    }

    uiItemFullR(col, ptr, prop, -1, 0, compact ? UI_ITEM_R_COMPACT : 0, name, ICON_NONE);
    return_info &= ~UI_PROP_BUTS_NONE_ADDED;

    if (use_activate_init) {
      uiLayoutSetActivateInit(col, false);
    }
  }

  return return_info;
}

void UI_but_func_identity_compare_set(uiBut *but, uiButIdentityCompareFunc cmp_fn)
{
  but->identity_cmp_func = cmp_fn;
}

/* *** RNA collection search menu *** */

struct CollItemSearch
{
  struct CollItemSearch *next, *prev;
  void *data;
  char *name;
  int index;
  int iconid;
  bool is_id;
  int name_prefix_offset;
  uint has_sep_char : 1;
};

static bool add_collection_search_item(CollItemSearch *cis,
                                       const bool requires_exact_data_name,
                                       const bool has_id_icon,
                                       uiSearchItems *items)
{
  char name_buf[UI_MAX_DRAW_STR];

  /* If no item has an own icon to display, libraries can use the library icons rather than the
   * name prefix for showing the library status. */
  int name_prefix_offset = cis->name_prefix_offset;
  if (!has_id_icon && cis->is_id && !requires_exact_data_name) {
    cis->iconid = UI_icon_from_library(static_cast<const ID *>(cis->data));
    /* No need to re-allocate, string should be shorter than before (lib status prefix is
     * removed). */
    // KKE_id_full_name_ui_prefix_get(name_buf,
    //                                static_cast<const ID *>(cis->data),
    //                                false,
    //                                UI_SEP_CHAR,
    //                                &name_prefix_offset);
    // KLI_assert(strlen(name_buf) <= MEM_allocN_len(cis->name));
    // strcpy(cis->name, name_buf);
  }

  return UI_search_item_add(items,
                            cis->name,
                            cis->data,
                            cis->iconid,
                            cis->has_sep_char ? int(UI_BUT_HAS_SEP_CHAR) : 0,
                            name_prefix_offset);
}

int UI_icon_from_id(const ID *id)
{
  if (id == nullptr) {
    return ICON_NONE;
  }

  /* exception for objects */
  // if (GS(id->name) == ID_OB) {
  //   Object *ob = (Object *)id;

  //   if (ob->type == OB_EMPTY) {
  //     return ICON_EMPTY_DATA;
  //   }
  //   return UI_icon_from_id(static_cast<const ID *>(ob->data));
  // }

  /* otherwise get it through RNA, creating the pointer
   * will set the right type, also with subclassing */
  // KrakenPRIM ptr;
  // (ID *)id->
  // LUXO_id_pointer_create((ID *)id, &ptr);

  return /*(ptr.type) ? LUXO_prim_ui_icon(ptr.type) :*/ ICON_NONE;
}

int UI_icon_from_report_type(int type)
{
  if (type & RPT_ERROR_ALL) {
    return ICON_CANCEL;
  }
  if (type & RPT_WARNING_ALL) {
    return ICON_ERROR;
  }
  if (type & RPT_INFO_ALL) {
    return ICON_INFO;
  }
  if (type & RPT_DEBUG_ALL) {
    return ICON_SYSTEM;
  }
  if (type & RPT_PROPERTY) {
    return ICON_OPTIONS;
  }
  if (type & RPT_OPERATOR) {
    return ICON_CHECKMARK;
  }
  return ICON_INFO;
}

int UI_icon_colorid_from_report_type(int type)
{
  if (type & RPT_ERROR_ALL) {
    return TH_INFO_ERROR;
  }
  if (type & RPT_WARNING_ALL) {
    return TH_INFO_WARNING;
  }
  if (type & RPT_INFO_ALL) {
    return TH_INFO_INFO;
  }
  if (type & RPT_DEBUG_ALL) {
    return TH_INFO_DEBUG;
  }
  if (type & RPT_PROPERTY) {
    return TH_INFO_PROPERTY;
  }
  if (type & RPT_OPERATOR) {
    return TH_INFO_OPERATOR;
  }
  return TH_INFO_WARNING;
}

int UI_text_colorid_from_report_type(int type)
{
  if (type & RPT_ERROR_ALL) {
    return TH_INFO_ERROR_TEXT;
  }
  if (type & RPT_WARNING_ALL) {
    return TH_INFO_WARNING_TEXT;
  }
  if (type & RPT_INFO_ALL) {
    return TH_INFO_INFO_TEXT;
  }
  if (type & RPT_DEBUG_ALL) {
    return TH_INFO_DEBUG_TEXT;
  }
  if (type & RPT_PROPERTY) {
    return TH_INFO_PROPERTY_TEXT;
  }
  if (type & RPT_OPERATOR) {
    return TH_INFO_OPERATOR_TEXT;
  }
  return TH_INFO_WARNING_TEXT;
}

/********************************** Misc **************************************/

int UI_calc_float_precision(int prec, double value)
{
  static const double pow10_neg[UI_PRECISION_FLOAT_MAX + 1] =
    {1e0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6};
  static const double max_pow = 10000000.0; /* pow(10, UI_PRECISION_FLOAT_MAX) */

  KLI_assert(prec <= UI_PRECISION_FLOAT_MAX);
  KLI_assert(fabs(pow10_neg[prec] - pow(10, -prec)) < 1e-16);

  /* Check on the number of decimal places need to display the number,
   * this is so 0.00001 is not displayed as 0.00,
   * _but_, this is only for small values as 10.0001 will not get the same treatment.
   */
  value = fabs(value);
  if ((value < pow10_neg[prec]) && (value > (1.0 / max_pow))) {
    int value_i = int(lround(value * max_pow));
    if (value_i != 0) {
      const int prec_span = 3; /* show: 0.01001, 5 would allow 0.0100001 for eg. */
      int test_prec;
      int prec_min = -1;
      int dec_flag = 0;
      int i = UI_PRECISION_FLOAT_MAX;
      while (i && value_i) {
        if (value_i % 10) {
          dec_flag |= 1 << i;
          prec_min = i;
        }
        value_i /= 10;
        i--;
      }

      /* even though its a small value, if the second last digit is not 0, use it */
      test_prec = prec_min;

      dec_flag = (dec_flag >> (prec_min + 1)) & ((1 << prec_span) - 1);

      while (dec_flag) {
        test_prec++;
        dec_flag = dec_flag >> 1;
      }

      if (test_prec > prec) {
        prec = test_prec;
      }
    }
  }

  CLAMP(prec, 0, UI_PRECISION_FLOAT_MAX);

  return prec;
}

bool UI_but_online_manual_id(const uiBut *but, char *r_str, size_t maxlength)
{
  if (but->stagepoin->owner_id && but->stagepoin->data && but->stageprop) {
    KLI_snprintf(r_str,
                 maxlength,
                 "%s.%s",
                 LUXO_prim_identifier(but->stagepoin->type).data(),
                 LUXO_prop_identifier(but->stageprop).data());
    return true;
  }
  if (but->optype) {
    WM_operator_py_idname(r_str, but->optype->idname.data());
    return true;
  }

  *r_str = '\0';
  return false;
}

bool UI_but_online_manual_id_from_active(const kContext *C, char *r_str, size_t maxlength)
{
  uiBut *but = UI_context_active_but_get(C);

  if (but) {
    return UI_but_online_manual_id(but, r_str, maxlength);
  }

  *r_str = '\0';
  return false;
}

/* -------------------------------------------------------------------- */

static rctf ui_but_rect_to_view(const uiBut *but, const ARegion *region, const View2D *v2d)
{
  rctf region_rect;
  ui_block_to_region_rctf(region, but->block, &region_rect, &but->rect);

  rctf view_rect;
  UI_view2d_region_to_view_rctf(v2d, &region_rect, &view_rect);

  return view_rect;
}

/**
 * To get a margin (typically wanted), add the margin to \a rect directly.
 *
 * Based on #file_ensure_inside_viewbounds(), could probably share code.
 *
 * \return true if anything changed.
 */
static bool ui_view2d_cur_ensure_rect_in_view(View2D *v2d, const rctf *rect)
{
  const float rect_width = KLI_rctf_size_x(rect);
  const float rect_height = KLI_rctf_size_y(rect);

  rctf *cur = &v2d->cur;
  const float cur_width = KLI_rctf_size_x(cur);
  const float cur_height = KLI_rctf_size_y(cur);

  bool changed = false;

  /* Snap to bottom edge. Also use if rect is higher than view bounds (could be a parameter). */
  if ((cur->ymin > rect->ymin) || (rect_height > cur_height)) {
    cur->ymin = rect->ymin;
    cur->ymax = cur->ymin + cur_height;
    changed = true;
  }
  /* Snap to upper edge. */
  else if (cur->ymax < rect->ymax) {
    cur->ymax = rect->ymax;
    cur->ymin = cur->ymax - cur_height;
    changed = true;
  }
  /* Snap to left edge. Also use if rect is wider than view bounds. */
  else if ((cur->xmin > rect->xmin) || (rect_width > cur_width)) {
    cur->xmin = rect->xmin;
    cur->xmax = cur->xmin + cur_width;
    changed = true;
  }
  /* Snap to right edge. */
  else if (cur->xmax < rect->xmax) {
    cur->xmax = rect->xmax;
    cur->xmin = cur->xmax - cur_width;
    changed = true;
  } else {
    KLI_assert(KLI_rctf_inside_rctf(cur, rect));
  }

  return changed;
}

void UI_but_ensure_in_view(const kContext *C, ARegion *region, const uiBut *but)
{
  View2D *v2d = &region->v2d;
  /* Uninitialized view or region that doesn't use View2D. */
  if ((v2d->flag & V2D_IS_INIT) == 0) {
    return;
  }

  rctf rect = ui_but_rect_to_view(but, region, v2d);

  const int margin = UI_UNIT_X * 0.5f;
  KLI_rctf_pad(&rect, margin, margin);

  const bool changed = ui_view2d_cur_ensure_rect_in_view(v2d, &rect);
  if (changed) {
    UI_view2d_curRect_changed(C, v2d);
    ED_region_tag_redraw_no_rebuild(region);
  }
}

/* -------------------------------------------------------------------- */
/** \name Button Store
 *
 * Modal Button Store API.
 *
 * Store for modal operators & handlers to register button pointers
 * which are maintained while drawing or nullptr when removed.
 *
 * This is needed since button pointers are continuously freed and re-allocated.
 *
 * \{ */

struct uiButStore
{
  struct uiButStore *next, *prev;
  uiBlock *block;
  ListBase items;
};

struct uiButStoreElem
{
  struct uiButStoreElem *next, *prev;
  uiBut **but_p;
};

uiButStore *UI_butstore_create(uiBlock *block)
{
  uiButStore *bs_handle = MEM_cnew<uiButStore>(__func__);

  bs_handle->block = block;
  KLI_addtail(&block->butstore, bs_handle);

  return bs_handle;
}

void UI_butstore_free(uiBlock *block, uiButStore *bs_handle)
{
  /* Workaround for button store being moved into new block,
   * which then can't use the previous buttons state
   * ('ui_but_update_from_old_block' fails to find a match),
   * keeping the active button in the old block holding a reference
   * to the button-state in the new block: see T49034.
   *
   * Ideally we would manage moving the 'uiButStore', keeping a correct state.
   * All things considered this is the most straightforward fix - Campbell.
   */
  if (block != bs_handle->block && bs_handle->block != nullptr) {
    block = bs_handle->block;
  }

  KLI_freelistN(&bs_handle->items);
  KLI_assert(KLI_findindex(&block->butstore, bs_handle) != -1);
  KLI_remlink(&block->butstore, bs_handle);

  MEM_freeN(bs_handle);
}

bool UI_butstore_is_valid(uiButStore *bs)
{
  return (bs->block != nullptr);
}

bool UI_butstore_is_registered(uiBlock *block, uiBut *but)
{
  LISTBASE_FOREACH(uiButStore *, bs_handle, &block->butstore)
  {
    LISTBASE_FOREACH(uiButStoreElem *, bs_elem, &bs_handle->items)
    {
      if (*bs_elem->but_p == but) {
        return true;
      }
    }
  }

  return false;
}

void UI_butstore_register(uiButStore *bs_handle, uiBut **but_p)
{
  uiButStoreElem *bs_elem = MEM_cnew<uiButStoreElem>(__func__);
  KLI_assert(*but_p);
  bs_elem->but_p = but_p;

  KLI_addtail(&bs_handle->items, bs_elem);
}

void UI_butstore_unregister(uiButStore *bs_handle, uiBut **but_p)
{
  LISTBASE_FOREACH_MUTABLE(uiButStoreElem *, bs_elem, &bs_handle->items)
  {
    if (bs_elem->but_p == but_p) {
      KLI_remlink(&bs_handle->items, bs_elem);
      MEM_freeN(bs_elem);
    }
  }

  KLI_assert(0);
}

bool UI_butstore_register_update(uiBlock *block, uiBut *but_dst, const uiBut *but_src)
{
  bool found = false;

  LISTBASE_FOREACH(uiButStore *, bs_handle, &block->butstore)
  {
    LISTBASE_FOREACH(uiButStoreElem *, bs_elem, &bs_handle->items)
    {
      if (*bs_elem->but_p == but_src) {
        *bs_elem->but_p = but_dst;
        found = true;
      }
    }
  }

  return found;
}

void UI_butstore_clear(uiBlock *block)
{
  LISTBASE_FOREACH(uiButStore *, bs_handle, &block->butstore)
  {
    bs_handle->block = nullptr;
    LISTBASE_FOREACH(uiButStoreElem *, bs_elem, &bs_handle->items)
    {
      *bs_elem->but_p = nullptr;
    }
  }
}

void UI_butstore_update(uiBlock *block)
{
  /* move this list to the new block */
  if (block->oldblock) {
    if (block->oldblock->butstore.first) {
      KLI_movelisttolist(&block->butstore, &block->oldblock->butstore);
    }
  }

  if (LIKELY(block->butstore.first == nullptr)) {
    return;
  }

  /* warning, loop-in-loop, in practice we only store <10 buttons at a time,
   * so this isn't going to be a problem, if that changes old-new mapping can be cached first */
  LISTBASE_FOREACH(uiButStore *, bs_handle, &block->butstore)
  {
    KLI_assert(ELEM(bs_handle->block, nullptr, block) ||
               (block->oldblock && block->oldblock == bs_handle->block));

    if (bs_handle->block == block->oldblock) {
      bs_handle->block = block;

      LISTBASE_FOREACH(uiButStoreElem *, bs_elem, &bs_handle->items)
      {
        if (*bs_elem->but_p) {
          uiBut *but_new = ui_but_find_new(block, *bs_elem->but_p);

          /* can be nullptr if the buttons removed,
           * NOTE: we could allow passing in a callback when buttons are removed
           * so the caller can cleanup */
          *bs_elem->but_p = but_new;
        }
      }
    }
  }
}

/** \} */
