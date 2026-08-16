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
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using Unity.Collections.LowLevel.Unsafe;
using ZLua.DelegateImpl;
using ZLua.Emit;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Lvm
{
    /// <summary>
    /// Registers <c>__zlua_*</c> globals.
    /// </summary>
    internal static class ZLuaLib
    {
        private const int MaxMdArrayRank = 32;

        private static readonly LuaCSFunction s_typeof = Guard(ZLuaTypeOf);
        private static readonly LuaCSFunction s_getTypeFromName = Guard(ZLuaGetTypeFromName);
        private static readonly LuaCSFunction s_box = Guard(ZLuaBox);
        private static readonly LuaCSFunction s_unbox = Guard(ZLuaUnbox);
        private static readonly LuaCSFunction s_cast = Guard(ZLuaCast);
        private static readonly LuaCSFunction s_createSignature = Guard(ZLuaCreateSignature);
        private static readonly LuaCSFunction s_makeGenericType = Guard(ZLuaMakeGenericType);
        private static readonly LuaCSFunction s_makeSzArrayType = Guard(ZLuaMakeSzArrayType);
        private static readonly LuaCSFunction s_makeMdArrayType = Guard(ZLuaMakeMdArrayType);
        private static readonly LuaCSFunction s_newSzArrayByElementType = Guard(ZLuaNewSzArrayByElementType);
        private static readonly LuaCSFunction s_newSzArrayBySzArrayType = Guard(ZLuaNewSzArrayBySzArrayType);
        private static readonly LuaCSFunction s_newMdArrayByMdArrayType = Guard(ZLuaNewMdArrayByMdArrayType);
        private static readonly LuaCSFunction s_newMdArrayBySpec = Guard(ZLuaNewMdArrayBySpec);
        private static readonly LuaCSFunction s_toDelegate = Guard(ZLuaToDelegate);
        private static readonly LuaCSFunction s_toBytes = Guard(ZLuaToBytes);
        private static readonly LuaCSFunction s_toTable = Guard(ZLuaToTable);
        private static readonly LuaCSFunction s_makeGenericMethod = Guard(ZLuaMakeGenericMethod);
        private static readonly LuaCSFunction s_registerMethod = Guard(ZLuaRegisterMethod);
        private static readonly LuaCSFunction s_getOpaqueValue = Guard(ZLuaGetOpaqueValue);
        private static readonly LuaCSFunction s_setOpaqueValue = Guard(ZLuaSetOpaqueValue);

        internal static void RegisterGlobals(LuaEnv env)
        {
            IntPtr L = env.L;

            Register(L, "__zlua_typeof", s_typeof);
            Register(L, "__zlua_get_type_from_name", s_getTypeFromName);
            Register(L, "__zlua_box", s_box);
            Register(L, "__zlua_unbox", s_unbox);
            Register(L, "__zlua_cast", s_cast);

            Register(L, "__zlua_create_signature", s_createSignature);
            Register(L, "__zlua_make_generic_type", s_makeGenericType);
            Register(L, "__zlua_make_generic_method", s_makeGenericMethod);
            Register(L, "__zlua_make_szarray_type", s_makeSzArrayType);
            Register(L, "__zlua_make_mdarray_type", s_makeMdArrayType);
            Register(L, "__zlua_new_szarray_by_element_type", s_newSzArrayByElementType);
            Register(L, "__zlua_new_szarray_by_szarray_type", s_newSzArrayBySzArrayType);
            Register(L, "__zlua_new_mdarray_by_mdarray_type", s_newMdArrayByMdArrayType);
            Register(L, "__zlua_new_mdarray_by_spec", s_newMdArrayBySpec);
            Register(L, "__zlua_register_method", s_registerMethod);
            Register(L, "__zlua_to_delegate", s_toDelegate);
            Register(L, "__zlua_get_opaquevalue", s_getOpaqueValue);
            Register(L, "__zlua_set_opaquevalue", s_setOpaqueValue);
            Register(L, "__zlua_to_bytes", s_toBytes);
            Register(L, "__zlua_to_table", s_toTable);
        }

        private static void Register(IntPtr L, string name, LuaCSFunction fn)
        {
            LuaCallbackGate.PushCFunction(
                L,
                global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(fn));
            LuaDll.lua_setglobal(L, name);
        }

        /// <summary>
        /// Mark Lua→C# entry so <see cref="LuaCallbackBoundary.Throw"/> can use <c>lua_error</c>
        /// instead of managed throw (Tuanjie Mono SIGSEGV under outer <c>lua_pcall</c>).
        /// Does not touch <see cref="StructOpaqueScope"/> (opaque handles must survive).
        /// </summary>
        private static LuaCSFunction Guard(LuaCSFunction inner)
        {
            LuaCSFunction wrapped = L =>
            {
                LuaCallbackBoundary.Enter();
                try
                {
                    try
                    {
                        return inner(L);
                    }
                    catch (Exception ex)
                    {
                        return LuaCallbackBoundary.ToLuaError(L, ex);
                    }
                }
                finally
                {
                    LuaCallbackBoundary.Leave();
                }
            };
            return wrapped;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaTypeOf(IntPtr L)
        {
            try
            {
                if (!LuaDll.lua_istable(L, 1))
                {
                    LuaCallbackBoundary.Throw("zlua.typeof expects a csharp type table");
                }

                Type type = TypeRegistry.GetTypeFromTypeTable(L, 1);
                if (type == null)
                {
                    LuaCallbackBoundary.Throw("zlua.typeof expects a csharp type table");
                }

                // System.Type reflection object (same kind as C# typeof(T)).
                ObjectMarshal.Push(L, type, typeof(Type));
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaGetTypeFromName(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_gettop(L) != 1 || LuaDll.lua_type(L, 1) != LuaDataType.String)
                {
                    LuaCallbackBoundary.Throw("zlua.get_type_from_name expects a type name string");
                }

                string typeName = LuaDllExtension.tostring(L, 1);
                Type type = TypeRegistry.ResolveTypeFromName(typeName);
                if (type == null)
                {
                    LuaCallbackBoundary.Throw($"zlua.get_type_from_name: type not found: {typeName}");
                }

                TypeRegistry.PushInternedTypeTable(L, type);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaBox(IntPtr L)
        {
            try
            {
                Type type = TypeRegistry.ResolveTypeArg(L, 1);
                if (type == null)
                {
                    LuaCallbackBoundary.Throw("zlua.box expects type as first argument");
                }

                if (!type.IsValueType)
                {
                    LuaCallbackBoundary.Throw($"zlua.box expects value type, got: {type.FullName}");
                }

                object value;
                if (type.IsEnum)
                {
                    object underlying = TypedMarshal.PopObject(L, 2, Enum.GetUnderlyingType(type));
                    value = Enum.ToObject(type, underlying);
                }
                else if (StructMarshal.IsStructType(type))
                {
                    value = TypedMarshal.PopObject(L, 2, type);
                }
                else
                {
                    value = TypedMarshal.PopObject(L, 2, type);
                }

                object boxed = BoxValue(type, value);
                ObjectMarshal.Push(L, boxed, type);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaUnbox(IntPtr L)
        {
            try
            {
                object obj = ObjectRegistry.Pop(L, 1);
                if (obj == null)
                {
                    LuaCallbackBoundary.Throw("zlua.unbox value is not an object");
                }

                Type type = obj.GetType();
                if (!type.IsValueType)
                {
                    LuaCallbackBoundary.Throw($"zlua.unbox expects value type, got: {type.FullName}");
                }

                TypedMarshal.PushObject(L, obj, type);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaCast(IntPtr L)
        {
            try
            {
                object obj = ObjectRegistry.Pop(L, 1);
                if (obj == null)
                {
                    LuaDll.lua_pushnil(L);
                    return 1;
                }

                Type targetType = TypeRegistry.ResolveTypeArg(L, 2);
                if (targetType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.cast expects type as second argument");
                }

                Type runtimeType = obj.GetType();
                if (!targetType.IsAssignableFrom(runtimeType))
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua.cast failed: {runtimeType.FullName} is not assignable to {targetType.FullName}");
                }

                ObjectMarshal.Push(L, obj, targetType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaCreateSignature(IntPtr L)
        {
            try
            {
                int top = LuaDll.lua_gettop(L);
                var signature = new StringBuilder("(");
                for (int i = 1; i <= top; i++)
                {
                    if (i > 1)
                    {
                        signature.Append(',');
                    }

                    Type type = TypeRegistry.ResolveTypeArg(L, i);
                    if (type == null)
                    {
                        LuaCallbackBoundary.Throw($"zlua.signature arg {i} is not a type");
                    }

                    signature.Append(TypeRegistry.GetLuaFullName(type));
                }

                signature.Append(')');
                LuaDll.lua_pushstring(L, signature.ToString());
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeGenericType(IntPtr L)
        {
            try
            {
                Type genericDef = TypeRegistry.ResolveTypeArg(L, 1);
                if (genericDef == null)
                {
                    LuaCallbackBoundary.Throw("zlua.make_generic_type expects generic type as first arg");
                }

                if (!genericDef.IsGenericTypeDefinition)
                {
                    LuaCallbackBoundary.Throw($"type is not a generic definition: {genericDef.FullName}");
                }

                Type[] genericParams = genericDef.GetGenericArguments();
                int argCount = LuaDll.lua_gettop(L) - 1;
                if (argCount != genericParams.Length)
                {
                    LuaCallbackBoundary.Throw(
                        $"generic arg count mismatch: expected {genericParams.Length}, got {argCount}");
                }

                var typeArgs = new Type[argCount];
                for (int i = 0; i < argCount; i++)
                {
                    Type argType = TypeRegistry.ResolveTypeArg(L, i + 2);
                    if (argType == null)
                    {
                        LuaCallbackBoundary.Throw($"generic arg {i + 1} is not a type");
                    }

                    typeArgs[i] = argType;
                }

                Type closedType = genericDef.MakeGenericType(typeArgs);
                TypeRegistry.PushInternedTypeTable(L, closedType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeSzArrayType(IntPtr L)
        {
            try
            {
                Type elementType = TypeRegistry.ResolveTypeArg(L, 1);
                if (elementType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.make_szarray_type expects element type");
                }

                Type arrayType = elementType.MakeArrayType();
                TypeRegistry.PushInternedTypeTable(L, arrayType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeMdArrayType(IntPtr L)
        {
            try
            {
                Type elementType = TypeRegistry.ResolveTypeArg(L, 1);
                if (elementType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.make_mdarray_type expects element type");
                }

                if (LuaDll.lua_isinteger(L, 2) == 0)
                {
                    LuaCallbackBoundary.Throw("zlua.make_mdarray_type rank must be >= 1 && <= 32");
                }

                int rank = (int)LuaDll.lua_tointeger(L, 2);
                if (rank < 1 || rank > MaxMdArrayRank)
                {
                    LuaCallbackBoundary.Throw("zlua.make_mdarray_type rank must be >= 1 && <= 32");
                }

                Type arrayType = elementType.MakeArrayType(rank);
                TypeRegistry.PushInternedTypeTable(L, arrayType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewSzArrayByElementType(IntPtr L)
        {
            try
            {
                Type elementType = TypeRegistry.ResolveTypeArg(L, 1);
                if (elementType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_element_type expects element type");
                }

                if (LuaDll.lua_isinteger(L, 2) == 0)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_element_type expects integer length");
                }

                long length = LuaDll.lua_tointeger(L, 2);
                if (length < 0)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_element_type length must be >= 0");
                }

                Array array = Array.CreateInstance(elementType, (int)length);
                ObjectMarshal.Push(L, array);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewSzArrayBySzArrayType(IntPtr L)
        {
            try
            {
                Type arrayType = TypeRegistry.ResolveTypeArg(L, 1);
                if (arrayType == null || !arrayType.IsArray || arrayType.GetArrayRank() != 1)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_szarray_type expects szarray type table");
                }

                if (LuaDll.lua_isinteger(L, 2) == 0)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_szarray_type expects integer length");
                }

                long length = LuaDll.lua_tointeger(L, 2);
                if (length < 0)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_szarray_type length must be >= 0");
                }

                // Il2Cpp: Array::NewSpecific(arrayClass, length). CreateInstance expects the
                // *element* type; passing the array type itself yields a jagged array (T[][]).
                Type elementType = arrayType.GetElementType();
                if (elementType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.new_szarray_by_szarray_type expects szarray type table");
                }

                Array array = Array.CreateInstance(elementType, (int)length);
                ObjectMarshal.Push(L, array);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewMdArrayByMdArrayType(IntPtr L)
        {
            try
            {
                Type arrayType = TypeRegistry.ResolveTypeArg(L, 1);
                if (arrayType == null || !arrayType.IsArray || arrayType.GetArrayRank() < 2)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_by_mdarray_type expects mdarray type table");
                }

                Array array = CreateMdArrayInstance(L, arrayType, 2, 3);
                if (array == null)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_by_mdarray_type failed");
                }

                ObjectMarshal.Push(L, array);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewMdArrayBySpec(IntPtr L)
        {
            try
            {
                Type elementType = TypeRegistry.ResolveTypeArg(L, 1);
                if (elementType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_by_spec expects element type");
                }

                if (!TryGetConsecutiveTableLength(L, 3, out int rank))
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_by_spec sizes is not a table");
                }

                if (rank < 1 || rank > MaxMdArrayRank)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_by_spec rank must be >= 1 && <= 32");
                }

                Type arrayType = elementType.MakeArrayType(rank);
                Array array = CreateMdArrayInstance(L, arrayType, 2, 3);
                if (array == null)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_by_spec failed");
                }

                ObjectMarshal.Push(L, array);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaToDelegate(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_type(L, 1) != LuaDataType.Function)
                {
                    LuaCallbackBoundary.Throw("zlua.to_delegate expects Lua function");
                }

                Type delegateType = TypeRegistry.ResolveTypeArg(L, 2);
                if (delegateType == null)
                {
                    LuaCallbackBoundary.Throw("zlua.to_delegate expects closed delegate type");
                }

                if (!typeof(Delegate).IsAssignableFrom(delegateType))
                {
                    LuaCallbackBoundary.Throw("zlua.to_delegate expects delegate type");
                }

                Delegate del = DelegateMarshal.Pop(L, 1, delegateType);
                if (del == null)
                {
                    LuaCallbackBoundary.Throw("unsupported delegate signature for Lua callback");
                }

                ObjectMarshal.Push(L, del);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaToBytes(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_gettop(L) != 1)
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: zlua.to_bytes expects (szarray)");
                }

                if (!LuaDll.lua_isuserdata(L, 1))
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: zlua.to_bytes expects (szarray)");
                }

                object obj = ObjectRegistry.Pop(L, 1);
                if (obj == null)
                {
                    return 0;
                }

                Array array = obj as Array;
                Type arrayType = obj.GetType();
                if (array == null || !arrayType.IsArray || array.Rank != 1 || !IsSzArrayType(arrayType))
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: zlua.to_bytes expects szarray, got: {arrayType.FullName}");
                }

                Type elementType = arrayType.GetElementType();
                if (elementType == null || !UnsafeUtility.IsBlittable(elementType))
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: element type is not blittable: {elementType?.FullName}");
                }

                int byteLength = GetToBytesByteLength(array, elementType);
                if (byteLength == 0)
                {
                    LuaDll.lua_pushstring(L, string.Empty);
                    return 1;
                }

                unsafe
                {
                    GCHandle handle = GCHandle.Alloc(array, GCHandleType.Pinned);
                    try
                    {
                        IntPtr data = handle.AddrOfPinnedObject();
                        LuaDll.lua_pushlstring(L, data, (UIntPtr)byteLength);
                    }
                    finally
                    {
                        handle.Free();
                    }
                }

                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        /// <summary>
        /// Spec: primitives (incl. bool) and POD structs. Unity marks bool as non-blittable
        /// for marshalling, but managed <c>bool[]</c> is still a contiguous 1-byte layout.
        /// </summary>
        private static bool IsToBytesElementType(Type elementType)
        {
            if (elementType.IsPrimitive || elementType.IsEnum)
            {
                return true;
            }

            return StructMarshal.IsBlittable(elementType);
        }

        private static int GetToBytesByteLength(Array array, Type elementType)
        {
            if (elementType.IsPrimitive || elementType.IsEnum)
            {
                return Buffer.ByteLength(array);
            }

            return checked(array.Length * UnsafeUtility.SizeOf(elementType));
        }

        private static bool IsSzArrayType(Type arrayType)
        {
            // Prefer IsSZArray when available; fall back to name form "T[]" vs "T[*]" / "T[,]".
            return arrayType.Name.EndsWith("[]", StringComparison.Ordinal);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaToTable(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_gettop(L) != 1)
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: zlua.to_table expects (szarray)");
                }

                object obj = ObjectRegistry.Pop(L, 1);
                if (obj == null)
                {
                    return 0;
                }

                Array array = obj as Array;
                if (array == null || array.Rank != 1)
                {
                    Type runtimeType = obj.GetType();
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: expected szarray, got: {runtimeType.FullName}");
                }

                ArrayMarshal.PushSzArrayAsTable(L, array);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeGenericMethod(IntPtr L)
        {
            try
            {
                const int closureIndex = 1;
                if (LuaDll.lua_iscfunction(L, closureIndex) == 0)
                {
                    LuaCallbackBoundary.Throw("zlua.make_generic_method expects callable closure");
                }

                if (!ClosurePin.TryGetMethodTag(L, closureIndex, out MethodClosureTag tag) || tag?.Method == null)
                {
                    LuaCallbackBoundary.Throw("zlua.make_generic_method expects direct method closure");
                }

                MethodInfo openMethod = tag.Method;
                if (openMethod == null
                    || (!openMethod.IsGenericMethodDefinition && !openMethod.ContainsGenericParameters))
                {
                    LuaCallbackBoundary.Throw("zlua.make_generic_method expects generic method");
                }

                Type[] genericParams = openMethod.GetGenericArguments();
                int argCount = LuaDll.lua_gettop(L) - closureIndex;
                if (argCount != genericParams.Length)
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua.make_generic_method expects {genericParams.Length} arguments, got {argCount}");
                }

                var typeArgs = new Type[argCount];
                for (int i = 0; i < argCount; i++)
                {
                    Type argType = TypeRegistry.ResolveTypeArg(L, closureIndex + i + 1);
                    if (argType == null)
                    {
                        LuaCallbackBoundary.Throw($"zlua.make_generic_method arg {i + 1} is not a type");
                    }

                    typeArgs[i] = argType;
                }

                MethodInfo closedMethod = openMethod.MakeGenericMethod(typeArgs);
                int closureRef = MethodEmitter.GetOrCreateClosedMethodClosureRef(L, closedMethod, tag);
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, closureRef);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaRegisterMethod(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_gettop(L) != 2)
                {
                    LuaCallbackBoundary.Throw("zlua.register_method expects (aliasName, methodOrClosure)");
                }

                if (LuaDll.lua_type(L, 1) != LuaDataType.String)
                {
                    LuaCallbackBoundary.Throw("zlua.register_method argument mismatch: expects string alias name");
                }

                string aliasName = LuaDllExtension.tostring(L, 1);
                if (string.IsNullOrEmpty(aliasName))
                {
                    LuaCallbackBoundary.Throw("zlua.register_method expects non-empty alias name");
                }

                if (LuaDll.lua_iscfunction(L, 2) == 0)
                {
                    LuaCallbackBoundary.Throw("zlua.register_method argument mismatch: expects callable closure");
                }

                if (!TryRequireMethodClosure(L, 2, out MethodClosureTag tag))
                {
                    LuaCallbackBoundary.Throw("zlua.register_method argument mismatch: expects direct method closure");
                }

                TypeBinding binding = MetaBinding.EnsureBinding(tag.OwnerType);
                MemberTableSet tables = tag.IsStatic
                    ? binding.StaticTables
                    : (tag.IsByVal ? binding.ByValInstanceTables : binding.ByObjInstanceTables);
                Dictionary<string, MetaInfo> map = tag.IsStatic
                    ? binding.StaticMap
                    : (tag.IsByVal ? binding.ByValInstanceMap : binding.ByObjInstanceMap);

                if (map.ContainsKey(aliasName))
                {
                    LuaCallbackBoundary.Throw($"zlua: method alias already exists: {aliasName}");
                }

                if (tables == null || !tables.IsValid)
                {
                    LuaCallbackBoundary.Throw($"zlua: method table not bound for {tag.OwnerType.FullName}");
                }

                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, tables.MethodTableRef);
                LuaDll.lua_getfield(L, -1, aliasName);
                if (!LuaDll.lua_isnil(L, -1))
                {
                    LuaDll.lua_pop(L, 2);
                    LuaCallbackBoundary.Throw($"zlua: method alias already exists: {aliasName}");
                }

                LuaDll.lua_pop(L, 1);
                LuaDll.lua_pushvalue(L, 2);
                LuaDll.lua_setfield(L, -2, aliasName);
                LuaDll.lua_pop(L, 1);

                map[aliasName] = new MetaInfo
                {
                    Kind = MetaKind.Method,
                    Name = aliasName,
                    IsStatic = tag.IsStatic,
                    Method = tag.Method,
                };
                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaGetOpaqueValue(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_gettop(L) != 1 || LuaDll.lua_type(L, 1) != LuaDataType.LightUserData)
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: invalid opaque value handle");
                }

                IntPtr handle = LuaDll.lua_touserdata(L, 1);
                if (!StructOpaqueScope.TryResolveEntry(handle, out object value, out Type valueType))
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: invalid opaque value handle");
                }

                TypedMarshal.PushObject(L, value, valueType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaSetOpaqueValue(IntPtr L)
        {
            try
            {
                if (LuaDll.lua_gettop(L) != 2 || LuaDll.lua_type(L, 1) != LuaDataType.LightUserData)
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: invalid opaque value handle");
                }

                IntPtr handle = LuaDll.lua_touserdata(L, 1);
                if (!StructOpaqueScope.TryResolveEntry(handle, out _, out Type valueType))
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: invalid opaque value handle");
                }

                object newValue = TypedMarshal.PopObject(L, 2, valueType);
                if (!StructOpaqueScope.TryUpdate(handle, newValue))
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: invalid opaque value handle");
                }

                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        private static bool TryRequireMethodClosure(IntPtr L, int index, out MethodClosureTag tag)
        {
            if (LuaDll.lua_iscfunction(L, index) == 0)
            {
                tag = null;
                return false;
            }

            if (!ClosurePin.TryGetMethodTag(L, index, out tag) || tag?.Method == null || tag.OwnerType == null)
            {
                tag = null;
                return false;
            }

            return true;
        }

        private static object BoxValue(Type type, object value)
        {
            if (value != null)
            {
                return value;
            }

            if (type.IsValueType)
            {
                return Activator.CreateInstance(type);
            }

            return null;
        }

        private static Array CreateMdArrayInstance(IntPtr L, Type arrayType, int lowboundsIndex, int sizesIndex)
        {
            int rank = arrayType.GetArrayRank();
            if (!TryGetConsecutiveTableLength(L, lowboundsIndex, out int lowboundsLength))
            {
                LuaCallbackBoundary.Throw("zlua.new_mdarray_* lowbounds is not a table");
            }

            if (!TryGetConsecutiveTableLength(L, sizesIndex, out int sizesLength))
            {
                LuaCallbackBoundary.Throw("zlua.new_mdarray_* sizes is not a table");
            }

            if (lowboundsLength != rank)
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* lowbounds length must be {rank}");
            }

            if (sizesLength != rank)
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* sizes length must be {rank}");
            }

            var lowerBounds = new int[rank];
            var sizes = new int[rank];
            if (!TryReadIntSequence(L, lowboundsIndex, rank, lowerBounds))
            {
                LuaCallbackBoundary.Throw("zlua.new_mdarray_* lowbounds is not a sequence of integers");
            }

            if (!TryReadIntSequence(L, sizesIndex, rank, sizes))
            {
                LuaCallbackBoundary.Throw("zlua.new_mdarray_* sizes is not a sequence of integers");
            }

            for (int i = 0; i < rank; i++)
            {
                if (sizes[i] < 0)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_* sizes must be >= 0");
                }
            }

            Type elementType = arrayType.GetElementType();
            return Array.CreateInstance(elementType, sizes, lowerBounds);
        }

        private static bool TryGetConsecutiveTableLength(IntPtr L, int index, out int length)
        {
            length = 0;
            if (!LuaDll.lua_istable(L, index))
            {
                return false;
            }

            for (int i = 1;; i++)
            {
                LuaDll.lua_rawgeti(L, index, i);
                if (LuaDll.lua_isnil(L, -1))
                {
                    LuaDll.lua_pop(L, 1);
                    break;
                }

                LuaDll.lua_pop(L, 1);
                length = i;
            }

            return true;
        }

        private static bool TryReadIntSequence(IntPtr L, int index, int expectedCount, int[] values)
        {
            if (!LuaDll.lua_istable(L, index))
            {
                return false;
            }

            for (int i = 0; i < expectedCount; i++)
            {
                LuaDll.lua_rawgeti(L, index, i + 1);
                if (LuaDll.lua_isinteger(L, -1) == 0)
                {
                    LuaDll.lua_pop(L, 1);
                    return false;
                }

                values[i] = (int)LuaDll.lua_tointeger(L, -1);
                LuaDll.lua_pop(L, 1);
            }

            return true;
        }

        private static void PushByteString(IntPtr L, byte[] bytes)
        {
            if (bytes.Length == 0)
            {
                LuaDll.lua_pushstring(L, string.Empty);
                return;
            }

            unsafe
            {
                fixed (byte* data = bytes)
                {
                    LuaDll.lua_pushlstring(L, (IntPtr)data, (UIntPtr)bytes.Length);
                }
            }
        }
    }
}
