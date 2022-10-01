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
 * Universe.
 * Set the Stage.
 */

#pragma once

#include "USD_api.h"

#include <wabi/base/gf/vec4f.h>

struct IDProperty;

KRAKEN_NAMESPACE_BEGIN

/* general defines for kernel functions */
#define CM_RESOL 32
#define CM_TABLE 256
#define CM_TABLEDIV (1.0f / 256.0f)

#define CM_TOT 4

#define GPU_SKY_WIDTH 512
#define GPU_SKY_HEIGHT 128

struct CurveMapPoint
{
  float x, y;
  /** Shorty for result lookup. */
  short flag, shorty;
};

/** #CurveMapPoint.flag */
enum
{
  CUMA_SELECT = (1 << 0),
  CUMA_HANDLE_VECTOR = (1 << 1),
  CUMA_HANDLE_AUTO_ANIM = (1 << 2),
};

struct CurveMap
{
  short totpoint;

  /** Quick multiply value for reading table. */
  float range;
  /** The x-axis range for the table. */
  float mintable, maxtable;
  /** For extrapolated curves, the direction vector. */
  float ext_in[2], ext_out[2];
  /** Actual curve. */
  CurveMapPoint *curve;
  /** Display and evaluate table. */
  CurveMapPoint *table;

  /** For RGB curves, pre-multiplied table. */
  CurveMapPoint *premultable;
  /** For RGB curves, pre-multiplied extrapolation vector. */
  float premul_ext_in[2];
  float premul_ext_out[2];
};

struct CurveMapping
{
  /** Cur; for buttons, to show active curve. */
  int flag, cur;
  int preset;
  int changed_timestamp;

  /** Current rect, clip rect (is default rect too). */
  wabi::GfVec4f curr, clipr;

  /** Max 4 builtin curves per mapping struct now. */
  CurveMap cm[4];
  /** Black/white point (black[0] abused for current frame). */
  float black[3], white[3];
  /** Black/white point multiply value, for speed. */
  float bwmul[3];

  /** Sample values, if flag set it draws line and intersection. */
  float sample[3];

  short tone;
};

/** #CurveMapping.flag */
enum eCurveMappingFlags
{
  CUMA_DO_CLIP = (1 << 0),
  CUMA_PREMULLED = (1 << 1),
  CUMA_DRAW_CFRA = (1 << 2),
  CUMA_DRAW_SAMPLE = (1 << 3),

  /** The curve is extended by extrapolation. When not set the curve is extended horizontally. */
  CUMA_EXTEND_EXTRAPOLATE = (1 << 4),
};

/** #CurveMapping.preset */
enum eCurveMappingPreset
{
  CURVE_PRESET_LINE = 0,
  CURVE_PRESET_SHARP = 1,
  CURVE_PRESET_SMOOTH = 2,
  CURVE_PRESET_MAX = 3,
  CURVE_PRESET_MID9 = 4,
  CURVE_PRESET_ROUND = 5,
  CURVE_PRESET_ROOT = 6,
  CURVE_PRESET_GAUSS = 7,
  CURVE_PRESET_BELL = 8,
};

/** #CurveMapping.tone */
enum eCurveMappingTone
{
  CURVE_TONE_STANDARD = 0,
  CURVE_TONE_FILMLIKE = 2,
};

/** #Histogram.mode */
enum
{
  HISTO_MODE_LUMA = 0,
  HISTO_MODE_RGB = 1,
  HISTO_MODE_R = 2,
  HISTO_MODE_G = 3,
  HISTO_MODE_B = 4,
  HISTO_MODE_ALPHA = 5,
};

enum
{
  HISTO_FLAG_LINE = (1 << 0),
  HISTO_FLAG_SAMPLELINE = (1 << 1),
};

struct Histogram
{
  int channels;
  int x_resolution;
  float data_luma[256];
  float data_r[256];
  float data_g[256];
  float data_b[256];
  float data_a[256];
  float xmax, ymax;
  short mode;
  short flag;
  int height;

  /** Sample line only (image coords: source -> destination). */
  float co[2][2];
};

struct Scopes
{
  int ok;
  int sample_full;
  int sample_lines;
  float accuracy;
  int wavefrm_mode;
  float wavefrm_alpha;
  float wavefrm_yfac;
  int wavefrm_height;
  float vecscope_alpha;
  int vecscope_height;
  float minmax[3][2];
  struct Histogram hist;
  float *waveform_1;
  float *waveform_2;
  float *waveform_3;
  float *vecscope;
  int waveform_tot;
  char _pad[4];
};

