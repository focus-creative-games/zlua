using System;
using System.Reflection;

namespace ZLua.Emit
{
    internal sealed class MethodClosureTag
    {
        public MethodInfo Method;
        public Type OwnerType;
        public bool IsStatic;
        public bool IsByVal;
    }
}
