// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
