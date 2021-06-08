# 
#  Copyright 2021 Pixar. All Rights Reserved.
# 
#  Portions of this file are derived from original work by Pixar
#  distributed with Universal Scene Description, a project of the
#  Academy Software Foundation (ASWF). https://www.aswf.io/
# 
#  Licensed under the Apache License, Version 2.0 (the "Apache License")
#  with the following modification; you may not use this file except in
#  compliance with the Apache License and the following modification:
#  Section 6. Trademarks. is deleted and replaced with:
# 
#  6. Trademarks. This License does not grant permission to use the trade
#     names, trademarks, service marks, or product names of the Licensor
#     and its affiliates, except as required to comply with Section 4(c)
#     of the License and to reproduce the content of the NOTICE file.
#
#  You may obtain a copy of the Apache License at:
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the Apache License with the above modification is
#  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
#  ANY KIND, either express or implied. See the Apache License for the
#  specific language governing permissions and limitations under the
#  Apache License.
#
#  Modifications copyright (C) 2020-2021 Wabi.
#

def AddCmdlineArgs(argsParser, defaultValue=None, altHelpText=''):
    """
    Adds camera-related command line arguments to argsParser.

    The resulting 'camera' argument will be an Sdf.Path. If no value is given
    and defaultValue is not overridden, 'camera' will be a single-element path
    containing the primary camera name.
    """
    from wabi import Sdf
    from wabi import UsdUtils

    if defaultValue is None:
        defaultValue = UsdUtils.GetPrimaryCameraName()

    helpText = altHelpText
    if not helpText:
        helpText = (
            'Which camera to use - may be given as either just the '
            'camera\'s prim name (i.e. just the last element in the prim '
            'path), or as a full prim path. Note that if only the prim name '
            'is used and more than one camera exists with that name, which '
            'one is used will effectively be random (default=%(default)s)')

    # This avoids an Sdf warning if an empty string is given, which someone
    # might do for example with usdview to open the app using the 'Free' camera
    # instead of the primary camera.
    def _ToSdfPath(inputArg):
        if not inputArg:
            return Sdf.Path.emptyPath
        return Sdf.Path(inputArg)

    argsParser.add_argument('--camera', '-cam', action='store',
        type=_ToSdfPath, default=defaultValue, help=helpText)
