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
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include "IMB_colormanagement.h"
#include "IMB_colormanagement_intern.h"

#include <math.h>
#include <string.h>

#include "USD_color_types.h"
// #include "USD_image_types.h"
// #include "USD_movieclip_types.h"
#include "USD_scene_types.h"
#include "USD_space_types.h"

#include "IMB_filetype.h"
// #include "IMB_filter.h"
#include "IMB_imbuf.h"
// #include "IMB_imbuf_types.h"
// #include "IMB_metadata.h"
// #include "IMB_moviecache.h"

#include "MEM_guardedalloc.h"

#include "KLI_kraklib.h"
#include "KLI_math.h"
#include "KLI_math_color.h"
#include "KLI_rect.h"
#include "KLI_string.h"
#include "KLI_threads.h"

#include "KKE_appdir.h"
#include "KKE_colortools.h"
#include "KKE_main.h"
// #include "KKE_context.h"
// #include "KKE_image.h"
// #include "KKE_image_format.h"
// #include "KKE_main.h"

// #include "LUXO_define.h"

// #include "SEQ_iterator.h"

#include <ocio_capi.h>

/* -------------------------------------------------------------------- */
/** \name Global declarations
 * \{ */

#define DISPLAY_BUFFER_CHANNELS 4

/* ** list of all supported color spaces, displays and views */
static char global_role_data[MAX_COLORSPACE_NAME];
static char global_role_scene_linear[MAX_COLORSPACE_NAME];
static char global_role_color_picking[MAX_COLORSPACE_NAME];
static char global_role_texture_painting[MAX_COLORSPACE_NAME];
static char global_role_default_byte[MAX_COLORSPACE_NAME];
static char global_role_default_float[MAX_COLORSPACE_NAME];
static char global_role_default_sequencer[MAX_COLORSPACE_NAME];

static ListBase global_colorspaces = {NULL, NULL};
static ListBase global_displays = {NULL, NULL};
static ListBase global_views = {NULL, NULL};
static ListBase global_looks = {NULL, NULL};

static int global_tot_colorspace = 0;
static int global_tot_display = 0;
static int global_tot_view = 0;
static int global_tot_looks = 0;

/* Luma coefficients and XYZ to RGB to be initialized by OCIO. */
float imbuf_luma_coefficients[3] = {0.0f};
float imbuf_scene_linear_to_xyz[3][3] = {{0.0f}};
float imbuf_xyz_to_scene_linear[3][3] = {{0.0f}};
float imbuf_scene_linear_to_rec709[3][3] = {{0.0f}};
float imbuf_rec709_to_scene_linear[3][3] = {{0.0f}};
float imbuf_scene_linear_to_aces[3][3] = {{0.0f}};
float imbuf_aces_to_scene_linear[3][3] = {{0.0f}};

/* lock used by pre-cached processors getters, so processor wouldn't
 * be created several times
 * LOCK_COLORMANAGE can not be used since this mutex could be needed to
 * be locked before pre-cached processor are creating
 */
static pthread_mutex_t processor_lock = KLI_MUTEX_INITIALIZER;

typedef struct ColormanageProcessor
{
  OCIO_ConstCPUProcessorRcPtr *cpu_processor;
  CurveMapping *curve_mapping;
  bool is_data_result;
} ColormanageProcessor;

static struct global_gpu_state
{
  /* GPU shader currently bound. */
  bool gpu_shader_bound;

  /* Curve mapping. */
  CurveMapping *curve_mapping, *orig_curve_mapping;
  bool use_curve_mapping;
  int curve_mapping_timestamp;
  OCIO_CurveMappingSettings curve_mapping_settings;
} global_gpu_state = {false};

static struct global_color_picking_state
{
  /* Cached processor for color picking conversion. */
  OCIO_ConstCPUProcessorRcPtr *cpu_processor_to;
  OCIO_ConstCPUProcessorRcPtr *cpu_processor_from;
  bool failed;
} global_color_picking_state = {NULL};

/** \} */

typedef struct ColormanageCacheViewSettings
{
  int flag;
  int look;
  int view;
  float exposure;
  float gamma;
  float dither;
  CurveMapping *curve_mapping;
} ColormanageCacheViewSettings;

typedef struct ColormanageCacheDisplaySettings
{
  int display;
} ColormanageCacheDisplaySettings;

typedef struct ColormanageCacheKey
{
  int view;    /* view transformation used for display buffer */
  int display; /* display device name */
} ColormanageCacheKey;

