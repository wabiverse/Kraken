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

#include <cassert>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string.h>

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4251 4275)
#endif
#include <OpenColorIO/OpenColorIO.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace OCIO_NAMESPACE;

#include "MEM_guardedalloc.h"

#include "KLI_math.h"
#include "KLI_math_color.h"
#include "KLI_math_matrix.h"

#include "ocio_impl.h"

#if !defined(WITH_ASSERT_ABORT)
#  define OCIO_abort()
#else
#  include <stdlib.h>
#  define OCIO_abort() abort()
#endif

#if defined(_MSC_VER)
#  define __func__ __FUNCTION__
#endif

static void OCIO_reportError(const char *err)
{
  std::cerr << "OpenColorIO Error: " << err << std::endl;

  OCIO_abort();
}

static void OCIO_reportException(Exception &exception)
{
  OCIO_reportError(exception.what());
}

OCIO_ConstConfigRcPtr *OCIOImpl::getCurrentConfig(void)
{
  ConstConfigRcPtr *config = MEM_new<ConstConfigRcPtr>(__func__);

  try {
    *config = GetCurrentConfig();

    if (*config)
      return (OCIO_ConstConfigRcPtr *)config;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(config);

  return NULL;
}

void OCIOImpl::setCurrentConfig(const OCIO_ConstConfigRcPtr *config)
{
  try {
    SetCurrentConfig(*(ConstConfigRcPtr *)config);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }
}

OCIO_ConstConfigRcPtr *OCIOImpl::configCreateFromEnv(void)
{
  ConstConfigRcPtr *config = MEM_new<ConstConfigRcPtr>(__func__);

  try {
    *config = Config::CreateFromEnv();

    if (*config)
      return (OCIO_ConstConfigRcPtr *)config;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(config);

  return NULL;
}

OCIO_ConstConfigRcPtr *OCIOImpl::configCreateFromFile(const char *filename)
{
  ConstConfigRcPtr *config = MEM_new<ConstConfigRcPtr>(__func__);

  try {
    *config = Config::CreateFromFile(filename);

    if (*config)
      return (OCIO_ConstConfigRcPtr *)config;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(config);

  return NULL;
}

void OCIOImpl::configRelease(OCIO_ConstConfigRcPtr *config)
{
  MEM_delete((ConstConfigRcPtr *)config);
}

int OCIOImpl::configGetNumColorSpaces(OCIO_ConstConfigRcPtr *config)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getNumColorSpaces();
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return 0;
}

const char *OCIOImpl::configGetColorSpaceNameByIndex(OCIO_ConstConfigRcPtr *config, int index)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getColorSpaceNameByIndex(index);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}

OCIO_ConstColorSpaceRcPtr *OCIOImpl::configGetColorSpace(OCIO_ConstConfigRcPtr *config,
                                                         const char *name)
{
  ConstColorSpaceRcPtr *cs = MEM_new<ConstColorSpaceRcPtr>(__func__);

  try {
    *cs = (*(ConstConfigRcPtr *)config)->getColorSpace(name);

    if (*cs)
      return (OCIO_ConstColorSpaceRcPtr *)cs;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(cs);

  return NULL;
}

int OCIOImpl::configGetIndexForColorSpace(OCIO_ConstConfigRcPtr *config, const char *name)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getIndexForColorSpace(name);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return -1;
}

const char *OCIOImpl::configGetDefaultDisplay(OCIO_ConstConfigRcPtr *config)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getDefaultDisplay();
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}

