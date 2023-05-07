/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2001-2002 NaN Holding BV. All rights reserved. */

/** 
 * @file
 * \ingroup edinterface
 */

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "MEM_guardedalloc.h"

#include "GPU_batch.h"
#include "GPU_batch_presets.h"
#include "GPU_immediate.h"
#include "GPU_matrix.h"
#include "GPU_shader_shared.h"
#include "GPU_state.h"
#include "GPU_texture.h"

#include "KLI_kraklib.h"
#include "KLI_fileops_types.h"
#include "KLI_math_color_blend.h"
#include "KLI_math_vector.h"
#include "KLI_utildefines.h"

#include "USD_ID_enums.h"
#include "USD_wm_types.h"
#include "USD_space_types.h"
#include "USD_userdef_types.h"
#include "USD_collection.h"

#include "LUXO_access.h"
// #include "LUXO_prototypes.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_icons.h"
// #include "KKE_paint.h"
// #include "KKE_studiolight.h"

#include "IMB_imbuf.h"

#include "ED_datafiles.h"
// #include "ED_render.h"

#include "UI_interface.h"
#include "UI_interface_icons.h"

#include "WM_api.h"
// #include "WM_types.h"

#include "interface_intern.h"

#ifndef WITH_HEADLESS
#  define ICON_GRID_COLS 26
#  define ICON_GRID_ROWS 30

#  define ICON_MONO_BORDER_OUTSET 2
#  define ICON_GRID_MARGIN 10
#  define ICON_GRID_W 32
#  define ICON_GRID_H 32
#endif /* WITH_HEADLESS */

struct IconImage
{
  int w;
  int h;
  uint *rect;
  const uchar *datatoc_rect;
  int datatoc_size;
};

using VectorDrawFunc = void (*)(int x, int y, int w, int h, float alpha);

#define ICON_TYPE_PREVIEW 0
#define ICON_TYPE_COLOR_TEXTURE 1
#define ICON_TYPE_MONO_TEXTURE 2
#define ICON_TYPE_BUFFER 3
#define ICON_TYPE_IMBUF 4
#define ICON_TYPE_VECTOR 5
#define ICON_TYPE_GEOM 6
#define ICON_TYPE_EVENT 7 /* draw keymap entries using custom renderer. */
#define ICON_TYPE_GPLAYER 8
#define ICON_TYPE_BLANK 9

struct DrawInfo 
{
  int type;

  union 
  {
    /* type specific data */
    struct 
    {
      VectorDrawFunc func;
    } vector;
    struct 
    {
      ImBuf *image_cache;
      bool inverted;
    } geom;
    struct 
    {
      IconImage *image;
    } buffer;
    struct 
    {
      int x, y, w, h;
      int theme_color;
    } texture;
    struct 
    {
      /* Can be packed into a single int. */
      short event_type;
      short event_value;
      int icon;
      /* Allow lookups. */
      struct DrawInfo *next;
    } input;
  } data;
};

struct IconTexture 
{
  GPUTexture *tex[2];
  int num_textures;
  int w;
  int h;
  float invw;
  float invh;
};

struct IconType 
{
  int type;
  int theme_color;
};

/* ******************* STATIC LOCAL VARS ******************* */
/* Static here to cache results of icon directory scan, so it's not
 * scanning the file-system each time the menu is drawn. */
static ListBase iconfilelist = {nullptr, nullptr};
static IconTexture icongltex = {{nullptr, nullptr}, 0, 0, 0, 0.0f, 0.0f};

static const IconType icontypes[] = {
#  define DEF_ICON(name) {ICON_TYPE_MONO_TEXTURE, 0},
#  define DEF_ICON_SCENE(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_SCENE},
#  define DEF_ICON_COLLECTION(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_COLLECTION},
#  define DEF_ICON_OBJECT(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_OBJECT},
#  define DEF_ICON_OBJECT_DATA(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_OBJECT_DATA},
#  define DEF_ICON_MODIFIER(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_MODIFIER},
#  define DEF_ICON_SHADING(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_SHADING},
#  define DEF_ICON_FOLDER(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_FOLDER},
#  define DEF_ICON_FUND(name) {ICON_TYPE_MONO_TEXTURE, TH_ICON_FUND},
#  define DEF_ICON_VECTOR(name) {ICON_TYPE_VECTOR, 0},
#  define DEF_ICON_COLOR(name) {ICON_TYPE_COLOR_TEXTURE, 0},
#  define DEF_ICON_BLANK(name) {ICON_TYPE_BLANK, 0},
#  include "UI_icons.h"
};

/* **************************************************** */

static DrawInfo *def_internal_icon(ImBuf *bbuf, int icon_id, int xofs, int yofs, int size, int type, int theme_color)
{
  Icon *new_icon = MEM_cnew<Icon>(__func__);

  new_icon->obj = nullptr; /* icon is not for library object */
  new_icon->id_type = 0;

  DrawInfo *di = MEM_cnew<DrawInfo>(__func__);
  di->type = type;

  if (ELEM(type, ICON_TYPE_COLOR_TEXTURE, ICON_TYPE_MONO_TEXTURE)) {
    di->data.texture.theme_color = theme_color;
    di->data.texture.x = xofs;
    di->data.texture.y = yofs;
    di->data.texture.w = size;
    di->data.texture.h = size;
  }
  else if (type == ICON_TYPE_BUFFER) {
    IconImage *iimg = MEM_cnew<IconImage>(__func__);
    iimg->w = size;
    iimg->h = size;

    /* icon buffers can get initialized runtime now, via datatoc */
    if (bbuf) {
      int y, imgsize;

      iimg->rect = static_cast<uint *>(MEM_mallocN(size * size * sizeof(uint), __func__));

      /* Here we store the rect in the icon - same as before */
      if (size == bbuf->x && size == bbuf->y && xofs == 0 && yofs == 0) {
        memcpy(iimg->rect, bbuf->rect, size * size * sizeof(int));
      }
      else {
        /* this code assumes square images */
        imgsize = bbuf->x;
        for (y = 0; y < size; y++) {
          memcpy(&iimg->rect[y * size], &bbuf->rect[(y + yofs) * imgsize + xofs], size * sizeof(int));
        }
      }
    }
    di->data.buffer.image = iimg;
  }

  new_icon->drawinfo_free = UI_icons_free_drawinfo;
  new_icon->drawinfo = di;

  KKE_icon_set(icon_id, new_icon);

  return di;
}

static void def_internal_vicon(int icon_id, VectorDrawFunc drawFunc)
{
  Icon *new_icon = MEM_cnew<Icon>("texicon");

  new_icon->obj = nullptr; /* icon is not for library object */
  new_icon->id_type = 0;

  DrawInfo *di = MEM_cnew<DrawInfo>("drawinfo");
  di->type = ICON_TYPE_VECTOR;
  di->data.vector.func = drawFunc;

  new_icon->drawinfo_free = nullptr;
  new_icon->drawinfo = di;

  KKE_icon_set(icon_id, new_icon);
}

/* Vector Icon Drawing Routines */

/* Utilities */

static void vicon_colorset_draw(int index, int x, int y, int w, int h, float UNUSED(alpha))
{
  kTheme *ktheme = UI_GetTheme();
  const ThemeWireColor *cs = &ktheme->tarm[index];

  /* Draw three bands of color: One per color
   *    x-----a-----b-----c
   *    |  N  |  S  |  A  |
   *    x-----a-----b-----c
   */
  const int a = x + w / 3;
  const int b = x + w / 3 * 2;
  const int c = x + w;

  uint pos =
    GPU_vertformat_attr_add(immVertexFormat(), "pos", GPU_COMP_I32, 2, GPU_FETCH_INT_TO_FLOAT);
  immBindBuiltinProgram(GPU_SHADER_3D_UNIFORM_COLOR);

  /* XXX: Include alpha into this... */
  /* normal */
  immUniformColor3ubv(cs->solid);
  immRecti(pos, x, y, a, y + h);

  /* selected */
  immUniformColor3ubv(cs->select);
  immRecti(pos, a, y, b, y + h);

  /* active */
  immUniformColor3ubv(cs->active);
  immRecti(pos, b, y, c, y + h);

  immUnbindProgram();
}

