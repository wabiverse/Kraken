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
 * Copyright 2021, Wabi.
 */

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"

#if 0
void EditTransform(const Camera& camera, matrix_t& matrix)
{
   static AnchorGizmo::OPERATION mCurrentGizmoOperation(AnchorGizmo::ROTATE);
   static AnchorGizmo::MODE mCurrentGizmoMode(AnchorGizmo::WORLD);
   if (ANCHOR::IsKeyPressed(90))
      mCurrentGizmoOperation = AnchorGizmo::TRANSLATE;
   if (ANCHOR::IsKeyPressed(69))
      mCurrentGizmoOperation = AnchorGizmo::ROTATE;
   if (ANCHOR::IsKeyPressed(82)) // r Key
      mCurrentGizmoOperation = AnchorGizmo::SCALE;
   if (ANCHOR::RadioButton("Translate", mCurrentGizmoOperation == AnchorGizmo::TRANSLATE))
      mCurrentGizmoOperation = AnchorGizmo::TRANSLATE;
   ANCHOR::SameLine();
   if (ANCHOR::RadioButton("Rotate", mCurrentGizmoOperation == AnchorGizmo::ROTATE))
      mCurrentGizmoOperation = AnchorGizmo::ROTATE;
   ANCHOR::SameLine();
   if (ANCHOR::RadioButton("Scale", mCurrentGizmoOperation == AnchorGizmo::SCALE))
      mCurrentGizmoOperation = AnchorGizmo::SCALE;
   float matrixTranslation[3], matrixRotation[3], matrixScale[3];
   AnchorGizmo::DecomposeMatrixToComponents(matrix.m16, matrixTranslation, matrixRotation, matrixScale);
   ANCHOR::InputFloat3("Tr", matrixTranslation, 3);
   ANCHOR::InputFloat3("Rt", matrixRotation, 3);
   ANCHOR::InputFloat3("Sc", matrixScale, 3);
   AnchorGizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.m16);

   if (mCurrentGizmoOperation != AnchorGizmo::SCALE)
   {
      if (ANCHOR::RadioButton("Local", mCurrentGizmoMode == AnchorGizmo::LOCAL))
         mCurrentGizmoMode = AnchorGizmo::LOCAL;
      ANCHOR::SameLine();
      if (ANCHOR::RadioButton("World", mCurrentGizmoMode == AnchorGizmo::WORLD))
         mCurrentGizmoMode = AnchorGizmo::WORLD;
   }
   static bool useSnap(false);
   if (ANCHOR::IsKeyPressed(83))
      useSnap = !useSnap;
   ANCHOR::Checkbox("", &useSnap);
   ANCHOR::SameLine();
   vec_t snap;
   switch (mCurrentGizmoOperation)
   {
   case AnchorGizmo::TRANSLATE:
      snap = config.mSnapTranslation;
      ANCHOR::InputFloat3("Snap", &snap.x);
      break;
   case AnchorGizmo::ROTATE:
      snap = config.mSnapRotation;
      ANCHOR::InputFloat("Angle Snap", &snap.x);
      break;
   case AnchorGizmo::SCALE:
      snap = config.mSnapScale;
      ANCHOR::InputFloat("Scale Snap", &snap.x);
      break;
   }
   AnchorIO& io = ANCHOR::GetIO();
   AnchorGizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
   AnchorGizmo::Manipulate(camera.mView.m16, camera.mProjection.m16, mCurrentGizmoOperation, mCurrentGizmoMode, matrix.m16, NULL, useSnap ? &snap.x : NULL);
}
#endif

namespace AnchorGizmo
{
  // call inside your own window and before Manipulate() in order to draw gizmo to that window.
  // Or pass a specific AnchorDrawList to draw to (e.g. ANCHOR::GetForegroundDrawList()).
  ANCHOR_API void SetDrawlist(AnchorDrawList *drawlist = nullptr);

  // call BeginFrame right after Anchor_XXXX_NewFrame();
  ANCHOR_API void BeginFrame();

  // this is necessary because when imguizmo is compiled into a dll, and imgui into another
  // globals are not shared between them.
  // More details at
  // https://stackoverflow.com/questions/19373061/what-happens-to-global-and-static-variables-in-a-shared-library-when-it-is-dynam
  // expose method to set imgui context
  ANCHOR_API void SetAnchorContext(AnchorContext *ctx);

