from _kpy import types as kpy_types

StructLUXO = kpy_types.kpy_struct
StructMetaPropGroup = kpy_types.kpy_struct_meta_idprop

_sentinel = object()

class Context(StructLUXO):
    __slots__ = ()

    def path_resolve(self, path, coerce=True):
        """
        Returns the property from the path, raise an exception when not found.

        :arg path: patch which this property resolves.
        :type path: string
        :arg coerce: optional argument, when True, the property will be converted into its Python representation.
        :type coerce: boolean
        """
        # This is a convenience wrapper around `StructRNA.path_resolve` which doesn't support accessing context members.
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
                raise ValueError("Path %s resolves to a non LUXO value" % attr)
            return path_resolve_fn(path_rest, coerce)

        return value

    def copy(self):
        from types import BuiltinMethodType
        new_context = {}
        generic_attrs = (
            *StructLUXO.__dict__.keys(),
            "kr_stage", "stage_type", "copy",
        )
        for attr in dir(self):
            if not (attr.startswith("_") or attr in generic_attrs):
                value = getattr(self, attr)
                if type(value) != BuiltinMethodType:
                    new_context[attr] = value

        return new_context