#define DEF_ICON_VECTOR_COLORSET_DRAW_NTH(prefix, index)                            \
  static void vicon_colorset_draw_##prefix(int x, int y, int w, int h, float alpha) \
  {                                                                                 \
    vicon_colorset_draw(index, x, y, w, h, alpha);                                  \
  }

DEF_ICON_VECTOR_COLORSET_DRAW_NTH(01, 0)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(02, 1)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(03, 2)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(04, 3)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(05, 4)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(06, 5)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(07, 6)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(08, 7)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(09, 8)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(10, 9)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(11, 10)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(12, 11)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(13, 12)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(14, 13)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(15, 14)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(16, 15)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(17, 16)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(18, 17)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(19, 18)
DEF_ICON_VECTOR_COLORSET_DRAW_NTH(20, 19)

#undef DEF_ICON_VECTOR_COLORSET_DRAW_NTH

static void vicon_collection_color_draw(short color_tag,
                                        int x,
                                        int y,
                                        int w,
                                        int UNUSED(h),
                                        float UNUSED(alpha))
{
  kTheme *ktheme = UI_GetTheme();
  const ThemeCollectionColor *collection_color = &ktheme->collection_color[color_tag];

  const float aspect = (float)ICON_DEFAULT_WIDTH / (float)w;

  UI_icon_draw_ex(x,
                  y,
                  ICON_OUTLINER_COLLECTION,
                  aspect,
                  1.0f,
                  0.0f,
                  collection_color->color,
                  true);
}

#define DEF_ICON_COLLECTION_COLOR_DRAW(index, color)                                       \
  static void vicon_collection_color_draw_##index(int x, int y, int w, int h, float alpha) \
  {                                                                                        \
    vicon_collection_color_draw(color, x, y, w, h, alpha);                                 \
  }

DEF_ICON_COLLECTION_COLOR_DRAW(01, COLLECTION_COLOR_01);
DEF_ICON_COLLECTION_COLOR_DRAW(02, COLLECTION_COLOR_02);
DEF_ICON_COLLECTION_COLOR_DRAW(03, COLLECTION_COLOR_03);
DEF_ICON_COLLECTION_COLOR_DRAW(04, COLLECTION_COLOR_04);
DEF_ICON_COLLECTION_COLOR_DRAW(05, COLLECTION_COLOR_05);
DEF_ICON_COLLECTION_COLOR_DRAW(06, COLLECTION_COLOR_06);
DEF_ICON_COLLECTION_COLOR_DRAW(07, COLLECTION_COLOR_07);
DEF_ICON_COLLECTION_COLOR_DRAW(08, COLLECTION_COLOR_08);

#undef DEF_ICON_COLLECTION_COLOR_DRAW

static void vicon_strip_color_draw(short color_tag,
                                   int x,
                                   int y,
                                   int w,
                                   int UNUSED(h),
                                   float UNUSED(alpha))
{
  kTheme *ktheme = UI_GetTheme();
  const ThemeStripColor *strip_color = &ktheme->strip_color[color_tag];

  const float aspect = (float)ICON_DEFAULT_WIDTH / (float)w;

  UI_icon_draw_ex(x, y, ICON_SNAP_FACE, aspect, 1.0f, 0.0f, strip_color->color, true);
}

#define DEF_ICON_STRIP_COLOR_DRAW(index, color)                                       \
  static void vicon_strip_color_draw_##index(int x, int y, int w, int h, float alpha) \
  {                                                                                   \
    vicon_strip_color_draw(color, x, y, w, h, alpha);                                 \
  }

DEF_ICON_STRIP_COLOR_DRAW(01, SEQUENCE_COLOR_01);
DEF_ICON_STRIP_COLOR_DRAW(02, SEQUENCE_COLOR_02);
DEF_ICON_STRIP_COLOR_DRAW(03, SEQUENCE_COLOR_03);
DEF_ICON_STRIP_COLOR_DRAW(04, SEQUENCE_COLOR_04);
DEF_ICON_STRIP_COLOR_DRAW(05, SEQUENCE_COLOR_05);
DEF_ICON_STRIP_COLOR_DRAW(06, SEQUENCE_COLOR_06);
DEF_ICON_STRIP_COLOR_DRAW(07, SEQUENCE_COLOR_07);
DEF_ICON_STRIP_COLOR_DRAW(08, SEQUENCE_COLOR_08);
DEF_ICON_STRIP_COLOR_DRAW(09, SEQUENCE_COLOR_09);

#undef DEF_ICON_STRIP_COLOR_DRAW

#define ICON_INDIRECT_DATA_ALPHA 0.6f

static void vicon_strip_color_draw_library_data_indirect(int x,
                                                         int y,
                                                         int w,
                                                         int UNUSED(h),
                                                         float alpha)
{
  const float aspect = (float)ICON_DEFAULT_WIDTH / (float)w;

  UI_icon_draw_ex(x,
                  y,
                  ICON_LIBRARY_DATA_DIRECT,
                  aspect,
                  ICON_INDIRECT_DATA_ALPHA * alpha,
                  0.0f,
                  NULL,
                  false);
}

static void vicon_strip_color_draw_library_data_override_noneditable(int x,
                                                                     int y,
                                                                     int w,
                                                                     int UNUSED(h),
                                                                     float alpha)
{
  const float aspect = (float)ICON_DEFAULT_WIDTH / (float)w;

  UI_icon_draw_ex(x,
                  y,
                  ICON_LIBRARY_DATA_OVERRIDE,
                  aspect,
                  ICON_INDIRECT_DATA_ALPHA * alpha * 0.75f,
                  0.0f,
                  NULL,
                  false);
}

