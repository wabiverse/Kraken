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

#ifndef __OCIO_IMPL_H__
#define __OCIO_IMPL_H__

#include "ocio_capi.h"

class IOCIOImpl
{
 public:

  virtual ~IOCIOImpl() {}

  virtual OCIO_ConstConfigRcPtr *getCurrentConfig(void) = 0;
  virtual void setCurrentConfig(const OCIO_ConstConfigRcPtr *config) = 0;

  virtual OCIO_ConstConfigRcPtr *configCreateFromEnv(void) = 0;
  virtual OCIO_ConstConfigRcPtr *configCreateFromFile(const char *filename) = 0;

  virtual void configRelease(OCIO_ConstConfigRcPtr *config) = 0;

  virtual int configGetNumColorSpaces(OCIO_ConstConfigRcPtr *config) = 0;
  virtual const char *configGetColorSpaceNameByIndex(OCIO_ConstConfigRcPtr *config, int index) = 0;
  virtual OCIO_ConstColorSpaceRcPtr *configGetColorSpace(OCIO_ConstConfigRcPtr *config,
                                                         const char *name) = 0;
  virtual int configGetIndexForColorSpace(OCIO_ConstConfigRcPtr *config, const char *name) = 0;


  virtual const char *configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config) = 0;

  virtual OCIO_ConstProcessorRcPtr *configGetProcessorWithNames(OCIO_ConstConfigRcPtr *config,
                                                                const char *srcName,
                                                                const char *dstName) = 0;

  virtual void processorRelease(OCIO_ConstProcessorRcPtr *processor) = 0;
  virtual void cpuProcessorRelease(OCIO_ConstCPUProcessorRcPtr *cpu_processor) = 0;

  virtual const char *colorSpaceGetName(OCIO_ConstColorSpaceRcPtr *cs) = 0;
  virtual void colorSpaceRelease(OCIO_ConstColorSpaceRcPtr *cs) = 0;
  virtual const char *colorSpaceGetDescription(OCIO_ConstColorSpaceRcPtr *cs) = 0;
  virtual int colorSpaceIsInvertible(OCIO_ConstColorSpaceRcPtr *cs) = 0;
  virtual int colorSpaceIsData(OCIO_ConstColorSpaceRcPtr *cs) = 0;
  virtual int colorSpaceGetNumAliases(OCIO_ConstColorSpaceRcPtr *cs) = 0;
  virtual const char *colorSpaceGetAlias(OCIO_ConstColorSpaceRcPtr *cs, const int index) = 0;

  virtual int configGetNumDisplays(OCIO_ConstConfigRcPtr *config) = 0;
  virtual const char *configGetDefaultView(OCIO_ConstConfigRcPtr *config, const char *display) = 0;
  virtual const char *configGetDisplay(OCIO_ConstConfigRcPtr *config, int index) = 0;
  virtual int configGetNumViews(OCIO_ConstConfigRcPtr *config, const char *display) = 0;
  virtual const char *configGetView(OCIO_ConstConfigRcPtr *config,
                                    const char *display,
                                    int index) = 0;

  virtual int configGetNumLooks(OCIO_ConstConfigRcPtr *config) = 0;
  virtual const char *configGetLookNameByIndex(OCIO_ConstConfigRcPtr *config, int index) = 0;     
  virtual OCIO_ConstLookRcPtr *configGetLook(OCIO_ConstConfigRcPtr *config, const char *name) = 0;

  virtual const char *lookGetProcessSpace(OCIO_ConstLookRcPtr *look) = 0;
  virtual void lookRelease(OCIO_ConstLookRcPtr *look) = 0;

  virtual void configGetDefaultLumaCoefs(OCIO_ConstConfigRcPtr *config, float *rgb) = 0;
  virtual void configGetXYZtoSceneLinear(OCIO_ConstConfigRcPtr *config,
                                        float xyz_to_scene_linear[3][3]) = 0;

  virtual OCIO_ConstProcessorRcPtr *createDisplayProcessor(OCIO_ConstConfigRcPtr *config,
                                                           const char *input,
                                                           const char *view,
                                                           const char *display,
                                                           const char *look,
                                                           const float scale,
                                                           const float exponent,
                                                           const bool inverse) = 0;

  /* Optional GPU support. */
  virtual bool supportGPUShader()
  {
    return false;
  }

  virtual bool gpuDisplayShaderBind(OCIO_ConstConfigRcPtr * /*config*/,
                                    const char * /*input*/,
                                    const char * /*view*/,
                                    const char * /*display*/,
                                    const char * /*look*/,
                                    OCIO_CurveMappingSettings * /*curve_mapping_settings*/,
                                    const float /*scale*/,
                                    const float /*exponent*/,
                                    const float /*dither*/,
                                    const bool /*use_predivide*/,
                                    const bool /*use_overlay*/)
  {
    return false;
  }

  virtual void gpuDisplayShaderUnbind(void) {}
  virtual void gpuCacheFree(void) {}
};

