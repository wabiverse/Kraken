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
 * Copyright 2020 Blender Foundation. All rights reserved.
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#import <Foundation/Foundation.h>
#include <string>
#include <sys/xattr.h>

#include "KLI_fileops.h"
#include "KLI_path_utils.h"

const char *KLI_expand_tilde(const char *path_with_tilde)
{
  static char path_expanded[FILE_MAX];
  @autoreleasepool {
    const NSString *const str_with_tilde = [[NSString alloc] initWithCString:path_with_tilde
                                                                    encoding:NSUTF8StringEncoding];
    if (!str_with_tilde) {
      return nullptr;
    }
    const NSString *const str_expanded = [str_with_tilde stringByExpandingTildeInPath];
    [str_expanded getCString:path_expanded
                   maxLength:sizeof(path_expanded)
                    encoding:NSUTF8StringEncoding];
  }
  return path_expanded;
}
