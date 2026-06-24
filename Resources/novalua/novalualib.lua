novalua = novalua or {}

function novalua.typeof(typeTable)
    return __novalua_typeof(typeTable)
end

function novalua.create_signature(methodName, ...)
    return __novalua_create_signature(methodName, ...)
end

function novalua.signature(methodName, ...)
    return novalua.create_signature(methodName, ...)
end

function novalua.make_generic_type(genericType, ...)
    return __novalua_make_generic_type(genericType, ...)
end

function novalua.make_szarray_type(elementType)
    return __novalua_make_szarray_type(elementType)
end

function novalua.make_mdarray_type(elementType, rank)
    return __novalua_make_mdarray_type(elementType, rank)
end

function novalua.to_bytes(szarray)
    return __novalua_to_bytes(szarray)
end

function novalua.to_table(szarray)
    return __novalua_to_table(szarray)
end

function novalua.get_method(obj, sig)
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

function novalua.register_method(obj, name, fn)
    rawset(obj, name, fn)
end

novalua.types = {
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
    float = "System.Float",
    double = "System.Double",
    intptr = "System.IntPtr",
    uintptr = "System.UIntPtr",
    decimal = "System.Decimal",
    object = "System.Object",
    string = "System.String",
}
