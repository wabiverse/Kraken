/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2008 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup edinterface
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstring>

#include "MEM_guardedalloc.h"

#include "USD_scene.h"
#include "USD_userpref.h"
#include "USD_view2d.h"

#include "KLI_array.h"
#include "KLI_easing.h"
#include "KLI_link_utils.h"
#include "KLI_listbase.h"
#include "KLI_math.h"
// #include "KLI_memarena.h"
#include "KLI_rect.h"
#include "KLI_string.h"
// #include "KLI_timecode.h"
#include "KLI_utildefines.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"

// #include "GPU_immediate.h"
#include "GPU_matrix.h"
// #include "GPU_state.h"

#include "WM_api.h"

// #include "KRF_api.h"

#include "ED_screen.h"

#include "UI_interface.h"
#include "UI_view2d.h"

#include "interface_intern.h"


float UI_view2d_scale_get_x(const View2D *v2d)
{
  GfVec4i mask;
  mask[0] = v2d->mask.xmin;
  mask[1] = v2d->mask.xmax;
  mask[2] = v2d->mask.ymin;
  mask[3] = v2d->mask.ymax;

  GfVec4f cur;
  cur[0] = v2d->cur.xmin;
  cur[1] = v2d->cur.xmax;
  cur[2] = v2d->cur.ymin;
  cur[3] = v2d->cur.ymax;

  return KLI_rcti_size_x(mask) / KLI_rctf_size_x(cur);
}