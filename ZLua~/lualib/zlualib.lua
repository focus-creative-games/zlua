zlua = zlua or {}

function zlua.typeof(typeTable)
    return __zlua_typeof(typeTable)
end

function zlua.create_signature(methodName, ...)
    return __zlua_create_signature(methodName, ...)
end

function zlua.signature(methodName, ...)
    return zlua.create_signature(methodName, ...)
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

function zlua.get_method(obj, sig)
    if type(obj) == "table" then
        return obj[sig]
    end
    local mt = getmetatable(obj)
    if mt and mt.__index and mt.__index[sig] then
        return function(self, ...)
            return mt.__index[sig](self, ...)
        end
    end
    return nil
end

function zlua.register_method(obj, name, fn)
    rawset(obj, name, fn)
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
