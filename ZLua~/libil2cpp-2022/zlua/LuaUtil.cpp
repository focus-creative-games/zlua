#include "LuaUtil.h"

#include <string>
#include "lua/lua.h"
#include "utils/StringUtils.h"
#include "vm/Type.h"

namespace zlua
{
    bool LuaUtil::IsStrictLuaInteger(lua_State* L, int index)
    {
        if (lua_isinteger(L, index))
            return true;
        if (!lua_isnumber(L, index))
            return false;
        const lua_Number number = lua_tonumber(L, index);
        return number == (lua_Number)(lua_Integer)number;
    }

    int LuaUtil::RaiseAsLuaError(lua_State* L, const char* msg, Il2CppException* ex)
    {
        std::string error_message = msg;
        error_message += ": ";

        error_message += il2cpp::vm::Type::GetName(&ex->klass->byval_arg, Il2CppTypeNameFormat::IL2CPP_TYPE_NAME_FORMAT_IL);
        if (ex->message)
        {
            error_message += ": ";
            error_message += "\n";
            error_message += il2cpp::utils::StringUtils::Utf16ToUtf8(ex->message->chars, ex->message->length);
        }
        lua_pushstring(L, error_message.c_str());
        return lua_error(L);
    }
    }
