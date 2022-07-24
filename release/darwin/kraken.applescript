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
# Script to create a macOS dmg file for Kraken builds, including code
# signing and notarization for releases.
#
# Original Copyright (C) 2020 Blender Foundation. All Rights Reserved.
# Modifications copyright (C) 2022 Wabi.

tell application "Finder"
           tell disk "Kraken"
                open
                set current view of container window to icon view
                set toolbar visible of container window to false
                set statusbar visible of container window to false
                set the bounds of container window to {100, 100, 640, 472}
                set theViewOptions to icon view options of container window
                set arrangement of theViewOptions to not arranged
                set icon size of theViewOptions to 128
                set background picture of theViewOptions to file ".background:background.tif"
                set position of item " " of container window to {400, 190}
                set position of item "kraken.app" of container window to {135, 190}
                update without registering applications
                delay 5
                close
      end tell
 end tell