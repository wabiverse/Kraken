from _kpy import types as kpy_types

KrakenPRIM = kpy_types.kpy_struct
StructMetaPropGroup = kpy_types.kpy_struct_meta_idprop

_sentinel = object()

class Context(KrakenPRIM):
    __slots__ = ()

    def path_resolve(self, path, coerce=True):
        """
        Returns the property from the path, raise an exception when not found.

        :arg path: patch which this property resolves.
        :type path: string
        :arg coerce: optional argument, when True, the property will be converted into its Python representation.
        :type coerce: boolean
        """
        # This is a convenience wrapper around `KrakenSTAGE.path_resolve` which doesn't support accessing context members.
        # Without this wrapper many users were writing `exec("context.%s" % data_path)` which is a security
        # concern if the `data_path` comes from an unknown source.
        # This function performs the initial lookup, after that the regular `path_resolve` function is used.

        # Extract the initial attribute into `(attr, path_rest)`.
        sep = len(path)
        div = ""
        for div_test in (".", "["):
            sep_test = path.find(div_test, 0, sep)
            if sep_test != -1 and sep_test < sep:
                sep = sep_test
                div = div_test
        if div:
            attr = path[:sep]
            if div == ".":
                sep += 1
            path_rest = path[sep:]
        else:
            attr = path
            path_rest = ""

        # Retrieve the value for `attr`.
        # Match the value error exception with that of "path_resolve"
        # to simplify exception handling for the caller.
        value = getattr(self, attr, _sentinel)
        if value is _sentinel:
            raise ValueError("Path could not be resolved: %r" % attr)

        if value is None:
            return value

        # Resolve the rest of the path if necessary.
        if path_rest:
            path_resolve_fn = getattr(value, "path_resolve", None)
            if path_resolve_fn is None:
                raise ValueError("Path %s resolves to a non PRIM value" % attr)
            return path_resolve_fn(path_rest, coerce)

        return value

    def copy(self):
        from types import BuiltinMethodType
        new_context = {}
        generic_attrs = (
            *KrakenPRIM.__dict__.keys(),
            "kr_prim", "prim_type", "copy",
        )
        for attr in dir(self):
            if not (attr.startswith("_") or attr in generic_attrs):
                value = getattr(self, attr)
                if type(value) != BuiltinMethodType:
                    new_context[attr] = value

        return new_context


class PRIMMeta(type):
    @property
    def is_registered(cls):
        return "kr_prim" in cls.__dict__


class PRIMMetaPropGroup(StructMetaPropGroup, PRIMMeta):
    pass


class Library(KrakenPRIM):
    __slots__ = ()

    @property
    def users_id(self):
        """ID data blocks which use this library"""
        import kpy

        attr_links = (
            "actions", "armatures", "brushes", "cameras",
            "curves", "grease_pencils", "collections", "images",
            "lights", "lattices", "materials", "metaballs",
            "meshes", "node_groups", "objects", "scenes",
            "sounds", "speakers", "textures", "texts",
            "fonts", "worlds",
        )

        return tuple(id_block
                     for attr in attr_links
                     for id_block in getattr(kpy.data, attr)
                     if id_block.library == self)


class Texture(KrakenPRIM):
    __slots__ = ()

    @property
    def users_material(self):
        """Materials that use this texture"""
        import kpy
        return tuple(mat for mat in kpy.data.materials
                     if self in [slot.texture
                                 for slot in mat.texture_slots
                                 if slot]
                     )

    @property
    def users_object_modifier(self):
        """Object modifiers that use this texture"""
        import kpy
        return tuple(
            obj for obj in kpy.data.objects if
            self in [
                mod.texture
                for mod in obj.modifiers
                if mod.type == 'DISPLACE']
        )


class Collection(KrakenPRIM):
    __slots__ = ()

    @property
    def children_recursive(self):
        """A list of all children from this collection."""
        children_recursive = []

        def recurse(parent):
            for child in parent.children:
                children_recursive.append(child)
                recurse(child)

        recurse(self)
        return children_recursive

    @property
    def users_dupli_group(self):
        """The collection instance objects this collection is used in"""
        import kpy
        return tuple(obj for obj in kpy.data.objects
                     if self == obj.instance_collection)