typedef struct ColormanageCacheData
{
  int flag;                    /* view flags of cached buffer */
  int look;                    /* Additional artistics transform */
  float exposure;              /* exposure value cached buffer is calculated with */
  float gamma;                 /* gamma value cached buffer is calculated with */
  float dither;                /* dither value cached buffer is calculated with */
  CurveMapping *curve_mapping; /* curve mapping used for cached buffer */
  int curve_mapping_timestamp; /* time stamp of curve mapping used for cached buffer */
} ColormanageCacheData;

typedef struct ColormanageCache
{
  struct MovieCache *moviecache;

  ColormanageCacheData *data;
} ColormanageCache;

/* -------------------------------------------------------------------- */
/** \name Initialization / De-initialization
 * \{ */

static void colormanage_role_color_space_name_get(OCIO_ConstConfigRcPtr *config,
                                                  char *colorspace_name,
                                                  const char *role,
                                                  const char *backup_role)
{
  OCIO_ConstColorSpaceRcPtr *ociocs;

  ociocs = OCIO_configGetColorSpace(config, role);

  if (!ociocs && backup_role) {
    ociocs = OCIO_configGetColorSpace(config, backup_role);
  }

  if (ociocs) {
    const char *name = OCIO_colorSpaceGetName(ociocs);

    /* assume function was called with buffer properly allocated to MAX_COLORSPACE_NAME chars */
    KLI_strncpy(colorspace_name, name, MAX_COLORSPACE_NAME);
    OCIO_colorSpaceRelease(ociocs);
  } else {
    printf("Color management: Error could not find role %s role.\n", role);
  }
}

static void colormanage_load_config(OCIO_ConstConfigRcPtr *config)
{
  int tot_colorspace, tot_display, tot_display_view, tot_looks;
  int index, viewindex, viewindex2;
  const char *name;

  /* get roles */
  colormanage_role_color_space_name_get(config, global_role_data, OCIO_ROLE_DATA, NULL);
  colormanage_role_color_space_name_get(config,
                                        global_role_scene_linear,
                                        OCIO_ROLE_SCENE_LINEAR,
                                        NULL);
  colormanage_role_color_space_name_get(config,
                                        global_role_color_picking,
                                        OCIO_ROLE_COLOR_PICKING,
                                        NULL);
  colormanage_role_color_space_name_get(config,
                                        global_role_texture_painting,
                                        OCIO_ROLE_TEXTURE_PAINT,
                                        NULL);
  colormanage_role_color_space_name_get(config,
                                        global_role_default_sequencer,
                                        OCIO_ROLE_DEFAULT_SEQUENCER,
                                        OCIO_ROLE_SCENE_LINEAR);
  colormanage_role_color_space_name_get(config,
                                        global_role_default_byte,
                                        OCIO_ROLE_DEFAULT_BYTE,
                                        OCIO_ROLE_TEXTURE_PAINT);
  colormanage_role_color_space_name_get(config,
                                        global_role_default_float,
                                        OCIO_ROLE_DEFAULT_FLOAT,
                                        OCIO_ROLE_SCENE_LINEAR);

  /* load colorspaces */
  tot_colorspace = OCIO_configGetNumColorSpaces(config);
  for (index = 0; index < tot_colorspace; index++) {
    OCIO_ConstColorSpaceRcPtr *ocio_colorspace;
    const char *description;
    bool is_invertible, is_data;

    name = OCIO_configGetColorSpaceNameByIndex(config, index);

    ocio_colorspace = OCIO_configGetColorSpace(config, name);
    description = OCIO_colorSpaceGetDescription(ocio_colorspace);
    is_invertible = OCIO_colorSpaceIsInvertible(ocio_colorspace);
    is_data = OCIO_colorSpaceIsData(ocio_colorspace);

    ColorSpace *colorspace = colormanage_colorspace_add(name, description, is_invertible, is_data);

    colorspace->num_aliases = OCIO_colorSpaceGetNumAliases(ocio_colorspace);
    if (colorspace->num_aliases > 0) {
      colorspace->aliases = MEM_callocN(sizeof(*colorspace->aliases) * colorspace->num_aliases,
                                        "ColorSpace aliases");
      for (int i = 0; i < colorspace->num_aliases; i++) {
        KLI_strncpy(colorspace->aliases[i],
                    OCIO_colorSpaceGetAlias(ocio_colorspace, i),
                    MAX_COLORSPACE_NAME);
      }
    }

    OCIO_colorSpaceRelease(ocio_colorspace);
  }

  /* load displays */
  viewindex2 = 0;
  tot_display = OCIO_configGetNumDisplays(config);

  for (index = 0; index < tot_display; index++) {
    const char *displayname;
    ColorManagedDisplay *display;

    displayname = OCIO_configGetDisplay(config, index);

    display = colormanage_display_add(displayname);

    /* load views */
    tot_display_view = OCIO_configGetNumViews(config, displayname);
    for (viewindex = 0; viewindex < tot_display_view; viewindex++, viewindex2++) {
      const char *viewname;
      ColorManagedView *view;
      LinkData *display_view;

      viewname = OCIO_configGetView(config, displayname, viewindex);

      /* first check if view transform with given name was already loaded */
      view = colormanage_view_get_named(viewname);

      if (!view) {
        view = colormanage_view_add(viewname);
      }

      display_view = KLI_genericNodeN(view);

      KLI_addtail(&display->views, display_view);
    }
  }

  global_tot_display = tot_display;

  /* load looks */
  tot_looks = OCIO_configGetNumLooks(config);
  colormanage_look_add("None", "", true);
  for (index = 0; index < tot_looks; index++) {
    OCIO_ConstLookRcPtr *ocio_look;
    const char *process_space;

    name = OCIO_configGetLookNameByIndex(config, index);
    ocio_look = OCIO_configGetLook(config, name);
    process_space = OCIO_lookGetProcessSpace(ocio_look);
    OCIO_lookRelease(ocio_look);

    colormanage_look_add(name, process_space, false);
  }

  /* Load luminance coefficients. */
  OCIO_configGetDefaultLumaCoefs(config, imbuf_luma_coefficients);

  /* Load standard color spaces. */
  OCIO_configGetXYZtoSceneLinear(config, imbuf_xyz_to_scene_linear);
  invert_m3_m3(imbuf_scene_linear_to_xyz, imbuf_xyz_to_scene_linear);

  mul_m3_m3m3(imbuf_scene_linear_to_rec709, OCIO_XYZ_TO_REC709, imbuf_scene_linear_to_xyz);
  invert_m3_m3(imbuf_rec709_to_scene_linear, imbuf_scene_linear_to_rec709);

  mul_m3_m3m3(imbuf_aces_to_scene_linear, imbuf_xyz_to_scene_linear, OCIO_ACES_TO_XYZ);
  invert_m3_m3(imbuf_scene_linear_to_aces, imbuf_aces_to_scene_linear);
}