OCIO_ConstProcessorRcPtr *OCIOImpl::configGetProcessorWithNames(OCIO_ConstConfigRcPtr *config,
                                                                const char *srcName,
                                                                const char *dstName)
{
  ConstProcessorRcPtr *processor = MEM_new<ConstProcessorRcPtr>(__func__);

  try {
    *processor = (*(ConstConfigRcPtr *)config)->getProcessor(srcName, dstName);

    if (*processor)
      return (OCIO_ConstProcessorRcPtr *)processor;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(processor);

  return 0;
}

void OCIOImpl::cpuProcessorRelease(OCIO_ConstCPUProcessorRcPtr *cpu_processor)
{
  MEM_delete(cpu_processor);
}

void OCIOImpl::processorRelease(OCIO_ConstProcessorRcPtr *processor)
{
  MEM_delete(processor);
}

const char *OCIOImpl::colorSpaceGetName(OCIO_ConstColorSpaceRcPtr *cs)
{
  return (*(ConstColorSpaceRcPtr *)cs)->getName();
}

void OCIOImpl::colorSpaceRelease(OCIO_ConstColorSpaceRcPtr *cs)
{
  MEM_delete((ConstColorSpaceRcPtr *)cs);
}

const char *OCIOImpl::colorSpaceGetDescription(OCIO_ConstColorSpaceRcPtr *cs)
{
  return (*(ConstColorSpaceRcPtr *)cs)->getDescription();
}

int OCIOImpl::colorSpaceIsInvertible(OCIO_ConstColorSpaceRcPtr *cs_)
{
  ConstColorSpaceRcPtr *cs = (ConstColorSpaceRcPtr *)cs_;
  const char *family = (*cs)->getFamily();

  if (!strcmp(family, "rrt") || !strcmp(family, "display")) {
    /* assume display and rrt transformations are not invertible in fact some of them could be,
     * but it doesn't make much sense to allow use them as invertible. */
    return false;
  }

  if ((*cs)->isData()) {
    /* data color spaces don't have transformation at all */
    return true;
  }

  if ((*cs)->getTransform(COLORSPACE_DIR_TO_REFERENCE)) {
    /* if there's defined transform to reference space,
     * color space could be converted to scene linear. */
    return true;
  }

  return true;
}

int OCIOImpl::colorSpaceIsData(OCIO_ConstColorSpaceRcPtr *cs)
{
  return (*(ConstColorSpaceRcPtr *)cs)->isData();
}

int OCIOImpl::colorSpaceGetNumAliases(OCIO_ConstColorSpaceRcPtr *cs)
{
  return (*(ConstColorSpaceRcPtr *)cs)->getNumAliases();
}

const char *OCIOImpl::colorSpaceGetAlias(OCIO_ConstColorSpaceRcPtr *cs, const int index)
{
  return (*(ConstColorSpaceRcPtr *)cs)->getAlias(index);
}

int OCIOImpl::configGetNumDisplays(OCIO_ConstConfigRcPtr *config)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getNumDisplays();
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return 0;
}

const char *OCIOImpl::configGetDisplay(OCIO_ConstConfigRcPtr *config, int index)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getDisplay(index);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}

int OCIOImpl::configGetNumViews(OCIO_ConstConfigRcPtr *config, const char *display)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getNumViews(display);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return 0;
}

const char *OCIOImpl::configGetView(OCIO_ConstConfigRcPtr *config, const char *display, int index)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getView(display, index);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}

int OCIOImpl::configGetNumLooks(OCIO_ConstConfigRcPtr *config)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getNumLooks();
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return 0;
}

const char *OCIOImpl::configGetLookNameByIndex(OCIO_ConstConfigRcPtr *config, int index)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getLookNameByIndex(index);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}

