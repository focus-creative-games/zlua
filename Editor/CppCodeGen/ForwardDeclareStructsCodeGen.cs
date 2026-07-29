// Copyright 2026 Code Philosophy

using dnlib.DotNet;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{
    public class ForwardDeclareStructsCodeGen
    {
        private class ReducedFieldInfo
        {
            public FieldDef field;

            public TypeSig type;
        }

        private class ReducedTypeInfo
        {
            public string cppFullName;
            public TypeSig type;
            public List<ReducedFieldInfo> fields;
            public uint originalPackingSize;
            public uint packingSize;
            public uint classSize;
            public LayoutKind layout;
            public bool blittable;
        }

        private readonly HashSet<TypeSig> _structTypes = new HashSet<TypeSig>();

        private readonly Dictionary<TypeSig, ReducedTypeInfo> _reducedTypeInfos = new Dictionary<TypeSig, ReducedTypeInfo>(TypeEqualityComparer.Instance);

        public void CollectStructDefs(List<MethodDesc> methods)
        {

            foreach (var method in methods)
            {
                CollectStructDefs(method);
            }
        }

        public void CollectStructDefs(MethodDesc method)
        {
            foreach (var param in method.ParamInfos)
            {
                if (MetaUtil.IsStruct(param.type))
                {
                    _structTypes.Add(param.type);
                }
            }
            if (MetaUtil.IsStruct(method.ReturnInfo.type))
            {
                _structTypes.Add(method.ReturnInfo.type);
            }
        }

        public void CollectStructDefs(MethodDef method)
        {
            MethodDesc methodDesc = MethodDesc.CreateMethodDesc(method, null, null, true);
            CollectStructDefs(methodDesc);
        }

        public void CollectStructDefs(List<MethodDef> methods)
        {
            foreach (var method in methods)
            {
                CollectStructDefs(method);
            }
        }

        public void CollectStructDefs(MethodDef method, List<TypeSig> klassInst, List<TypeSig> methodInst)
        {
            MethodDesc methodDesc = MethodDesc.CreateMethodDesc(method, klassInst, methodInst, true);
            CollectStructDefs(methodDesc);
        }

        public void CollectStructDefs(PropertyDef property)
        {
            TypeSig retType = property.PropertySig.RetType;
            if (MetaUtil.IsStruct(retType))
            {
                _structTypes.Add(retType);
            }
        }

        public void CollectStructType(TypeSig type)
        {
            if (MetaUtil.IsStruct(type))
            {
                _structTypes.Add(type);
            }
        }

        /// <summary>
        /// Maps a CLR instance field name to the ForwardDeclareStructs cpp field name (__N).
        /// </summary>
        public bool TryGetCppFieldName(TypeSig structType, string clrFieldName, out string cppFieldName)
        {
            cppFieldName = null;
            if (structType == null || string.IsNullOrEmpty(clrFieldName))
            {
                return false;
            }

            ReducedTypeInfo info = GetReducedTypeInfo(structType);
            for (int i = 0; i < info.fields.Count; i++)
            {
                if (info.fields[i].field.Name == clrFieldName)
                {
                    cppFieldName = "__" + i;
                    return true;
                }
            }

            return false;
        }

        ReducedTypeInfo GetReducedTypeInfo(TypeSig type)
        {
            if (_reducedTypeInfos.TryGetValue(type, out var reducedTypeInfo))
            {
                return reducedTypeInfo;
            }
            var analyzeTypeInfo = CalculateReducedTypeInfo(type);
            return analyzeTypeInfo;
        }


        private void WriteForwardDeclareStruct(CodeWriter writer, ReducedTypeInfo ci, HashSet<string> writtenTypes)
        {
            string typeName = NameUtil.GetTypeName(ci.type);
            if (!writtenTypes.Add(typeName))
            {
                return;
            }

            // write fields first
            foreach (var field in ci.fields)
            {
                if (MetaUtil.IsStruct(field.type))
                {
                    WriteForwardDeclareStruct(writer, GetReducedTypeInfo(field.type), writtenTypes);
                }
            }

            writer.WriteLine($"// {typeName} {ci.type.FullName}");
            uint packingSize = ci.packingSize;
            uint classSize = ci.classSize;

            if (ci.layout == LayoutKind.Explicit)
            {
                writer.WriteLine($"struct {typeName} {{");
                writer.WriteLine("\tunion {");
                if (classSize > 0)
                {
                    writer.WriteLine($"\tstruct {{ char __fieldSize_offsetPadding[{classSize}];}};");
                }
                int index = 0;
                foreach (var field in ci.fields)
                {
                    uint offset = field.field.FieldOffset.Value;
                    string fieldName = $"__{index}";
                    string fieldTypeName = NameUtil.GetTypeName(field.type);
                    string commentFieldName = $"{field.field.Name}";
                    writer.WriteLine("\t#pragma pack(push, 1)");
                    writer.WriteLine($"\tstruct {{ {(offset > 0 ? $"char {fieldName}_offsetPadding[{offset}]; " : "")}{fieldTypeName} {fieldName};}}; // {commentFieldName}");
                    writer.WriteLine($"\t#pragma pack(pop)");
                    if (packingSize > 0)
                    {
                        writer.WriteLine($"\t#pragma pack(push, {packingSize})");
                    }
                    writer.WriteLine($"\tstruct {{ {(offset > 0 ? $"char {fieldName}_offsetPadding_forAlignmentOnly[{offset}]; " : "")}{fieldTypeName} {fieldName}_forAlignmentOnly;}}; // {commentFieldName}");
                    if (packingSize > 0)
                    {
                        writer.WriteLine($"\t#pragma pack(pop)");
                    }
                    ++index;
                }
                writer.WriteLine("\t};");
                writer.WriteLine("};");
            }
            else
            {
                if (packingSize != 0)
                {
                    writer.WriteLine($"#pragma pack(push, {packingSize})");
                }
                writer.WriteLine($"{(classSize > 0 ? "union" : "struct")} {typeName} {{");
                if (classSize > 0)
                {
                    writer.WriteLine($"\tstruct {{ char __fieldSize_offsetPadding[{classSize}];}};");
                    writer.WriteLine("\tstruct {");
                }
                int index = 0;
                foreach (var field in ci.fields)
                {
                    string fieldName = $"__{index}";
                    string fieldTypeName = NameUtil.GetTypeName(field.type);
                    string commentFieldName = $"{field.field.Name}";
                    writer.WriteLine($"\t{fieldTypeName} {fieldName}; // {commentFieldName}");
                    ++index;
                }
                if (classSize > 0)
                {
                    writer.WriteLine("\t};");
                }
                writer.WriteLine("};");
                if (packingSize != 0)
                {
                    writer.WriteLine($"#pragma pack(pop)");
                }
            }
        }

        public void WriteForwardDeclareStructs(CodeWriter writer)
        {
            var structTypeAndNames = _structTypes.Select(t => (t, NameUtil.GetTypeName(t))).ToList();
            structTypeAndNames.Sort((a, b) => a.Item2.CompareTo(b.Item2));

            var sortedStructTypes = structTypeAndNames.Select(t => t.Item1).ToList();
            var writtenTypes = new HashSet<string>();

            foreach (var type in sortedStructTypes)
            {
                ReducedTypeInfo ci = GetReducedTypeInfo(type);
                WriteForwardDeclareStruct(writer, ci, writtenTypes);
            }
        }

        private bool IsBlittable(TypeSig typeSig)
        {
            typeSig = typeSig.RemovePinnedAndModifiers();
            if (typeSig.IsByRef)
            {
                return true;
            }
            switch (typeSig.ElementType)
            {
            case ElementType.Void: return false;
            case ElementType.Boolean:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.Char:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
            case ElementType.I8:
            case ElementType.U8:
            case ElementType.R4:
            case ElementType.R8:
            case ElementType.I:
            case ElementType.U:
            case ElementType.Ptr:
            case ElementType.ByRef:
            case ElementType.FnPtr:
            case ElementType.TypedByRef: return true;
            case ElementType.String:
            case ElementType.Class:
            case ElementType.Array:
            case ElementType.SZArray:
            case ElementType.Object:
            case ElementType.Module:
            case ElementType.Var:
            case ElementType.MVar: return false;
            case ElementType.ValueType:
            {
                TypeDef typeDef = typeSig.ToTypeDefOrRef().ResolveTypeDef();
                if (typeDef == null)
                {
                    throw new Exception($"type:{typeSig} definition could not be found. Please try `ZLua/Genergate/LinkXml`, then Build once to generate the AOT dll, and then regenerate the bridge function");
                }
                if (typeDef.IsEnum)
                {
                    return true;
                }
                return CalculateReducedTypeInfo(typeSig).blittable;
            }
            case ElementType.GenericInst:
            {
                GenericInstSig gis = (GenericInstSig)typeSig;
                if (!gis.GenericType.IsValueType)
                {
                    return false;
                }
                TypeDef typeDef = gis.GenericType.ToTypeDefOrRef().ResolveTypeDef();
                if (typeDef.IsEnum)
                {
                    return true;
                }
                return CalculateReducedTypeInfo(typeSig).blittable;
            }
            default: throw new NotSupportedException($"{typeSig.ElementType}");
            }
        }

        private ReducedTypeInfo CalculateReducedTypeInfo(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            Debug.Assert(MetaUtil.IsStruct(t));
            if (_reducedTypeInfos.TryGetValue(t, out var ati))
            {
                return ati;
            }
            TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();

            List<TypeSig> klassInst = t.ToGenericInstSig()?.GenericArguments?.ToList();
            GenericArgumentContext ctx = klassInst != null ? new GenericArgumentContext(klassInst, null) : null;

            var fields = new List<ReducedFieldInfo>();

            bool blittable = true;
            foreach (FieldDef field in typeDef.Fields)
            {
                if (field.IsStatic)
                {
                    continue;
                }
                TypeSig fieldType = ctx != null ? MetaUtil.Inflate(field.FieldType, ctx) : field.FieldType;
                blittable &= IsBlittable(fieldType);
                TypeSig sharedFieldTypeInfo = MetaUtil.ToSharedTypeSig(typeDef.Module.CorLibTypes, fieldType);
                fields.Add(new ReducedFieldInfo { field = field, type = sharedFieldTypeInfo });
            }
            //analyzeTypeInfo.blittable = blittable;
            //analyzeTypeInfo.packingSize = blittable ? analyzeTypeInfo.originalPackingSize : 0;

            ClassLayout sa = typeDef.ClassLayout;
            uint originalPackingSize = sa?.PackingSize ?? 0;
            var analyzeTypeInfo = new ReducedTypeInfo()
            {
                cppFullName = NameUtil.GetTypeName(t),
                type = t,
                originalPackingSize = originalPackingSize,
                packingSize = blittable && !typeDef.IsAutoLayout ? originalPackingSize : 0,
                classSize = sa?.ClassSize ?? 0,
                layout = typeDef.IsAutoLayout ? LayoutKind.Auto : (typeDef.IsExplicitLayout ? LayoutKind.Explicit : LayoutKind.Sequential),
                fields = fields,
                blittable = blittable,
            };
            _reducedTypeInfos.Add(t, analyzeTypeInfo);
            return analyzeTypeInfo;
        }
    }
}
