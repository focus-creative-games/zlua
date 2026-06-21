using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using dnlib.DotNet;
using dnlib.DotNet.Emit;
using dnlib.DotNet.Writer;
using UnityEditor;
using UnityEditor.Compilation;
using MethodAttributes = dnlib.DotNet.MethodAttributes;
using MethodImplAttributes = dnlib.DotNet.MethodImplAttributes;
using NextLua;
using UnityEngine;

namespace NextLua
{
    [InitializeOnLoad]
    internal static class LuaInvokeWeaver
    {
        private static readonly MethodInfo EditorRunLuaFuncVoidMethod = typeof(LuaMonoAppDomain).GetMethods(BindingFlags.Public | BindingFlags.Static)
            .First(m =>
                m.Name == nameof(LuaMonoAppDomain.RunLuaFunc) &&
                !m.IsGenericMethodDefinition &&
                m.GetParameters().Length == 3);

        private static readonly MethodInfo EditorRunLuaFuncGenericDefinition = typeof(LuaMonoAppDomain)
            .GetMethods(BindingFlags.Public | BindingFlags.Static)
            .First(m =>
                m.Name == nameof(LuaMonoAppDomain.RunLuaFunc) &&
                m.IsGenericMethodDefinition &&
                m.GetParameters().Length == 3);

        static LuaInvokeWeaver()
        {
            CompilationPipeline.assemblyCompilationFinished += OnAssemblyCompilationFinished;
            EditorApplication.delayCall += WeaveOnEditorStartup;
        }

        private static void OnAssemblyCompilationFinished(string assemblyPath, CompilerMessage[] messages)
        {
            if (messages != null && messages.Any(m => m.type == CompilerMessageType.Error))
            {
                return;
            }

            if (string.IsNullOrWhiteSpace(assemblyPath) || !File.Exists(assemblyPath))
            {
                return;
            }

            if (ShouldSkipAssembly(assemblyPath))
            {
                return;
            }

            bool isEditorDevelopment = !BuildPipeline.isBuildingPlayer;
            RewriteAssemblyIfNeeded(assemblyPath, isEditorDevelopment);
        }

        private static bool ShouldSkipAssembly(string assemblyPath)
        {
            string fileName = Path.GetFileNameWithoutExtension(assemblyPath);
            if (string.IsNullOrEmpty(fileName))
            {
                return true;
            }

            // 快速忽略 BCL 核心库
            if (string.Equals(fileName, "mscorlib", StringComparison.Ordinal) ||
                string.Equals(fileName, "netstandard", StringComparison.Ordinal) ||
                string.Equals(fileName, "System", StringComparison.Ordinal) ||
                fileName.StartsWith("System.", StringComparison.Ordinal))
            {
                return true;
            }

            // 快速忽略 Unity 引擎内置程序集
            if (string.Equals(fileName, "UnityEngine", StringComparison.Ordinal) ||
                string.Equals(fileName, "UnityEditor", StringComparison.Ordinal) ||
                fileName.StartsWith("UnityEngine.", StringComparison.Ordinal) ||
                fileName.StartsWith("UnityEditor.", StringComparison.Ordinal) ||
                fileName.StartsWith("Unity.", StringComparison.Ordinal))
            {
                return true;
            }

            return false;
        }

        private static void WeaveOnEditorStartup()
        {
            if (BuildPipeline.isBuildingPlayer || EditorApplication.isCompiling || EditorApplication.isUpdating)
            {
                return;
            }

            bool changed = false;
            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            UnityEditor.Compilation.Assembly[] editorAssemblies = CompilationPipeline.GetAssemblies(AssembliesType.Editor);
            UnityEditor.Compilation.Assembly[] playerAssemblies = CompilationPipeline.GetAssemblies(AssembliesType.Player);

            for (int i = 0; i < editorAssemblies.Length; i++)
            {
                string assemblyPath = editorAssemblies[i].outputPath;
                if (string.IsNullOrWhiteSpace(assemblyPath) || !visited.Add(assemblyPath))
                {
                    continue;
                }
                if (ShouldSkipAssembly(assemblyPath) || !File.Exists(assemblyPath))
                {
                    continue;
                }
                changed |= RewriteAssemblyIfNeeded(assemblyPath, isEditorDevelopment: true);
            }

            for (int i = 0; i < playerAssemblies.Length; i++)
            {
                string assemblyPath = playerAssemblies[i].outputPath;
                if (string.IsNullOrWhiteSpace(assemblyPath) || !visited.Add(assemblyPath))
                {
                    continue;
                }
                if (ShouldSkipAssembly(assemblyPath) || !File.Exists(assemblyPath))
                {
                    continue;
                }
                changed |= RewriteAssemblyIfNeeded(assemblyPath, isEditorDevelopment: true);
            }

            if (changed)
            {
                EditorUtility.RequestScriptReload();
            }
        }

