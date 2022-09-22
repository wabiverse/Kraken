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

#ifndef __OCIO_CAPI_H__
#define __OCIO_CAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OCIO_GPUShader OCIO_GPUShader;

#define OCIO_DECLARE_HANDLE(name) \
  typedef struct name##__ { \
    int unused; \
  } * name

#define OCIO_ROLE_DATA "data"
#define OCIO_ROLE_SCENE_LINEAR "scene_linear"
#define OCIO_ROLE_COLOR_PICKING "color_picking"
#define OCIO_ROLE_TEXTURE_PAINT "texture_paint"
#define OCIO_ROLE_DEFAULT_BYTE "default_byte"
#define OCIO_ROLE_DEFAULT_FLOAT "default_float"
#define OCIO_ROLE_DEFAULT_SEQUENCER "default_sequencer"

OCIO_DECLARE_HANDLE(OCIO_ConstConfigRcPtr);
OCIO_DECLARE_HANDLE(OCIO_ConstColorSpaceRcPtr);
OCIO_DECLARE_HANDLE(OCIO_ConstProcessorRcPtr);
OCIO_DECLARE_HANDLE(OCIO_ConstCPUProcessorRcPtr);
OCIO_DECLARE_HANDLE(OCIO_ConstContextRcPtr);
OCIO_DECLARE_HANDLE(OCIO_PackedImageDesc);
OCIO_DECLARE_HANDLE(OCIO_ConstLookRcPtr);

/* Standard XYZ (D65) to linear Rec.709 transform. */
static const float OCIO_XYZ_TO_REC709[3][3] = {
  {3.2404542f,  -0.9692660f, 0.0556434f },
  {-1.5371385f, 1.8760108f,  -0.2040259f},
  {-0.4985314f, 0.0415560f,  1.0572252f }
};
/* Standard ACES to XYZ (D65) transform.
 * Matches OpenColorIO builtin transform: UTILITY - ACES-AP0_to_CIE-XYZ-D65_BFD. */
static const float OCIO_ACES_TO_XYZ[3][3] = {
  {0.938280f,  0.337369f,  0.001174f },
  {-0.004451f, 0.729522f,  -0.003711f},
  {0.016628f,  -0.066890f, 1.091595f }
};

void OCIO_init(void);
void OCIO_exit(void);

OCIO_ConstConfigRcPtr *OCIO_getCurrentConfig(void);
void OCIO_setCurrentConfig(const OCIO_ConstConfigRcPtr *config);

void OCIO_configRelease(OCIO_ConstConfigRcPtr *config);

const char *OCIO_configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config);

#ifdef __cplusplus
}
#endif

#endif /* __OCIO_CAPI_H__ */