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

using dnlib.DotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ZLua.Utils;

namespace ZLua.Meta
{
    public class ParamInfo
    {
        public TypeSig type;
        public ParamDef paramDef;
        public string name;
        public int indexExcludedThis;
    }

    public class ReturnInfo
    {
        public TypeSig type;
        public ParamDef paramDef;
    }

    public class MethodDesc : IEquatable<MethodDesc>
    {
        public static MethodDesc CreateMethodDesc(MethodDef methodDef, List<TypeSig> klassInst, List<TypeSig> methodInst, bool toSharedTypeSig)
        {
            ICorLibTypes corLibTypes = methodDef.Module.CorLibTypes;
            TypeSig returnType;
            List<TypeSig> parameters;
            if (klassInst == null && methodInst == null)
            {
                if (methodDef.HasGenericParameters)
                {
                    throw new Exception($"[PreservedMethod] method:{methodDef} has generic parameters");
                }
                returnType = toSharedTypeSig ? MetaUtil.ToShareTypeSig(corLibTypes, methodDef.ReturnType) : methodDef.ReturnType;
                parameters = methodDef.Parameters.Select(p => toSharedTypeSig ? MetaUtil.ToShareTypeSig(corLibTypes, p.Type) : p.Type).ToList();
            }
            else
            {
                var gc = new GenericArgumentContext(klassInst, methodInst);
                returnType = toSharedTypeSig ? MetaUtil.ToShareTypeSig(corLibTypes, MetaUtil.Inflate(methodDef.ReturnType, gc)) : MetaUtil.Inflate(methodDef.ReturnType, gc);
                parameters = methodDef.Parameters.Select(p => toSharedTypeSig ? MetaUtil.ToShareTypeSig(corLibTypes, MetaUtil.Inflate(p.Type, gc)) : MetaUtil.Inflate(p.Type, gc)).ToList();
            }


            var paramInfos = new List<ParamInfo>();
            int paramOffset = 0;
            if (!methodDef.IsStatic)
            {
                parameters.RemoveAt(0);
                paramOffset = 1;
            }
            if (returnType.ContainsGenericParameter)
            {
                throw new Exception($"[PreservedMethod] method:{methodDef} has generic parameters");
            }
            int index = 0;
            foreach (var paramInfo in parameters)
            {
                if (paramInfo.ContainsGenericParameter)
                {
                    throw new Exception($"[PreservedMethod] method:{methodDef} has generic parameters");
                }
                paramInfos.Add(new ParamInfo()
                {
                    type = toSharedTypeSig ? MetaUtil.ToSharedTypeSig(corLibTypes, paramInfo) : paramInfo,
                    indexExcludedThis = index,
                    name = $"__p{index}",
                    paramDef = methodDef.Parameters[index + paramOffset].ParamDef,
                });
                index++;
            }
            var mbs = new MethodDesc(methodDef,
                klassInst,
                methodInst,
                paramInfos,
                new ReturnInfo()
                {
                    type = returnType != null ? (toSharedTypeSig ? MetaUtil.ToSharedTypeSig(corLibTypes, returnType) : returnType) : corLibTypes.Void,
                    paramDef = methodDef.Parameters.ReturnParameter.ParamDef,
                }
            );
            return mbs;
        }

        public string Sig { get; }

        public MethodDef MethodDef { get; }
        public List<TypeSig> KlassInst { get; }

        public List<TypeSig> MethodInst { get; }

        public ReturnInfo ReturnInfo { get; }

        public List<ParamInfo> ParamInfos { get; }

        public MethodDesc(MethodDef methodDef, List<TypeSig> klassInst, List<TypeSig> methodInst, List<ParamInfo> paramInfos, ReturnInfo returnInfo)
        {
            MethodDef = methodDef;
            KlassInst = klassInst;
            MethodInst = methodInst;
            ParamInfos = paramInfos;
            ReturnInfo = returnInfo;
            Sig = BuildSig(methodDef, klassInst, methodInst);
        }

        private string BuildSig(MethodDef methodDef, List<TypeSig> klassInst, List<TypeSig> methodInst)
        {
            return NameUtil.CreateUniqueName(methodDef, KlassInst, MethodInst);
        }

        public override bool Equals(object obj)
        {
            return Equals((MethodDesc)obj);
        }

        public bool Equals(MethodDesc other)
        {
            return Sig == other.Sig;
        }

        public override int GetHashCode()
        {
            return Sig.GetHashCode();
        }
    }
}
