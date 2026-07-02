#include "BuiltinScripts.h"

#include "LuaEnv.h"

namespace zlua
{
#if __has_include("generated/BuiltinScripts.inc")
#include "generated/BuiltinScripts.inc"
#else
    static const char kZLuaGlobalsLua[] = R"zlua(
function __zluaErrorHandler(err)
    return debug.traceback(err, 2)
end
)zlua";

    static const char kZLuaLibLua[] = R"zlua(
zlua = zlua or {}
function zlua.typeof(typeTable) return __zlua_typeof(typeTable) end
function zlua.create_signature(methodName, ...) return __zlua_create_signature(methodName, ...) end
function zlua.signature(methodName, ...) return zlua.create_signature(methodName, ...) end
function zlua.make_generic_type(genericType, ...) return __zlua_make_generic_type(genericType, ...) end
function zlua.to_bytes(szarray) return __zlua_to_bytes(szarray) end
function zlua.to_table(szarray) return __zlua_to_table(szarray) end
function zlua.get_method(obj, sig)
    if type(obj) == "table" then return obj[sig] end
    local mt = getmetatable(obj)
    if mt and mt.__index and mt.__index[sig] then
        return function(self, ...) return mt.__index[sig](self, ...) end
    end
    return nil
end
function zlua.register_method(obj, name, fn) rawset(obj, name, fn) end
zlua.corlibtypes = zlua.corlibtypes or {}
)zlua";
#endif

    void BuiltinScripts::LoadGlobals()
    {
        LuaEnv::DoStringIgnoreResult(kZLuaGlobalsLua);
        LuaEnv::EnsureErrorHandlerCached();
    }

    void BuiltinScripts::LoadZLuaLib()
    {
        LuaEnv::DoStringIgnoreResult(kZLuaLibLua);
    }
}