static void init_internal_icons(void)
{
  /* Define icons. */
  for (int y = 0; y < ICON_GRID_ROWS; y++) {
    /* Row W has monochrome icons. */
    for (int x = 0; x < ICON_GRID_COLS; x++) {
      const IconType icontype = icontypes[y * ICON_GRID_COLS + x];
      if (!ELEM(icontype.type, ICON_TYPE_COLOR_TEXTURE, ICON_TYPE_MONO_TEXTURE)) {
        continue;
      }

      def_internal_icon(NULL,
                        KIFICONID_FIRST + y * ICON_GRID_COLS + x,
                        x * (ICON_GRID_W + ICON_GRID_MARGIN) + ICON_GRID_MARGIN,
                        y * (ICON_GRID_H + ICON_GRID_MARGIN) + ICON_GRID_MARGIN,
                        ICON_GRID_W,
                        icontype.type,
                        icontype.theme_color);
    }
  }

  def_internal_vicon(ICON_COLORSET_01_VEC, vicon_colorset_draw_01);
  def_internal_vicon(ICON_COLORSET_02_VEC, vicon_colorset_draw_02);
  def_internal_vicon(ICON_COLORSET_03_VEC, vicon_colorset_draw_03);
  def_internal_vicon(ICON_COLORSET_04_VEC, vicon_colorset_draw_04);
  def_internal_vicon(ICON_COLORSET_05_VEC, vicon_colorset_draw_05);
  def_internal_vicon(ICON_COLORSET_06_VEC, vicon_colorset_draw_06);
  def_internal_vicon(ICON_COLORSET_07_VEC, vicon_colorset_draw_07);
  def_internal_vicon(ICON_COLORSET_08_VEC, vicon_colorset_draw_08);
  def_internal_vicon(ICON_COLORSET_09_VEC, vicon_colorset_draw_09);
  def_internal_vicon(ICON_COLORSET_10_VEC, vicon_colorset_draw_10);
  def_internal_vicon(ICON_COLORSET_11_VEC, vicon_colorset_draw_11);
  def_internal_vicon(ICON_COLORSET_12_VEC, vicon_colorset_draw_12);
  def_internal_vicon(ICON_COLORSET_13_VEC, vicon_colorset_draw_13);
  def_internal_vicon(ICON_COLORSET_14_VEC, vicon_colorset_draw_14);
  def_internal_vicon(ICON_COLORSET_15_VEC, vicon_colorset_draw_15);
  def_internal_vicon(ICON_COLORSET_16_VEC, vicon_colorset_draw_16);
  def_internal_vicon(ICON_COLORSET_17_VEC, vicon_colorset_draw_17);
  def_internal_vicon(ICON_COLORSET_18_VEC, vicon_colorset_draw_18);
  def_internal_vicon(ICON_COLORSET_19_VEC, vicon_colorset_draw_19);
  def_internal_vicon(ICON_COLORSET_20_VEC, vicon_colorset_draw_20);

  def_internal_vicon(ICON_COLLECTION_COLOR_01, vicon_collection_color_draw_01);
  def_internal_vicon(ICON_COLLECTION_COLOR_02, vicon_collection_color_draw_02);
  def_internal_vicon(ICON_COLLECTION_COLOR_03, vicon_collection_color_draw_03);
  def_internal_vicon(ICON_COLLECTION_COLOR_04, vicon_collection_color_draw_04);
  def_internal_vicon(ICON_COLLECTION_COLOR_05, vicon_collection_color_draw_05);
  def_internal_vicon(ICON_COLLECTION_COLOR_06, vicon_collection_color_draw_06);
  def_internal_vicon(ICON_COLLECTION_COLOR_07, vicon_collection_color_draw_07);
  def_internal_vicon(ICON_COLLECTION_COLOR_08, vicon_collection_color_draw_08);

  def_internal_vicon(ICON_SEQUENCE_COLOR_01, vicon_strip_color_draw_01);
  def_internal_vicon(ICON_SEQUENCE_COLOR_02, vicon_strip_color_draw_02);
  def_internal_vicon(ICON_SEQUENCE_COLOR_03, vicon_strip_color_draw_03);
  def_internal_vicon(ICON_SEQUENCE_COLOR_04, vicon_strip_color_draw_04);
  def_internal_vicon(ICON_SEQUENCE_COLOR_05, vicon_strip_color_draw_05);
  def_internal_vicon(ICON_SEQUENCE_COLOR_06, vicon_strip_color_draw_06);
  def_internal_vicon(ICON_SEQUENCE_COLOR_07, vicon_strip_color_draw_07);
  def_internal_vicon(ICON_SEQUENCE_COLOR_08, vicon_strip_color_draw_08);
  def_internal_vicon(ICON_SEQUENCE_COLOR_09, vicon_strip_color_draw_09);

  def_internal_vicon(ICON_LIBRARY_DATA_INDIRECT, vicon_strip_color_draw_library_data_indirect);
  def_internal_vicon(ICON_LIBRARY_DATA_OVERRIDE_NONEDITABLE,
                     vicon_strip_color_draw_library_data_override_noneditable);
}

static void init_iconfile_list(ListBase *list)
{
  KLI_listbase_clear(list);
  const char *icondir = KKE_appdir_folder_id(KRAKEN_DATAFILES, "icons");

  if (icondir == nullptr) {
    return;
  }

  direntry *dir;
  const int totfile = KLI_filelist_dir_contents(icondir, &dir);

  int index = 1;
  for (int i = 0; i < totfile; i++) {
    if (dir[i].type & S_IFREG) {
      const char *filename = dir[i].relname;

      if (KLI_path_extension_check(filename, ".png")) {
        /* loading all icons on file start is overkill & slows startup
         * its possible they change size after kraken loads anyway. */

        /* found a potential icon file, so make an entry for it in the cache list */
        IconFile *ifile = MEM_cnew<IconFile>(__func__);

        KLI_strncpy(ifile->filename, filename, sizeof(ifile->filename));
        ifile->index = index;

        KLI_addtail(list, ifile);

        index++;
      }
    }
  }

  KLI_filelist_free(dir, totfile);
  dir = nullptr;
}

static DrawInfo *g_di_event_list = NULL;

int UI_icon_from_event_type(short event_type, short event_value)
{
  if (event_type == EVT_RIGHTSHIFTKEY) {
    event_type = EVT_LEFTSHIFTKEY;
  } else if (event_type == EVT_RIGHTCTRLKEY) {
    event_type = EVT_LEFTCTRLKEY;
  } else if (event_type == EVT_RIGHTALTKEY) {
    event_type = EVT_LEFTALTKEY;
  }

  DrawInfo *di = g_di_event_list;
  do {
    if (di->data.input.event_type == event_type) {
      return di->data.input.icon;
    }
  } while ((di = di->data.input.next));

  if (event_type == LEFTMOUSE) {
    return ELEM(event_value, KM_CLICK, KM_PRESS) ? ICON_MOUSE_LMB : ICON_MOUSE_LMB_DRAG;
  }
  if (event_type == MIDDLEMOUSE) {
    return ELEM(event_value, KM_CLICK, KM_PRESS) ? ICON_MOUSE_MMB : ICON_MOUSE_MMB_DRAG;
  }
  if (event_type == RIGHTMOUSE) {
    return ELEM(event_value, KM_CLICK, KM_PRESS) ? ICON_MOUSE_RMB : ICON_MOUSE_RMB_DRAG;
  }

  return ICON_NONE;
}

int UI_icon_from_keymap_item(const wmKeyMapItem *kmi, int r_icon_mod[4])
{
  if (r_icon_mod) {
    memset(r_icon_mod, 0x0, sizeof(int[4]));
    int i = 0;
    if (!ELEM(kmi->ctrl, KM_NOTHING, KM_ANY)) {
      r_icon_mod[i++] = ICON_EVENT_CTRL;
    }
    if (!ELEM(kmi->alt, KM_NOTHING, KM_ANY)) {
      r_icon_mod[i++] = ICON_EVENT_ALT;
    }
    if (!ELEM(kmi->shift, KM_NOTHING, KM_ANY)) {
      r_icon_mod[i++] = ICON_EVENT_SHIFT;
    }
    if (!ELEM(kmi->oskey, KM_NOTHING, KM_ANY)) {
      r_icon_mod[i++] = ICON_EVENT_OS;
    }
  }
  return UI_icon_from_event_type(kmi->type, kmi->val);
}

