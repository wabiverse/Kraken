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
"""
kpy.utils -- Kraken Python Base Functionality Utilities.
"""

__all__ = (
    "kraken_paths",
    "escape_identifier",
    "unescape_identifier",
    "register_class",
    "resource_path",
    "script_paths",
    "unregister_class",
    "user_resource",
)

from _kpy import (
    kraken_paths,
    escape_identifier,
    unescape_identifier,
    register_class,
    resource_path,
    script_paths as _kpy_script_paths,
    unregister_class,
    user_resource as _user_resource,
    system_resource,
)

import kpy as _kpy
import os as _os
import sys as _sys

import addon_utils as _addon_utils

# _preferences = _kpy.context.preferences
_script_module_dirs = "startup", "modules"
_is_factory_startup = _kpy.app.factory_startup


def execfile(filepath, *, mod=None):
    """
    Execute a file path as a Python script.

    :arg filepath: Path of the script to execute.
    :type filepath: string
    :arg mod: Optional cached module, the result of a previous execution.
    :type mod: Module or None
    :return: The module which can be passed back in as ``mod``.
    :rtype: ModuleType
    """

    import importlib.util
    mod_name = "__main__"
    mod_spec = importlib.util.spec_from_file_location(mod_name, filepath)
    if mod is None:
        mod = importlib.util.module_from_spec(mod_spec)

    # While the module name is not added to `sys.modules`, it's important to temporarily
    # include this so statements such as `sys.modules[cls.__module__].__dict__` behave as expected.
    modules = _sys.modules
    mod_orig = modules.get(mod_name, None)
    modules[mod_name] = mod

    # No error supression, just ensure `sys.modules[mod_name]` is properly restored in the case of an error.
    try:
        mod_spec.loader.exec_module(mod)
    finally:
        if mod_orig is None:
            modules.pop(mod_name, None)
        else:
            modules[mod_name] = mod_orig

    return mod


def _test_import(module_name, loaded_modules):
    use_time = _kpy.app.debug_python

    if module_name in loaded_modules:
        return None
    if "." in module_name:
        print("Ignoring '%s', can't import files containing "
              "multiple periods" % module_name)
        return None

    if use_time:
        import time
        t = time.time()

    try:
        mod = __import__(module_name)
    except:
        import traceback
        traceback.print_exc()
        return None

    if use_time:
        print("time %s %.4f" % (module_name, time.time() - t))

    loaded_modules.add(mod.__name__)  # should match mod.__name__ too
    return mod


# Check before adding paths as reloading would add twice.

# Storing and restoring the full `sys.path` is risky as this may be intentionally modified
# by technical users/developers.
#
# Instead, track which paths have been added, clearing them before refreshing.
# This supports the case of loading a new preferences file which may reset scripts path.
_sys_path_ensure_paths = set()


def _sys_path_ensure_prepend(path):
    if path not in _sys.path:
        _sys.path.insert(0, path)
        _sys_path_ensure_paths.add(path)


def _sys_path_ensure_append(path):
    if path not in _sys.path:
        _sys.path.append(path)
        _sys_path_ensure_paths.add(path)


def modules_from_path(path, loaded_modules):
    """
    Load all modules in a path and return them as a list.

    :arg path: this path is scanned for scripts and packages.
    :type path: string
    :arg loaded_modules: already loaded module names, files matching these
       names will be ignored.
    :type loaded_modules: set
    :return: all loaded modules.
    :rtype: list
    """
    modules = []

    for mod_name, _mod_path in _kpy.path.module_names(path):
        mod = _test_import(mod_name, loaded_modules)
        if mod:
            modules.append(mod)

    return modules


_global_loaded_modules = []  # store loaded module names for reloading.
# import kpy_types as _kpy_types  # keep for comparisons, never ever reload this.


