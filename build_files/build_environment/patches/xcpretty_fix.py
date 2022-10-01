#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# Copyright 2022, Wabi Animation Studios, Ltd. Co.
#
"""
Small fixup for xc | pretty utility we use to dial down the output
from Xcode during builds on macOS, which is a great utility to get
to the raw output of what you need to see from your code, however
there is a bug which doesn't render the undefined symbols which
gets quite annoying. This fix ensure that if xcode complains about
any potential undefined symbols, they are printed to the terminal
at the end of the build.
"""

import sys
import re

has_undefined = False
while True:
  line = sys.stdin.readline()
  if line == "":
    break
  if line.startswith("Undefined symbols"):
    has_undefined = True
  elif has_undefined and not line.startswith("  "):
    has_undefined = False
  if has_undefined:
    line = "ld: " + line
  print(line)