  // return true if mouse cursor is over any gizmo control (axis, plan or screen component)
  ANCHOR_API bool IsOver();

  // return true if mouse IsOver or if the gizmo is in moving state
  ANCHOR_API bool IsUsing();

  // enable/disable the gizmo. Stay in the state until next call to Enable.
  // gizmo is rendered with gray half transparent color when disabled
  ANCHOR_API void Enable(bool enable);

  // helper functions for manualy editing translation/rotation/scale with an input float
  // translation, rotation and scale float points to 3 floats each
  // Angles are in degrees (more suitable for human editing)
  // example:
  // float matrixTranslation[3], matrixRotation[3], matrixScale[3];
  // AnchorGizmo::DecomposeMatrixToComponents(gizmoMatrix.m16, matrixTranslation, matrixRotation,
  // matrixScale); ANCHOR::InputFloat3("Tr", matrixTranslation, 3); ANCHOR::InputFloat3("Rt", matrixRotation,
  // 3); ANCHOR::InputFloat3("Sc", matrixScale, 3);
  // AnchorGizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale,
  // gizmoMatrix.m16);
  //
  // These functions have some numerical stability issues for now. Use with caution.
  ANCHOR_API void DecomposeMatrixToComponents(const float *matrix,
                                              float *translation,
                                              float *rotation,
                                              float *scale);
  ANCHOR_API void RecomposeMatrixFromComponents(const float *translation,
                                                const float *rotation,
                                                const float *scale,
                                                float *matrix);

  ANCHOR_API void SetRect(float x, float y, float width, float height);
  // default is false
  ANCHOR_API void SetOrthographic(bool isOrthographic);

  // Render a cube with face color corresponding to face normal. Usefull for debug/tests
  ANCHOR_API void DrawCubes(const float *view,
                            const float *projection,
                            const float *matrices,
                            int matrixCount);
  ANCHOR_API void DrawGrid(const float *view,
                           const float *projection,
                           const float *matrix,
                           const float gridSize);

  // call it when you want a gizmo
  // Needs view and projection matrices.
  // matrix parameter is the source matrix (where will be gizmo be drawn) and might be transformed by the
  // function. Return deltaMatrix is optional translation is applied in world space
  enum OPERATION
  {
    TRANSLATE_X = (1u << 0),
    TRANSLATE_Y = (1u << 1),
    TRANSLATE_Z = (1u << 2),
    ROTATE_X = (1u << 3),
    ROTATE_Y = (1u << 4),
    ROTATE_Z = (1u << 5),
    ROTATE_SCREEN = (1u << 6),
    SCALE_X = (1u << 7),
    SCALE_Y = (1u << 8),
    SCALE_Z = (1u << 9),
    BOUNDS = (1u << 10),
    TRANSLATE = TRANSLATE_X | TRANSLATE_Y | TRANSLATE_Z,
    ROTATE = ROTATE_X | ROTATE_Y | ROTATE_Z | ROTATE_SCREEN,
    SCALE = SCALE_X | SCALE_Y | SCALE_Z
  };

  inline OPERATION operator|(OPERATION lhs, OPERATION rhs)
  {
    return static_cast<OPERATION>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }

  enum MODE
  {
    LOCAL,
    WORLD
  };

  ANCHOR_API bool Manipulate(const float *view,
                             const float *projection,
                             OPERATION operation,
                             MODE mode,
                             float *matrix,
                             float *deltaMatrix = NULL,
                             const float *snap = NULL,
                             const float *localBounds = NULL,
                             const float *boundsSnap = NULL);
  //
  // Please note that this cubeview is patented by Autodesk :
  // https://patents.google.com/patent/US7782319B2/en It seems to be a defensive patent in the US. I don't
  // think it will bring troubles using it as other software are using the same mechanics. But just in case,
  // you are now warned!
  //
  ANCHOR_API void ViewManipulate(float *view,
                                 float length,
                                 wabi::GfVec2f position,
                                 wabi::GfVec2f size,
                                 AnchorU32 backgroundColor);

  ANCHOR_API void SetID(int id);

  // return true if the cursor is over the operation's gizmo
  ANCHOR_API bool IsOver(OPERATION op);
  ANCHOR_API void SetGizmoSizeClipSpace(float value);

  // Allow axis to flip
  // When true (default), the guizmo axis flip for better visibility
  // When false, they always stay along the positive world/local axis
  ANCHOR_API void AllowAxisFlip(bool value);
}  // namespace AnchorGizmo
