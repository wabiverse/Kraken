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
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_PNG = wabi::TfToken("PNG");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_TGA = wabi::TfToken("TGA");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_RAWTGA = wabi::TfToken("RAWTGA");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_JPG = wabi::TfToken("JPG");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_JPEG = wabi::TfToken("JPEG");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_OPEN_EXR = wabi::TfToken("OPEN_EXR");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_OPEN_EXR_MULTILAYER = wabi::TfToken("OPEN_EXR_MULTILAYER");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_IMAGIC = wabi::TfToken("OPEN_EXR_IMAGIC");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_BMP = wabi::TfToken("BMP");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_IRIS = wabi::TfToken("IRIS");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_IRIZ = wabi::TfToken("IRIZ");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_AVIRAW = wabi::TfToken("AVIRAW");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_AVIJPEG = wabi::TfToken("AVIJPEG");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_PSD = wabi::TfToken("PSD");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_JP2 = wabi::TfToken("JP2");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_HDR = wabi::TfToken("HDR");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_RADHDR = wabi::TfToken("RADHDR");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_TIFF = wabi::TfToken("TIFF");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_TIF = wabi::TfToken("TIF");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_CINEON = wabi::TfToken("CINEON");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_DPX = wabi::TfToken("DPX");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_DDS = wabi::TfToken("DDS");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_MPEG = wabi::TfToken("MPEG");
  const inline static wabi::TfToken R_IMF_IMTYPE_TOKEN_WEBP = wabi::TfToken("WEBP");
};

struct ImbFileTypeDotExtToken
{
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_PNG = wabi::TfToken(".png");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_TGA = wabi::TfToken(".tga");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_RAWTGA = wabi::TfToken(".tga");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_JPG = wabi::TfToken(".jpg");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_JPEG = wabi::TfToken(".jpeg");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_OPEN_EXR = wabi::TfToken(".exr");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_OPEN_EXR_MULTILAYER = wabi::TfToken(".exr");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_IMAGIC = wabi::TfToken(".exr");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_BMP = wabi::TfToken(".bmp");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_IRIS = wabi::TfToken(".iris");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_IRIZ = wabi::TfToken(".iriz");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_AVIRAW = wabi::TfToken(".avi");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_AVIJPEG = wabi::TfToken(".avi");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_PSD = wabi::TfToken(".psd");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_JP2 = wabi::TfToken(".jp2");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_HDR = wabi::TfToken(".hdr");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_RADHDR = wabi::TfToken(".hdr");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_TIFF = wabi::TfToken(".tiff");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_TIF = wabi::TfToken(".tif");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_CINEON = wabi::TfToken(".cin");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_DPX = wabi::TfToken(".dpx");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_DDS = wabi::TfToken(".dds");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_MPEG = wabi::TfToken(".mpeg");
  const inline static wabi::TfToken R_IMF_IMTYPE_DOT_EXT_TOKEN_WEBP = wabi::TfToken(".webp");
};
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

#define IM_MAX_SPACE 64

struct ImBuf;
struct rctf;
struct rcti;

struct anim;

struct ColorManagedDisplay;

struct RSet;

struct ImageFormatData;
struct Stereo3dFormat;

#define IMB_MIPMAP_LEVELS 20
#define IMB_FILEPATH_SIZE 1024

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
  struct IDProperty *metadata;
  /** temporary storage */
  void *userdata;

  /* file information */
  /** file type we are going to save as */
  enum eImbFileType ftype;
  /** file format specific flags */
  ImbFormatOptions foptions;
  /** The absolute file path associated with this image. */
  char filepath[IMB_FILEPATH_SIZE];

  /* memory cache limiter */
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

void IMB_freeImBuf(struct ImBuf *ibuf);

struct ImBuf *IMB_allocImBuf(unsigned int x,
                             unsigned int y,
                             unsigned char planes,
                             unsigned int flags);

/**
 * Initialize given ImBuf.
 *
 * Use in cases when temporary image buffer is allocated on stack.
 *
 * @attention Defined in allocimbuf.cc
 */
bool IMB_initImBuf(struct ImBuf *ibuf, unsigned int x, unsigned int y, unsigned char planes, unsigned int flags);

/**
 * Create a copy of a pixel buffer and wrap it to a new ImBuf
 * (transferring ownership to the in imbuf).
 * @attention Defined in allocimbuf.cc
 */
struct ImBuf *IMB_allocFromBufferOwn(unsigned int *rect, float *rectf, unsigned int w, unsigned int h, unsigned int channels);

/**
 * Create a copy of a pixel buffer and wrap it to a new ImBuf
 * @attention Defined in allocimbuf.cc
 */
struct ImBuf *IMB_allocFromBuffer(const unsigned int *rect,
                                  const float *rectf,
                                  unsigned int w,
                                  unsigned int h,
                                  unsigned int channels);

/**
 * Increase reference count to imbuf
 * (to delete an imbuf you have to call freeImBuf as many times as it
 * is referenced)
 *
 * @attention Defined in allocimbuf.cc
 */

void IMB_refImBuf(struct ImBuf *ibuf);
struct ImBuf *IMB_makeSingleUser(struct ImBuf *ibuf);

/**
 * @attention Defined in allocimbuf.cc
 */
struct ImBuf *IMB_dupImBuf(const struct ImBuf *ibuf1);

/**
 * @attention Defined in allocimbuf.cc
 */
bool addzbufImBuf(struct ImBuf *ibuf);
bool addzbuffloatImBuf(struct ImBuf *ibuf);

/**
 * Approximate size of ImBuf in memory
 *
 * @attention Defined in allocimbuf.cc
 */
size_t IMB_get_size_in_memory(struct ImBuf *ibuf);

/**
 * @brief Get the length of the rect of the given image buffer in terms of pixels.
 *
 * This is the width * the height of the image buffer.
 * This function is preferred over `ibuf->x * ibuf->y` due to overflow issues when
 * working with large resolution images (30kx30k).
 *
 * @attention Defined in allocimbuf.cc
 */
size_t IMB_get_rect_len(const struct ImBuf *ibuf);

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


#ifdef __cplusplus
/**
 * @UTILITY: "Out of the box" supported file types for quick lookups.
 * Quickly flip a token into a #ImageFormatData.imtype.
 */
int IMF_imtype_from_token(const wabi::TfToken &ftype);
const wabi::TfToken IMF_imtype_dotext_from_token(const wabi::TfToken &ftype);

#endif /* __cplusplus */