/** #Scopes.wavefrm_mode */
enum
{
  SCOPES_WAVEFRM_LUMA = 0,
  SCOPES_WAVEFRM_RGB_PARADE = 1,
  SCOPES_WAVEFRM_YCC_601 = 2,
  SCOPES_WAVEFRM_YCC_709 = 3,
  SCOPES_WAVEFRM_YCC_JPEG = 4,
  SCOPES_WAVEFRM_RGB = 5,
};

struct ColorManagedViewSettings
{
  int flag;
  /** Look which is being applied when displaying buffer on the screen
   * (prior to view transform). */
  char look[64];
  /** View transform which is being applied when displaying buffer on the screen. */
  char view_transform[64];
  /** F-stop exposure. */
  float exposure;
  /** Post-display gamma transform. */
  float gamma;
  /** Pre-display RGB curves transform. */
  struct CurveMapping *curve_mapping;
};

struct ColorManagedDisplaySettings
{
  char display_device[64];
};

struct ColorManagedColorspaceSettings
{
  char name[64];
};

/** #ColorManagedViewSettings.flag */
enum
{
  COLORMANAGE_VIEW_USE_CURVES = (1 << 0),
};

#define IMB_MIPMAP_LEVELS 20
#define IMB_FILENAME_SIZE 1024

/* -------------------------------------------------------------------- */
/** \name Image Buffer
 * \{ */

typedef struct ImBuf {
  /* dimensions */
  /** Width and Height of our image buffer.
   * Should be 'unsigned int' since most formats use this.
   * but this is problematic with texture math in `imagetexture.c`
   * avoid problems and use int. - campbell */
  int x, y;

  /** Active amount of bits/bit-planes. */
  unsigned char planes;
  /** Number of channels in `rect_float` (0 = 4 channel default) */
  int channels;

  /* flags */
  /** Controls which components should exist. */
  int flags;
  /** what is malloced internal, and can be freed */
  int mall;

  /* pixels */

  /** Image pixel buffer (8bit representation):
   * - color space defaults to `sRGB`.
   * - alpha defaults to 'straight'.
   */
  unsigned int *rect;
  /** Image pixel buffer (float representation):
   * - color space defaults to 'linear' (`rec709`).
   * - alpha defaults to 'premul'.
   * \note May need gamma correction to `sRGB` when generating 8bit representations.
   * \note Formats that support higher more than 8 but channels load as floats.
   */
  float *rect_float;

  /** Resolution in pixels per meter. Multiply by `0.0254` for DPI. */
  double ppm[2];

  /* tiled pixel storage */
  int tilex, tiley;
  int xtiles, ytiles;
  unsigned int **tiles;

  /* zbuffer */
  /** z buffer data, original zbuffer */
  int *zbuf;
  /** z buffer data, camera coordinates */
  float *zbuf_float;

  /* parameters used by conversion between byte and float */
  /** random dither value, for conversion from float -> byte rect */
  float dither;

  /* mipmapping */
  /** MipMap levels, a series of halved images */
  struct ImBuf *mipmap[IMB_MIPMAP_LEVELS];
  int miptot, miplevel;

  /* externally used data */
  /** reference index for ImBuf lists */
  int index;
  /** used to set imbuf to dirty and other stuff */
  int userflags;
  /** image metadata */
  IDProperty *metadata;
  /** temporary storage */
  void *userdata;

  /* file information */
  /** file type we are going to save as */
  // enum eImbFileType ftype;
  /** file format specific flags */
  // ImbFormatOptions foptions;
  /** filename associated with this image */
  char name[IMB_FILENAME_SIZE];
  /** full filename used for reading from cache */
  char cachename[IMB_FILENAME_SIZE];

  /* memory cache limiter */
  /** handle for cache limiter */
  // struct MEM_CacheLimiterHandle_s *c_handle;
  /** reference counter for multiple users */
  int refcounter;

  /* some parameters to pass along for packing images */
  /** Compressed image only used with PNG and EXR currently. */
  unsigned char *encodedbuffer;
  /** Size of data written to `encodedbuffer`. */
  unsigned int encodedsize;
  /** Size of `encodedbuffer` */
  unsigned int encodedbuffersize;

  /* color management */
  /** color space of byte buffer */
  struct ColorSpace *rect_colorspace;
  /** color space of float buffer, used by sequencer only */
  struct ColorSpace *float_colorspace;
  /** array of per-display display buffers dirty flags */
  unsigned int *display_buffer_flags;
  /** cache used by color management */
  // struct ColormanageCache *colormanage_cache;
  int colormanage_flag;
  // rcti invalid_rect;

  /* information for compressed textures */
  // struct DDSData dds_data;
} ImBuf;

KRAKEN_NAMESPACE_END