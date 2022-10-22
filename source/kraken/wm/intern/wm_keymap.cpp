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
 * Window Manager.
 * Making GUI Fly.
 */

#include "kraken/kraken.h"

#include "USD_object.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"

#include "MEM_guardedalloc.h"

#include "KLI_kraklib.h"
#include "KLI_math.h"
#include "KLI_utildefines.h"

#include "KRF_api.h"

#include "KKE_context.h"
#include "KKE_idprop.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"
#include "KKE_utils.h"
#include "KKE_global.h"

#include "LUXO_access.h"
#include "LUXO_enum_types.h"

#include "WM_api.h"
#include "WM_debug_codes.h"
#include "WM_event_system.h"
#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_tokens.h"
#include "WM_window.hh"
#include "WM_keymap.h"

struct wmKeyMapItemFind_Params
{
  bool (*filter_fn)(const wmKeyMap *km, const wmKeyMapItem *kmi, void *user_data);
  void *user_data;
};

static bool kmi_filter_is_visible(const wmKeyMap *UNUSED(km),
                                  const wmKeyMapItem *kmi,
                                  void *UNUSED(user_data))
{
  return ((WM_key_event_string(kmi->type, false)[0] != '\0') &&
          (IS_EVENT_ACTIONZONE(kmi->type) == false));
}

static const char *key_event_glyph_or_text(const int font_id,
                                           const char *text,
                                           const char *single_glyph)
{
  KLI_assert(single_glyph == NULL || (KLI_strlen_utf8(single_glyph) == 1));
  return (single_glyph && KRF_has_glyph(font_id, KLI_str_utf8_as_unicode(single_glyph))) ?
           single_glyph :
           text;
}

const char *WM_key_event_string(const short type, const bool compact)
{
  if (compact) {
    /* String storing a single unicode character or NULL. */
    const char *single_glyph = NULL;
    int font_id = KRF_default();
    const enum {
      UNIX,
      MACOS,
      MSWIN,
    } platform =

#if defined(__APPLE__)
      MACOS
#elif defined(_WIN32)
      MSWIN
#else
      UNIX
#endif
      ;

    switch (type) {
      case EVT_LEFTSHIFTKEY:
      case EVT_RIGHTSHIFTKEY: {
        if (platform == MACOS) {
          single_glyph = "\xe2\x87\xa7";
        }
        return key_event_glyph_or_text(font_id, CTX_IFACE_("Shift"), single_glyph);
      }
      case EVT_LEFTCTRLKEY:
      case EVT_RIGHTCTRLKEY:
        if (platform == MACOS) {
          return key_event_glyph_or_text(font_id, "^", "\xe2\x8c\x83");
        }
        return IFACE_("Ctrl");
      case EVT_LEFTALTKEY:
      case EVT_RIGHTALTKEY: {
        if (platform == MACOS) {
          /* Option symbol on Mac keyboard. */
          single_glyph = "\xe2\x8c\xa5";
        }
        return key_event_glyph_or_text(font_id, IFACE_("Alt"), single_glyph);
      }
      case EVT_OSKEY: {
        if (platform == MACOS) {
          return key_event_glyph_or_text(font_id, IFACE_("Cmd"), "\xe2\x8c\x98");
        }
        if (platform == MSWIN) {
          return key_event_glyph_or_text(font_id, IFACE_("Win"), "\xe2\x9d\x96");
        }
        return IFACE_("OS");
      } break;
      case EVT_TABKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Tab"), "\xe2\xad\xbe");
      case EVT_BACKSPACEKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Bksp"), "\xe2\x8c\xab");
      case EVT_ESCKEY:
        if (platform == MACOS) {
          single_glyph = "\xe2\x8e\x8b";
        }
        return key_event_glyph_or_text(font_id, IFACE_("Esc"), single_glyph);
      case EVT_RETKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Enter"), "\xe2\x86\xb5");
      case EVT_SPACEKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Space"), "\xe2\x90\xa3");
      case EVT_LEFTARROWKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Left"), "\xe2\x86\x90");
      case EVT_UPARROWKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Up"), "\xe2\x86\x91");
      case EVT_RIGHTARROWKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Right"), "\xe2\x86\x92");
      case EVT_DOWNARROWKEY:
        return key_event_glyph_or_text(font_id, IFACE_("Down"), "\xe2\x86\x93");
    }
  }

  // const EnumPropertyItem *it;
  // const int i = LUXO_enum_from_value(luxo_enum_event_type_items, (int)type);

  // if (i == -1) {
  return "";
  // }
  // it = &luxo_enum_event_type_items[i];

  /* We first try enum items' description (abused as shortname here),
   * and fall back to usual name if empty. */
  // if (compact && it->description[0]) {
    /* XXX No context for enum descriptions... In practice shall not be an issue though. */
    // return IFACE_(it->description);
  // }

  // return CTX_IFACE_(it->name);
}


