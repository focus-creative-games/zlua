using System;

namespace ZLua
{
    /// <summary>
    /// Fast C# → Lua path for woven <see cref="LuaInvoke"/> (default marshal only).
    /// </summary>
    internal static partial class LuaInvokeBridge
    {
        private readonly struct CallScope : IDisposable
        {
            internal readonly IntPtr L;
            private readonly int _oldTop;
            private readonly int _funcRef;

            private CallScope(IntPtr luaState, int oldTop, int funcRef)
            {
                L = luaState;
                _oldTop = oldTop;
                _funcRef = funcRef;
            }

            internal static CallScope Begin(int funcRef)
            {
                IntPtr luaState = LuaMonoAppDomain.LuaEnv.LuaState;
                int oldTop = LuaDll.lua_gettop(luaState);
                LuaPrintBuffer.EnterManagedPcall();
                StructOpaqueScope.EnterStandaloneCSharpToLua();
                return new CallScope(luaState, oldTop, funcRef);
            }

            internal void PushFunction()
            {
                LuaMonoAppDomain.LuaEnv.PushErrorHandler(L);
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, _funcRef);
            }

            internal void PCall(int nArgs, int nRet)
            {
                int errFuncIndex = _oldTop + 1;
                int err = LuaDll.lua_pcall(L, nArgs, nRet, errFuncIndex);
                if (err != 0)
                {
                    throw new Exception(LuaDllExtension.tostring(L, -1));
                }
            }

            public void Dispose()
            {
                try
                {
                    LuaDll.lua_settop(L, _oldTop);
                }
                finally
                {
                    LuaPrintBuffer.LeaveManagedPcall();
                }
            }
        }

        internal static void InvokeVoid(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 0);
        }

        internal static bool Invoke_Boolean(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return LuaDll.lua_toboolean(scope.L, -1) != 0;
        }

        internal static char Invoke_Char(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (char)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static byte Invoke_Byte(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (byte)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static sbyte Invoke_SByte(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (sbyte)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static short Invoke_Int16(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (short)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static ushort Invoke_UInt16(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (ushort)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static int Invoke_Int32(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static uint Invoke_UInt32(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (uint)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static long Invoke_Int64(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static ulong Invoke_UInt64(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static float Invoke_Single(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        internal static double Invoke_Double(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return LuaDll.lua_tonumber(scope.L, -1);
        }

        internal static IntPtr Invoke_IntPtr(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return new IntPtr(LuaDll.lua_tointeger(scope.L, -1));
        }

        internal static UIntPtr Invoke_UIntPtr(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return (UIntPtr)(ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static string Invoke_String(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return LuaDllExtension.tostring(scope.L, -1);
        }

        internal static bool Invoke_Boolean_Boolean(int funcRef, bool p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushboolean(scope.L, p0 ? 1 : 0);
            scope.PCall(1, 1);
            return LuaDll.lua_toboolean(scope.L, -1) != 0;
        }

        internal static char Invoke_Char_Char(int funcRef, char p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (char)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static byte Invoke_Byte_Byte(int funcRef, byte p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (byte)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static sbyte Invoke_SByte_SByte(int funcRef, sbyte p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (sbyte)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static short Invoke_Int16_Int16(int funcRef, short p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (short)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static ushort Invoke_UInt16_UInt16(int funcRef, ushort p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (ushort)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static int Invoke_Int32_Int32(int funcRef, int p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static uint Invoke_UInt32_UInt32(int funcRef, uint p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return (uint)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static long Invoke_Int64_Int64(int funcRef, long p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            scope.PCall(1, 1);
            return LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static ulong Invoke_UInt64_UInt64(int funcRef, ulong p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, (long)p0);
            scope.PCall(1, 1);
            return (ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static float Invoke_Single_Single(int funcRef, float p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushnumber(scope.L, p0);
            scope.PCall(1, 1);
            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        internal static double Invoke_Double_Double(int funcRef, double p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushnumber(scope.L, p0);
            scope.PCall(1, 1);
            return LuaDll.lua_tonumber(scope.L, -1);
        }

        internal static IntPtr Invoke_IntPtr_IntPtr(int funcRef, IntPtr p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0.ToInt64());
            scope.PCall(1, 1);
            return new IntPtr(LuaDll.lua_tointeger(scope.L, -1));
        }

        internal static UIntPtr Invoke_UIntPtr_UIntPtr(int funcRef, UIntPtr p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, (long)(ulong)p0);
            scope.PCall(1, 1);
            return (UIntPtr)(ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        internal static string Invoke_String_String(int funcRef, string p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (p0 == null)
            {
                LuaDll.lua_pushnil(scope.L);
            }
            else
            {
                LuaDll.lua_pushstring(scope.L, p0);
            }

            scope.PCall(1, 1);
            return LuaDllExtension.tostring(scope.L, -1);
        }

        internal static int Invoke_Int32_Int32_Int32(int funcRef, int p0, int p1)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            LuaDll.lua_pushinteger(scope.L, p1);
            scope.PCall(2, 1);
            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }
    }
}