def load_scripts(*, reload_scripts=False, refresh_scripts=False):
    """
    Load scripts and run each modules register function.

    :arg reload_scripts: Causes all scripts to have their unregister method
       called before loading.
    :type reload_scripts: bool
    :arg refresh_scripts: only load scripts which are not already loaded
       as modules.
    :type refresh_scripts: bool
    """
    use_time = use_class_register_check = _kpy.app.debug_python
    use_user = not _is_factory_startup
    
    use_time = False
    use_user = False

    if use_time:
        import time
        t_main = time.time()

    loaded_modules = set()

    if refresh_scripts:
        original_modules = _sys.modules.values()

    # if reload_scripts:
    #     # just unload, don't change user defaults, this means we can sync
    #     # to reload. note that they will only actually reload of the
    #     # modification time changes. This `won't` work for packages so...
    #     # its not perfect.
    #     for module_name in [ext.module for ext in _preferences.addons]:
    #         _addon_utils.disable(module_name)

    def register_module_call(mod):
        register = getattr(mod, "register", None)
        if register:
            try:
                register()
            except:
                import traceback
                traceback.print_exc()
        else:
            print("\nWarning! '%s' has no register function, "
                  "this is now a requirement for registerable scripts" %
                  mod.__file__)

    def unregister_module_call(mod):
        unregister = getattr(mod, "unregister", None)
        if unregister:
            try:
                unregister()
            except:
                import traceback
                traceback.print_exc()

    def test_reload(mod):
        import importlib
        # reloading this causes internal errors
        # because the classes from this module are stored internally
        # possibly to refresh internal references too but for now, best not to.
        # if mod == _kpy_types:
        #     return mod

        try:
            return importlib.reload(mod)
        except:
            import traceback
            traceback.print_exc()

    def test_register(mod):

        if refresh_scripts and mod in original_modules:
            return

        if reload_scripts and mod:
            print("Reloading:", mod)
            mod = test_reload(mod)

        if mod:
            register_module_call(mod)
            _global_loaded_modules.append(mod.__name__)

    if reload_scripts:

        # module names -> modules
        _global_loaded_modules[:] = [_sys.modules[mod_name]
                                     for mod_name in _global_loaded_modules]

        # loop over and unload all scripts
        _global_loaded_modules.reverse()
        for mod in _global_loaded_modules:
            unregister_module_call(mod)

        for mod in _global_loaded_modules:
            test_reload(mod)

        del _global_loaded_modules[:]

    from kpy_restrict_state import RestrictKraken

    with RestrictKraken():
        for base_path in script_paths(use_user=use_user):
            for path_subdir in _script_module_dirs:
                path = _os.path.join(base_path, path_subdir)
                if _os.path.isdir(path):
                    _sys_path_ensure_prepend(path)

                    # Only add to 'sys.modules' unless this is 'startup'.
                    if path_subdir == "startup":
                        for mod in modules_from_path(path, loaded_modules):
                            test_register(mod)

    # load template (if set)
    if any(_kpy.utils.app_template_paths()):
        import kr_app_template_utils
        kr_app_template_utils.reset(reload_scripts=reload_scripts)
        del kr_app_template_utils

    # deal with addons separately
    _initialize = getattr(_addon_utils, "_initialize", None)
    if _initialize is not None:
        # first time, use fast-path
        _initialize()
        del _addon_utils._initialize
    else:
        _addon_utils.reset_all(reload_scripts=reload_scripts)
    del _initialize

    if reload_scripts:
        _kpy.context.window_manager.tag_script_reload()

        import gc
        print("gc.collect() -> %d" % gc.collect())

    if use_time:
        print("Python Script Load Time %.4f" % (time.time() - t_main))

    if use_class_register_check:
        for cls in _kpy.types.uni_object.__subclasses__():
            if getattr(cls, "is_registered", False):
                for subcls in cls.__subclasses__():
                    if not subcls.is_registered:
                        print(
                            "Warning, unregistered class: %s(%s)" %
                            (subcls.__name__, cls.__name__)
                        )


# base scripts
_scripts = (
    _os.path.dirname(_os.path.dirname(_os.path.dirname(__file__))),
)

def script_path_user():
    """returns the env var and falls back to home dir or None"""
    path = _user_resource('SCRIPTS')
    return _os.path.normpath(path) if path else None


def script_path_pref():
    """returns the user preference or None"""
    # TODO: Implement kpy.context.preferences
    # path = _preferences.filepaths.script_directory
    # return _os.path.normpath(path) if path else None
    return None