int WM_keymap_item_raw_to_string(const short shift,
                                 const short ctrl,
                                 const short alt,
                                 const short oskey,
                                 const short keymodifier,
                                 const short val,
                                 const short type,
                                 const bool compact,
                                 char *result,
                                 const int result_len)
{
  /* TODO: also support (some) value, like e.g. double-click? */

#define ADD_SEP   \
  if (p != buf) { \
    *p++ = ' ';   \
  }               \
  (void)0

  char buf[128];
  char *p = buf;

  buf[0] = '\0';

  if (shift == KM_ANY && ctrl == KM_ANY && alt == KM_ANY && oskey == KM_ANY) {
    /* Don't show anything for any mapping. */
  } else {
    if (shift) {
      ADD_SEP;
      p += KLI_strcpy_rlen(p, WM_key_event_string(EVT_LEFTSHIFTKEY, true));
    }

    if (ctrl) {
      ADD_SEP;
      p += KLI_strcpy_rlen(p, WM_key_event_string(EVT_LEFTCTRLKEY, true));
    }

    if (alt) {
      ADD_SEP;
      p += KLI_strcpy_rlen(p, WM_key_event_string(EVT_LEFTALTKEY, true));
    }

    if (oskey) {
      ADD_SEP;
      p += KLI_strcpy_rlen(p, WM_key_event_string(EVT_OSKEY, true));
    }
  }

  if (keymodifier) {
    ADD_SEP;
    p += KLI_strcpy_rlen(p, WM_key_event_string(keymodifier, compact));
  }

  if (type) {
    ADD_SEP;
    if (val == KM_DBL_CLICK) {
      p += KLI_strcpy_rlen(p, IFACE_("dbl-"));
    }
    p += KLI_strcpy_rlen(p, WM_key_event_string(type, compact));
  }

  /* We assume size of buf is enough to always store any possible shortcut,
   * but let's add a debug check about it! */
  KLI_assert(p - buf < sizeof(buf));

  /* We need utf8 here, otherwise we may 'cut' some unicode chars like arrows... */
  return KLI_strncpy_utf8_rlen(result, buf, result_len);

#undef ADD_SEP
}

/* -------------------------------------------------------------------- */
/** \name Storage in WM
 *
 * Name id's are for storing general or multiple keymaps.
 *
 * - Space/region ids are same as DNA_space_types.h
 * - Gets freed in wm.c
 * \{ */

