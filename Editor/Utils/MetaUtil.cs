using dnlib.DotNet;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;

namespace ZLua.Meta
{
public static class MetaUtil
{

    public static bool EqualsTypeSig(TypeSig a, TypeSig b)
    {
        if (a == b)
        {
            return true;
        }
        if (a != null && b != null)
        {
            return TypeEqualityComparer.Instance.Equals(a, b);
        }
        return false;
    }

    public static bool EqualsTypeSigArray(List<TypeSig> a, List<TypeSig> b)
    {
        if (a == b)
        {
            return true;
        }
        if (a != null && b != null)
        {
            if (a.Count != b.Count)
            {
                return false;
            }
            for (int i = 0; i < a.Count; i++)
            {
                if (!TypeEqualityComparer.Instance.Equals(a[i], b[i]))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    public static TypeSig Inflate(TypeSig sig, GenericArgumentContext ctx)
    {
        if (!sig.ContainsGenericParameter)
        {
            return sig;
        }
        return ctx.Resolve(sig);
    }

    public static TypeSig ToShareTypeSig(ICorLibTypes corTypes, TypeSig typeSig)
    {
        var a = typeSig.RemovePinnedAndModifiers();
        switch (a.ElementType)
        {
        case ElementType.Void:
            return corTypes.Void;
        case ElementType.Boolean:
            return corTypes.Boolean;
        case ElementType.Char:
            return corTypes.UInt16;
        case ElementType.I1:
            return corTypes.SByte;
        case ElementType.U1:
            return corTypes.Byte;
        case ElementType.I2:
            return corTypes.Int16;
        case ElementType.U2:
            return corTypes.UInt16;
        case ElementType.I4:
            return corTypes.Int32;
        case ElementType.U4:
            return corTypes.UInt32;
        case ElementType.I8:
            return corTypes.Int64;
        case ElementType.U8:
            return corTypes.UInt64;
        case ElementType.R4:
            return corTypes.Single;
        case ElementType.R8:
            return corTypes.Double;
        case ElementType.String:
            return corTypes.Object;
        case ElementType.TypedByRef:
            return corTypes.TypedReference;
        case ElementType.I:
            return corTypes.IntPtr;
        case ElementType.U:
            return corTypes.UIntPtr;
        case ElementType.Object:
            return corTypes.Object;
        case ElementType.Sentinel:
            return typeSig;
        case ElementType.Ptr:
            return corTypes.UIntPtr;
        case ElementType.ByRef:
            return corTypes.UIntPtr;
        case ElementType.SZArray:
            return corTypes.Object;
        case ElementType.Array:
            return corTypes.Object;
        case ElementType.ValueType: {
            TypeDef typeDef = a.ToTypeDefOrRef().ResolveTypeDef();
            if (typeDef == null)
            {
                throw new Exception($"type:{a} definition could not be found");
            }
            if (typeDef.IsEnum)
            {
                return ToShareTypeSig(corTypes, typeDef.GetEnumUnderlyingType());
            }
            return typeSig;
        }
        case ElementType.Var:
        case ElementType.MVar:
        case ElementType.Class:
            return corTypes.Object;
        case ElementType.GenericInst: {
            var gia = (GenericInstSig)a;
            TypeDef typeDef = gia.GenericType.ToTypeDefOrRef().ResolveTypeDef();
            if (typeDef == null)
            {
                throw new Exception($"type:{a} definition could not be found");
            }
            if (typeDef.IsEnum)
            {
                return ToShareTypeSig(corTypes, typeDef.GetEnumUnderlyingType());
            }
            if (!typeDef.IsValueType)
            {
                return corTypes.Object;
            }
            return new GenericInstSig(gia.GenericType,
                                      gia.GenericArguments.Select(ga => ToShareTypeSig(corTypes, ga)).ToList());
        }
        case ElementType.FnPtr:
            return corTypes.UIntPtr;
        case ElementType.ValueArray:
            return typeSig;
        case ElementType.Module:
            return typeSig;
        default:
            throw new NotSupportedException(typeSig.ToString());
        }
    }

    public static List<TypeSig> ToShareTypeSigs(ICorLibTypes corTypes, IList<TypeSig> typeSigs)
    {
        if (typeSigs == null)
        {
            return null;
        }
        return typeSigs.Select(s => ToShareTypeSig(corTypes, s)).ToList();
    }

    public static IAssemblyResolver CreateAOTAssemblyResolver(BuildTarget target)
    {
        return new PathAssemblyResolver(CommonDirs.GetManagedStrippedDuplicatePath(target));
    }

    public static List<TypeSig> CreateDefaultGenericParams(ModuleDef module, int genericParamCount)
    {
        var methodGenericParams = new List<TypeSig>();
        for (int i = 0; i < genericParamCount; i++)
        {
            methodGenericParams.Add(module.CorLibTypes.Object);
        }
        return methodGenericParams;
    }

    public static bool IsVoid(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.Void;
    }

    public static bool IsString(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.String;
    }

    public static bool IsSZArray(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.SZArray;
    }

    public static bool IsMdArray(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.Array;
    }

    public static bool IsSZOrMdArray(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.SZArray || t.ElementType == ElementType.Array;
    }

    public static bool IsByteArray(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.SZArray && t.Next.ElementType == ElementType.U1;
    }

    public static bool IsStruct(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        if (t.IsByRef)
        {
            return false;
        }
        switch (t.ElementType)
        {
        case ElementType.ValueType: {
            TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();
            if (typeDef.IsEnum)
            {
                return false;
            }
            return true;
        }
        case ElementType.GenericInst: {
            var gis = t.ToGenericInstSig();
            return IsStruct(gis.GenericType);
        }
        default:
            return false;
        }
    }

    public static bool IsNullable(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        if (t.ElementType != ElementType.GenericInst)
        {
            return false;
        }
        var gis = t.ToGenericInstSig();
        return gis.GenericType.FullName == "System.Nullable`1";
    }

    public static bool IsReferenceType(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        switch (t.ElementType)
        {
        case ElementType.Object:
        case ElementType.Class:
        case ElementType.String:
        case ElementType.Array:
        case ElementType.SZArray:
            return true;
        case ElementType.GenericInst:
            return !t.IsValueType;
        default:
            return false;
        }
    }

    private static bool IsSubTypeOf(TypeDef typeDef, string typeName)
    {
        for (TypeDef t = typeDef.BaseType?.ResolveTypeDef(); t != null; t = t.BaseType?.ResolveTypeDef())
        {
            if (t.FullName == typeName)
            {
                return true;
            }
        }
        return false;
    }

    public static bool IsSubOfMulticastDelegate(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        switch (t.ElementType)
        {
        case ElementType.Class:
        {
            TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();
            return IsSubTypeOf(typeDef, "System.MulticastDelegate");
        }
        case ElementType.GenericInst:
        {
            var gis = t.ToGenericInstSig();
            return IsSubTypeOf(gis.GenericType.ToTypeDefOrRef().ResolveTypeDef(), "System.MulticastDelegate");
        }
        default:
            return false;
        }
    }

    public static bool IsTypedReference(TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        return t.ElementType == ElementType.TypedByRef;
    }

    public static TypeSig ToSharedTypeSig(ICorLibTypes corTypes, TypeSig typeSig)
    {
        TypeSig t = typeSig.RemovePinnedAndModifiers();
        if (t.IsByRef)
        {
            return corTypes.IntPtr;
        }
        switch (t.ElementType)
        {
        case ElementType.Void:
        case ElementType.Boolean:
        case ElementType.Char:
        case ElementType.I1:
        case ElementType.U1:
        case ElementType.I2:
        case ElementType.U2:
        case ElementType.I4:
        case ElementType.U4:
        case ElementType.I8:
        case ElementType.U8:
        case ElementType.R4:
        case ElementType.R8:
        case ElementType.I:
        case ElementType.U:
        case ElementType.Object:
            return t;
        case ElementType.String:
        case ElementType.SZArray:
        case ElementType.Array:
        case ElementType.Class:
            return corTypes.Object;
        case ElementType.ValueType: {
            TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDefThrow();
            if (typeDef.IsEnum)
            {
                return typeDef.GetEnumUnderlyingType();
            }
            return t;
        }
        case ElementType.GenericInst: {
            var gis = t.ToGenericInstSig();
            TypeDef genericTypeDef = gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
            if (genericTypeDef.IsEnum)
            {
                return genericTypeDef.GetEnumUnderlyingType();
            }
            if (!genericTypeDef.IsValueType)
            {
                return corTypes.Object;
            }
            return new GenericInstSig(gis.GenericType,
                                      gis.GenericArguments.Select(ga => ToSharedTypeSig(corTypes, ga)).ToList());
        }
        case ElementType.Ptr:
        case ElementType.FnPtr:
            return corTypes.IntPtr;
        case ElementType.TypedByRef:
            return corTypes.TypedReference;
        default:
            throw new NotSupportedException(t.ToString());
        }
    }
}
}
