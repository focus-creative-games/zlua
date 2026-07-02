// Copyright 2026 Code Philosophy

using dnlib.DotNet;
using System;
using System.Collections.Generic;

namespace ZLua.CppCodeGen
{

    public sealed class ParamMarshalInfo
    {
        public int indexExcludedThis; // start from 0
        public string name;
        public LuaMarshalMetaInfo marshalMetaInfo;
    }

    public sealed class LuaInvokeBindingInfo
    {
        public MethodDef methodDef;
        public string luaModuleName;
        public string luaFunctionName;
        public string uniqueName;
        public string internalCallSignature;
        public List<ParamMarshalInfo> parameters;
        public LuaMarshalMetaInfo returnMetaInfo;

        public string SiteId => $"kSite_{uniqueName}";
        public string MethodMarshalCtxId => $"kMethodMarshalCtx_{uniqueName}";
        public string InitializedFlagId => $"kInitialized_{uniqueName}";
        public string IcFunctionName => $"IC_{uniqueName}";
        public string InternalCallName => internalCallSignature;


    }

}
