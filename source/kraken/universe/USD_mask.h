/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2012 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup DNA
 *
 * Mask data-blocks are collections of 2D curves to be used
 * for image masking in the compositor and sequencer.
 */

#pragma once

#include "USD_ID.h"
#include "USD_curveprofile_types.h"
#include "USD_defs.h"
#include "USD_listBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MaskLayer->visibility_flag */
#define MASK_HIDE_VIEW (1 << 0)
#define MASK_HIDE_SELECT (1 << 1)
#define MASK_HIDE_RENDER (1 << 2)

/* SpaceClip->mask_draw_flag */
#define MASK_DRAWFLAG_OVERLAY (1 << 1)
#define MASK_DRAWFLAG_SPLINE (1 << 2)

#ifdef __cplusplus
}
#endif
