using System;
using UnityEngine;
using ZLua.Lvm;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Bridge
{
    /// <summary>
    /// Default-marshal Unity vector catalog entries for woven <c>[LuaInvoke]</c>.
    /// </summary>
    public static partial class LuaInvokeBridge
    {
        public static Vector2 Invoke_Vector2(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector2(scope.L, -1);
        }

        public static Vector3 Invoke_Vector3(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector3(scope.L, -1);
        }

        public static Vector4 Invoke_Vector4(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector4(scope.L, -1);
        }

        public static Vector2 Invoke_Vector2_Vector2(int funcRef, Vector2 p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector2(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector2(scope.L, -1);
        }

        public static Vector3 Invoke_Vector3_Vector3(int funcRef, Vector3 p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector3(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector3(scope.L, -1);
        }

        public static Vector4 Invoke_Vector4_Vector4(int funcRef, Vector4 p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector4(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector4(scope.L, -1);
        }

        public static float Invoke_Single_Vector2(int funcRef, Vector2 p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector2(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        public static float Invoke_Single_Vector3(int funcRef, Vector3 p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector3(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        public static float Invoke_Single_Vector4(int funcRef, Vector4 p0)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector4(scope.L, p0);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return (float)LuaDll.lua_tonumber(scope.L, -1);
        }

        public static Vector3 Invoke_Vector3_Vector3_Vector3(int funcRef, Vector3 p0, Vector3 p1)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushVector3(scope.L, p0);
            LuaInvokeBridgeMarshaling.PushVector3(scope.L, p1);
            if (!scope.TryPCall(2, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopVector3(scope.L, -1);
        }
    }
}