static void colormanage_free_config(void)
{
  ColorSpace *colorspace;
  ColorManagedDisplay *display;

  /* free color spaces */
  colorspace = global_colorspaces.first;
  while (colorspace) {
    ColorSpace *colorspace_next = colorspace->next;

    /* Free precomputed processors. */
    if (colorspace->to_scene_linear) {
      OCIO_cpuProcessorRelease((OCIO_ConstCPUProcessorRcPtr *)colorspace->to_scene_linear);
    }
    if (colorspace->from_scene_linear) {
      OCIO_cpuProcessorRelease((OCIO_ConstCPUProcessorRcPtr *)colorspace->from_scene_linear);
    }

    /* free color space itself */
    MEM_SAFE_FREE(colorspace->aliases);
    MEM_freeN(colorspace);

    colorspace = colorspace_next;
  }
  KLI_listbase_clear(&global_colorspaces);
  global_tot_colorspace = 0;

  /* free displays */
  display = global_displays.first;
  while (display) {
    ColorManagedDisplay *display_next = display->next;

    /* free precomputer processors */
    if (display->to_scene_linear) {
      OCIO_cpuProcessorRelease((OCIO_ConstCPUProcessorRcPtr *)display->to_scene_linear);
    }
    if (display->from_scene_linear) {
      OCIO_cpuProcessorRelease((OCIO_ConstCPUProcessorRcPtr *)display->from_scene_linear);
    }

    /* free list of views */
    KLI_freelistN(&display->views);

    MEM_freeN(display);
    display = display_next;
  }
  KLI_listbase_clear(&global_displays);
  global_tot_display = 0;

  /* free views */
  KLI_freelistN(&global_views);
  global_tot_view = 0;

  /* free looks */
  KLI_freelistN(&global_looks);
  global_tot_looks = 0;

  OCIO_exit();
}