static void init_event_icons(void)
{
  DrawInfo *di_next = NULL;

#define INIT_EVENT_ICON(icon_id, type, value)                                     \
  {                                                                               \
    DrawInfo *di = def_internal_icon(NULL, icon_id, 0, 0, w, ICON_TYPE_EVENT, 0); \
    di->data.input.event_type = type;                                             \
    di->data.input.event_value = value;                                           \
    di->data.input.icon = icon_id;                                                \
    di->data.input.next = di_next;                                                \
    di_next = di;                                                                 \
  }                                                                               \
  ((void)0)
  /* end INIT_EVENT_ICON */

  const int w = 16; /* DUMMY */

  INIT_EVENT_ICON(ICON_EVENT_A, EVT_AKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_B, EVT_BKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_C, EVT_CKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_D, EVT_DKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_E, EVT_EKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F, EVT_FKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_G, EVT_GKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_H, EVT_HKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_I, EVT_IKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_J, EVT_JKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_K, EVT_KKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_L, EVT_LKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_M, EVT_MKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_N, EVT_NKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_O, EVT_OKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_P, EVT_PKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_Q, EVT_QKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_R, EVT_RKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_S, EVT_SKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_T, EVT_TKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_U, EVT_UKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_V, EVT_VKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_W, EVT_WKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_X, EVT_XKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_Y, EVT_YKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_Z, EVT_ZKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_SHIFT, EVT_LEFTSHIFTKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_CTRL, EVT_LEFTCTRLKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_ALT, EVT_LEFTALTKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_OS, EVT_OSKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F1, EVT_F1KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F2, EVT_F2KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F3, EVT_F3KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F4, EVT_F4KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F5, EVT_F5KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F6, EVT_F6KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F7, EVT_F7KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F8, EVT_F8KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F9, EVT_F9KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F10, EVT_F10KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F11, EVT_F11KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_F12, EVT_F12KEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_ESC, EVT_ESCKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_TAB, EVT_TABKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_PAGEUP, EVT_PAGEUPKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_PAGEDOWN, EVT_PAGEDOWNKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_RETURN, EVT_RETKEY, KM_ANY);
  INIT_EVENT_ICON(ICON_EVENT_SPACEKEY, EVT_SPACEKEY, KM_ANY);

  g_di_event_list = di_next;

#undef INIT_EVENT_ICON
}

static void icon_verify_datatoc(IconImage *iimg)
{
  /* if it has own rect, things are all OK */
  if (iimg->rect) {
    return;
  }

  if (iimg->datatoc_rect) {
    ImBuf *bbuf = IMB_ibImageFromMemory(iimg->datatoc_rect,
                                        iimg->datatoc_size,
                                        IB_rect,
                                        NULL,
                                        "<matcap icon>");
    /* w and h were set on initialize */
    if (bbuf->x != iimg->h && bbuf->y != iimg->w) {
      // IMB_scaleImBuf(bbuf, iimg->w, iimg->h);
    }

    iimg->rect = bbuf->rect;
    bbuf->rect = NULL;
    // IMB_freeImBuf(bbuf);
  }
}

/**
 * @TODO: Need to define IMB_dupImBuf before we can call this.
 */
static ImBuf *create_mono_icon_with_border(ImBuf *buf,
                                           int resolution_divider,
                                           float border_intensity)
{
  ImBuf *result = IMB_dupImBuf(buf);
  const float border_sharpness = 16.0 / (resolution_divider * resolution_divider);

  float blurred_alpha_buffer[(ICON_GRID_W + 2 * ICON_MONO_BORDER_OUTSET) *
                             (ICON_GRID_H + 2 * ICON_MONO_BORDER_OUTSET)];
  const int icon_width = (ICON_GRID_W + 2 * ICON_MONO_BORDER_OUTSET) / resolution_divider;
  const int icon_height = (ICON_GRID_W + 2 * ICON_MONO_BORDER_OUTSET) / resolution_divider;

  for (int y = 0; y < ICON_GRID_ROWS; y++) {
    for (int x = 0; x < ICON_GRID_COLS; x++) {
      const IconType icontype = icontypes[y * ICON_GRID_COLS + x];
      if (icontype.type != ICON_TYPE_MONO_TEXTURE) {
        continue;
      }

      int sx = x * (ICON_GRID_W + ICON_GRID_MARGIN) + ICON_GRID_MARGIN - ICON_MONO_BORDER_OUTSET;
      int sy = y * (ICON_GRID_H + ICON_GRID_MARGIN) + ICON_GRID_MARGIN - ICON_MONO_BORDER_OUTSET;
      sx = sx / resolution_divider;
      sy = sy / resolution_divider;

      /* blur the alpha channel and store it in blurred_alpha_buffer */
      const int blur_size = 2 / resolution_divider;
      for (int bx = 0; bx < icon_width; bx++) {
        const int asx = MAX2(bx - blur_size, 0);
        const int aex = MIN2(bx + blur_size + 1, icon_width);
        for (int by = 0; by < icon_height; by++) {
          const int asy = MAX2(by - blur_size, 0);
          const int aey = MIN2(by + blur_size + 1, icon_height);

          /* blur alpha channel */
          const int write_offset = by * (ICON_GRID_W + 2 * ICON_MONO_BORDER_OUTSET) + bx;
          float alpha_accum = 0.0;
          uint alpha_samples = 0;
          for (int ax = asx; ax < aex; ax++) {
            for (int ay = asy; ay < aey; ay++) {
              const int offset_read = (sy + ay) * buf->x + (sx + ax);
              const uint color_read = buf->rect[offset_read];
              const float alpha_read = ((color_read & 0xff000000) >> 24) / 255.0;
              alpha_accum += alpha_read;
              alpha_samples += 1;
            }
          }
          blurred_alpha_buffer[write_offset] = alpha_accum / alpha_samples;
        }
      }

      /* apply blurred alpha */
      for (int bx = 0; bx < icon_width; bx++) {
        for (int by = 0; by < icon_height; by++) {
          const int blurred_alpha_offset = by * (ICON_GRID_W + 2 * ICON_MONO_BORDER_OUTSET) + bx;
          const int offset_write = (sy + by) * buf->x + (sx + bx);
          const float blurred_alpha = blurred_alpha_buffer[blurred_alpha_offset];
          const float border_srgb[4] = {
              0, 0, 0, MIN2(1.0f, blurred_alpha * border_sharpness) * border_intensity};

          const uint color_read = buf->rect[offset_write];
          const uchar *orig_color = (uchar *)&color_read;

          float border_rgba[4];
          float orig_rgba[4];
          float dest_rgba[4];
          float dest_srgb[4];

          srgb_to_linearrgb_v4(border_rgba, border_srgb);
          srgb_to_linearrgb_uchar4(orig_rgba, orig_color);
          blend_color_interpolate_float(dest_rgba, orig_rgba, border_rgba, 1.0 - orig_rgba[3]);
          linearrgb_to_srgb_v4(dest_srgb, dest_rgba);

          const uint alpha_mask = uint(dest_srgb[3] * 255) << 24;
          const uint cpack = rgb_to_cpack(dest_srgb[0], dest_srgb[1], dest_srgb[2]) | alpha_mask;
          result->rect[offset_write] = cpack;
        }
      }
    }
  }
  return result;
}

static void free_icons_textures(void)
{
  if (icongltex.num_textures > 0) {
    for (int i = 0; i < 2; i++) {
      if (icongltex.tex[i]) {
        GPU_texture_free(icongltex.tex[i]);
        icongltex.tex[i] = NULL;
      }
    }
    icongltex.num_textures = 0;
  }
}

static void free_iconfile_list(struct ListBase *list)
{
  LISTBASE_FOREACH_MUTABLE (IconFile *, ifile, &iconfilelist) {
    KLI_freelinkN(list, ifile);
  }
}

