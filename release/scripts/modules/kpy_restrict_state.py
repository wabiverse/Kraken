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
    _real_data = _kpy.data
    # safe, the pointer never changes
    _real_pref = _kpy.context.preferences

    @property
    def window_manager(self):
        return self._real_data.window_managers[0]

    @property
    def preferences(self):
        return self._real_pref


class _RestrictData:
    __slots__ = ()


_context_restrict = _RestrictContext()
_data_restrict = _RestrictData()


class RestrictKraken:
    __slots__ = ("context", "data")

    def __enter__(self):
        self.data = _kpy.data
        self.context = _kpy.context
        _kpy.data = _data_restrict
        _kpy.context = _context_restrict

    def __exit__(self, type, value, traceback):
        _kpy.data = self.data
        _kpy.context = self.context