class OCIOImpl : public IOCIOImpl
{
 public:

  OCIOImpl(){};

  OCIO_ConstConfigRcPtr *getCurrentConfig(void);
  void setCurrentConfig(const OCIO_ConstConfigRcPtr *config);

  OCIO_ConstConfigRcPtr *configCreateFromEnv(void);
  OCIO_ConstConfigRcPtr *configCreateFromFile(const char *filename);

  void configRelease(OCIO_ConstConfigRcPtr *config);

  int configGetNumColorSpaces(OCIO_ConstConfigRcPtr *config);
  const char *configGetColorSpaceNameByIndex(OCIO_ConstConfigRcPtr *config, int index);
  OCIO_ConstColorSpaceRcPtr *configGetColorSpace(OCIO_ConstConfigRcPtr *config, const char *name);
  int configGetIndexForColorSpace(OCIO_ConstConfigRcPtr *config, const char *name);

  const char *configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config);
  const char *configGetDisplay(OCIO_ConstConfigRcPtr *config, int index);
  int configGetNumViews(OCIO_ConstConfigRcPtr *config, const char *display);
  const char *configGetView(OCIO_ConstConfigRcPtr *config, const char *display, int index);

  int configGetNumLooks(OCIO_ConstConfigRcPtr *config);
  const char *configGetLookNameByIndex(OCIO_ConstConfigRcPtr *config, int index);
  OCIO_ConstLookRcPtr *configGetLook(OCIO_ConstConfigRcPtr *config, const char *name);

  const char *lookGetProcessSpace(OCIO_ConstLookRcPtr *look);
  void lookRelease(OCIO_ConstLookRcPtr *look);

  void configGetDefaultLumaCoefs(OCIO_ConstConfigRcPtr *config, float *rgb);
  void configGetXYZtoSceneLinear(OCIO_ConstConfigRcPtr *config, float xyz_to_scene_linear[3][3]);

  OCIO_ConstProcessorRcPtr *configGetProcessorWithNames(OCIO_ConstConfigRcPtr *config,
                                                        const char *srcName,
                                                        const char *dstName);

  void processorRelease(OCIO_ConstProcessorRcPtr *processor);
  void cpuProcessorRelease(OCIO_ConstCPUProcessorRcPtr *cpu_processor);

  const char *colorSpaceGetName(OCIO_ConstColorSpaceRcPtr *cs);
  void colorSpaceRelease(OCIO_ConstColorSpaceRcPtr *cs);
  const char *colorSpaceGetDescription(OCIO_ConstColorSpaceRcPtr *cs);
  int colorSpaceIsInvertible(OCIO_ConstColorSpaceRcPtr *cs);
  int colorSpaceIsData(OCIO_ConstColorSpaceRcPtr *cs);
  int colorSpaceGetNumAliases(OCIO_ConstColorSpaceRcPtr *cs);
  const char *colorSpaceGetAlias(OCIO_ConstColorSpaceRcPtr *cs, const int index);

  int configGetNumDisplays(OCIO_ConstConfigRcPtr *config);
  const char *configGetDefaultView(OCIO_ConstConfigRcPtr *config, const char *display);

  OCIO_ConstProcessorRcPtr *createDisplayProcessor(OCIO_ConstConfigRcPtr *config,
                                                   const char *input,
                                                   const char *view,
                                                   const char *display,
                                                   const char *look,
                                                   const float scale,
                                                   const float exponent,
                                                   const bool inverse);

  bool supportGPUShader();

  bool gpuDisplayShaderBind(OCIO_ConstConfigRcPtr *config,
                            const char *input,
                            const char *view,
                            const char *display,
                            const char *look,
                            OCIO_CurveMappingSettings *curve_mapping_settings,
                            const float scale,
                            const float exponent,
                            const float dither,
                            const bool use_predivide,
                            const bool use_overlay);

  void gpuDisplayShaderUnbind(void);
  void gpuCacheFree(void);
};

#endif /* __OCIO_IMPL_H__ */