        private static bool RewriteAssemblyIfNeeded(string assemblyPath, bool isEditorDevelopment)
        {
            using ModuleDefMD module = ModuleDefMD.Load(File.ReadAllBytes(assemblyPath));
            bool changed = false;

            foreach (TypeDef type in module.GetTypes())
            {
                foreach (MethodDef method in type.Methods)
                {
                    CustomAttribute luaInvokeAttr = FindLuaInvokeAttribute(method);
                    if (luaInvokeAttr == null)
                    {
                        continue;
                    }

                    (string moduleName, string methodName) = ReadLuaInvokeNames(luaInvokeAttr, method);
                    if (IsEditorWeavedMethod(method, moduleName, methodName))
                    {
                        continue;
                    }

                    ValidateLuaInvokeMethod(method);

                    if (isEditorDevelopment)
                    {
                        RewriteEditorMethod(module, method, moduleName, methodName);
                    }
                    else
                    {
                        RewritePlayerMethod(module, method, luaInvokeAttr);
                    }

                    changed = true;
                }
            }

            if (!changed)
            {
                return false;
            }

            File.Copy(assemblyPath, assemblyPath + ".backup", overwrite: true);
            var options = new ModuleWriterOptions(module);
            Debug.Log($"RewriteAssemblyIfNeeded: {assemblyPath}");

            module.Write(assemblyPath, options);
            return true;
        }

        private static void ValidateLuaInvokeMethod(MethodDef method)
        {
            if (!method.IsStatic)
            {
                throw new InvalidOperationException($"[LuaInvoke] method must be static: {method.FullName}");
            }

            if (method.HasBody)
            {
                throw new InvalidOperationException($"[LuaInvoke] method must be extern: {method.FullName}");
            }

            if (method.DeclaringType != null && method.DeclaringType.HasGenericParameters)
            {
                throw new InvalidOperationException($"[LuaInvoke] cannot be declared on generic type: {method.FullName}");
            }

            if (method.HasGenericParameters)
            {
                throw new InvalidOperationException($"[LuaInvoke] method cannot be generic: {method.FullName}");
            }
        }

        private static (string moduleName, string methodName) ReadLuaInvokeNames(CustomAttribute luaInvokeAttr, MethodDef method)
        {
            if (luaInvokeAttr.ConstructorArguments.Count < 2)
            {
                throw new InvalidOperationException($"[LuaInvoke] requires moduleName and methodName: {method.FullName}");
            }

            string moduleName = luaInvokeAttr.ConstructorArguments[0].Value as UTF8String;
            string methodName = luaInvokeAttr.ConstructorArguments[1].Value as UTF8String;

            if (string.IsNullOrWhiteSpace(moduleName) || string.IsNullOrWhiteSpace(methodName))
            {
                throw new InvalidOperationException($"[LuaInvoke] requires non-empty moduleName and methodName: {method.FullName}");
            }

            return (moduleName, methodName);
        }

        private static void RewriteEditorMethod(ModuleDefMD module, MethodDef method, string moduleName, string methodName)
        {
            method.ImplMap = null;
            method.Attributes &= ~MethodAttributes.PinvokeImpl;
            method.ImplAttributes &= ~MethodImplAttributes.InternalCall;
            method.ImplAttributes &= ~MethodImplAttributes.Native;
            method.ImplAttributes |= MethodImplAttributes.Managed | MethodImplAttributes.IL;

            var body = new CilBody
            {
                InitLocals = false
            };

            body.Instructions.Add(OpCodes.Ldstr.ToInstruction(moduleName));
            body.Instructions.Add(OpCodes.Ldstr.ToInstruction(methodName));
            EmitArgumentsArray(body, method);

            if (method.ReturnType.ElementType == ElementType.Void)
            {
                IMethod runMethod = module.Import(EditorRunLuaFuncVoidMethod);
                body.Instructions.Add(OpCodes.Call.ToInstruction(runMethod));
            }
            else
            {
                IMethod runGenericDef = module.Import(EditorRunLuaFuncGenericDefinition);
                var genericSig = new GenericInstMethodSig(method.ReturnType);
                var methodSpec = new MethodSpecUser((IMethodDefOrRef)runGenericDef, genericSig);
                body.Instructions.Add(OpCodes.Call.ToInstruction(methodSpec));
            }

            body.Instructions.Add(OpCodes.Ret.ToInstruction());
            method.Body = body;
        }

        private static bool IsEditorWeavedMethod(MethodDef method, string moduleName, string methodName)
        {
            if (!method.HasBody || method.Body == null)
            {
                return false;
            }

            IList<Instruction> instructions = method.Body.Instructions;
            if (instructions == null || instructions.Count < 4)
            {
                return false;
            }

            if (!IsLdstr(instructions[0], moduleName) || !IsLdstr(instructions[1], methodName))
            {
                return false;
            }

            if (instructions[instructions.Count - 1].OpCode != OpCodes.Ret)
            {
                return false;
            }

            for (int i = 0; i < instructions.Count; i++)
            {
                if (instructions[i].OpCode != OpCodes.Call)
                {
                    continue;
                }

                if (instructions[i].Operand is IMethod called &&
                    string.Equals(called.Name, nameof(LuaMonoAppDomain.RunLuaFunc), StringComparison.Ordinal) &&
                    called.DeclaringType != null &&
                    string.Equals(called.DeclaringType.FullName, typeof(LuaMonoAppDomain).FullName, StringComparison.Ordinal))
                {
                    return true;
                }
            }

            return false;
        }