void colormanagement_init(void)
{
  const char *ocio_env;
  const char *configdir;
  char configfile[FILE_MAX];
  OCIO_ConstConfigRcPtr *config = NULL;

  OCIO_init();

  ocio_env = KLI_getenv("OCIO");

  if (ocio_env && ocio_env[0] != '\0') {
    config = OCIO_configCreateFromEnv();
    if (config != NULL) {
      printf("Color management: Using %s as a configuration file\n", ocio_env);
    }
  }

  if (config == NULL) {
    configdir = KKE_appdir_folder_id(KRAKEN_DATAFILES, "colormanagement");

    if (configdir) {
      KLI_join_dirfile(configfile, sizeof(configfile), configdir, KCM_CONFIG_FILE);

#ifdef WIN32
      {
        /* Quite a hack to support loading configuration from path with non-ACII symbols. */

        char short_name[256];
        KLI_get_short_name(short_name, configfile);
        config = OCIO_configCreateFromFile(short_name);
      }
#else
      config = OCIO_configCreateFromFile(configfile);
#endif
    }
  }

  if (config == NULL) {
    printf("Color management: using fallback mode for management\n");

    /* there is no fallback. */
    // config = OCIO_configCreateFallback();
  }

  if (config) {
    OCIO_setCurrentConfig(config);

    colormanage_load_config(config);

    OCIO_configRelease(config);
  }

  /* If there are no valid display/views, use fallback mode. */
  if (global_tot_display == 0 || global_tot_view == 0) {
    printf("Color management: no displays/views in the config, using fallback mode instead\n");

    /* Free old config. */
    colormanage_free_config();

    /* Initialize fallback config. */
    /* there is no fallback. */
    // config = OCIO_configCreateFallback();
    colormanage_load_config(config);
  }

  KLI_init_srgb_conversion();
}

void colormanagement_exit(void)
{
  OCIO_gpuCacheFree();

  // if (global_gpu_state.curve_mapping) {
  //   KKE_curvemapping_free(global_gpu_state.curve_mapping);
  // }

  // if (global_gpu_state.curve_mapping_settings.lut) {
  //   MEM_freeN(global_gpu_state.curve_mapping_settings.lut);
  // }

  // if (global_color_picking_state.cpu_processor_to) {
  //   OCIO_cpuProcessorRelease(global_color_picking_state.cpu_processor_to);
  // }

  // if (global_color_picking_state.cpu_processor_from) {
  //   OCIO_cpuProcessorRelease(global_color_picking_state.cpu_processor_from);
  // }

  memset(&global_gpu_state, 0, sizeof(global_gpu_state));
  memset(&global_color_picking_state, 0, sizeof(global_color_picking_state));

  colormanage_free_config();
}


/* -------------------------------------------------------------------- */
/** \name Display Functions
 * \{ */

const char *colormanage_display_get_default_name(void)
{
  OCIO_ConstConfigRcPtr *config = OCIO_getCurrentConfig();
  const char *display_name;

  display_name = OCIO_configGetDefaultDisplay(config);

  OCIO_configRelease(config);

  return display_name;
}

ColorManagedDisplay *colormanage_display_get_default(void)
{
  const char *display_name = colormanage_display_get_default_name();

  if (display_name[0] == '\0') {
    return NULL;
  }

  return colormanage_display_get_named(display_name);
}

ColorManagedDisplay *colormanage_display_add(const char *name)
{
  ColorManagedDisplay *display;
  int index = 0;

  if (global_displays.last) {
    ColorManagedDisplay *last_display = global_displays.last;

    index = last_display->index;
  }

  display = MEM_callocN(sizeof(ColorManagedDisplay), "ColorManagedDisplay");

  display->index = index + 1;

  KLI_strncpy(display->name, name, sizeof(display->name));

  KLI_addtail(&global_displays, display);

  return display;
}

ColorManagedDisplay *colormanage_display_get_named(const char *name)
{
  ColorManagedDisplay *display;

  for (display = global_displays.first; display; display = display->next) {
    if (STREQ(display->name, name)) {
      return display;
    }
  }

  return NULL;
}

ColorManagedDisplay *colormanage_display_get_indexed(int index)
{
  /* display indices are 1-based */
  return KLI_findlink(&global_displays, index - 1);
}

int IMB_colormanagement_display_get_named_index(const char *name)
{
  ColorManagedDisplay *display;

  display = colormanage_display_get_named(name);

  if (display) {
    return display->index;
  }

  return 0;
}

const char *IMB_colormanagement_display_get_indexed_name(int index)
{
  ColorManagedDisplay *display;

  display = colormanage_display_get_indexed(index);

  if (display) {
    return display->name;
  }

  return NULL;
}

const char *IMB_colormanagement_display_get_default_name(void)
{
  ColorManagedDisplay *display = colormanage_display_get_default();

  return display->name;
}

ColorManagedDisplay *IMB_colormanagement_display_get_named(const char *name)
{
  return colormanage_display_get_named(name);
}

const char *IMB_colormanagement_display_get_none_name(void)
{
  if (colormanage_display_get_named("None") != NULL) {
    return "None";
  }

  return colormanage_display_get_default_name();
}

const char *IMB_colormanagement_display_get_default_view_transform_name(
  struct ColorManagedDisplay *display)
{
  return colormanage_view_get_default_name(display);
}

