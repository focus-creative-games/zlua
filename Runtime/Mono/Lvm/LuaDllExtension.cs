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

#if ZLUA_USE_LUAJIT || ZLUA_LUA_5_1
#define ZLUA_MONO_LUA51_API
#endif

using System;
using System.Runtime.InteropServices;
using System.Text;
using ZLua.Utils;

namespace ZLua
{
    public static class LuaDllExtension
    {
        [DllImport(LuaDll.LUA_DLL, EntryPoint = "luaL_loadstring", CallingConvention = LuaDll.CALLING_CONVENTION)]
        public static extern int loadstring(IntPtr luaState, string chunk);

#if ZLUA_MONO_LUA51_API
        // luaL_loadbufferx is Lua 5.2+ (LuaJIT keeps luaL_loadbuffer).
        [DllImport(LuaDll.LUA_DLL, EntryPoint = "luaL_loadbuffer", CallingConvention = LuaDll.CALLING_CONVENTION)]
        private static extern int luaL_loadbuffer(IntPtr luaState, byte[] buff, UIntPtr sz, string name);
#else
        [DllImport(LuaDll.LUA_DLL, EntryPoint = "luaL_loadbufferx", CallingConvention = LuaDll.CALLING_CONVENTION)]
        private static extern int luaL_loadbufferx(IntPtr luaState, byte[] buff, UIntPtr sz, string name, IntPtr mode);
#endif

        public static int loadbuffer(IntPtr luaState, byte[] buffer, string chunkName)
        {
            if (buffer == null || buffer.Length == 0)
            {
                return LUA_ERRSYNTAX;
            }

#if ZLUA_MONO_LUA51_API
            return luaL_loadbuffer(luaState, buffer, (UIntPtr)buffer.Length, chunkName);
#else
            return luaL_loadbufferx(luaState, buffer, (UIntPtr)buffer.Length, chunkName, IntPtr.Zero);
#endif
        }

        private const int LUA_ERRSYNTAX = 3;

        public static int dostring(IntPtr luaState, string chunk)
        {
            int result = loadstring(luaState, chunk);
            if (result != 0)
            {
                return result;
            }

            return LuaDll.lua_pcall(luaState, 0, LuaConsts.LuaMultiRet, 0);
        }

        public static unsafe string tostring(IntPtr luaState, int index)
        {
            IntPtr strPtr = LuaDll.lua_tolstring(luaState, index, out UIntPtr len);
            if (strPtr == IntPtr.Zero)
            {
                return null;
            }

            return Encoding.UTF8.GetString((byte*)strPtr, (int)len);
        }

        /// <summary>
        /// Convert a Lua error object to a non-empty string (never null/empty for pcall failures).
        /// </summary>
        public static unsafe string FormatErrorObject(IntPtr luaState, int index)
        {
            int top = LuaDll.lua_gettop(luaState);
            if (index < 0 && index > LuaConsts.LuaRegistryIndex)
            {
                index = top + index + 1;
            }

            IntPtr strPtr = LuaDll.luaL_tolstring(luaState, index, out UIntPtr len);
            try
            {
                if (strPtr != IntPtr.Zero && len != UIntPtr.Zero)
                {
                    return Encoding.UTF8.GetString((byte*)strPtr, (int)len);
                }
            }
            finally
            {
                LuaDll.lua_settop(luaState, top);
            }

            LuaDataType type = LuaDll.lua_type(luaState, index);
            return "lua pcall failed (error object is " + TypeName(type) + ")";
        }

        private static string TypeName(LuaDataType type)
        {
            switch (type)
            {
                case LuaDataType.Nil: return "nil";
                case LuaDataType.Boolean: return "boolean";
                case LuaDataType.LightUserData: return "userdata";
                case LuaDataType.Number: return "number";
                case LuaDataType.String: return "string";
                case LuaDataType.Table: return "table";
                case LuaDataType.Function: return "function";
                case LuaDataType.UserData: return "userdata";
                case LuaDataType.Thread: return "thread";
                default: return "no value";
            }
        }

        public static int error(IntPtr luaState, string msg)
        {
            LuaDll.lua_pushstring(luaState, msg);
            // Editor Mono: native gate calls lua_error after managed returns (all Lua series).
            return LuaCallbackGate.ErrorSentinel;
        }

        public static void RegisterCallback(IntPtr luaState, string globalName, LuaCSFunction func)
        {
            IntPtr function = global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(func);
            LuaCallbackGate.PushCFunction(luaState, function);
            LuaDll.lua_setglobal(luaState, globalName);
        }
    }
}
