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

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_assert.h"

#include <stdio.h>
#include <stdlib.h>

void _KLI_assert_print_pos(const char *file, const int line, const char *function, const char *id)
{
  fprintf(stderr, "KLI_assert failed: %s:%d, %s(), at \'%s\'\n", file, line, function, id);
}

void _KLI_assert_unreachable_print(const char *file, const int line, const char *function)
{
  fprintf(stderr, "Code marked as unreachable has been executed. Please report this as a bug.\n");
  fprintf(stderr, "Error found at %s:%d in %s.\n", file, line, function);
}

void _KLI_assert_print_backtrace(void)
{
#ifndef NDEBUG
  KLI_system_backtrace(stderr);
#endif
}

void _KLI_assert_abort(void)
{
  abort();
}
