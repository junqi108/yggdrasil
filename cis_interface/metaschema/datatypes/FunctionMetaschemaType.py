import types
import importlib
from cis_interface.metaschema.datatypes import register_type
from cis_interface.metaschema.datatypes.MetaschemaType import MetaschemaType


@register_type
class FunctionMetaschemaType(MetaschemaType):
    r"""Type for evaluating functions."""

    name = 'function'
    description = 'Type for callable Python functions.'
    python_types = (types.BuiltinFunctionType, types.FunctionType,
                    types.BuiltinMethodType, types.MethodType)

    @classmethod
    def encode_data(cls, obj, typedef):
        r"""Encode an object's data.

        Args:
            obj (object): Object to encode.
            typedef (dict): Type definition that should be used to encode the
                object.

        Returns:
            string: Encoded object.

        """
        # fname = obj.__globals__['__file__']
        mod = obj.__module__
        fun = obj.__name__
        out = '%s:%s' % (mod, fun)
        return out

    @classmethod
    def decode_data(cls, obj, typedef):
        r"""Decode an object.

        Args:
            obj (string): Encoded object to decode.
            typedef (dict): Type definition that should be used to decode the
                object.

        Returns:
            object: Decoded object.

        """
        pkg_mod = obj.split(':')
        if len(pkg_mod) != 2:
            raise ValueError("Could not parse function string: %s" % obj)
        mod, fun = pkg_mod[:]
        modobj = importlib.import_module(mod)
        if not hasattr(modobj, fun):
            raise AttributeError("Module %s has no funciton %s" % (modobj, fun))
        return getattr(modobj, fun)

    @classmethod
    def transform_type(cls, obj, typedef=None):
        r"""Transform an object based on type info.

        Args:
            obj (object): Object to transform.
            typedef (dict): Type definition that should be used to transform the
                object.

        Returns:
            object: Transformed object.

        """
        return obj