void UI_icons_reload_internal_textures(void)
{
  kTheme *ktheme = UI_GetTheme();
  ImBuf *b16buf = NULL, *b32buf = NULL, *b16buf_border = NULL, *b32buf_border = NULL;
  const float icon_border_intensity = ktheme->tui.icon_border_intensity;
  const bool need_icons_with_border = icon_border_intensity > 0.0f;

  if (b16buf == NULL) {
    b16buf = IMB_ibImageFromMemory((const uchar *)datatoc_kraken_icons16_png,
                                   datatoc_kraken_icons16_png_size,
                                   IB_rect,
                                   NULL,
                                   "<kraken icons>");
  }
  if (b16buf) {
    if (need_icons_with_border) {
      b16buf_border = create_mono_icon_with_border(b16buf, 2, icon_border_intensity);
      // IMB_premultiply_alpha(b16buf_border);
    }
    // IMB_premultiply_alpha(b16buf);
  }

  if (b32buf == NULL) {
    b32buf = IMB_ibImageFromMemory((const uchar *)datatoc_kraken_icons32_png,
                                   datatoc_kraken_icons32_png_size,
                                   IB_rect,
                                   NULL,
                                   "<kraken icons>");
  }
  if (b32buf) {
    if (need_icons_with_border) {
      b32buf_border = create_mono_icon_with_border(b32buf, 1, icon_border_intensity);
      // IMB_premultiply_alpha(b32buf_border);
    }
    // IMB_premultiply_alpha(b32buf);
  }

  if (b16buf && b32buf) {
    /* Free existing texture if any. */
    free_icons_textures();

    /* Allocate OpenGL texture. */
    icongltex.num_textures = need_icons_with_border ? 2 : 1;

    /* Note the filter and LOD bias were tweaked to better preserve icon
     * sharpness at different UI scales. */
    if (icongltex.tex[0] == NULL) {
      icongltex.w = b32buf->x;
      icongltex.h = b32buf->y;
      icongltex.invw = 1.0f / b32buf->x;
      icongltex.invh = 1.0f / b32buf->y;

      icongltex.tex[0] = GPU_texture_create_2d("icons", b32buf->x, b32buf->y, 2, GPU_RGBA8, NULL);
      GPU_texture_update_mipmap(icongltex.tex[0], 0, GPU_DATA_UBYTE, b32buf->rect);
      GPU_texture_update_mipmap(icongltex.tex[0], 1, GPU_DATA_UBYTE, b16buf->rect);
    }

    if (need_icons_with_border && icongltex.tex[1] == NULL) {
      icongltex.tex[1] = GPU_texture_create_2d("icons_border",
                                               b32buf_border->x,
                                               b32buf_border->y,
                                               2,
                                               GPU_RGBA8,
                                               NULL);
      GPU_texture_update_mipmap(icongltex.tex[1], 0, GPU_DATA_UBYTE, b32buf_border->rect);
      GPU_texture_update_mipmap(icongltex.tex[1], 1, GPU_DATA_UBYTE, b16buf_border->rect);
    }
  }

  // IMB_freeImBuf(b16buf);
  // IMB_freeImBuf(b32buf);
  // IMB_freeImBuf(b16buf_border);
  // IMB_freeImBuf(b32buf_border);
}

/* High enough to make a difference, low enough so that
 * small draws are still efficient with the use of glUniform.
 * NOTE TODO: We could use UBO but we would need some triple
 * buffer system + persistent mapping for this to be more
 * efficient than simple glUniform calls. */
#define ICON_DRAW_CACHE_SIZE 16

typedef struct IconDrawCall
{
  rctf pos;
  rctf tex;
  float color[4];
} IconDrawCall;

typedef struct IconTextureDrawCall
{
  IconDrawCall drawcall_cache[ICON_DRAW_CACHE_SIZE];
  int calls; /* Number of calls batched together */
} IconTextureDrawCall;

static struct
{
  IconTextureDrawCall normal;
  IconTextureDrawCall border;
  bool enabled;
} g_icon_draw_cache = {{{{{0}}}}};

void UI_icon_draw_cache_begin(void)
{
  KLI_assert(g_icon_draw_cache.enabled == false);
  g_icon_draw_cache.enabled = true;
}

static void icon_draw_cache_texture_flush_ex(GPUTexture *texture,
                                             IconTextureDrawCall *texture_draw_calls)
{
  if (texture_draw_calls->calls == 0) {
    return;
  }

  GPUShader *shader = GPU_shader_get_builtin_shader(GPU_SHADER_2D_IMAGE_MULTI_RECT_COLOR);
  GPU_shader_bind(shader);

  const int data_binding = GPU_shader_get_uniform_block_binding(shader, "multi_rect_data");
  GPUUniformBuf *ubo = GPU_uniformbuf_create_ex(sizeof(struct MultiRectCallData),
                                                texture_draw_calls->drawcall_cache,
                                                __func__);
  GPU_uniformbuf_bind(ubo, data_binding);

  const int img_binding = GPU_shader_get_texture_binding(shader, "image");
  GPU_texture_bind_ex(texture, GPU_SAMPLER_ICON, img_binding, false);

  GPUBatch *quad = GPU_batch_preset_quad();
  GPU_batch_set_shader(quad, shader);
  GPU_batch_draw_instanced(quad, texture_draw_calls->calls);

  GPU_texture_unbind(texture);
  GPU_uniformbuf_unbind(ubo);
  GPU_uniformbuf_free(ubo);

  texture_draw_calls->calls = 0;
}

static void icon_draw_cache_flush_ex(bool only_full_caches)
{
  bool should_draw = false;
  if (only_full_caches) {
    should_draw = g_icon_draw_cache.normal.calls == ICON_DRAW_CACHE_SIZE ||
                  g_icon_draw_cache.border.calls == ICON_DRAW_CACHE_SIZE;
  } else {
    should_draw = g_icon_draw_cache.normal.calls || g_icon_draw_cache.border.calls;
  }

  if (should_draw) {
    /* We need to flush widget base first to ensure correct ordering. */
    // UI_widgetbase_draw_cache_flush();

    GPU_blend(GPU_BLEND_ALPHA_PREMULT);

    if (!only_full_caches || g_icon_draw_cache.normal.calls == ICON_DRAW_CACHE_SIZE) {
      icon_draw_cache_texture_flush_ex(icongltex.tex[0], &g_icon_draw_cache.normal);
    }

    if (!only_full_caches || g_icon_draw_cache.border.calls == ICON_DRAW_CACHE_SIZE) {
      icon_draw_cache_texture_flush_ex(icongltex.tex[1], &g_icon_draw_cache.border);
    }

    GPU_blend(GPU_BLEND_ALPHA);
  }
}

void UI_icon_draw_cache_end(void)
{
  KLI_assert(g_icon_draw_cache.enabled == true);
  g_icon_draw_cache.enabled = false;

  /* Don't change blend state if it's not needed. */
  if (g_icon_draw_cache.border.calls == 0 && g_icon_draw_cache.normal.calls == 0) {
    return;
  }

  GPU_blend(GPU_BLEND_ALPHA);
  icon_draw_cache_flush_ex(false);
  GPU_blend(GPU_BLEND_NONE);
}

/**
 * #Icon.data_type and #Icon.obj
 */
static DrawInfo *icon_create_drawinfo(Icon *icon)
{
  const int icon_data_type = icon->obj_type;

  DrawInfo *di = MEM_cnew<DrawInfo>("di_icon");

  if (ELEM(icon_data_type, ICON_DATA_ID, ICON_DATA_PREVIEW)) {
    di->type = ICON_TYPE_PREVIEW;
  }
  else if (icon_data_type == ICON_DATA_IMBUF) {
    di->type = ICON_TYPE_IMBUF;
  }
  else if (icon_data_type == ICON_DATA_GEOM) {
    di->type = ICON_TYPE_GEOM;
  }
  else if (icon_data_type == ICON_DATA_STUDIOLIGHT) {
    di->type = ICON_TYPE_BUFFER;
  }
  else if (icon_data_type == ICON_DATA_GPLAYER) {
    di->type = ICON_TYPE_GPLAYER;
  }
  else {
    KLI_assert(0);
  }

  return di;
}

/* does not belong here. */
typedef struct IMMDrawPixelsTexState
{
  struct GPUShader *shader;
  unsigned int pos;
  unsigned int texco;
  bool do_shader_unbind;
} IMMDrawPixelsTexState;

static void immDrawPixelsTexSetupAttributes(IMMDrawPixelsTexState *state)
{
  GPUVertFormat *vert_format = immVertexFormat();
  state->pos = GPU_vertformat_attr_add(vert_format, "pos", GPU_COMP_F32, 2, GPU_FETCH_FLOAT);
  state->texco = GPU_vertformat_attr_add(vert_format, "texCoord", GPU_COMP_F32, 2, GPU_FETCH_FLOAT);
}

