# SPDX-License-Identifier: GPL-2.0-or-later

"""
This module contains RestrictKraken context manager.
"""

__all__ = (
    "RestrictKraken",
)

import kpy as _kpy


class _RestrictContext:
    __slots__ = ()
    # Enable once data is back on _kpy
    # _real_data = _kpy.data
    # safe, the pointer never changes
    # Enable once context is back on _kpy
    # _real_pref = _kpy.context.preferences

    @property
    def window_manager(self):
        # Enable once data is back on _kpy
        # return self._real_data.window_managers[0]
        return {}

    @property
    def preferences(self):
        # Enable once context is back on _kpy
        # return self._real_pref
        return {}


class _RestrictData:
    __slots__ = ()


_context_restrict = _RestrictContext()
_data_restrict = _RestrictData()


class RestrictKraken:
    __slots__ = ("context", "data")

    def __enter__(self):
        # Enable once data & context is back on _kpy
        # self.data = _kpy.data
        # self.context = _kpy.context
        # _kpy.data = _data_restrict
        # _kpy.context = _context_restrict
        return

    def __exit__(self, type, value, traceback):
        # Enable once data & context is back on _kpy
        # _kpy.data = self.data
        # _kpy.context = self.context
        return
