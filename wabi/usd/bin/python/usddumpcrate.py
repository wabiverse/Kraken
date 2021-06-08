#!/wabipythonsubst
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
from __future__ import print_function
import argparse, sys, os
from wabi import Sdf, Usd

def Err(msg):
    print('Error: ' + msg, file=sys.stderr)

def PrintReport(fname, info, summaryOnly):
    print('@%s@' % fname, 'file version', info.GetFileVersion())
    ss = info.GetSummaryStats()
    print ('  %s specs, %s paths, %s tokens, %s strings, '
           '%s fields, %s field sets' %
           (ss.numSpecs, ss.numUniquePaths, ss.numUniqueTokens,
            ss.numUniqueStrings, ss.numUniqueFields, ss.numUniqueFieldSets))
    if summaryOnly:
        return
    print('  Structural Sections:')
    for sec in info.GetSections():
        print('    %16s %16d bytes at offset 0x%X' % (
            sec.name, sec.size, sec.start))
    print()

def main():
    parser = argparse.ArgumentParser(
        prog=os.path.basename(sys.argv[0]),
        description='Write information about a usd crate (usdc) file to stdout')

    parser.add_argument('inputFiles', nargs='+')
    parser.add_argument('-s', '--summary', action='store_true',
                        help='report only a short summary')

    args = parser.parse_args()

    print('Usd crate software version', Usd.CrateInfo().GetSoftwareVersion())

    for fname in args.inputFiles:
        try:
            info = Usd.CrateInfo.Open(fname)
            if not info:
                Err('Failed to read %s' % fname)
                continue
        except Exception as e:
            Err('Failed to read %s\n %s' % (fname,e))
            continue
        PrintReport(fname, info, args.summary)

if __name__ == "__main__":
    # Restore signal handling defaults to allow output redirection and the like.
    import platform
    if platform.system() != 'Windows':
        import signal
        signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    main()
