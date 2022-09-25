/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2009 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup krf
 *
 * Internal utility API for KRF.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "KLI_utildefines.h"

#include "krf_internal.h"

unsigned int krf_next_p2(unsigned int x)
{
  x -= 1;
  x |= (x >> 16);
  x |= (x >> 8);
  x |= (x >> 4);
  x |= (x >> 2);
  x |= (x >> 1);
  x += 1;
  return x;
}

unsigned int krf_hash(unsigned int val)
{
  unsigned int key;

  key = val;
  key += ~(key << 16);
  key ^= (key >> 5);
  key += (key << 3);
  key ^= (key >> 13);
  key += ~(key << 9);
  key ^= (key >> 17);
  return key % 257;
}
