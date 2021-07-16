

__all__ = (
    "kraken_paths",
    "register_class",
    "script_paths",
    "unregister_class",
    "user_resource",
)

from _kpy import (
    kraken_paths,
    register_class,
    script_paths as _kpy_script_paths,
    unregister_class,
    user_resource as _user_resource,
)

import kpy as _kpy
import os as _os
import sys as _sys

# import addon_utils as _addon_utils

# _preferences = _kpy.context.preferences
_script_module_dirs = "startup", "modules"
# _is_factory_startup = _kpy.app.factory_startup

def user_resource(resource_type, *, path="", create=False):
    """
    Return a user resource path (normally from the users home directory).

    :arg type: Resource type in ['DATAFILES', 'CONFIG', 'SCRIPTS', 'AUTOSAVE'].
    :type type: string
    :arg path: Optional subdirectory.
    :type path: string
    :arg create: Treat the path as a directory and create
       it if its not existing.
    :type create: boolean
    :return: a path.
    :rtype: string
    """
    target_path = _user_resource(resource_type, path=path)

    if create:
        # should always be true.
        if target_path:
            # create path if not existing.
            if not _os.path.exists(target_path):
                try:
                    _os.makedirs(target_path)
                except:
                    import traceback
                    traceback.print_exc()
                    target_path = ""
            elif not _os.path.isdir(target_path):
                print("Path %r found but isn't a directory!" % target_path)
                target_path = ""

    return target_path