wmKeyMap *WM_keymap_list_find(ListBase *keymaps,
                              const wabi::TfToken &idname,
                              int spaceid,
                              int regionid)
{
  LISTBASE_FOREACH(wmKeyMap *, km, keymaps) {
    if (km->spaceid == spaceid && km->regionid == regionid) {
      if (km->idname == idname) {
        return km;
      }
    }
  }

  return NULL;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Event Handling
 *
 * Handlers have pointers to the keymap in the default configuration.
 * During event handling this function is called to get the keymap from the final configuration.
 * \{ */

wmKeyMap *WM_keymap_active(const wmWindowManager *wm, wmKeyMap *keymap)
{
  if (!keymap) {
    return NULL;
  }

  /* first user defined keymaps */
  wmKeyMap *km = WM_keymap_list_find(&wm->userconf->keymaps,
                                     wabi::TfToken(keymap->idname),
                                     keymap->spaceid,
                                     keymap->regionid);

  if (km) {
    return km;
  }

  return keymap;
}

/** \} */

bool WM_keymap_poll(kContext *C, wmKeyMap *keymap)
{
  /* If we're tagged, only use compatible. */
  if (!keymap->owner_id) {
    const WorkSpace *workspace = CTX_wm_workspace(C);
    if (KKE_workspace_owner_id_check(workspace, wabi::TfToken(keymap->owner_id)) == false) {
      return false;
    }
  }

  if (ARCH_UNLIKELY(KLI_listbase_is_empty(&keymap->items))) {
    /* Empty key-maps may be missing more there may be a typo in the name.
     * Warn early to avoid losing time investigating each case.
     * When developing a customized Blender though you may want empty keymaps. */
    // if (!U.app_template[0] &&
    //     /* Fallback key-maps may be intentionally empty, don't flood the output. */
    //     !keymap->idname.Find(" (fallback)") &&
    //     /* This is an exception which may be empty.
    //      * Longer term we might want a flag to indicate an empty key-map is intended. */
    //     !STREQ(keymap->idname, "Node Tool: Tweak")) {
    //   CLOG_WARN(WM_LOG_KEYMAPS, "empty keymap '%s'", keymap->idname);
    // }
  }

  if (keymap->poll != NULL) {
    return keymap->poll(C);
  }
  return true;
}

static wmKeyMapItem *wm_keymap_item_find_in_keymap(wmKeyMap *keymap,
                                                   const wabi::TfToken &opname,
                                                   IDProperty *properties,
                                                   const bool is_strict,
                                                   const struct wmKeyMapItemFind_Params *params)
{
  LISTBASE_FOREACH(wmKeyMapItem *, kmi, &keymap->items)
  {
    /* skip disabled keymap items [T38447] */
    if (kmi->flag & KMI_INACTIVE) {
      continue;
    }

    bool kmi_match = false;

    if (kmi->idname == opname) {
      if (properties) {
        /* example of debugging keymaps */
#if 0
        if (kmi->ptr) {
          if (opname == GEOM_ID_(MESH_OT_rip_move)) {
            printf("OPERATOR\n");
            IDP_print(properties);
            printf("KEYMAP\n");
            IDP_print(kmi->ptr->data);
          }
        }
#endif

        if (kmi->ptr &&
            IDP_EqualsProperties_ex(properties, (IDProperty *)kmi->ptr->data, is_strict)) {
          kmi_match = true;
        }
        /* Debug only, helps spotting mismatches between menu entries and shortcuts! */
        else if (G.debug & G_DEBUG_WM) {
          if (is_strict && kmi->ptr) {
            wmOperatorType *ot = WM_operatortype_find(opname);
            if (ot) {
              /* make a copy of the properties and set unset ones to their default values. */
              KrakenPRIM opptr;
              IDProperty *properties_default = IDP_CopyProperty((IDProperty *)kmi->ptr->data);

              LUXO_pointer_create(nullptr, ot->prim, properties_default, &opptr);
              WM_operator_properties_default(&opptr, true);

              if (IDP_EqualsProperties_ex(properties, properties_default, is_strict)) {
                char kmi_str[128];
                WM_keymap_item_to_string(kmi, false, kmi_str, sizeof(kmi_str));
                /* NOTE: given properties could come from other things than menu entry. */
                printf(
                  "%s: Some set values in menu entry match default op values, "
                  "this might not be desired!\n",
                  opname.GetText());
                printf("\tkm: '%s', kmi: '%s'\n", keymap->idname, kmi_str);
#ifndef NDEBUG
#  ifdef WITH_PYTHON
                printf("OPERATOR\n");
                // IDP_print(properties);
                printf("KEYMAP\n");
                // IDP_print(kmi->ptr->data);
#  endif
#endif
                printf("\n");
              }

              // IDP_FreeProperty(properties_default);
            }
          }
        }
      } else {
        kmi_match = true;
      }

      if (kmi_match) {
        if ((params == NULL) || params->filter_fn(keymap, kmi, params->user_data)) {
          return kmi;
        }
      }
    }
  }
  return NULL;
}

static wmKeyMapItem *wm_keymap_item_find_handlers(const kContext *C,
                                                  wmWindowManager *wm,
                                                  wmWindow *win,
                                                  ListBase *handlers,
                                                  const TfToken &opname,
                                                  eWmOperatorContext UNUSED(opcontext),
                                                  IDProperty *properties,
                                                  const bool is_strict,
                                                  const struct wmKeyMapItemFind_Params *params,
                                                  wmKeyMap **r_keymap)
{
  /* find keymap item in handlers */
  LISTBASE_FOREACH(wmEventHandler *, handler_base, handlers)
  {
    if (handler_base->type == WM_HANDLER_TYPE_KEYMAP) {
      wmEventHandler_Keymap *handler = (wmEventHandler_Keymap *)handler_base;
      wmEventHandler_KeymapResult km_result;
      WM_event_get_keymaps_from_handler(wm, win, handler, &km_result);
      for (int km_index = 0; km_index < km_result.keymaps_len; km_index++) {
        wmKeyMap *keymap = km_result.keymaps[km_index];
        if (WM_keymap_poll((kContext *)C, keymap)) {
          wmKeyMapItem *kmi =
            wm_keymap_item_find_in_keymap(keymap, opname, properties, is_strict, params);
          if (kmi != NULL) {
            if (r_keymap) {
              *r_keymap = keymap;
            }
            return kmi;
          }
        }
      }
    }
  }
  /* ensure un-initialized keymap is never used */
  if (r_keymap) {
    *r_keymap = NULL;
  }
  return NULL;
}

static wmKeyMapItem *wm_keymap_item_find_props(const kContext *C,
                                               const TfToken &opname,
                                               eWmOperatorContext opcontext,
                                               IDProperty *properties,
                                               const bool is_strict,
                                               const struct wmKeyMapItemFind_Params *params,
                                               wmKeyMap **r_keymap)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win = CTX_wm_window(C);
  ScrArea *area = CTX_wm_area(C);
  ARegion *region = CTX_wm_region(C);
  wmKeyMapItem *found = NULL;

  /* look into multiple handler lists to find the item */
  if (win) {
    found = wm_keymap_item_find_handlers(C,
                                         wm,
                                         win,
                                         &win->modalhandlers,
                                         opname,
                                         opcontext,
                                         properties,
                                         is_strict,
                                         params,
                                         r_keymap);
    if (found == NULL) {
      found = wm_keymap_item_find_handlers(C,
                                           wm,
                                           win,
                                           &win->handlers,
                                           opname,
                                           opcontext,
                                           properties,
                                           is_strict,
                                           params,
                                           r_keymap);
    }
  }

  if (area && found == NULL) {
    found = wm_keymap_item_find_handlers(C,
                                         wm,
                                         win,
                                         &area->handlers,
                                         opname,
                                         opcontext,
                                         properties,
                                         is_strict,
                                         params,
                                         r_keymap);
  }

  if (found == NULL) {
    if (ELEM(opcontext, WM_OP_EXEC_REGION_WIN, WM_OP_INVOKE_REGION_WIN)) {
      if (area) {
        if (!(region && region->regiontype == RGN_TYPE_WINDOW)) {
          region = KKE_area_find_region_type(area, RGN_TYPE_WINDOW);
        }

        if (region) {
          found = wm_keymap_item_find_handlers(C,
                                               wm,
                                               win,
                                               &region->handlers,
                                               opname,
                                               opcontext,
                                               properties,
                                               is_strict,
                                               params,
                                               r_keymap);
        }
      }
    } else if (ELEM(opcontext, WM_OP_EXEC_REGION_CHANNELS, WM_OP_INVOKE_REGION_CHANNELS)) {
      if (!(region && region->regiontype == RGN_TYPE_CHANNELS)) {
        region = KKE_area_find_region_type(area, RGN_TYPE_CHANNELS);
      }

      if (region) {
        found = wm_keymap_item_find_handlers(C,
                                             wm,
                                             win,
                                             &region->handlers,
                                             opname,
                                             opcontext,
                                             properties,
                                             is_strict,
                                             params,
                                             r_keymap);
      }
    } else if (ELEM(opcontext, WM_OP_EXEC_REGION_PREVIEW, WM_OP_INVOKE_REGION_PREVIEW)) {
      if (!(region && region->regiontype == RGN_TYPE_PREVIEW)) {
        region = KKE_area_find_region_type(area, RGN_TYPE_PREVIEW);
      }

      if (region) {
        found = wm_keymap_item_find_handlers(C,
                                             wm,
                                             win,
                                             &region->handlers,
                                             opname,
                                             opcontext,
                                             properties,
                                             is_strict,
                                             params,
                                             r_keymap);
      }
    } else {
      if (region) {
        found = wm_keymap_item_find_handlers(C,
                                             wm,
                                             win,
                                             &region->handlers,
                                             opname,
                                             opcontext,
                                             properties,
                                             is_strict,
                                             params,
                                             r_keymap);
      }
    }
  }

  return found;
}

static wmKeyMapItem *wm_keymap_item_find(const kContext *C,
                                         const TfToken &opname,
                                         eWmOperatorContext opcontext,
                                         IDProperty *properties,
                                         bool is_strict,
                                         const struct wmKeyMapItemFind_Params *params,
                                         wmKeyMap **r_keymap)
{
  /* XXX Hack! Macro operators in menu entry have their whole props defined,
   * which is not the case for relevant keymap entries.
   * Could be good to check and harmonize this,
   * but for now always compare non-strict in this case. */
  wmOperatorType *ot = WM_operatortype_find(opname);
  if (ot) {
    is_strict = is_strict && ((ot->flag & OPTYPE_MACRO) == 0);
  }

  wmKeyMapItem *found =
    wm_keymap_item_find_props(C, opname, opcontext, properties, is_strict, params, r_keymap);

  /* This block is *only* useful in one case: when op uses an enum menu in its prop member
   * (then, we want to rerun a comparison with that 'prop' unset). Note this remains brittle,
   * since now any enum prop may be used in UI (specified by name), ot->prop is not so much used...
   * Otherwise:
   *     * If non-strict, unset properties always match set ones in IDP_EqualsProperties_ex.
   *     * If strict, unset properties never match set ones in IDP_EqualsProperties_ex,
   *       and we do not want that to change (else we get things like T41757)!
   * ...so in either case, re-running a comparison with unset props set to default is useless.
   */
  // if (!found && properties) {
  //   if (ot && ot->prop) { /* XXX Shall we also check ot->prop is actually an enum? */
  //     /* make a copy of the properties and unset the 'ot->prop' one if set. */
  //     KrakenPRIM opptr;
  //     IDProperty *properties_temp = IDP_CopyProperty(properties);

  //     LUXO_pointer_create(NULL, &ot->prim, properties_temp, &opptr);

  //     if (LUXO_property_is_set(&opptr, ot->prop)) {
  //       /* For operator that has enum menu,
  //        * unset it so its value does not affect comparison result. */
  //       LUXO_property_unset(&opptr, ot->prop);

  //       found = wm_keymap_item_find_props(C,
  //                                         opname,
  //                                         opcontext,
  //                                         properties_temp,
  //                                         is_strict,
  //                                         params,
  //                                         r_keymap);
  //     }

  //     IDP_FreeProperty(properties_temp);
  //   }
  // }

  return found;
}

char *WM_key_event_operator_string(const kContext *C,
                                   const TfToken &opname,
                                   eWmOperatorContext opcontext,
                                   IDProperty *properties,
                                   const bool is_strict,
                                   char *result,
                                   const int result_len)
{
  const wmKeyMapItemFind_Params params = {
    .filter_fn = kmi_filter_is_visible,
    .user_data = NULL,
  };

  wmKeyMapItem *kmi =
    wm_keymap_item_find(C, opname, opcontext, properties, is_strict, &params, NULL);
  if (kmi) {
    WM_keymap_item_to_string(kmi, false, result, result_len);
    return result;
  }

  return NULL;
}

int WM_keymap_item_to_string(const wmKeyMapItem *kmi,
                             const bool compact,
                             char *result,
                             const int result_len)
{
  return WM_keymap_item_raw_to_string(kmi->shift,
                                      kmi->ctrl,
                                      kmi->alt,
                                      kmi->oskey,
                                      kmi->keymodifier,
                                      kmi->val,
                                      kmi->type,
                                      compact,
                                      result,
                                      result_len);
}
