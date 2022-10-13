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

#pragma once

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include "../kraklib/KLI_compiler_compat.h"
#include "../kraklib/KLI_sys_types.h"
#include "../kraklib/KLI_rect.h"
#include "../gpu/GPU_texture.h"

#ifdef __cplusplus
#  include <wabi/base/tf/token.h>

/**
 * @DEFAULT: "Out of the box" supported file types for quick lookups. 
 */
struct ImbFileTypeToken
{
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_PNG = wabi::TfToken("PNG");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_TGA = wabi::TfToken("TGA");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_RAWTGA = wabi::TfToken("RAWTGA");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_JPG = wabi::TfToken("JPG");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_JPEG = wabi::TfToken("JPEG");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_OPEN_EXR = wabi::TfToken("OPEN_EXR");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_OPEN_EXR_MULTILAYER = wabi::TfToken("OPEN_EXR_MULTILAYER");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_IMAGIC = wabi::TfToken("OPEN_EXR_IMAGIC");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_BMP = wabi::TfToken("BMP");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_IRIS = wabi::TfToken("IRIS");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_IRIZ = wabi::TfToken("IRIZ");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_AVIRAW = wabi::TfToken("AVIRAW");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_AVIJPEG = wabi::TfToken("AVIJPEG");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_PSD = wabi::TfToken("PSD");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_JP2 = wabi::TfToken("JP2");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_HDR = wabi::TfToken("HDR");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_RADHDR = wabi::TfToken("RADHDR");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_TIFF = wabi::TfToken("TIFF");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_TIF = wabi::TfToken("TIF");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_CINEON = wabi::TfToken("CINEON");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_DPX = wabi::TfToken("DPX");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_DDS = wabi::TfToken("DDS");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_MPEG = wabi::TfToken("MPEG");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_TOKEN_WEBP = wabi::TfToken("WEBP");
};

struct ImbFileTypeDotExtToken
{
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_PNG = wabi::TfToken(".png");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_TGA = wabi::TfToken(".tga");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_RAWTGA = wabi::TfToken(".tga");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_JPG = wabi::TfToken(".jpg");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_JPEG = wabi::TfToken(".jpeg");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_OPEN_EXR = wabi::TfToken(".exr");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_OPEN_EXR_MULTILAYER = wabi::TfToken(".exr");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_IMAGIC = wabi::TfToken(".exr");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_BMP = wabi::TfToken(".bmp");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_IRIS = wabi::TfToken(".iris");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_IRIZ = wabi::TfToken(".iriz");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_AVIRAW = wabi::TfToken(".avi");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_AVIJPEG = wabi::TfToken(".avi");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_PSD = wabi::TfToken(".psd");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_JP2 = wabi::TfToken(".jp2");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_HDR = wabi::TfToken(".hdr");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_RADHDR = wabi::TfToken(".hdr");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_TIFF = wabi::TfToken(".tiff");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_TIF = wabi::TfToken(".tif");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_CINEON = wabi::TfToken(".cin");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_DPX = wabi::TfToken(".dpx");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_DDS = wabi::TfToken(".dds");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_MPEG = wabi::TfToken(".mpeg");
  KLI_INLINE wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_WEBP = wabi::TfToken(".webp");
};
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

#define IM_MAX_SPACE 64

struct rctf;
struct rcti;

struct ColorManagedDisplay;

#define IMB_MIPMAP_LEVELS 20
#define IMB_FILENAME_SIZE 1024

typedef struct ImbFormatOptions
{
  short flag;
  /** Quality serves dual purpose as quality number for JPEG or compression amount for PNG. */
  char quality;
} ImbFormatOptions;

typedef struct DDSData
{
  /** DDS fourcc info */
  unsigned int fourcc;
  /** The number of mipmaps in the dds file */
  unsigned int nummipmaps;
  /** The compressed image data */
  unsigned char *data;
  /** The size of the compressed data */
  unsigned int size;
} DDSData;

enum eImbFileType
{
  IMB_FTYPE_PNG = 1,
  IMB_FTYPE_TGA = 2,
  IMB_FTYPE_JPG = 3,
  IMB_FTYPE_BMP = 4,
  IMB_FTYPE_OPENEXR = 5,
  IMB_FTYPE_IMAGIC = 6,
  IMB_FTYPE_PSD = 7,
  IMB_FTYPE_JP2 = 8,
  IMB_FTYPE_RADHDR = 9,
  IMB_FTYPE_TIF = 10,
  IMB_FTYPE_CINEON = 11,
  IMB_FTYPE_DPX = 12,
  IMB_FTYPE_DDS = 13,
  IMB_FTYPE_WEBP = 14,
};

