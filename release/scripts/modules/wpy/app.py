#
#  Copyright 2020 Wabi.  All rights reserved.
#
#  CONFIDENTIAL AND COPYRIGHT - FOR WABI INTERNAL USE ONLY - UNAUTHORIZED ACCESS IS STRICTLY
#  PROHIBITED. THIS SOURCE CODE IS PROPRIETARY AND CONFIDENTIAL AND THEREFORE SUBJECT TO THE
#  CONSTITUTIONAL LAWS AND RIGHTS GOVERNING THE UNITED STATES OF AMERICA. ALL RIGHTS, VIEWERSHIP,
#  DISTRIBUTION, POSESSION, AND CONTROL OF THIS SOURCE CODE BELONG SOLEY TO THE COMPANY "WABIXYZ
#  INC."
#
#  IF YOU CAN READ THIS TEXT, THEN YOU ARE IN STRICT VIOLATION OF THE WABI SOFTWARE LICENSE
#  AGREEMENT WHICH IS INCLUDED WITHIN THIS SOFTWARE DISTRIBUTION.
#

# <pep8 compliant>

"""
"This module contains application values that remain unchanged during runtime."
"""

import kpy

def name():
  return kpy.wg_info["name"]

def author():
  return kpy.wg_info["author"]

def version():
  return str(kpy.wg_info["kraken"])

def description():
  return kpy.wg_info["description"]

def build_hash(build_dir):
  import os
  import re
  buildinfo_h = os.path.join(build_dir, "source", "creator", "buildinfo.h")
  regex = re.compile(r"^#\s*define\s+%s\s+(.*)" % 'BUILD_HASH')
  hash = None
  with open(buildinfo_h, "r") as file:
    for l in file:
      match = regex.match(l)
      if match:
        hash = match.group(1)
  return hash[1:-1]