        private static bool IsLdstr(Instruction instruction, string value)
        {
            if (instruction.OpCode != OpCodes.Ldstr)
            {
                return false;
            }

            if (instruction.Operand is UTF8String utf8)
            {
                return string.Equals(utf8.String, value, StringComparison.Ordinal);
            }

            return string.Equals(instruction.Operand as string, value, StringComparison.Ordinal);
        }

        private static void EmitArgumentsArray(CilBody body, MethodDef method)
        {
            int parameterCount = method.Parameters.Count;
            if (parameterCount == 0)
            {
                body.Instructions.Add(OpCodes.Ldnull.ToInstruction());
                return;
            }

            body.Instructions.Add(OpCodes.Ldc_I4.ToInstruction(parameterCount));
            body.Instructions.Add(OpCodes.Newarr.ToInstruction(method.Module.CorLibTypes.Object.TypeDefOrRef));

            for (int i = 0; i < parameterCount; i++)
            {
                Parameter parameter = method.Parameters[i];
                TypeSig parameterType = parameter.Type;
                if (parameterType.IsByRef)
                {
                    throw new InvalidOperationException($"[LuaInvoke] does not support ref/out parameters in editor mode: {method.FullName}");
                }

                body.Instructions.Add(OpCodes.Dup.ToInstruction());
                body.Instructions.Add(OpCodes.Ldc_I4.ToInstruction(i));
                body.Instructions.Add(OpCodes.Ldarg.ToInstruction(parameter));

                if (parameterType.IsValueType)
                {
                    ITypeDefOrRef boxType = parameterType.ToTypeDefOrRef();
                    if (boxType == null)
                    {
                        throw new InvalidOperationException($"[LuaInvoke] cannot box parameter '{parameter.Name}' of method: {method.FullName}");
                    }
                    body.Instructions.Add(OpCodes.Box.ToInstruction(boxType));
                }

                body.Instructions.Add(OpCodes.Stelem_Ref.ToInstruction());
            }
        }

        private static void RewritePlayerMethod(ModuleDefMD module, MethodDef method, CustomAttribute luaInvokeAttr)
        {
            method.CustomAttributes.Remove(luaInvokeAttr);
            RemoveDllImportAttributeIfExists(method);
            AddDllImportAttribute(module, method, BuildEntryPoint(module, method));
        }

        private static string BuildEntryPoint(ModuleDefMD module, MethodDef method)
        {
            string assemblyName = module.Assembly?.Name?.String ?? module.Name;
            string fullTypeName = method.DeclaringType?.FullName ?? string.Empty;
            string rawName = $"{assemblyName}_{fullTypeName}_{method.Name}";
            return Regex.Replace(rawName, "[^A-Za-z0-9_]", string.Empty);
        }

        private static void AddDllImportAttribute(ModuleDefMD module, MethodDef method, string entryPoint)
        {
            TypeRef dllImportTypeRef = module.CorLibTypes.GetTypeRef("System.Runtime.InteropServices", nameof(DllImportAttribute));
            var ctorSig = MethodSig.CreateInstance(module.CorLibTypes.Void, module.CorLibTypes.String);
            var ctorRef = new MemberRefUser(module, ".ctor", ctorSig, dllImportTypeRef);

            var attribute = new CustomAttribute(ctorRef);
            attribute.ConstructorArguments.Add(new CAArgument(module.CorLibTypes.String, "__Internal"));
            attribute.NamedArguments.Add(new CANamedArgument(
                true,
                module.CorLibTypes.String,
                "EntryPoint",
                new CAArgument(module.CorLibTypes.String, entryPoint)));

            method.CustomAttributes.Add(attribute);
        }

        private static void RemoveDllImportAttributeIfExists(MethodDef method)
        {
            for (int i = method.CustomAttributes.Count - 1; i >= 0; i--)
            {
                if (IsAttribute(method.CustomAttributes[i], "System.Runtime.InteropServices", nameof(DllImportAttribute)))
                {
                    method.CustomAttributes.RemoveAt(i);
                }
            }
        }

        private static CustomAttribute FindLuaInvokeAttribute(MethodDef method)
        {
            for (int i = 0; i < method.CustomAttributes.Count; i++)
            {
                CustomAttribute attr = method.CustomAttributes[i];
                if (IsAttribute(attr, "NextLua", nameof(LuaInvokeAttribute)))
                {
                    return attr;
                }
            }

            return null;
        }

        private static bool IsAttribute(CustomAttribute attr, string @namespace, string name)
        {
            ITypeDefOrRef typeRef = attr.AttributeType;
            return typeRef != null &&
                   string.Equals(typeRef.Namespace, @namespace, StringComparison.Ordinal) &&
                   string.Equals(typeRef.Name, name, StringComparison.Ordinal);
        }

    }
}