typedef struct ImBuf
{
  int x, y;
  unsigned char planes;
  int channels;
  int flags;
  int mall;
  unsigned int *rect;
  float *rect_float;
  double ppm[2];
  int tilex, tiley;
  int xtiles, ytiles;
  unsigned int **tiles;
  int *zbuf;
  float *zbuf_float;
  float dither;
  struct ImBuf *mipmap[IMB_MIPMAP_LEVELS];
  int miptot, miplevel;
  int index;
  int userflags;
  struct IDProperty *metadata;
  void *userdata;
  enum eImbFileType ftype;
  ImbFormatOptions foptions;
  char name[IMB_FILENAME_SIZE];
  char cachename[IMB_FILENAME_SIZE];
  struct MEM_CacheLimiterHandle_s *c_handle;
  int refcounter;
  unsigned char *encodedbuffer;
  unsigned int encodedsize;
  unsigned int encodedbuffersize;
  struct ColorSpace *rect_colorspace;
  struct ColorSpace *float_colorspace;
  unsigned int *display_buffer_flags;
  struct ColormanageCache *colormanage_cache;
  int colormanage_flag;
  rcti invalid_rect;

  /* information for compressed textures */
  struct DDSData dds_data;
} ImBuf;


typedef enum IMB_BlendMode
{
  IMB_BLEND_MIX = 0,
  IMB_BLEND_ADD = 1,
  IMB_BLEND_SUB = 2,
  IMB_BLEND_MUL = 3,
  IMB_BLEND_LIGHTEN = 4,
  IMB_BLEND_DARKEN = 5,
  IMB_BLEND_ERASE_ALPHA = 6,
  IMB_BLEND_ADD_ALPHA = 7,
  IMB_BLEND_OVERLAY = 8,
  IMB_BLEND_HARDLIGHT = 9,
  IMB_BLEND_COLORBURN = 10,
  IMB_BLEND_LINEARBURN = 11,
  IMB_BLEND_COLORDODGE = 12,
  IMB_BLEND_SCREEN = 13,
  IMB_BLEND_SOFTLIGHT = 14,
  IMB_BLEND_PINLIGHT = 15,
  IMB_BLEND_VIVIDLIGHT = 16,
  IMB_BLEND_LINEARLIGHT = 17,
  IMB_BLEND_DIFFERENCE = 18,
  IMB_BLEND_EXCLUSION = 19,
  IMB_BLEND_HUE = 20,
  IMB_BLEND_SATURATION = 21,
  IMB_BLEND_LUMINOSITY = 22,
  IMB_BLEND_COLOR = 23,
  IMB_BLEND_INTERPOLATE = 24,

  IMB_BLEND_COPY = 1000,
  IMB_BLEND_COPY_RGB = 1001,
  IMB_BLEND_COPY_ALPHA = 1002,
} IMB_BlendMode;

typedef enum eImBufFlags
{
  IB_rect = 1 << 0,
  IB_test = 1 << 1,
  IB_zbuf = 1 << 3,
  IB_mem = 1 << 4,
  IB_rectfloat = 1 << 5,
  IB_zbuffloat = 1 << 6,
  IB_multilayer = 1 << 7,
  IB_metadata = 1 << 8,
  IB_animdeinterlace = 1 << 9,
  IB_tiles = 1 << 10,
  IB_tilecache = 1 << 11,
  /** indicates whether image on disk have premul alpha */
  IB_alphamode_premul = 1 << 12,
  /** if this flag is set, alpha mode would be guessed from file */
  IB_alphamode_detect = 1 << 13,
  /* alpha channel is unrelated to RGB and should not affect it */
  IB_alphamode_channel_packed = 1 << 14,
  /** ignore alpha on load and substitute it with 1.0f */
  IB_alphamode_ignore = 1 << 15,
  IB_thumbnail = 1 << 16,
  IB_multiview = 1 << 17,
  IB_halffloat = 1 << 18,
} eImBufFlags;

/** Image formats that can only be loaded via filepath. */
extern const char *imb_ext_image_filepath_only[];

void IMB_init(void);
void IMB_exit(void);

/**
 * @attention Defined in rectop.c
 */

void IMB_blend_color_byte(unsigned char dst[4],
                          const unsigned char src1[4],
                          const unsigned char src2[4],
                          IMB_BlendMode mode);
void IMB_blend_color_float(float dst[4],
                           const float src1[4],
                           const float src2[4],
                           IMB_BlendMode mode);

/**
 * In-place image crop.
 */
void IMB_rect_crop(struct ImBuf *ibuf, const struct rcti *crop);

/**
 * In-place size setting (caller must fill in buffer contents).
 */
void IMB_rect_size_set(struct ImBuf *ibuf, const uint size[2]);

void IMB_rectclip(struct ImBuf *dbuf,
                  const struct ImBuf *sbuf,
                  int *destx,
                  int *desty,
                  int *srcx,
                  int *srcy,
                  int *width,
                  int *height);
void IMB_rectcpy(struct ImBuf *dbuf,
                 const struct ImBuf *sbuf,
                 int destx,
                 int desty,
                 int srcx,
                 int srcy,
                 int width,
                 int height);