/* -------------------------------------------------------------------- */
/** \name View Functions
 * \{ */

const char *colormanage_view_get_default_name(const ColorManagedDisplay *display)
{
  OCIO_ConstConfigRcPtr *config = OCIO_getCurrentConfig();
  const char *name;

  name = OCIO_configGetDefaultView(config, display->name);

  OCIO_configRelease(config);

  return name;
}

ColorManagedView *colormanage_view_get_default(const ColorManagedDisplay *display)
{
  const char *name = colormanage_view_get_default_name(display);

  if (!name || name[0] == '\0') {
    return NULL;
  }

  return colormanage_view_get_named(name);
}

ColorManagedView *colormanage_view_add(const char *name)
{
  ColorManagedView *view;
  int index = global_tot_view;

  view = MEM_callocN(sizeof(ColorManagedView), "ColorManagedView");
  view->index = index + 1;
  KLI_strncpy(view->name, name, sizeof(view->name));

  KLI_addtail(&global_views, view);

  global_tot_view++;

  return view;
}

ColorManagedView *colormanage_view_get_named(const char *name)
{
  ColorManagedView *view;

  for (view = global_views.first; view; view = view->next) {
    if (STREQ(view->name, name)) {
      return view;
    }
  }

  return NULL;
}

ColorManagedView *colormanage_view_get_indexed(int index)
{
  /* view transform indices are 1-based */
  return KLI_findlink(&global_views, index - 1);
}

ColorManagedView *colormanage_view_get_named_for_display(const char *display_name,
                                                         const char *name)
{
  ColorManagedDisplay *display = colormanage_display_get_named(display_name);
  if (display == NULL) {
    return NULL;
  }
  LISTBASE_FOREACH(LinkData *, view_link, &display->views)
  {
    ColorManagedView *view = view_link->data;
    if (STRCASEEQ(name, view->name)) {
      return view;
    }
  }
  return NULL;
}

int IMB_colormanagement_view_get_named_index(const char *name)
{
  ColorManagedView *view = colormanage_view_get_named(name);

  if (view) {
    return view->index;
  }

  return 0;
}

const char *IMB_colormanagement_view_get_indexed_name(int index)
{
  ColorManagedView *view = colormanage_view_get_indexed(index);

  if (view) {
    return view->name;
  }

  return NULL;
}

