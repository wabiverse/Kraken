/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#include "wabi/imaging/cameraUtil/screenWindowParameters.h"

WABI_NAMESPACE_BEGIN

CameraUtilScreenWindowParameters::CameraUtilScreenWindowParameters(const GfCamera &camera,
                                                                   GfCamera::FOVDirection fitDirection)
  : _screenWindow(-camera.GetHorizontalAperture() + 2 * camera.GetHorizontalApertureOffset(),
                  +camera.GetHorizontalAperture() + 2 * camera.GetHorizontalApertureOffset(),
                  -camera.GetVerticalAperture() + 2 * camera.GetVerticalApertureOffset(),
                  +camera.GetVerticalAperture() + 2 * camera.GetVerticalApertureOffset()),
    _fieldOfView(camera.GetFieldOfView(fitDirection))
{
  if (camera.GetProjection() == GfCamera::Perspective)
  {
    const double denom = (fitDirection == GfCamera::FOVHorizontal) ? camera.GetHorizontalAperture() :
                                                                     camera.GetVerticalAperture();
    if (denom != 0.0)
    {
      _screenWindow /= denom;
    }
  } else
  {
    _screenWindow *= GfCamera::APERTURE_UNIT / 2.0;
  }

  static const GfMatrix4d zFlip(GfVec4d(1, 1, -1, 1));
  _zFacingViewMatrix = (zFlip * camera.GetTransform()).GetInverse();
}

WABI_NAMESPACE_END