void IMB_rectblend(struct ImBuf *dbuf,
                   const struct ImBuf *obuf,
                   const struct ImBuf *sbuf,
                   unsigned short *dmask,
                   const unsigned short *curvemask,
                   const unsigned short *texmask,
                   float mask_max,
                   int destx,
                   int desty,
                   int origx,
                   int origy,
                   int srcx,
                   int srcy,
                   int width,
                   int height,
                   IMB_BlendMode mode,
                   bool accumulate);
void IMB_rectblend_threaded(struct ImBuf *dbuf,
                            const struct ImBuf *obuf,
                            const struct ImBuf *sbuf,
                            unsigned short *dmask,
                            const unsigned short *curvemask,
                            const unsigned short *texmask,
                            float mask_max,
                            int destx,
                            int desty,
                            int origx,
                            int origy,
                            int srcx,
                            int srcy,
                            int width,
                            int height,
                            IMB_BlendMode mode,
                            bool accumulate);

/**
 * @attention Defined in readimage.c
 */
struct ImBuf *IMB_ibImageFromMemory(const unsigned char *mem,
                                    size_t size,
                                    int flags,
                                    char colorspace[IM_MAX_SPACE],
                                    const char *descr);
struct ImBuf *IMB_testiffname(const char *filepath, int flags);
struct ImBuf *IMB_loadiffname(const char *filepath, int flags, char colorspace[IM_MAX_SPACE]);
struct ImBuf *IMB_thumb_load_image(const char *filepath,
                                   const size_t max_thumb_size,
                                   char colorspace[IM_MAX_SPACE]);

/**
 * Change the ordering of the color bytes pointed to by rect from
 * rgba to abgr. size * 4 color bytes are reordered.
 *
 * @attention Defined in imageprocess.c
 *
 * Only this one is used liberally here, and in imbuf.
 */
void IMB_convert_rgba_to_abgr(struct ImBuf *ibuf);

/**
 * @attention defined in imageprocess.c
 */

void bicubic_interpolation(const struct ImBuf *in,
                           struct ImBuf *out,
                           float u,
                           float v,
                           int xout,
                           int yout);
void nearest_interpolation(const struct ImBuf *in,
                           struct ImBuf *out,
                           float u,
                           float v,
                           int xout,
                           int yout);
void bilinear_interpolation(const struct ImBuf *in,
                            struct ImBuf *out,
                            float u,
                            float v,
                            int xout,
                            int yout);

typedef void (*InterpolationColorFunction)(const struct ImBuf *in,
                                           unsigned char outI[4],
                                           float outF[4],
                                           float u,
                                           float v);
void bicubic_interpolation_color(const struct ImBuf *in,
                                 unsigned char outI[4],
                                 float outF[4],
                                 float u,
                                 float v);

/* Functions assumes out to be zeroed, only does RGBA. */

void nearest_interpolation_color_char(const struct ImBuf *in,
                                      unsigned char outI[4],
                                      float outF[4],
                                      float u,
                                      float v);
void nearest_interpolation_color_fl(const struct ImBuf *in,
                                    unsigned char outI[4],
                                    float outF[4],
                                    float u,
                                    float v);
void nearest_interpolation_color(const struct ImBuf *in,
                                 unsigned char outI[4],
                                 float outF[4],
                                 float u,
                                 float v);
void nearest_interpolation_color_wrap(const struct ImBuf *in,
                                      unsigned char outI[4],
                                      float outF[4],
                                      float u,
                                      float v);
void bilinear_interpolation_color(const struct ImBuf *in,
                                  unsigned char outI[4],
                                  float outF[4],
                                  float u,
                                  float v);
void bilinear_interpolation_color_char(const struct ImBuf *in,
                                       unsigned char outI[4],
                                       float outF[4],
                                       float u,
                                       float v);
void bilinear_interpolation_color_fl(const struct ImBuf *in,
                                     unsigned char outI[4],
                                     float outF[4],
                                     float u,
                                     float v);
/**
 * Note about wrapping, the u/v still needs to be within the image bounds,
 * just the interpolation is wrapped.
 * This the same as bilinear_interpolation_color except it wraps
 * rather than using empty and emptyI.
 */
void bilinear_interpolation_color_wrap(const struct ImBuf *in,
                                       unsigned char outI[4],
                                       float outF[4],
                                       float u,
                                       float v);

void IMB_alpha_under_color_float(float *rect_float, int x, int y, float backcol[3]);
void IMB_alpha_under_color_byte(unsigned char *rect, int x, int y, const float backcol[3]);

/**
 * Sample pixel of image using NEAREST method.
 */
void IMB_sampleImageAtLocation(struct ImBuf *ibuf,
                               float x,
                               float y,
                               bool make_linear_rgb,
                               float color[4]);

typedef void (*ScanlineThreadFunc)(void *custom_data, int scanline);
void IMB_processor_apply_threaded_scanlines(int total_scanlines,
                                            ScanlineThreadFunc do_thread,
                                            void *custom_data);

#ifdef __cplusplus
}
#endif