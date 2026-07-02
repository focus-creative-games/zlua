static const char kZLuaGlobalsLua[] = R"zlua(function __zluaErrorHandler(err)
    return debug.traceback(err, 2)
end
)zlua";

static const char kZLuaLibLua[] = R"zlua(zlua = zlua or {}

function zlua.typeof(typeTable)
    return __zlua_typeof(typeTable)
end

function zlua.signature(...)
    return __zlua_create_signature(...)
end

function zlua.make_generic_type(genericType, ...)
    return __zlua_make_generic_type(genericType, ...)
end

function zlua.make_szarray_type(elementType)
    return __zlua_make_szarray_type(elementType)
end

function zlua.make_mdarray_type(elementType, rank)
    return __zlua_make_mdarray_type(elementType, rank)
end

function zlua.new_szarray_by_element_type(elementType, length)
    return __zlua_new_szarray_by_element_type(elementType, length)
end

function zlua.new_szarray_by_szarray_type(szarrayType, length)
    return __zlua_new_szarray_by_szarray_type(szarrayType, length)
end

function zlua.new_mdarray_by_mdarray_type(mdarrayType, lowbounds, sizes)
    return __zlua_new_mdarray_by_mdarray_type(mdarrayType, lowbounds, sizes)
end

function zlua.new_mdarray_by_spec(elementType, lowbounds, sizes)
    return __zlua_new_mdarray_by_spec(elementType, lowbounds, sizes)
end

function zlua.to_bytes(szarray)
    return __zlua_to_bytes(szarray)
end

function zlua.to_table(szarray)
    return __zlua_to_table(szarray)
end

function zlua.to_delegate(func, delegateType)
    return __zlua_to_delegate(func, delegateType)
end

function zlua.to_user_data(opaque)
    return __zlua_to_user_data(opaque)
end

function zlua.box(typeArg, value)
    return __zlua_box(typeArg, value)
end

function zlua.unbox(boxedValue)
    return __zlua_unbox(boxedValue)
end

function zlua.get_method(target, methodName, signature, is_static)
    return __zlua_get_method(target, methodName, signature, is_static)
end

function zlua.register_method(static_class_mt_or_obj, aliasName, methodOrClosure)
    return __zlua_register_method(static_class_mt_or_obj, aliasName, methodOrClosure)
end

zlua.types = {
    void = "System.Void",
    bool = "System.Boolean",
    char = "System.Char",
    byte = "System.Byte",
    sbyte = "System.SByte",
    short = "System.Int16",
    ushort = "System.UInt16",
    int = "System.Int32",
    int32 = "System.Int32",
    uint = "System.UInt32",
    long = "System.Int64",
    ulong = "System.UInt64",
    float = "System.Single",
    double = "System.Double",
    intptr = "System.IntPtr",
    uintptr = "System.UIntPtr",
    decimal = "System.Decimal",
    object = "System.Object",
    string = "System.String",
}
)zlua";