static IMMDrawPixelsTexState immDrawPixelsTexSetup(int builtin)
{
  IMMDrawPixelsTexState state;
  immDrawPixelsTexSetupAttributes(&state);

  state.shader = GPU_shader_get_builtin_shader(static_cast<eGPUBuiltinShader>(builtin));

  /* Shader will be unbind by immUnbindProgram in a `immDrawPixelsTex` function. */
  immBindBuiltinProgram(static_cast<eGPUBuiltinShader>(builtin));
  immUniform1i("image", 0);
  state.do_shader_unbind = true;

  return state;
}

/* does not belong here. */
static void immDrawPixelsTexScaledFullSize(const IMMDrawPixelsTexState *state,
                                           const float x,
                                           const float y,
                                           const int img_w,
                                           const int img_h,
                                           const eGPUTextureFormat gpu_format,
                                           const bool use_filter,
                                           const void *rect,
                                           const float scaleX,
                                           const float scaleY,
                                           const float xzoom,
                                           const float yzoom,
                                           const float color[4])
{
  static const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  const float draw_width = img_w * scaleX * xzoom;
  const float draw_height = img_h * scaleY * yzoom;
  /* Down-scaling with regular bi-linear interpolation (i.e. #GL_LINEAR) doesn't give good
   * filtering results. Mipmaps can be used to get better results (i.e. #GL_LINEAR_MIPMAP_LINEAR),
   * so always use mipmaps when filtering. */
  const bool use_mipmap = use_filter && ((draw_width < img_w) || (draw_height < img_h));
  const int mip_len = use_mipmap ? 9999 : 1;

  GPUTexture *tex =
    GPU_texture_create_2d("immDrawPixels", img_w, img_h, mip_len, gpu_format, NULL);

  const bool use_float_data = ELEM(gpu_format, GPU_RGBA16F, GPU_RGB16F, GPU_R16F);
  eGPUDataFormat gpu_data_format = (use_float_data) ? GPU_DATA_FLOAT : GPU_DATA_UBYTE;
  GPU_texture_update(tex, gpu_data_format, rect);

  GPU_texture_filter_mode(tex, use_filter);
  if (use_mipmap) {
    GPU_texture_generate_mipmap(tex);
    GPU_texture_mipmap_mode(tex, true, true);
  }
  GPU_texture_wrap_mode(tex, false, true);

  GPU_texture_bind(tex, 0);

  /* optional */
  /* NOTE: Shader could be null for GLSL OCIO drawing, it is fine, since
   * it does not need color.
   */
  if (state->shader != NULL && GPU_shader_get_uniform(state->shader, "color") != -1) {
    immUniformColor4fv((color) ? color : white);
  }

  uint pos = state->pos, texco = state->texco;

  immBegin(GPU_PRIM_TRI_FAN, 4);
  immAttr2f(texco, 0.0f, 0.0f);
  immVertex2f(pos, x, y);

  immAttr2f(texco, 1.0f, 0.0f);
  immVertex2f(pos, x + draw_width, y);

  immAttr2f(texco, 1.0f, 1.0f);
  immVertex2f(pos, x + draw_width, y + draw_height);

  immAttr2f(texco, 0.0f, 1.0f);
  immVertex2f(pos, x, y + draw_height);
  immEnd();

  if (state->do_shader_unbind) {
    immUnbindProgram();
  }

  GPU_texture_unbind(tex);
  GPU_texture_free(tex);
}

static void icon_draw_rect(float x,
                           float y,
                           int w,
                           int h,
                           float UNUSED(aspect),
                           int rw,
                           int rh,
                           uint *rect,
                           float alpha,
                           const float desaturate)
{
  int draw_w = w;
  int draw_h = h;
  int draw_x = x;
  /* We need to round y, to avoid the icon jittering in some cases. */
  int draw_y = round_fl_to_int(y);

  /* sanity check */
  if (w <= 0 || h <= 0 || w > 2000 || h > 2000) {
    printf("%s: icons are %i x %i pixels?\n", __func__, w, h);
    KLI_assert_msg(0, "invalid icon size");
    return;
  }
  /* modulate color */
  const float col[4] = {alpha, alpha, alpha, alpha};

  float scale_x = 1.0f;
  float scale_y = 1.0f;
  /* rect contains image in 'rendersize', we only scale if needed */
  if (rw != w || rh != h) {
    /* preserve aspect ratio and center */
    if (rw > rh) {
      draw_w = w;
      draw_h = (int)(((float)rh / (float)rw) * (float)w);
      draw_y += (h - draw_h) / 2;
    } else if (rw < rh) {
      draw_w = (int)(((float)rw / (float)rh) * (float)h);
      draw_h = h;
      draw_x += (w - draw_w) / 2;
    }
    scale_x = draw_w / (float)rw;
    scale_y = draw_h / (float)rh;
    /* If the image is squared, the `draw_*` initialization values are good. */
  }

  /* draw */
  eGPUBuiltinShader shader;
  if (desaturate != 0.0f) {
    shader = GPU_SHADER_2D_IMAGE_DESATURATE_COLOR;
  } else {
    shader = GPU_SHADER_3D_IMAGE_COLOR;
  }
  IMMDrawPixelsTexState state = immDrawPixelsTexSetup(shader);

  if (shader == GPU_SHADER_2D_IMAGE_DESATURATE_COLOR) {
    immUniform1f("factor", desaturate);
  }

  immDrawPixelsTexScaledFullSize(&state,
                                 draw_x,
                                 draw_y,
                                 rw,
                                 rh,
                                 GPU_RGBA8,
                                 true,
                                 rect,
                                 scale_x,
                                 scale_y,
                                 1.0f,
                                 1.0f,
                                 col);
}

static void icon_draw_texture_cached(float x,
                                     float y,
                                     float w,
                                     float h,
                                     int ix,
                                     int iy,
                                     int UNUSED(iw),
                                     int ih,
                                     float alpha,
                                     const float rgb[3],
                                     bool with_border)
{

  float mvp[4][4];
  GPU_matrix_model_view_projection_get(mvp);

  IconTextureDrawCall *texture_call = with_border ? &g_icon_draw_cache.border :
                                                    &g_icon_draw_cache.normal;

  IconDrawCall *call = &texture_call->drawcall_cache[texture_call->calls];
  texture_call->calls++;

  /* Manual mat4*vec2 */
  call->pos.xmin = x * mvp[0][0] + y * mvp[1][0] + mvp[3][0];
  call->pos.ymin = x * mvp[0][1] + y * mvp[1][1] + mvp[3][1];
  call->pos.xmax = call->pos.xmin + w * mvp[0][0] + h * mvp[1][0];
  call->pos.ymax = call->pos.ymin + w * mvp[0][1] + h * mvp[1][1];

  call->tex.xmin = ix * icongltex.invw;
  call->tex.xmax = (ix + ih) * icongltex.invw;
  call->tex.ymin = iy * icongltex.invh;
  call->tex.ymax = (iy + ih) * icongltex.invh;

  if (rgb) {
    copy_v4_fl4(call->color, rgb[0], rgb[1], rgb[2], alpha);
  } else {
    copy_v4_fl(call->color, alpha);
  }

  if (texture_call->calls == ICON_DRAW_CACHE_SIZE) {
    icon_draw_cache_flush_ex(true);
  }
}

