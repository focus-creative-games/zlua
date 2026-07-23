using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using dnlib.DotNet;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{
    public class PropertyBindingInfo
    {
        public PropertyDef propertyDef;

        public TypeSig retType;

        public string uniqueName;

        public string stubName;

        public string StaticGetterMethodName => $"StaticGetter_{uniqueName}";
        public string StaticSetterMethodName => $"StaticSetter_{uniqueName}";
        public string InstanceGetterMethodName => $"InstanceGetter_{uniqueName}";
        public string InstanceSetterMethodName => $"InstanceSetter_{uniqueName}";
    }

    public class PropertyBridgdAnalyzer
    {
        public PropertyBridgdAnalyzer()
        {
        }

        private bool IsNonArgumentProperty(PropertyDef propertyDef)
        {
            MethodDef getter = propertyDef.GetMethod;
            if (getter != null && getter.Parameters.Count == 0)
            {
                return true;
            }
            MethodDef setter = propertyDef.SetMethod;
            if (setter != null && setter.Parameters.Count == 1)
            {
                return true;
            }
            return false;
        }

        public List<PropertyBindingInfo> Collect(AssemblyCache assemblyCache, List<string> assemblyNames)
        {
            var results = new List<PropertyBindingInfo>();

            foreach (string assName in assemblyNames)
            {
                ModuleDef mod = assemblyCache.LoadModule(assName);
                foreach (TypeDef typeDef in mod.GetTypes())
                {
                    if (typeDef.HasGenericParameters)
                    {
                        continue;
                    }
                    foreach (PropertyDef propertyDef in typeDef.Properties)
                    {
                        // only collect
                        if (!IsNonArgumentProperty(propertyDef))
                        {
                            continue;
                        }
                        TypeSig retType = propertyDef.PropertySig.RetType;
                        if (!MetaUtil.IsStruct(retType))
                        {
                            continue;
                        }
                        results.Add(new PropertyBindingInfo
                        {
                            propertyDef = propertyDef,
                            retType = propertyDef.PropertySig.RetType,
                            uniqueName = NameUtil.CreateUniqueName(propertyDef),
                            stubName = retType.FullName,
                        });
                    }
                }
            }
            return results;
        }
    }

    public class PropertyBridgdeCodegen
    {
        private readonly AssemblyCache _assemblyCache;
        private readonly List<string> _assemblyNames;
        private readonly string _outputDir;

        public PropertyBridgdeCodegen(AssemblyCache assemblyCache, List<string> assemblyNames, string outputDir)
        {
            _assemblyCache = assemblyCache;
            _assemblyNames = assemblyNames;
            _outputDir = outputDir;
        }

        public void Generate()
        {
            GenerateHeader();
            GenerateSource();
        }

        private void GenerateHeader()
        {
            var writer = new CodeWriter();
            writer.WriteLine("#pragma once");
            writer.WriteLine();
            writer.WriteLine("#include \"../bridge/BridgeDefs.h\"");
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace propertybridge");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"extern const {ConstStrings.namePropertyBridgeEntry} g_propertyBridges[];");
            writer.DecreaseIndent();
            writer.WriteLine("} // namespace propertybridge");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "PropertyBridgeStub.h"));
        }

        private void GenerateSource()
        {
            var analyzer = new PropertyBridgdAnalyzer();
            List<PropertyBindingInfo> originalBindings = analyzer.Collect(_assemblyCache, _assemblyNames);
            var sortedBindings = originalBindings.Select(b => (b, $"{b.propertyDef.DeclaringType.FullName}.{b.propertyDef.Name}"))
            .OrderBy(x => x.Item2).Select(x => x.Item1).ToList();

            var writer = new CodeWriter();
            writer.WriteLine("#include \"PropertyBridgeStub.h\"");
            writer.WriteLine();
            CodegenCommon.AddCommonIncludes(writer);
            writer.WriteLine();
            writer.WriteLine("#include \"vm/Class.h\"");
            writer.WriteLine("#include \"vm/InternalCalls.h\"");
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace propertybridge");
            writer.WriteLine("{");
            writer.IncreaseIndent();

            var forwardDeclareStructsGenerator = new ForwardDeclareStructsCodeGen();
            var uniqueRetTypes = new HashSet<TypeSig>(TypeEqualityComparer.Instance);

            var effectiveBindings = new List<PropertyBindingInfo>();
            foreach (var binding in sortedBindings)
            {
                if (!uniqueRetTypes.Add(binding.retType))
                {
                    continue;
                }
                forwardDeclareStructsGenerator.CollectStructDefs(binding.propertyDef);
                effectiveBindings.Add(binding);
            }
            forwardDeclareStructsGenerator.WriteForwardDeclareStructs(writer);

            foreach (var binding in effectiveBindings)
            {
                GeneratePropertyBridgeFunctions(writer, binding);
            }
            GeneratePropertyBridgeStub(writer, effectiveBindings);

            writer.DecreaseIndent();
            writer.WriteLine("} // namespace propertybridge");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "PropertyBridgeStub.cpp"));
        }

        private void GenerateStaticGetterFunction(CodeWriter writer, PropertyBindingInfo binding)
        {
            string retTypeName = NameUtil.GetTypeName(binding.retType);
            
            writer.WriteLine($"static void {binding.StaticGetterMethodName}(lua_State* L, void* target /* nullptr */, {ConstStrings.typeMethodInfo} method, {ConstStrings.typeConstPropertyMarshalCtxPtr} ctx)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"typedef {retTypeName} (*FnGetter)({ConstStrings.typeMethodInfo});");
            writer.WriteLine($"FnGetter fnGetter = (FnGetter)method->{ConstStrings.nameMethodPointer};");
            writer.WriteLine($"{retTypeName} result = fnGetter(method);");
            string marshalMethodName = MetaUtil.IsNullable(binding.retType) ? "PushNullableValue" : "PushValue";
            writer.WriteLine($"StructMarshal::{marshalMethodName}(L, &result, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));");
            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private void GenerateStaticSetterFunction(CodeWriter writer, PropertyBindingInfo binding)
        {
            string retTypeName = NameUtil.GetTypeName(binding.retType);

            writer.WriteLine($"static void {binding.StaticSetterMethodName}(lua_State* L, void* target, int valueIdx, {ConstStrings.typeMethodInfo} method, {ConstStrings.typeConstPropertyMarshalCtxPtr} ctx)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"typedef void (*FnSetter)({retTypeName}, {ConstStrings.typeMethodInfo});");
            writer.WriteLine($"{retTypeName} value;");
            string marshalMethodName = MetaUtil.IsNullable(binding.retType) ? "PopNullableValue" : "PopValue";
            writer.WriteLine($"StructMarshal::{marshalMethodName}(L, valueIdx, ctx->valueTypeKlass, &value);");
            writer.WriteLine($"FnSetter fnSetter = (FnSetter)method->{ConstStrings.nameMethodPointer};");
            writer.WriteLine($"fnSetter(value, method);");
            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private void GenerateInstanceGetterFunction(CodeWriter writer, PropertyBindingInfo binding)
        {
            string retTypeName = NameUtil.GetTypeName(binding.retType);
            
            writer.WriteLine($"static void {binding.InstanceGetterMethodName}(lua_State* L, void* target, {ConstStrings.typeMethodInfo} method, {ConstStrings.typeConstPropertyMarshalCtxPtr} ctx)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"typedef {retTypeName} (*FnGetter)(void*, {ConstStrings.typeMethodInfo});");
            writer.WriteLine($"FnGetter fnGetter = (FnGetter)method->{ConstStrings.nameMethodPointer};");
            writer.WriteLine($"{retTypeName} result = fnGetter(target, method);");
            string marshalMethodName = MetaUtil.IsNullable(binding.retType) ? "PushNullableValue" : "PushValue";
            writer.WriteLine($"StructMarshal::{marshalMethodName}(L, &result, ctx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->meta));");
            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private void GenerateInstanceSetterFunction(CodeWriter writer, PropertyBindingInfo binding)
        {
            string retTypeName = NameUtil.GetTypeName(binding.retType);
            writer.WriteLine($"static void {binding.InstanceSetterMethodName}(lua_State* L, void* target, int valueIdx, {ConstStrings.typeMethodInfo} method, {ConstStrings.typeConstPropertyMarshalCtxPtr} ctx)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"typedef void (*FnSetter)(void*, {retTypeName}, {ConstStrings.typeMethodInfo});");
            writer.WriteLine($"{retTypeName} value;");
            string marshalMethodName = MetaUtil.IsNullable(binding.retType) ? "PopNullableValue" : "PopValue";
            writer.WriteLine($"StructMarshal::{marshalMethodName}(L, valueIdx, ctx->valueTypeKlass, &value);");
            writer.WriteLine($"FnSetter fnSetter = (FnSetter)method->{ConstStrings.nameMethodPointer};");
            writer.WriteLine($"fnSetter(target, value, method);");
            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private void GeneratePropertyBridgeFunctions(CodeWriter writer, PropertyBindingInfo binding)
        {
            GenerateStaticGetterFunction(writer, binding);
            GenerateStaticSetterFunction(writer, binding);
            GenerateInstanceGetterFunction(writer, binding);
            GenerateInstanceSetterFunction(writer, binding);
        }

        private void GeneratePropertyBridgeStub(CodeWriter writer, List<PropertyBindingInfo> bindings)
        {
            writer.WriteLine($"const {ConstStrings.namePropertyBridgeEntry} g_propertyBridges[] = {{");
            writer.IncreaseIndent();
            foreach (var binding in bindings)
            {
                writer.WriteLine($"{{ \"{binding.stubName}\", {binding.StaticGetterMethodName}, {binding.StaticSetterMethodName}, {binding.InstanceGetterMethodName}, {binding.InstanceSetterMethodName}, }},");
            }
            writer.WriteLine($"{{ nullptr, nullptr, nullptr, nullptr, nullptr }},");
            writer.DecreaseIndent();
            writer.WriteLine($"}};");
        }
    }
}
