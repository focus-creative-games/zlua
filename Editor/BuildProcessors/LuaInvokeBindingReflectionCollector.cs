// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;

namespace ZLua.BuildProcessors
{
    internal static class LuaInvokeBindingReflectionCollector
    {
        private const BindingFlags MethodFlags = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.DeclaredOnly;

        internal static List<LuaInvokeBindingInfo> Collect(IEnumerable<Assembly> assemblies)
        {
            var results = new List<LuaInvokeBindingInfo>();
            var seen = new HashSet<string>(StringComparer.Ordinal);
            if (assemblies == null)
            {
                return results;
            }

            foreach (Assembly assembly in assemblies)
            {
                CollectFromAssembly(assembly, results, seen);
            }

            results.Sort((a, b) => string.CompareOrdinal(a.SiteId, b.SiteId));
            return results;
        }

        private static void CollectFromAssembly(
            Assembly assembly,
            List<LuaInvokeBindingInfo> results,
            HashSet<string> seen)
        {
            if (assembly == null || assembly.IsDynamic)
            {
                return;
            }

            Type[] types;
            try
            {
                types = assembly.GetTypes();
            }
            catch (ReflectionTypeLoadException ex)
            {
                types = ex.Types;
            }

            if (types == null)
            {
                return;
            }

            for (int i = 0; i < types.Length; i++)
            {
                Type type = types[i];
                if (type == null || type.IsGenericTypeDefinition || type.ContainsGenericParameters)
                {
                    continue;
                }

                MethodInfo[] methods;
                try
                {
                    methods = type.GetMethods(MethodFlags);
                }
                catch
                {
                    continue;
                }

                for (int j = 0; j < methods.Length; j++)
                {
                    if (!TryCreateBinding(methods[j], assembly, out LuaInvokeBindingInfo binding))
                    {
                        continue;
                    }

                    if (!seen.Add(binding.InternalCallName))
                    {
                        continue;
                    }

                    results.Add(binding);
                }
            }
        }

        private static bool TryCreateBinding(MethodInfo method, Assembly assembly, out LuaInvokeBindingInfo binding)
        {
            binding = null;
            if (method == null || !method.IsStatic || method.IsAbstract || method.ContainsGenericParameters)
            {
                return false;
            }

            LuaInvokeAttribute invokeAttribute = method.GetCustomAttribute<LuaInvokeAttribute>(inherit: false);
            if (invokeAttribute == null
                || string.IsNullOrWhiteSpace(invokeAttribute.Module)
                || string.IsNullOrWhiteSpace(invokeAttribute.Function))
            {
                return false;
            }

            Type declaringType = method.DeclaringType;
            if (declaringType == null)
            {
                return false;
            }

            Type returnType = method.ReturnType;
            LuaMarshalType returnMarshal = returnType == typeof(void)
                ? LuaMarshalType.Default
                : LuaMarshalAsValidation.ResolveReturnBinding(method, LuaMarshalDirection.LuaToCSharp).MarshalType;

            var parameters = new List<LuaInvokeParamInfo>();
            foreach (ParameterInfo parameter in method.GetParameters())
            {
                Type paramType = parameter.ParameterType;
                parameters.Add(new LuaInvokeParamInfo
                {
                    TypeName = LuaInvokeReflectionTypeUtil.GetTypeName(paramType),
                    FullTypeName = paramType.FullName ?? paramType.Name,
                    Name = parameter.Name,
                    MarshalType = LuaMarshalAsValidation.ResolveParameterBinding(
                        parameter,
                        method,
                        LuaMarshalDirection.CSharpToLua).MarshalType,
                    ClrType = paramType,
                    IsEnum = paramType.IsEnum,
                    IsStruct = paramType.IsValueType && !paramType.IsEnum,
                });
            }

            binding = new LuaInvokeBindingInfo
            {
                ClassName = declaringType.Name,
                NamespaceName = declaringType.Namespace ?? string.Empty,
                MethodName = method.Name,
                ModuleName = invokeAttribute.Module,
                LuaMethodName = invokeAttribute.Function,
                AssemblyName = assembly.GetName().Name ?? "Unknown",
                ReturnType = LuaInvokeReflectionTypeUtil.GetTypeName(returnType),
                ReturnFullTypeName = returnType.FullName ?? returnType.Name,
                ReturnClrType = returnType,
                ReturnIsEnum = returnType.IsEnum,
                ReturnMarshal = returnMarshal,
                Parameters = parameters,
                UseFullBinding = returnMarshal != LuaMarshalType.Default
                    || parameters.Any(p => p.MarshalType != LuaMarshalType.Default),
            };
            return true;
        }
    }

    internal static class LuaInvokeReflectionTypeUtil
    {
        internal static string GetTypeName(Type type)
        {
            if (type == null || type == typeof(void))
            {
                return "void";
            }

            if (type == typeof(bool)) return "bool";
            if (type == typeof(char)) return "char";
            if (type == typeof(byte)) return "byte";
            if (type == typeof(sbyte)) return "sbyte";
            if (type == typeof(short)) return "short";
            if (type == typeof(ushort)) return "ushort";
            if (type == typeof(int)) return "int";
            if (type == typeof(uint)) return "uint";
            if (type == typeof(long)) return "long";
            if (type == typeof(ulong)) return "ulong";
            if (type == typeof(float)) return "float";
            if (type == typeof(double)) return "double";
            if (type == typeof(string)) return "string";
            if (type == typeof(IntPtr)) return "IntPtr";
            if (type == typeof(UIntPtr)) return "UIntPtr";
            if (type.IsArray && type.GetElementType() == typeof(byte))
            {
                return "byte[]";
            }

            return type.Name;
        }
    }
}