def script_paths(*, subdir=None, user_pref=True, check_all=False, use_user=True):
    """
    Returns a list of valid script paths.

    :arg subdir: Optional subdir.
    :type subdir: string
    :arg user_pref: Include the user preference script path.
    :type user_pref: bool
    :arg check_all: Include local, user and system paths rather just the paths
       kraken uses.
    :type check_all: bool
    :return: script paths.
    :rtype: list
    """
    scripts = list(_scripts)

    # Only paths Kraken uses.
    #
    # Needed this is needed even when 'check_all' is enabled,
    # so the 'KRAKEN_SYSTEM_SCRIPTS' environment variable will be used.
    base_paths = _kpy_script_paths()

    # Defined to be (system, user) so we can skip the second if needed.
    if not use_user:
        base_paths = base_paths[:1]

    if check_all:
        # All possible paths, no duplicates, keep order.
        if use_user:
            test_paths = ('LOCAL', 'USER', 'SYSTEM')
        else:
            test_paths = ('LOCAL', 'SYSTEM')

        base_paths = (
            *(path for path in (
                _os.path.join(resource_path(res), "scripts")
                for res in test_paths) if path not in base_paths),
            *base_paths,
        )

    test_paths = (
        *base_paths,
        *((script_path_user(),) if use_user else ()),
        *((script_path_pref(),) if user_pref else ()),
    )

    for path in test_paths:
        if path:
            path = _os.path.normpath(path)
            if path not in scripts and _os.path.isdir(path):
                scripts.append(path)

    if subdir is None:
        return scripts

    scripts_subdir = []
    for path in scripts:
        path_subdir = _os.path.join(path, subdir)
        if _os.path.isdir(path_subdir):
            scripts_subdir.append(path_subdir)

    return scripts_subdir


def refresh_script_paths():
    """
    Run this after creating new script paths to update sys.path
    """

    for path in _sys_path_ensure_paths:
        try:
            _sys.path.remove(path)
        except ValueError:
            pass
    _sys_path_ensure_paths.clear()

    for base_path in script_paths():
        for path_subdir in _script_module_dirs:
            path = _os.path.join(base_path, path_subdir)
            if _os.path.isdir(path):
                _sys_path_ensure_prepend(path)

    for path in _addon_utils.paths():
        _sys_path_ensure_append(path)
        path = _os.path.join(path, "modules")
        if _os.path.isdir(path):
            _sys_path_ensure_append(path)


def app_template_paths(*, path=None):
    """
    Returns valid application template paths.

    :arg path: Optional subdir.
    :type path: string
    :return: app template paths.
    :rtype: generator
    """
    subdir_args = (path,) if path is not None else ()
    # Note: keep in sync with: Kraken's 'KKE_appdir_app_template_any'.
    # Uses 'KRAKEN_USER_SCRIPTS', 'KRAKEN_SYSTEM_SCRIPTS'
    # ... in this case 'system' accounts for 'local' too.
    for resource_fn, module_name in (
            (_user_resource, "kr_app_templates_user"),
            (system_resource, "kr_app_templates_system"),
    ):
        path_test = resource_fn('SCRIPTS', path=_os.path.join("startup", module_name, *subdir_args))
        if path_test and _os.path.isdir(path_test):
            yield path_test


def preset_paths(subdir):
    """
    Returns a list of paths for a specific preset.

    :arg subdir: preset subdirectory (must not be an absolute path).
    :type subdir: string
    :return: script paths.
    :rtype: list
    """
    dirs = []
    for path in script_paths(subdir="presets", check_all=True):
        directory = _os.path.join(path, subdir)
        if not directory.startswith(path):
            raise Exception("invalid subdir given %r" % subdir)
        elif _os.path.isdir(directory):
            dirs.append(directory)

    # Find addons preset paths
    for path in _addon_utils.paths():
        directory = _os.path.join(path, "presets", subdir)
        if _os.path.isdir(directory):
            dirs.append(directory)

    return dirs


