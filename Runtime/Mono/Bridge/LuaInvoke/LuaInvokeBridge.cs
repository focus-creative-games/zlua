using System;
using ZLua.Lvm;
using ZLua.Utils;

namespace ZLua.Bridge
{
    /// <summary>
    /// Fast C#→Lua path for woven <c>[LuaInvoke]</c> (Editor). Default-marshal catalog + typed <c>InvokeM_*</c> in partials.
    /// </summary>
    public static partial class LuaInvokeBridge
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
                IntPtr luaState = LuaMonoAppDomain.LuaEnv.L;
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

            /// <summary>
            /// Runs <c>lua_pcall</c>. On failure: outermost CallScope throws; nested CallScope
            /// (Lua→C#→LuaInvoke) stashes <see cref="LuaInvokePendingError"/> — never throw
            /// (Unity Mono SIGSEGVs capturing exception stacks during an outer lua_pcall).
            /// </summary>
            /// <returns>false when a nested invoke failed (pending error set).</returns>
            internal bool TryPCall(int nArgs, int nRet)
            {
                int errFuncIndex = _oldTop + 1;
                int err = LuaDll.lua_pcall(L, nArgs, nRet, errFuncIndex);
                if (err == 0)
                {
                    return true;
                }

                string message = LuaDllExtension.tostring(L, -1) ?? "lua pcall failed";
                if (LuaPrintBuffer.IsNestedManagedPcall)
                {
                    LuaInvokePendingError.Set(message);
                    return false;
                }

                throw new LuaScriptException(message);
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

        public static void InvokeVoid(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.TryPCall(0, 0);
        }

        public static bool Invoke_Boolean(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaDll.lua_toboolean(scope.L, -1) != 0;
        }

        public static char Invoke_Char(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (char)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static byte Invoke_Byte(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (byte)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static sbyte Invoke_SByte(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (sbyte)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static short Invoke_Int16(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (short)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static ushort Invoke_UInt16(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (ushort)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static int Invoke_Int32(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static uint Invoke_UInt32(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (uint)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static long Invoke_Int64(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaDll.lua_tointeger(scope.L, -1);
        }

        public static ulong Invoke_UInt64(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static float Invoke_Single(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        public static double Invoke_Double(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaDll.lua_tonumber(scope.L, -1);
        }

        public static IntPtr Invoke_IntPtr(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return new IntPtr(LuaDll.lua_tointeger(scope.L, -1));
        }

        public static UIntPtr Invoke_UIntPtr(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return (UIntPtr)(ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static string Invoke_String(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaDllExtension.tostring(scope.L, -1);
        }

        public static object Invoke_Object(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopObject(scope.L, -1, typeof(object), LuaMarshalType.Default);
        }

        public static byte[] Invoke_ByteArray(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopByteArray(scope.L, -1, LuaMarshalType.Default);
        }

        public static bool Invoke_Boolean_Boolean(int funcRef, bool p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushboolean(scope.L, p0 ? 1 : 0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaDll.lua_toboolean(scope.L, -1) != 0;
        }

        public static char Invoke_Char_Char(int funcRef, char p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (char)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static byte Invoke_Byte_Byte(int funcRef, byte p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (byte)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static sbyte Invoke_SByte_SByte(int funcRef, sbyte p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (sbyte)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static short Invoke_Int16_Int16(int funcRef, short p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (short)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static ushort Invoke_UInt16_UInt16(int funcRef, ushort p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (ushort)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static int Invoke_Int32_Int32(int funcRef, int p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static uint Invoke_UInt32_UInt32(int funcRef, uint p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (uint)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static long Invoke_Int64_Int64(int funcRef, long p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaDll.lua_tointeger(scope.L, -1);
        }

        public static ulong Invoke_UInt64_UInt64(int funcRef, ulong p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, (long)p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static float Invoke_Single_Single(int funcRef, float p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushnumber(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        public static double Invoke_Double_Double(int funcRef, double p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushnumber(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaDll.lua_tonumber(scope.L, -1);
        }

        public static IntPtr Invoke_IntPtr_IntPtr(int funcRef, IntPtr p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0.ToInt64());
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return new IntPtr(LuaDll.lua_tointeger(scope.L, -1));
        }

        public static UIntPtr Invoke_UIntPtr_UIntPtr(int funcRef, UIntPtr p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, (long)(ulong)p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (UIntPtr)(ulong)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static string Invoke_String_String(int funcRef, string p0)
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

            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaDllExtension.tostring(scope.L, -1);
        }

        public static byte[] Invoke_ByteArray_ByteArray(int funcRef, byte[] p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushByteArray(scope.L, p0, LuaMarshalType.Default);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopByteArray(scope.L, -1, LuaMarshalType.Default);
        }

        public static int Invoke_Int32_ByteArray(int funcRef, byte[] p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushByteArray(scope.L, p0, LuaMarshalType.Default);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }

        public static object Invoke_Object_Object(int funcRef, object p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushObject(scope.L, p0, p0?.GetType() ?? typeof(object), LuaMarshalType.Default);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopObject(scope.L, -1, typeof(object), LuaMarshalType.Default);
        }

        public static int Invoke_Int32_Int32_Int32(int funcRef, int p0, int p1)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaDll.lua_pushinteger(scope.L, p0);
            LuaDll.lua_pushinteger(scope.L, p1);
            if (!scope.TryPCall(2, 1))
            {
                return default;
            }

            return (int)LuaDll.lua_tointeger(scope.L, -1);
        }
    }
}