static void icon_draw_texture(float x,
                              float y,
                              float w,
                              float h,
                              int ix,
                              int iy,
                              int iw,
                              int ih,
                              float alpha,
                              const float rgb[3],
                              bool with_border)
{
  if (g_icon_draw_cache.enabled) {
    icon_draw_texture_cached(x, y, w, h, ix, iy, iw, ih, alpha, rgb, with_border);
    return;
  }

  /* We need to flush widget base first to ensure correct ordering. */
  // UI_widgetbase_draw_cache_flush();

  GPU_blend(GPU_BLEND_ALPHA_PREMULT);

  const float x1 = ix * icongltex.invw;
  const float x2 = (ix + ih) * icongltex.invw;
  const float y1 = iy * icongltex.invh;
  const float y2 = (iy + ih) * icongltex.invh;

  GPUTexture *texture = with_border ? icongltex.tex[1] : icongltex.tex[0];

  GPUShader *shader = GPU_shader_get_builtin_shader(GPU_SHADER_2D_IMAGE_RECT_COLOR);
  GPU_shader_bind(shader);

  const int img_binding = GPU_shader_get_texture_binding(shader, "image");
  const int color_loc = GPU_shader_get_builtin_uniform(shader, GPU_UNIFORM_COLOR);
  const int rect_tex_loc = GPU_shader_get_uniform(shader, "rect_icon");
  const int rect_geom_loc = GPU_shader_get_uniform(shader, "rect_geom");

  if (rgb) {
    GPU_shader_uniform_vector(shader, color_loc, 4, 1, (float[4]){UNPACK3(rgb), alpha});
  } else {
    GPU_shader_uniform_vector(shader, color_loc, 4, 1, (float[4]){alpha, alpha, alpha, alpha});
  }

  GPU_shader_uniform_vector(shader, rect_tex_loc, 4, 1, (float[4]){x1, y1, x2, y2});
  GPU_shader_uniform_vector(shader, rect_geom_loc, 4, 1, (float[4]){x, y, x + w, y + h});

  GPU_texture_bind_ex(texture, GPU_SAMPLER_ICON, img_binding, false);

  GPUBatch *quad = GPU_batch_preset_quad();
  GPU_batch_set_shader(quad, shader);
  GPU_batch_draw(quad);

  GPU_texture_unbind(texture);

  GPU_blend(GPU_BLEND_ALPHA);
}

static DrawInfo *icon_ensure_drawinfo(Icon *icon)
{
  if (icon->drawinfo) {
    return static_cast<DrawInfo *>(icon->drawinfo);
  }
  DrawInfo *di = icon_create_drawinfo(icon);
  icon->drawinfo = di;
  icon->drawinfo_free = UI_icons_free_drawinfo;
  return di;
}

static void icon_draw_size(float x,
                           float y,
                           int icon_id,
                           float aspect,
                           float alpha,
                           enum eIconSizes size,
                           int draw_size,
                           const float desaturate,
                           const uchar mono_rgba[4],
                           const bool mono_border)
{
  kTheme *ktheme = UI_GetTheme();
  const float fdraw_size = (float)draw_size;

  Icon *icon = KKE_icon_get(icon_id);
  alpha *= ktheme->tui.icon_alpha;

  if (icon == NULL) {
    if (G.debug & G_DEBUG) {
      printf("%s: Internal error, no icon for icon ID: %d\n", __func__, icon_id);
    }
    return;
  }

  /* scale width and height according to aspect */
  int w = (int)(fdraw_size / aspect + 0.5f);
  int h = (int)(fdraw_size / aspect + 0.5f);

  DrawInfo *di = icon_ensure_drawinfo(icon);

  /* We need to flush widget base first to ensure correct ordering. */
  // UI_widgetbase_draw_cache_flush();

  if (di->type == ICON_TYPE_IMBUF) {
    ImBuf *ibuf = static_cast<ImBuf *>(icon->obj);

    GPU_blend(GPU_BLEND_ALPHA_PREMULT);
    icon_draw_rect(x, y, w, h, aspect, ibuf->x, ibuf->y, ibuf->rect, alpha, desaturate);
    GPU_blend(GPU_BLEND_ALPHA);
  } else if (di->type == ICON_TYPE_VECTOR) {
    /* vector icons use the uiBlock transformation, they are not drawn
     * with untransformed coordinates like the other icons */
    di->data.vector.func((int)x, (int)y, w, h, 1.0f);
  } else if (di->type == ICON_TYPE_GEOM) {
#ifdef USE_UI_TOOLBAR_HACK
    /* TODO(@campbellbarton): scale icons up for toolbar,
     * we need a way to detect larger buttons and do this automatic. */
    {
      float scale = (float)ICON_DEFAULT_HEIGHT_TOOLBAR / (float)ICON_DEFAULT_HEIGHT;
      y = (y + (h / 2)) - ((h * scale) / 2);
      w *= scale;
      h *= scale;
    }
#endif

    /* If the theme is light, we will adjust the icon colors. */
    const bool invert = (rgb_to_grayscale_byte(ktheme->tui.wcol_toolbar_item.inner) > 128);
    const bool geom_inverted = di->data.geom.inverted;

    /* This could re-generate often if rendered at different sizes in the one interface.
     * TODO(@campbellbarton): support caching multiple sizes. */
    ImBuf *ibuf = di->data.geom.image_cache;
    if ((ibuf == NULL) || (ibuf->x != w) || (ibuf->y != h) || (invert != geom_inverted)) {
      if (ibuf) {
        // IMB_freeImBuf(ibuf);
      }
      if (invert != geom_inverted) {
        // KKE_icon_geom_invert_lightness(icon->obj);
      }
      // ibuf = KKE_icon_geom_rasterize(icon->obj, w, h);
      di->data.geom.image_cache = ibuf;
      di->data.geom.inverted = invert;
    }

    GPU_blend(GPU_BLEND_ALPHA_PREMULT);
    icon_draw_rect(x, y, w, h, aspect, w, h, ibuf->rect, alpha, desaturate);
    GPU_blend(GPU_BLEND_ALPHA);
  } else if (di->type == ICON_TYPE_EVENT) {
    const short event_type = di->data.input.event_type;
    const short event_value = di->data.input.event_value;
    // icon_draw_rect_input(x, y, w, h, alpha, event_type, event_value);
  } else if (di->type == ICON_TYPE_COLOR_TEXTURE) {
    /* texture image use premul alpha for correct scaling */
    icon_draw_texture(x,
                      y,
                      (float)w,
                      (float)h,
                      di->data.texture.x,
                      di->data.texture.y,
                      di->data.texture.w,
                      di->data.texture.h,
                      alpha,
                      NULL,
                      false);
  } else if (di->type == ICON_TYPE_MONO_TEXTURE) {
    /* Monochrome icon that uses text or theme color. */
    const bool with_border = mono_border && (ktheme->tui.icon_border_intensity > 0.0f);
    float color[4];
    if (mono_rgba) {
      rgba_uchar_to_float(color, (const uchar *)mono_rgba);
    } else {
      UI_GetThemeColor4fv(TH_TEXT, color);
    }

    mul_v4_fl(color, alpha);

    float border_outset = 0.0;
    uint border_texel = 0;
#ifndef WITH_HEADLESS
    if (with_border) {
      const float scale = (float)ICON_GRID_W / (float)ICON_DEFAULT_WIDTH;
      border_texel = ICON_MONO_BORDER_OUTSET;
      border_outset = ICON_MONO_BORDER_OUTSET / (scale * aspect);
    }
#endif
    icon_draw_texture(x - border_outset,
                      y - border_outset,
                      (float)w + 2 * border_outset,
                      (float)h + 2 * border_outset,
                      di->data.texture.x - border_texel,
                      di->data.texture.y - border_texel,
                      di->data.texture.w + 2 * border_texel,
                      di->data.texture.h + 2 * border_texel,
                      color[3],
                      color,
                      with_border);
  }

  else if (di->type == ICON_TYPE_BUFFER) {
    /* it is a builtin icon */
    IconImage *iimg = di->data.buffer.image;
#ifndef WITH_HEADLESS
    icon_verify_datatoc(iimg);
#endif
    if (!iimg->rect) {
      /* something has gone wrong! */
      return;
    }

    icon_draw_rect(x, y, w, h, aspect, iimg->w, iimg->h, iimg->rect, alpha, desaturate);
  } else if (di->type == ICON_TYPE_PREVIEW) {
    // PreviewImage *pi = (icon->id_type != 0) ? KKE_previewimg_id_ensure((ID *)icon->obj) :
    //                                           icon->obj;

    // if (pi) {
    //   /* no create icon on this level in code */
    //   if (!pi->rect[size]) {
    //     /* Something has gone wrong! */
    //     return;
    //   }

    //   /* Preview images use premultiplied alpha. */
    //   GPU_blend(GPU_BLEND_ALPHA_PREMULT);
    //   icon_draw_rect(x,
    //                  y,
    //                  w,
    //                  h,
    //                  aspect,
    //                  pi->w[size],
    //                  pi->h[size],
    //                  pi->rect[size],
    //                  alpha,
    //                  desaturate);
    //   GPU_blend(GPU_BLEND_ALPHA);
    // }
  } else if (di->type == ICON_TYPE_GPLAYER) {
    KLI_assert(icon->obj != NULL);

    /* Just draw a colored rect - Like for vicon_colorset_draw() */
    // #ifndef WITH_HEADLESS
    //     vicon_gplayer_color_draw(icon, (int)x, (int)y, w, h);
    // #endif
  }
}

