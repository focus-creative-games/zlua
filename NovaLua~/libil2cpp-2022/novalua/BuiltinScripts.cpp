#include "BuiltinScripts.h"

#include "LuaEnv.h"

namespace novalua
{
#if __has_include("generated/BuiltinScripts.inc")
#include "generated/BuiltinScripts.inc"
#else
    static const char kNovaLuaGlobalsLua[] = R"novalua(
function __novaluaErrorHandler(err)
    return debug.traceback(err, 2)
end
)novalua";

    static const char kNovaLuaLibLua[] = R"novalua(
novalua = novalua or {}
function novalua.typeof(typeTable) return __novalua_typeof(typeTable) end
function novalua.create_signature(methodName, ...) return __novalua_create_signature(methodName, ...) end
function novalua.signature(methodName, ...) return novalua.create_signature(methodName, ...) end
function novalua.make_generic_type(genericType, ...) return __novalua_make_generic_type(genericType, ...) end
function novalua.get_method(obj, sig)
    if type(obj) == "table" then return obj[sig] end
    local mt = getmetatable(obj)
    if mt and mt.__index and mt.__index[sig] then
        return function(self, ...) return mt.__index[sig](self, ...) end
    end
    return nil
end
function novalua.register_method(obj, name, fn) rawset(obj, name, fn) end
novalua.corlibtypes = novalua.corlibtypes or {}
)novalua";
#endif

    void BuiltinScripts::LoadGlobals()
    {
        LuaEnv::DoStringIgnoreResult(kNovaLuaGlobalsLua);
    }

    void BuiltinScripts::LoadNovaLuaLib()
    {
        LuaEnv::DoStringIgnoreResult(kNovaLuaLibLua);
    }
}
