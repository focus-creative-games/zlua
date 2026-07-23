using System;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Mt
{
    /// <summary>
    /// Resolves instance <c>this</c> for ByObj / ByVal userdata. Full Emit use in Phase 3.
    /// </summary>
    internal static class InstanceTarget
    {
        internal static object PopByObjThis(IntPtr L, int index)
        {
            return ObjectMarshal.Pop(L, index, typeof(object));
        }

        internal static object PopByObjThisAs(IntPtr L, int index, Type declaredType)
        {
            return ObjectMarshal.Pop(L, index, declaredType);
        }

        internal static object PopByValAsBoxed(IntPtr L, int index, Type structType)
        {
            return StructMarshal.PopValue(L, index, structType);
        }

        internal static UserDataKind PeekKind(IntPtr L, int index)
        {
            if (LuaDll.lua_type(L, index) != LuaDataType.UserData)
            {
                return UserDataKind.Unknown;
            }

            unsafe
            {
                UserDataHeader* header = (UserDataHeader*)LuaDll.lua_touserdata(L, index);
                return header != null ? header->Kind : UserDataKind.Unknown;
            }
        }
    }
}