int UI_icon_from_object_mode(const int mode)
{
  switch ((eObjectMode)mode) {
    case OB_MODE_OBJECT:
      return ICON_OBJECT_DATAMODE;
    case OB_MODE_EDIT:
    case OB_MODE_EDIT_GPENCIL:
      return ICON_EDITMODE_HLT;
    case OB_MODE_SCULPT:
    case OB_MODE_SCULPT_GPENCIL:
    case OB_MODE_SCULPT_CURVES:
      return ICON_SCULPTMODE_HLT;
    case OB_MODE_VERTEX_PAINT:
    case OB_MODE_VERTEX_GPENCIL:
      return ICON_VPAINT_HLT;
    case OB_MODE_WEIGHT_PAINT:
    case OB_MODE_WEIGHT_GPENCIL:
      return ICON_WPAINT_HLT;
    case OB_MODE_TEXTURE_PAINT:
      return ICON_TPAINT_HLT;
    case OB_MODE_PARTICLE_EDIT:
      return ICON_PARTICLEMODE;
    case OB_MODE_POSE:
      return ICON_POSE_HLT;
    case OB_MODE_PAINT_GPENCIL:
      return ICON_GREASEPENCIL;
  }
  return ICON_NONE;
}

// int UI_icon_color_from_collection(const Collection *collection)
// {
//   int icon = ICON_OUTLINER_COLLECTION;

//   if (collection->color_tag != COLLECTION_COLOR_NONE) {
//     icon = ICON_COLLECTION_COLOR_01 + collection->color_tag;
//   }

//   return icon;
// }

/* Drawing size for preview images */
static int get_draw_size(enum eIconSizes size)
{
  switch (size) {
    case ICON_SIZE_ICON:
      return ICON_DEFAULT_HEIGHT;
    case ICON_SIZE_PREVIEW:
      return PREVIEW_DEFAULT_HEIGHT;
    default:
      return 0;
  }
}

void UI_icon_draw(float x, float y, int icon_id)
{
  UI_icon_draw_ex(x, y, icon_id, U.inv_dpi_fac, 1.0f, 0.0f, NULL, false);
}

void UI_icon_draw_alpha(float x, float y, int icon_id, float alpha)
{
  UI_icon_draw_ex(x, y, icon_id, U.inv_dpi_fac, alpha, 0.0f, NULL, false);
}

void UI_icon_draw_preview(float x, float y, int icon_id, float aspect, float alpha, int size)
{
  icon_draw_size(x, y, icon_id, aspect, alpha, ICON_SIZE_PREVIEW, size, false, NULL, false);
}

void UI_icon_draw_ex(float x,
                     float y,
                     int icon_id,
                     float aspect,
                     float alpha,
                     float desaturate,
                     const uchar mono_color[4],
                     const bool mono_border)
{
  const int draw_size = get_draw_size(ICON_SIZE_ICON);
  icon_draw_size(x,
                 y,
                 icon_id,
                 aspect,
                 alpha,
                 ICON_SIZE_ICON,
                 draw_size,
                 desaturate,
                 mono_color,
                 mono_border);
}



/* ********** Move these to imbuf. ********** */

static void IMB_premultiply_rect(uint *rect, char planes, int w, int h)
{
  char *cp;
  int x, y, val;

  if (planes == 24) { /* put alpha at 255 */
    cp = (char *)(rect);

    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        cp[3] = 255;
      }
    }
  }
  else {
    cp = (char *)(rect);

    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        val = cp[3];
        cp[0] = (cp[0] * val) >> 8;
        cp[1] = (cp[1] * val) >> 8;
        cp[2] = (cp[2] * val) >> 8;
      }
    }
  }
}

static void IMB_premultiply_rect_float(float *rect_float, int channels, int w, int h)
{
  float val, *cp;
  int x, y;

  if (channels == 4) {
    cp = rect_float;
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++, cp += 4) {
        val = cp[3];
        cp[0] = cp[0] * val;
        cp[1] = cp[1] * val;
        cp[2] = cp[2] * val;
      }
    }
  }
}

static void IMB_premultiply_alpha(ImBuf *ibuf)
{
  if (ibuf == nullptr) {
    return;
  }

  if (ibuf->rect) {
    IMB_premultiply_rect(ibuf->rect, ibuf->planes, ibuf->x, ibuf->y);
  }

  if (ibuf->rect_float) {
    IMB_premultiply_rect_float(ibuf->rect_float, ibuf->channels, ibuf->x, ibuf->y);
  }
}



/* ********** Alert Icons ********** */

ImBuf *UI_icon_alert_imbuf_get(eAlertIcon icon)
{
#ifdef WITH_HEADLESS
  UNUSED_VARS(icon);
  return nullptr;
#else
  const int ALERT_IMG_SIZE = 256;
  icon = eAlertIcon(MIN2(icon, ALERT_ICON_MAX - 1));
  const int left = icon * ALERT_IMG_SIZE;
  const rcti crop = {left, left + ALERT_IMG_SIZE - 1, 0, ALERT_IMG_SIZE - 1};
  ImBuf *ibuf = IMB_ibImageFromMemory((const uchar *)datatoc_alert_icons_png,
                                      datatoc_alert_icons_png_size,
                                      IB_rect,
                                      nullptr,
                                      "alert_icon");
  IMB_rect_crop(ibuf, &crop);
  IMB_premultiply_alpha(ibuf);
  return ibuf;
#endif
}

void UI_icons_init()
{
  init_iconfile_list(&iconfilelist);
  // UI_icons_reload_internal_textures();
  init_internal_icons();
  // init_brush_icons();
  init_event_icons();
}

void UI_icons_free_drawinfo(void *drawinfo)
{
  DrawInfo *di = static_cast<DrawInfo *>(drawinfo);

  if (di == nullptr) {
    return;
  }

  if (di->type == ICON_TYPE_BUFFER) {
    if (di->data.buffer.image) {
      if (di->data.buffer.image->rect) {
        MEM_freeN(di->data.buffer.image->rect);
      }
      MEM_freeN(di->data.buffer.image);
    }
  }
  else if (di->type == ICON_TYPE_GEOM) {
    if (di->data.geom.image_cache) {
      IMB_freeImBuf(di->data.geom.image_cache);
    }
  }

  MEM_freeN(di);
}

void UI_icons_free(void)
{
  free_icons_textures();
  free_iconfile_list(&iconfilelist);
  KKE_icons_free();
}