const char *IMB_colormanagement_view_get_default_name(const char *display_name)
{
  ColorManagedDisplay *display = colormanage_display_get_named(display_name);
  ColorManagedView *view = NULL;

  if (display) {
    view = colormanage_view_get_default(display);
  }

  if (view) {
    return view->name;
  }

  return NULL;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Color Space Functions
 * \{ */

static void colormanage_description_strip(char *description)
{
  int i, n;

  for (i = (int)strlen(description) - 1; i >= 0; i--) {
    if (ELEM(description[i], '\r', '\n')) {
      description[i] = '\0';
    } else {
      break;
    }
  }

  for (i = 0, n = strlen(description); i < n; i++) {
    if (ELEM(description[i], '\r', '\n')) {
      description[i] = ' ';
    }
  }
}

ColorSpace *colormanage_colorspace_add(const char *name,
                                       const char *description,
                                       bool is_invertible,
                                       bool is_data)
{
  ColorSpace *colorspace, *prev_space;
  int counter = 1;

  colorspace = MEM_callocN(sizeof(ColorSpace), "ColorSpace");

  KLI_strncpy(colorspace->name, name, sizeof(colorspace->name));

  if (description) {
    KLI_strncpy(colorspace->description, description, sizeof(colorspace->description));

    colormanage_description_strip(colorspace->description);
  }

  colorspace->is_invertible = is_invertible;
  colorspace->is_data = is_data;

  for (prev_space = global_colorspaces.first; prev_space; prev_space = prev_space->next) {
    if (KLI_strcasecmp(prev_space->name, colorspace->name) > 0) {
      break;
    }

    prev_space->index = counter++;
  }

  if (!prev_space) {
    KLI_addtail(&global_colorspaces, colorspace);
  } else {
    KLI_insertlinkbefore(&global_colorspaces, prev_space, colorspace);
  }

  colorspace->index = counter++;
  for (; prev_space; prev_space = prev_space->next) {
    prev_space->index = counter++;
  }

  global_tot_colorspace++;

  return colorspace;
}

ColorSpace *colormanage_colorspace_get_named(const char *name)
{
  ColorSpace *colorspace;

  for (colorspace = global_colorspaces.first; colorspace; colorspace = colorspace->next) {
    if (STREQ(colorspace->name, name)) {
      return colorspace;
    }

    for (int i = 0; i < colorspace->num_aliases; i++) {
      if (STREQ(colorspace->aliases[i], name)) {
        return colorspace;
      }
    }
  }

  return NULL;
}

ColorSpace *colormanage_colorspace_get_roled(int role)
{
  const char *role_colorspace = IMB_colormanagement_role_colorspace_name_get(role);

  return colormanage_colorspace_get_named(role_colorspace);
}

ColorSpace *colormanage_colorspace_get_indexed(int index)
{
  /* color space indices are 1-based */
  return KLI_findlink(&global_colorspaces, index - 1);
}

int IMB_colormanagement_colorspace_get_named_index(const char *name)
{
  ColorSpace *colorspace;

  colorspace = colormanage_colorspace_get_named(name);

  if (colorspace) {
    return colorspace->index;
  }

  return 0;
}

const char *IMB_colormanagement_colorspace_get_indexed_name(int index)
{
  ColorSpace *colorspace;

  colorspace = colormanage_colorspace_get_indexed(index);

  if (colorspace) {
    return colorspace->name;
  }

  return "";
}

const char *IMB_colormanagement_colorspace_get_name(const ColorSpace *colorspace)
{
  return colorspace->name;
}

void IMB_colormanagement_colorspace_from_ibuf_ftype(
  ColorManagedColorspaceSettings *colorspace_settings,
  ImBuf *ibuf)
{
  /* Don't modify non-color data space, it does not change with file type. */
  ColorSpace *colorspace = colormanage_colorspace_get_named(colorspace_settings->name);

  if (colorspace && colorspace->is_data) {
    return;
  }

  /* Get color space from file type. */
  const ImFileType *type = IMB_file_type_from_ibuf(ibuf);
  if (type != NULL) {
    if (type->save != NULL) {
      const char *role_colorspace = IMB_colormanagement_role_colorspace_name_get(
        type->default_save_role);
      KLI_strncpy(colorspace_settings->name, role_colorspace, sizeof(colorspace_settings->name));
    }
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Looks Functions
 * \{ */

ColorManagedLook *colormanage_look_add(const char *name, const char *process_space, bool is_noop)
{
  ColorManagedLook *look;
  int index = global_tot_looks;

  look = MEM_callocN(sizeof(ColorManagedLook), "ColorManagedLook");
  look->index = index + 1;
  KLI_strncpy(look->name, name, sizeof(look->name));
  KLI_strncpy(look->ui_name, name, sizeof(look->ui_name));
  KLI_strncpy(look->process_space, process_space, sizeof(look->process_space));
  look->is_noop = is_noop;

  /* Detect view specific looks. */
  const char *separator_offset = strstr(look->name, " - ");
  if (separator_offset) {
    KLI_strncpy(look->view, look->name, separator_offset - look->name + 1);
    KLI_strncpy(look->ui_name, separator_offset + strlen(" - "), sizeof(look->ui_name));
  }

  KLI_addtail(&global_looks, look);

  global_tot_looks++;

  return look;
}

ColorManagedLook *colormanage_look_get_named(const char *name)
{
  ColorManagedLook *look;

  for (look = global_looks.first; look; look = look->next) {
    if (STREQ(look->name, name)) {
      return look;
    }
  }

  return NULL;
}

ColorManagedLook *colormanage_look_get_indexed(int index)
{
  /* look indices are 1-based */
  return KLI_findlink(&global_looks, index - 1);
}

int IMB_colormanagement_look_get_named_index(const char *name)
{
  ColorManagedLook *look;

  look = colormanage_look_get_named(name);

  if (look) {
    return look->index;
  }

  return 0;
}

const char *IMB_colormanagement_look_get_indexed_name(int index)
{
  ColorManagedLook *look;

  look = colormanage_look_get_indexed(index);

  if (look) {
    return look->name;
  }

  return NULL;
}

static void colormanage_check_colorspace_settings(
  ColorManagedColorspaceSettings *colorspace_settings,
  const char *what)
{
  if (colorspace_settings->name[0] == '\0') {
    /* pass */
  } else {
    ColorSpace *colorspace = colormanage_colorspace_get_named(colorspace_settings->name);

    if (!colorspace) {
      printf("Color management: %s colorspace \"%s\" not found, will use default instead.\n",
             what,
             colorspace_settings->name);

      KLI_strncpy(colorspace_settings->name, "", sizeof(colorspace_settings->name));
    }
  }

  (void)what;
}

// static bool seq_callback(Sequence *seq, void *UNUSED(user_data))
// {
//   if (seq->strip) {
//     colormanage_check_colorspace_settings(&seq->strip->colorspace_settings, "sequencer strip");
//   }
//   return true;
// }

void IMB_colormanagement_check_file_config(Main *kmain)
{
  // kScene *scene;
  // Image *image;
  // MovieClip *clip;

  ColorManagedDisplay *default_display;

  default_display = colormanage_display_get_default();

  if (!default_display) {
    /* happens when OCIO configuration is incorrect */
    return;
  }

  // for (scene = kmain->scenes.first; scene; scene = scene->id.next) {
  //   ColorManagedColorspaceSettings *sequencer_colorspace_settings;

  //   /* check scene color management settings */
  //   colormanage_check_display_settings(&scene->display_settings, "scene", default_display);
  //   colormanage_check_view_settings(&scene->display_settings, &scene->view_settings, "scene");

  //   sequencer_colorspace_settings = &scene->sequencer_colorspace_settings;

  //   colormanage_check_colorspace_settings(sequencer_colorspace_settings, "sequencer");

  //   if (sequencer_colorspace_settings->name[0] == '\0') {
  //     KLI_strncpy(
  //         sequencer_colorspace_settings->name, global_role_default_sequencer,
  //         MAX_COLORSPACE_NAME);
  //   }

  //   /* check sequencer strip input color space settings */
  //   if (scene->ed != NULL) {
  //     SEQ_for_each_callback(&scene->ed->seqbase, seq_callback, NULL);
  //   }
  // }

  /* ** check input color space settings ** */

  // for (image = bmain->images.first; image; image = image->id.next) {
  //   colormanage_check_colorspace_settings(&image->colorspace_settings, "image");
  // }

  // for (clip = bmain->movieclips.first; clip; clip = clip->id.next) {
  //   colormanage_check_colorspace_settings(&clip->colorspace_settings, "clip");
  // }
}

void IMB_colormanagement_validate_settings(const ColorManagedDisplaySettings *display_settings,
                                           ColorManagedViewSettings *view_settings)
{
  ColorManagedDisplay *display;
  ColorManagedView *default_view = NULL;
  LinkData *view_link;

  display = colormanage_display_get_named(display_settings->display_device);

  default_view = colormanage_view_get_default(display);

  for (view_link = display->views.first; view_link; view_link = view_link->next) {
    ColorManagedView *view = view_link->data;

    if (STREQ(view->name, view_settings->view_transform)) {
      break;
    }
  }

  if (view_link == NULL && default_view) {
    KLI_strncpy(view_settings->view_transform,
                default_view->name,
                sizeof(view_settings->view_transform));
  }
}

const char *IMB_colormanagement_role_colorspace_name_get(int role)
{
  switch (role) {
    case COLOR_ROLE_DATA:
      return global_role_data;
    case COLOR_ROLE_SCENE_LINEAR:
      return global_role_scene_linear;
    case COLOR_ROLE_COLOR_PICKING:
      return global_role_color_picking;
    case COLOR_ROLE_TEXTURE_PAINTING:
      return global_role_texture_painting;
    case COLOR_ROLE_DEFAULT_SEQUENCER:
      return global_role_default_sequencer;
    case COLOR_ROLE_DEFAULT_FLOAT:
      return global_role_default_float;
    case COLOR_ROLE_DEFAULT_BYTE:
      return global_role_default_byte;
    default:
      printf("Unknown role was passed to %s\n", __func__);
      KLI_assert(0);
      break;
  }

  return NULL;
}

void IMB_colormanagement_check_is_data(ImBuf *ibuf, const char *name)
{
  ColorSpace *colorspace = colormanage_colorspace_get_named(name);

  if (colorspace && colorspace->is_data) {
    ibuf->colormanage_flag |= IMB_COLORMANAGE_IS_DATA;
  } else {
    ibuf->colormanage_flag &= ~IMB_COLORMANAGE_IS_DATA;
  }
}

// void IMB_colormanagegent_copy_settings(ImBuf *ibuf_src, ImBuf *ibuf_dst)
// {
//   IMB_colormanagement_assign_rect_colorspace(ibuf_dst,
//                                              IMB_colormanagement_get_rect_colorspace(ibuf_src));
//   IMB_colormanagement_assign_float_colorspace(ibuf_dst,
//                                               IMB_colormanagement_get_float_colorspace(ibuf_src));
//   if (ibuf_src->flags & IB_alphamode_premul) {
//     ibuf_dst->flags |= IB_alphamode_premul;
//   } else if (ibuf_src->flags & IB_alphamode_channel_packed) {
//     ibuf_dst->flags |= IB_alphamode_channel_packed;
//   } else if (ibuf_src->flags & IB_alphamode_ignore) {
//     ibuf_dst->flags |= IB_alphamode_ignore;
//   }
// }

// void IMB_colormanagement_assign_float_colorspace(ImBuf *ibuf, const char *name)
// {
//   ColorSpace *colorspace = colormanage_colorspace_get_named(name);

//   ibuf->float_colorspace = colorspace;

//   if (colorspace && colorspace->is_data) {
//     ibuf->colormanage_flag |= IMB_COLORMANAGE_IS_DATA;
//   } else {
//     ibuf->colormanage_flag &= ~IMB_COLORMANAGE_IS_DATA;
//   }
// }

// void IMB_colormanagement_assign_rect_colorspace(ImBuf *ibuf, const char *name)
// {
//   ColorSpace *colorspace = colormanage_colorspace_get_named(name);

//   ibuf->rect_colorspace = colorspace;

//   if (colorspace && colorspace->is_data) {
//     ibuf->colormanage_flag |= IMB_COLORMANAGE_IS_DATA;
//   } else {
//     ibuf->colormanage_flag &= ~IMB_COLORMANAGE_IS_DATA;
//   }
// }

// const char *IMB_colormanagement_get_float_colorspace(ImBuf *ibuf)
// {
//   if (ibuf->float_colorspace) {
//     return ibuf->float_colorspace->name;
//   }

//   return IMB_colormanagement_role_colorspace_name_get(COLOR_ROLE_SCENE_LINEAR);
// }

// const char *IMB_colormanagement_get_rect_colorspace(ImBuf *ibuf)
// {
//   if (ibuf->rect_colorspace) {
//     return ibuf->rect_colorspace->name;
//   }

//   return IMB_colormanagement_role_colorspace_name_get(COLOR_ROLE_DEFAULT_BYTE);
// }

// bool IMB_colormanagement_space_is_data(ColorSpace *colorspace)
// {
//   return (colorspace && colorspace->is_data);
// }

// static void colormanage_ensure_srgb_scene_linear_info(ColorSpace *colorspace)
// {
//   if (!colorspace->info.cached) {
//     OCIO_ConstConfigRcPtr *config = OCIO_getCurrentConfig();
//     OCIO_ConstColorSpaceRcPtr *ocio_colorspace = OCIO_configGetColorSpace(config,
//                                                                           colorspace->name);

//     bool is_scene_linear, is_srgb;
//     OCIO_colorSpaceIsBuiltin(config, ocio_colorspace, &is_scene_linear, &is_srgb);

//     OCIO_colorSpaceRelease(ocio_colorspace);
//     OCIO_configRelease(config);

//     colorspace->info.is_scene_linear = is_scene_linear;
//     colorspace->info.is_srgb = is_srgb;
//     colorspace->info.cached = true;
//   }
// }

// bool IMB_colormanagement_space_is_scene_linear(ColorSpace *colorspace)
// {
//   colormanage_ensure_srgb_scene_linear_info(colorspace);
//   return (colorspace && colorspace->info.is_scene_linear);
// }

// bool IMB_colormanagement_space_is_srgb(ColorSpace *colorspace)
// {
//   colormanage_ensure_srgb_scene_linear_info(colorspace);
//   return (colorspace && colorspace->info.is_srgb);
// }

// bool IMB_colormanagement_space_name_is_data(const char *name)
// {
//   ColorSpace *colorspace = colormanage_colorspace_get_named(name);
//   return (colorspace && colorspace->is_data);
// }

// bool IMB_colormanagement_space_name_is_scene_linear(const char *name)
// {
//   ColorSpace *colorspace = colormanage_colorspace_get_named(name);
//   return (colorspace && IMB_colormanagement_space_is_scene_linear(colorspace));
// }

// bool IMB_colormanagement_space_name_is_srgb(const char *name)
// {
//   ColorSpace *colorspace = colormanage_colorspace_get_named(name);
//   return (colorspace && IMB_colormanagement_space_is_srgb(colorspace));
// }

// const float *IMB_colormanagement_get_xyz_to_scene_linear()
// {
//   return &imbuf_xyz_to_scene_linear[0][0];
// }

void IMB_colormanagement_display_to_scene_linear_v3(float pixel[3], ColorManagedDisplay *display)
{
  // OCIO_ConstCPUProcessorRcPtr *processor = display_to_scene_linear_processor(display);

  // if (processor != NULL) {
  //   OCIO_cpuProcessorApplyRGB(processor, pixel);
  // }
}

void IMB_colormanagement_finish_glsl_draw(void)
{
  if (global_gpu_state.gpu_shader_bound) {
    OCIO_gpuDisplayShaderUnbind();
    global_gpu_state.gpu_shader_bound = false;
  }
}

/** \} */
