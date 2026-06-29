using System;
using System.Collections.Concurrent;
using System.Linq.Expressions;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Compiles Lua → C# constructor bridges for default-marshal public ctors (P0).
    /// </summary>
    internal static class LuaToCSharpConstructorBridgeFactory
    {
        private readonly struct FactoryKey : IEquatable<FactoryKey>
        {
            internal readonly ConstructorInfo Constructor;
            internal readonly int ArgStartIndex;

            internal FactoryKey(ConstructorInfo constructor, int argStartIndex)
            {
                Constructor = constructor;
                ArgStartIndex = argStartIndex;
            }

            public bool Equals(FactoryKey other)
            {
                return Constructor == other.Constructor && ArgStartIndex == other.ArgStartIndex;
            }

            public override bool Equals(object obj)
            {
                return obj is FactoryKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    return ((Constructor?.MetadataToken ?? 0) * 397) ^ ArgStartIndex;
                }
            }
        }

        private static readonly ConcurrentDictionary<FactoryKey, LuaCSFunction> Cache =
            new ConcurrentDictionary<FactoryKey, LuaCSFunction>();

        private static readonly MethodInfo ValidateExactArgCountMethod = typeof(LuaToCSharpBridgeMarshaling).GetMethod(
            nameof(LuaToCSharpBridgeMarshaling.ValidateExactArgCount),
            BindingFlags.NonPublic | BindingFlags.Static,
            binder: null,
            types: new[] { typeof(IntPtr), typeof(int), typeof(int), typeof(ConstructorInfo) },
            modifiers: null);

        private static readonly MethodInfo PushConstructorInstanceMethod = typeof(LuaManagerObject).GetMethod(
            nameof(LuaManagerObject.PushConstructorInstance),
            BindingFlags.NonPublic | BindingFlags.Static,
            binder: null,
            types: new[] { typeof(IntPtr), typeof(object), typeof(Type) },
            modifiers: null);

        private static readonly MethodInfo ToLuaErrorMethod = typeof(LuaCallbackBoundary).GetMethod(
            nameof(LuaCallbackBoundary.ToLuaError),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo EnterLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.EnterLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        private static readonly MethodInfo LeaveLuaToCSharpMethod = typeof(StructOpaqueScope).GetMethod(
            nameof(StructOpaqueScope.LeaveLuaToCSharp),
            BindingFlags.NonPublic | BindingFlags.Static);

        internal static bool TryCreate(ConstructorInfo ctor, int argStartIndex, out LuaCSFunction invoker)
        {
            invoker = null;
            if (!CanFastInvoke(ctor) || !EnsureHelperMethods())
            {
                return false;
            }

            try
            {
                invoker = Cache.GetOrAdd(
                    new FactoryKey(ctor, argStartIndex),
                    key => Compile(key.Constructor, key.ArgStartIndex));
                return invoker != null;
            }
            catch
            {
                return false;
            }
        }

        internal static bool CanFastInvoke(ConstructorInfo ctor)
        {
            if (ctor == null || ctor.DeclaringType == null || ctor.DeclaringType.ContainsGenericParameters)
            {
                return false;
            }

            ParameterInfo[] parameters = ctor.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                ParameterInfo parameter = parameters[i];
                if (parameter.ParameterType.IsByRef
                    || parameter.IsDefined(typeof(ParamArrayAttribute), inherit: false)
                    || parameter.HasDefaultValue)
                {
                    return false;
                }

                if (!LuaToCSharpMethodBridgeFactory.CanFastType(parameter.ParameterType))
                {
                    return false;
                }
            }

            return LuaToCSharpMethodBridgeFactory.CanFastType(ctor.DeclaringType);
        }

        private static bool EnsureHelperMethods()
        {
            return ValidateExactArgCountMethod != null
                && PushConstructorInstanceMethod != null
                && ToLuaErrorMethod != null
                && EnterLuaToCSharpMethod != null
                && LeaveLuaToCSharpMethod != null;
        }

        private static LuaCSFunction Compile(ConstructorInfo ctor, int argStartIndex)
        {
            ParameterExpression luaStateParam = Expression.Parameter(typeof(IntPtr), "luaState");
            ParameterExpression exceptionParam = Expression.Parameter(typeof(Exception), "ex");
            ParameterInfo[] parameters = ctor.GetParameters();
            Type ownerType = ctor.DeclaringType;

            Expression validateArgs = Expression.Call(
                ValidateExactArgCountMethod,
                luaStateParam,
                Expression.Constant(argStartIndex),
                Expression.Constant(parameters.Length),
                Expression.Constant(ctor));

            Expression[] callArguments = new Expression[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                callArguments[i] = LuaToCSharpBridgeExpressionBuilder.BuildPopArgument(
                    luaStateParam,
                    argStartIndex + i,
                    parameters[i].ParameterType);
            }

            Expression newExpression = Expression.New(ctor, callArguments);

            Expression pushInstance = Expression.Call(
                PushConstructorInstanceMethod,
                luaStateParam,
                Expression.Convert(newExpression, typeof(object)),
                Expression.Constant(ownerType));

            Expression setReturnCount = Expression.Block(pushInstance, Expression.Constant(1));

            Expression enterScope = Expression.Call(EnterLuaToCSharpMethod);
            Expression leaveScope = Expression.Call(LeaveLuaToCSharpMethod);

            Expression tryBody = Expression.Block(
                validateArgs,
                Expression.TryFinally(
                    Expression.Block(enterScope, setReturnCount),
                    leaveScope));

            Expression tryCatch = Expression.TryCatch(
                tryBody,
                Expression.Catch(exceptionParam, Expression.Call(ToLuaErrorMethod, luaStateParam, exceptionParam)));

            return Expression.Lambda<LuaCSFunction>(tryCatch, luaStateParam).Compile();
        }
    }
}