class Object(KrakenPRIM):
    __slots__ = ()

    @property
    def children(self):
        """All the children of this object.

        .. note:: Takes ``O(len(kpy.data.objects))`` time."""
        import kpy
        return tuple(child for child in kpy.data.objects
                     if child.parent == self)

    @property
    def children_recursive(self):
        """A list of all children from this object.

        .. note:: Takes ``O(len(kpy.data.objects))`` time."""
        import kpy
        parent_child_map = {}
        for child in kpy.data.objects:
            if (parent := child.parent) is not None:
                parent_child_map.setdefault(parent, []).append(child)

        children_recursive = []

        def recurse(parent):
            for child in parent_child_map.get(parent, ()):
                children_recursive.append(child)
                recurse(child)

        recurse(self)
        return children_recursive

    @property
    def users_collection(self):
        """
        The collections this object is in.

        .. note:: Takes ``O(len(kpy.data.collections) + len(kpy.data.scenes))`` time."""
        import kpy
        return (
            tuple(
                collection for collection in kpy.data.collections
                if self in collection.objects[:]
            ) + tuple(
                scene.collection for scene in kpy.data.scenes
                if self in scene.collection.objects[:]
            )
        )

    @property
    def users_scene(self):
        """The scenes this object is in.

        .. note:: Takes ``O(len(kpy.data.scenes) * len(kpy.data.objects))`` time."""
        import kpy
        return tuple(scene for scene in kpy.data.scenes
                     if self in scene.objects[:])


class WindowManager(KrakenPRIM):
    __slots__ = ()

    def popup_menu(
            self, draw_func, *,
            title="",
            icon='NONE',
    ):
        import kpy
        popup = self.popmenu_begin__internal(title, icon=icon)

        try:
            draw_func(popup, kpy.context)
        finally:
            self.popmenu_end__internal(popup)

    def popover(
            self, draw_func, *,
            ui_units_x=0,
            keymap=None,
            from_active_button=False,
    ):
        import kpy
        popup = self.popover_begin__internal(
            ui_units_x=ui_units_x,
            from_active_button=from_active_button,
        )

        try:
            draw_func(popup, kpy.context)
        finally:
            self.popover_end__internal(popup, keymap=keymap)

    def popup_menu_pie(
            self, event, draw_func, *,
            title="",
            icon='NONE',
    ):
        import kpy
        pie = self.piemenu_begin__internal(title, icon=icon, event=event)

        if pie:
            try:
                draw_func(pie, kpy.context)
            finally:
                self.piemenu_end__internal(pie)


class Operator(KrakenPRIM, metaclass=PRIMMeta):
    __slots__ = ()

    def __getattribute__(self, attr):
        properties = KrakenPRIM.path_resolve(self, "properties")
        kr_prim = getattr(properties, "kr_prim", None)
        if (kr_prim is not None) and (attr in kr_prim.properties):
            return getattr(properties, attr)
        return super().__getattribute__(attr)

    def __setattr__(self, attr, value):
        properties = KrakenPRIM.path_resolve(self, "properties")
        kr_prim = getattr(properties, "kr_prim", None)
        if (kr_prim is not None) and (attr in kr_prim.properties):
            return setattr(properties, attr, value)
        return super().__setattr__(attr, value)

    def __delattr__(self, attr):
        properties = KrakenPRIM.path_resolve(self, "properties")
        kr_prim = getattr(properties, "kr_prim", None)
        if (kr_prim is not None) and (attr in kr_prim.properties):
            return delattr(properties, attr)
        return super().__delattr__(attr)

    def as_keywords(self, *, ignore=()):
        """Return a copy of the properties as a dictionary"""
        ignore = ignore + ("prim_type",)
        return {attr: getattr(self, attr)
                for attr in self.properties.prim_type.properties.keys()
                if attr not in ignore}


class Macro(KrakenPRIM):
    # kpy_types is imported before ops is defined
    # so we have to do a local import on each run
    __slots__ = ()

    @classmethod
    def define(cls, opname):
        from _kpy import ops
        return ops.macro_define(cls, opname)


class PropertyGroup(KrakenPRIM, metaclass=PRIMMetaPropGroup):
    __slots__ = ()


class RenderEngine(KrakenPRIM, metaclass=PRIMMeta):
    __slots__ = ()


class KeyingSetInfo(KrakenPRIM, metaclass=PRIMMeta):
    __slots__ = ()


class AddonPreferences(KrakenPRIM, metaclass=PRIMMeta):
    __slots__ = ()

