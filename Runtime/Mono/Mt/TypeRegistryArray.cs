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
using ZLua.Emit;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Mt
{
    internal static class TypeRegistryArray
    {
        private static readonly LuaCSFunction s_arrayLen = ArrayInstanceLen;
        private static readonly LuaCSFunction s_arrayGet = ArrayInstanceGet;
        private static readonly LuaCSFunction s_arraySet = ArrayInstanceSet;

        internal static void CreateTypeTable(IntPtr L, Type type)
        {
            TypeBinding binding = MetaBinding.EnsureBinding(type);
            // CLR exposes DeclaredOnly Get/Set/Address on T[]; Address returns T& and fails Emit
            // (declaredOn==binding.Type → hard error). Native get/set replace them. Also clear
            // leftover lowercase get/set if a prior CreateTypeTable failed after RegisterNative.
            StripClrArrayAccessors(binding);

            LuaDll.lua_createtable(L, 0, 8);
            int typeTableIndex = LuaDll.lua_gettop(L);

            TypeRegistryCommon.WriteCommonTypeFields(L, type, typeTableIndex);
            TypeRegistryCommon.AttachReferenceInstanceMetatable(L, type, typeTableIndex, binding);

            // Il2Cpp: native get/set on instance method table + __len on IMT.
            RegisterArrayElementAccessMethods(L, binding);

            LuaDll.lua_getfield(L, typeTableIndex, LuaConsts.ByObjInstanceMt);
            if (LuaDll.lua_istable(L, -1))
            {
                LuaCallbackGate.PushCFunction(L, global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_arrayLen));
                LuaDll.lua_setfield(L, -2, LuaConsts.MetaLen);
            }

            LuaDll.lua_pop(L, 1);

            TypeRegistryCommon.AttachStaticTypeMetatable(
                L,
                type,
                typeTableIndex,
                binding,
                enableConstructorCall: false,
                enableStructDefault: false);
            MemberTableEmitter.Fill(L, binding, typeTableIndex);
        }

        private static void StripClrArrayAccessors(TypeBinding binding)
        {
            // Pascal CLR accessors + lowercase native keys from a failed prior bind attempt.
            string[] names = { "Get", "Set", "Address", LuaConsts.Get, LuaConsts.Set };
            for (int i = 0; i < names.Length; i++)
            {
                binding.ByObjInstanceMap.Remove(names[i]);
                binding.ByValInstanceMap.Remove(names[i]);
                binding.StaticMap.Remove(names[i]);
            }
        }

        private static void RegisterArrayElementAccessMethods(IntPtr L, TypeBinding binding)
        {
            TypeRegistryCommon.RegisterNativeInstanceMethod(L, binding, LuaConsts.Get, s_arrayGet);
            TypeRegistryCommon.RegisterNativeInstanceMethod(L, binding, LuaConsts.Set, s_arraySet);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ArrayInstanceLen(IntPtr L)
        {
            try
            {
                object obj = ObjectRegistry.PopThis(L, 1);
                if (obj is Array array)
                {
                    LuaDll.lua_pushinteger(L, array.LongLength);
                    return 1;
                }

                LuaCallbackBoundary.Throw("zlua: __len expects array userdata");
                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ArrayInstanceGet(IntPtr L)
        {
            try
            {
                Array array = RequireArrayThis(L, 1);
                int rank = array.Rank;
                int argCount = LuaDll.lua_gettop(L) - 1;
                if (argCount != rank)
                {
                    LuaCallbackBoundary.Throw($"zlua: get expects {rank} index argument(s)");
                }

                if (rank == 1)
                {
                    int index = ReadIndex1D(L, array, stackIndex: 2);
                    ArrayMarshal.PushElement1D(L, array, index);
                    return 1;
                }

                int[] indices = ReadIndices(L, array, indexStart: 2, indexCount: rank);
                object element = array.GetValue(indices);
                Type elementType = array.GetType().GetElementType();
                TypedMarshal.PushObject(L, element, elementType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ArrayInstanceSet(IntPtr L)
        {
            try
            {
                Array array = RequireArrayThis(L, 1);
                int rank = array.Rank;
                int argCount = LuaDll.lua_gettop(L) - 1;
                if (argCount != rank + 1)
                {
                    LuaCallbackBoundary.Throw($"zlua: set expects {rank} index argument(s) and a value");
                }

                int valueIndex = LuaDll.lua_gettop(L);
                if (rank == 1)
                {
                    int index = ReadIndex1D(L, array, stackIndex: 2);
                    Type elementType = array.GetType().GetElementType();
                    ArrayMarshal.SetElement1D(L, array, index, valueIndex, elementType);
                    return 0;
                }

                int[] indices = ReadIndices(L, array, indexStart: 2, indexCount: rank);
                Type mdElementType = array.GetType().GetElementType();
                object value = TypedMarshal.PopObject(L, valueIndex, mdElementType);
                value = ArrayMarshal.CoerceToElementType(value, mdElementType);
                array.SetValue(value, indices);
                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        private static Array RequireArrayThis(IntPtr L, int index)
        {
            object obj = ObjectRegistry.PopThis(L, index);
            if (obj is Array array)
            {
                return array;
            }

            LuaCallbackBoundary.Throw("zlua: expected array userdata");
            return null;
        }

        private static int ReadIndex1D(IntPtr L, Array array, int stackIndex)
        {
            if (LuaDll.lua_type(L, stackIndex) != LuaDataType.Number
                || LuaDll.lua_isinteger(L, stackIndex) == 0)
            {
                LuaCallbackBoundary.Throw("zlua: expected integer index");
            }

            long raw = LuaDll.lua_tointeger(L, stackIndex);
            int index = checked((int)raw);
            int lower = array.GetLowerBound(0);
            int upper = lower + array.GetLength(0) - 1;
            if (index < lower || index > upper)
            {
                LuaCallbackBoundary.Throw($"zlua: array index out of range: {index}");
            }

            return index;
        }

        private static int[] ReadIndices(IntPtr L, Array array, int indexStart, int indexCount)
        {
            var indices = new int[indexCount];
            for (int i = 0; i < indexCount; i++)
            {
                int stackIndex = indexStart + i;
                if (LuaDll.lua_type(L, stackIndex) != LuaDataType.Number
                    || LuaDll.lua_isinteger(L, stackIndex) == 0)
                {
                    LuaCallbackBoundary.Throw("zlua: expected integer indices");
                }

                long raw = LuaDll.lua_tointeger(L, stackIndex);
                int index = checked((int)raw);
                int lower = array.GetLowerBound(i);
                int upper = lower + array.GetLength(i) - 1;
                if (index < lower || index > upper)
                {
                    LuaCallbackBoundary.Throw($"zlua: array index out of range: {index}");
                }

                indices[i] = index;
            }

            return indices;
        }
    }
}
