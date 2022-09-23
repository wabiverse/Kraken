/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2001-2002 NaN Holding BV. All rights reserved. */

/** \file
 * \ingroup DNA
 * \brief These structs are the foundation for all linked lists in the library system.
 *
 * Doubly-linked lists start from a ListBase and contain elements beginning
 * with Link.
 */

#pragma once

#include "kraken/kraken.h"

KRAKEN_NAMESPACE_BEGIN

/** Generic - all structs which are put into linked lists begin with this. */
struct Link {
  struct Link *next, *prev;
};

/** Simple subclass of Link. Use this when it is not worth defining a custom one. */
struct LinkData {
  struct LinkData *next, *prev;
  void *data;
};

/** Never change the size of this! dna_genfile.c detects pointer_size with it. */
struct ListBase {
  void *first, *last;
};

/* 8 byte alignment! */

KRAKEN_NAMESPACE_END