OCIO_ConstLookRcPtr *OCIOImpl::configGetLook(OCIO_ConstConfigRcPtr *config, const char *name)
{
  ConstLookRcPtr *look = MEM_new<ConstLookRcPtr>(__func__);

  try {
    *look = (*(ConstConfigRcPtr *)config)->getLook(name);

    if (*look)
      return (OCIO_ConstLookRcPtr *)look;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(look);

  return NULL;
}

const char *OCIOImpl::lookGetProcessSpace(OCIO_ConstLookRcPtr *look)
{
  return (*(ConstLookRcPtr *)look)->getProcessSpace();
}

void OCIOImpl::lookRelease(OCIO_ConstLookRcPtr *look)
{
  MEM_delete((ConstLookRcPtr *)look);
}

void OCIOImpl::configGetDefaultLumaCoefs(OCIO_ConstConfigRcPtr *config, float *rgb)
{
  try {
    double rgb_double[3];
    (*(ConstConfigRcPtr *)config)->getDefaultLumaCoefs(rgb_double);
    rgb[0] = rgb_double[0];
    rgb[1] = rgb_double[1];
    rgb[2] = rgb_double[2];
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }
}

static bool to_scene_linear_matrix(ConstConfigRcPtr &config,
                                   const char *colorspace,
                                   float to_scene_linear[3][3])
{
  ConstProcessorRcPtr processor;
  try {
    processor = config->getProcessor(colorspace, ROLE_SCENE_LINEAR);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
    return false;
  }

  if (!processor) {
    return false;
  }

  ConstCPUProcessorRcPtr cpu_processor = processor->getDefaultCPUProcessor();
  if (!cpu_processor) {
    return false;
  }

  unit_m3(to_scene_linear);
  cpu_processor->applyRGB(to_scene_linear[0]);
  cpu_processor->applyRGB(to_scene_linear[1]);
  cpu_processor->applyRGB(to_scene_linear[2]);
  return true;
}

void OCIOImpl::configGetXYZtoSceneLinear(OCIO_ConstConfigRcPtr *config_,
                                         float xyz_to_scene_linear[3][3])
{
  ConstConfigRcPtr config = (*(ConstConfigRcPtr *)config_);

  /* Default to ITU-BT.709 in case no appropriate transform found.
   * Note XYZ is defined here as having a D65 white point. */
  memcpy(xyz_to_scene_linear, OCIO_XYZ_TO_REC709, sizeof(OCIO_XYZ_TO_REC709));

  /* Get from OpenColorO config if it has the required roles. */
  if (!config->hasRole(ROLE_SCENE_LINEAR)) {
    return;
  }

  if (config->hasRole("aces_interchange")) {
    /* Standard OpenColorIO role, defined as ACES AP0 (ACES2065-1). */
    float aces_to_scene_linear[3][3];
    if (to_scene_linear_matrix(config, "aces_interchange", aces_to_scene_linear)) {
      float xyz_to_aces[3][3];
      invert_m3_m3(xyz_to_aces, OCIO_ACES_TO_XYZ);

      mul_m3_m3m3(xyz_to_scene_linear, aces_to_scene_linear, xyz_to_aces);
    }
  }
  else if (config->hasRole("XYZ")) {
    /* Custom role used before the standard existed. */
    to_scene_linear_matrix(config, "XYZ", xyz_to_scene_linear);
  }
}

const char *OCIOImpl::configGetDefaultView(OCIO_ConstConfigRcPtr *config, const char *display)
{
  try {
    return (*(ConstConfigRcPtr *)config)->getDefaultView(display);
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  return NULL;
}

OCIO_ConstProcessorRcPtr *OCIOImpl::createDisplayProcessor(OCIO_ConstConfigRcPtr *config_,
                                                           const char *input,
                                                           const char *view,
                                                           const char *display,
                                                           const char *look,
                                                           const float scale,
                                                           const float exponent,
                                                           const bool inverse)

{
  ConstConfigRcPtr config = *(ConstConfigRcPtr *)config_;
  GroupTransformRcPtr group = GroupTransform::Create();

  /* Exposure. */
  if (scale != 1.0f) {
    /* Always apply exposure in scene linear. */
    ColorSpaceTransformRcPtr ct = ColorSpaceTransform::Create();
    ct->setSrc(input);
    ct->setDst(ROLE_SCENE_LINEAR);
    group->appendTransform(ct);

    /* Make further transforms aware of the color space change. */
    input = ROLE_SCENE_LINEAR;

    /* Apply scale. */
    MatrixTransformRcPtr mt = MatrixTransform::Create();
    const double matrix[16] =
      {scale, 0.0, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 0.0, 1.0};
    mt->setMatrix(matrix);
    group->appendTransform(mt);
  }

  /* Add look transform. */
  bool use_look = (look != nullptr && look[0] != 0);
  if (use_look) {
    const char *look_output = LookTransform::GetLooksResultColorSpace(config,
                                                                      config->getCurrentContext(),
                                                                      look);

    if (look_output != nullptr && look_output[0] != 0) {
      LookTransformRcPtr lt = LookTransform::Create();
      lt->setSrc(input);
      lt->setDst(look_output);
      lt->setLooks(look);
      group->appendTransform(lt);

      /* Make further transforms aware of the color space change. */
      input = look_output;
    } else {
      /* For empty looks, no output color space is returned. */
      use_look = false;
    }
  }

  /* Add view and display transform. */
  DisplayViewTransformRcPtr dvt = DisplayViewTransform::Create();
  dvt->setSrc(input);
  dvt->setLooksBypass(use_look);
  dvt->setView(view);
  dvt->setDisplay(display);
  group->appendTransform(dvt);

  /* Gamma. */
  if (exponent != 1.0f) {
    ExponentTransformRcPtr et = ExponentTransform::Create();
    const double value[4] = {exponent, exponent, exponent, 1.0};
    et->setValue(value);
    group->appendTransform(et);
  }

  if (inverse) {
    group->setDirection(TRANSFORM_DIR_INVERSE);
  }

  /* Create processor from transform. This is the moment were OCIO validates
   * the entire transform, no need to check for the validity of inputs above. */
  ConstProcessorRcPtr *p = MEM_new<ConstProcessorRcPtr>(__func__);

  try {
    *p = config->getProcessor(group);

    if (*p)
      return (OCIO_ConstProcessorRcPtr *)p;
  }
  catch (Exception &exception) {
    OCIO_reportException(exception);
  }

  MEM_delete(p);
  return NULL;
}