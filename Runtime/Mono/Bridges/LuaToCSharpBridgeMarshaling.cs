using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Typed Lua stack pop/push for woven Lua → C# method bridges (default marshal).
    /// </summary>
    internal static class LuaToCSharpBridgeMarshaling
    {
        private static void EnsureArgumentType(IntPtr luaState, int index, Type parameterType)
        {
            if (!TypeMethodRegistration.CanConvertArgumentValue(luaState, index, parameterType))
            {
                LuaDllExtension.error(
                    luaState,
                    $"zlua: argument mismatch at index {index} for type {parameterType.FullName}");
            }
        }

        internal static void ValidateExactArgCount(IntPtr luaState, int argStartIndex, int expectedCount, MethodInfo method)
        {
            int luaArgCount = Math.Max(0, LuaDll.lua_gettop(luaState) - argStartIndex + 1);
            if (luaArgCount != expectedCount)
            {
                LuaDllExtension.error(
                    luaState,
                    $"zlua: argument mismatch for {method.DeclaringType.FullName}.{method.Name}{TypeMethodRegistration.GetParameterSignaturePublic(method.GetParameters())}");
            }
        }

        internal static void ValidateExactArgCount(IntPtr luaState, int argStartIndex, int expectedCount, ConstructorInfo ctor)
        {
            int luaArgCount = Math.Max(0, LuaDll.lua_gettop(luaState) - argStartIndex + 1);
            if (luaArgCount != expectedCount)
            {
                LuaDllExtension.error(
                    luaState,
                    $"zlua: argument mismatch for {ctor.DeclaringType.FullName}..ctor{TypeMethodRegistration.GetParameterSignaturePublic(ctor.GetParameters())}");
            }
        }

        internal static bool PopBool(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(bool));
            return LuaDll.lua_toboolean(luaState, index) != 0;
        }

        internal static char PopChar(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(char));
            return (char)LuaDll.lua_tointeger(luaState, index);
        }

        internal static byte PopByte(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(byte));
            return (byte)LuaDll.lua_tointeger(luaState, index);
        }

        internal static sbyte PopSByte(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(sbyte));
            return (sbyte)LuaDll.lua_tointeger(luaState, index);
        }

        internal static short PopInt16(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(short));
            return (short)LuaDll.lua_tointeger(luaState, index);
        }

        internal static ushort PopUInt16(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(ushort));
            return (ushort)LuaDll.lua_tointeger(luaState, index);
        }

        internal static int PopInt32(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(int));
            return (int)LuaDll.lua_tointeger(luaState, index);
        }

        internal static uint PopUInt32(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(uint));
            return (uint)LuaDll.lua_tointeger(luaState, index);
        }

        internal static long PopInt64(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(long));
            return LuaDll.lua_tointeger(luaState, index);
        }

        internal static ulong PopUInt64(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(ulong));
            return (ulong)LuaDll.lua_tointeger(luaState, index);
        }

        internal static float PopSingle(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(float));
            return (float)LuaDll.lua_tonumber(luaState, index);
        }

        internal static double PopDouble(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(double));
            return LuaDll.lua_tonumber(luaState, index);
        }

        internal static IntPtr PopIntPtr(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(IntPtr));
            return new IntPtr(LuaDll.lua_tointeger(luaState, index));
        }

        internal static UIntPtr PopUIntPtr(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(UIntPtr));
            return (UIntPtr)(ulong)LuaDll.lua_tointeger(luaState, index);
        }

        internal static string PopString(IntPtr luaState, int index)
        {
            EnsureArgumentType(luaState, index, typeof(string));
            return LuaDllExtension.tostring(luaState, index);
        }

        internal static object PopTarget(IntPtr luaState, int index, Type targetType)
        {
            EnsureArgumentType(luaState, index, targetType);
            return TypeMethodRegistration.ReadValuePublic(luaState, index, targetType);
        }

        internal static int PushReturn(IntPtr luaState, Type returnType, object value)
        {
            return TypeMethodRegistration.PushReturnDefaultPublic(luaState, returnType, value);
        }

        /// <summary>
        /// Assigns an instance field on a boxed struct userdata (mutates the GCHandle target).
        /// </summary>
        internal static int SetBoxedInstanceField(IntPtr luaState, FieldInfo field, int targetIndex, int valueIndex)
        {
            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, targetIndex, out object target))
            {
                return LuaDllExtension.error(luaState, "zlua: invalid userdata for field assignment");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadValuePublic(luaState, valueIndex, field.FieldType);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            field.SetValue(target, value);
            return 0;
        }

        /// <summary>
        /// Assigns an instance property on a boxed struct userdata (mutates the GCHandle target).
        /// </summary>
        internal static int SetBoxedInstanceProperty(IntPtr luaState, PropertyInfo property, int targetIndex, int valueIndex)
        {
            MethodInfo setter = property.GetSetMethod(nonPublic: false);
            if (setter == null)
            {
                return LuaDllExtension.error(luaState, $"zlua: property is read-only: {property.Name}");
            }

            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, targetIndex, out object target))
            {
                return LuaDllExtension.error(luaState, "zlua: invalid userdata for property assignment");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadValuePublic(luaState, valueIndex, property.PropertyType);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            try
            {
                setter.Invoke(target, new[] { value });
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            return 0;
        }
    }
}