def is_path_builtin(path):
    """
    Returns True if the path is one of the built-in paths used by Kraken.

    :arg path: Path you want to check if it is in the built-in settings directory
    :type path: str
    :rtype: bool
    """
    # Note that this function is is not optimized for speed,
    # it's intended to be used to check if it's OK to remove presets.
    #
    # If this is used in a draw-loop for example, we could cache some of the values.
    user_path = resource_path('USER')

    for res in ('SYSTEM', 'LOCAL'):
        parent_path = resource_path(res)
        if not parent_path or parent_path == user_path:
            # Make sure that the current path is not empty string and that it is
            # not the same as the user config path. IE "~/.config/kraken" on Linux
            # This can happen on portable installs.
            continue

        try:
            if _os.path.samefile(
                    _os.path.commonpath([parent_path]),
                    _os.path.commonpath([parent_path, path])
            ):
                return True
        except FileNotFoundError:
            # The path we tried to look up doesn't exist.
            pass
        except ValueError:
            # Happens on Windows when paths don't have the same drive.
            pass

    return False


def smpte_from_seconds(time, *, fps=None, fps_base=None):
    """
    Returns an SMPTE formatted string from the *time*:
    ``HH:MM:SS:FF``.

    If *fps* and *fps_base* are not given the current scene is used.

    :arg time: time in seconds.
    :type time: int, float or ``datetime.timedelta``.
    :return: the frame string.
    :rtype: string
    """

    return smpte_from_frame(
        time_to_frame(time, fps=fps, fps_base=fps_base),
        fps=fps,
        fps_base=fps_base
    )


def smpte_from_frame(frame, *, fps=None, fps_base=None):
    """
    Returns an SMPTE formatted string from the *frame*:
    ``HH:MM:SS:FF``.

    If *fps* and *fps_base* are not given the current scene is used.

    :arg frame: frame number.
    :type frame: int or float.
    :return: the frame string.
    :rtype: string
    """


    # TODO: Implement Scene (Stage) Object on Kraken Context
    # if fps is None:
    #     fps = _kpy.context.scene.render.fps

    # if fps_base is None:
    #     fps_base = _kpy.context.scene.render.fps_base

    fps = fps / fps_base
    sign = "-" if frame < 0 else ""
    frame = abs(frame)

    return (
        "%s%02d:%02d:%02d:%02d" % (
            sign,
            int(frame / (3600 * fps)),          # HH
            int((frame / (60 * fps)) % 60),     # MM
            int((frame / fps) % 60),            # SS
            int(frame % fps),                   # FF
        ))


def time_from_frame(frame, *, fps=None, fps_base=None):
    """
    Returns the time from a frame number .

    If *fps* and *fps_base* are not given the current scene is used.

    :arg frame: number.
    :type frame: int or float.
    :return: the time in seconds.
    :rtype: datetime.timedelta
    """

    # TODO: Implement Scene (Stage) Object on Kraken Context
    # if fps is None:
    #     fps = _kpy.context.scene.render.fps

    # if fps_base is None:
    #     fps_base = _kpy.context.scene.render.fps_base

    fps = fps / fps_base

    from datetime import timedelta

    return timedelta(0, frame / fps)


def time_to_frame(time, *, fps=None, fps_base=None):
    """
    Returns a float frame number from a time given in seconds or
    as a datetime.timedelta object.

    If *fps* and *fps_base* are not given the current scene is used.

    :arg time: time in seconds.
    :type time: number or a ``datetime.timedelta`` object
    :return: the frame.
    :rtype: float
    """

    # TODO: Implement Scene (Stage) Object on Kraken Context
    # if fps is None:
    #     fps = _kpy.context.scene.render.fps

    # if fps_base is None:
    #     fps_base = _kpy.context.scene.render.fps_base

    fps = fps / fps_base

    from datetime import timedelta

    if isinstance(time, timedelta):
        time = time.total_seconds()

    return time * fps


def preset_find(name, preset_path, *, display_name=False, ext=".py"):
    if not name:
        return None

    for directory in preset_paths(preset_path):

        if display_name:
            filename = ""
            for fn in _os.listdir(directory):
                if fn.endswith(ext) and name == _kpy.path.display_name(fn, title_case=False):
                    filename = fn
                    break
        else:
            filename = name + ext

        if filename:
            filepath = _os.path.join(directory, filename)
            if _os.path.exists(filepath):
                return filepath


# TODO: Implement Kraken Python preferences module
def keyconfig_init():
    # Key configuration initialization and refresh, called from the Kraken
    # window manager on startup and refresh.
    # active_config = _preferences.keymap.active_keyconfig

    # Load the default key configuration.
    default_filepath = preset_find("Kraken", "keyconfig")
    keyconfig_set(default_filepath)

    # Set the active key configuration if different
    # filepath = preset_find(active_config, "keyconfig")

    # if filepath and filepath != default_filepath:
        # keyconfig_set(filepath)


# TODO: Implement Kraken Python window_manager module
def keyconfig_set(filepath, *, report=None):
    from os.path import basename, splitext

    if _kpy.app.debug_python:
        print("loading preset:", filepath)

    keyconfigs = _kpy.context.window_manager.keyconfigs

    try:
        error_msg = ""
        execfile(filepath)
    except:
        import traceback
        error_msg = traceback.format_exc()

    name = splitext(basename(filepath))[0]
    kc_new = keyconfigs.get(name)

    if error_msg:
        if report is not None:
            report({'ERROR'}, error_msg)
        print(error_msg)
        if kc_new is not None:
            keyconfigs.remove(kc_new)
        return False

    # Get name, exception for default keymap to keep backwards compatibility.
    if kc_new is None:
        if report is not None:
            report({'ERROR'}, "Failed to load keymap %r" % filepath)
        return False
    else:
        keyconfigs.active = kc_new
        return True


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


def register_classes_factory(classes):
    """
    Utility function to create register and unregister functions
    which simply registers and unregisters a sequence of classes.
    """
    def register():
        from kpy.utils import register_class
        for cls in classes:
            register_class(cls)

    def unregister():
        from kpy.utils import unregister_class
        for cls in reversed(classes):
            unregister_class(cls)

    return register, unregister


def register_submodule_factory(module_name, submodule_names):
    """
    Utility function to create register and unregister functions
    which simply load submodules,
    calling their register & unregister functions.

    .. note::

       Modules are registered in the order given,
       unregistered in reverse order.

    :arg module_name: The module name, typically ``__name__``.
    :type module_name: string
    :arg submodule_names: List of submodule names to load and unload.
    :type submodule_names: list of strings
    :return: register and unregister functions.
    :rtype: tuple pair of functions
    """

    module = None
    submodules = []

    def register():
        nonlocal module
        module = __import__(name=module_name, fromlist=submodule_names)
        submodules[:] = [getattr(module, name) for name in submodule_names]
        for mod in submodules:
            mod.register()

    def unregister():
        from sys import modules
        for mod in reversed(submodules):
            mod.unregister()
            name = mod.__name__
            delattr(module, name.partition(".")[2])
            del modules[name]
        submodules.clear()

    return register, unregister


# def _kraken_default_map():
#     import universe_manual_reference as ref_mod
#     ret = (ref_mod.url_manual_prefix, ref_mod.url_manual_mapping)
#     # avoid storing in memory
#     del _sys.modules["universe_manual_reference"]
#     return ret


# hooks for doc lookups
# _manual_map = [_universe_default_map]


# def register_manual_map(manual_hook):
    _manual_map.append(manual_hook)


# def unregister_manual_map(manual_hook):
    # _manual_map.remove(manual_hook)


# def manual_map():
#     # reverse so default is called last
#     for cb in reversed(_manual_map):
#         try:
#             prefix, url_manual_mapping = cb()
#         except:
#             print("Error calling %r" % cb)
#             import traceback
#             traceback.print_exc()
#             continue

#         yield prefix, url_manual_mapping


def make_sdf_paths(struct_name, prop_name, enum_name):
    """
    Create Pixar SdfPaths from given names.

    :arg struct_name: Name of a UNI struct (like e.g. "Scene").
    :type struct_name: string
    :arg prop_name: Name of a UNI struct's property.
    :type prop_name: string
    :arg enum_name: Name of a UNI enum identifier.
    :type enum_name: string
    :return: A triple of three "UNI paths"
       (most_complete_path, "struct.prop", "struct.prop:'enum'").
       If no enum_name is given, the third element will always be void.
    :rtype: tuple of strings
    """
    src = src_uni = src_enum = ""
    if struct_name:
        if prop_name:
            src = src_uni = ".".join((struct_name, prop_name))
            if enum_name:
                src = src_enum = "%s:'%s'" % (src_uni, enum_name)
        else:
            src = src_uni = struct_name
    return src, src_uni, src_enum