using System;
using System.Reflection;
using System.Text;

namespace ZLua
{
    internal static partial class PointerMarshaling
    {
        internal static bool MethodSignatureContainsUnmanagedPointer(MethodBase method)
        {
            if (method == null)
            {
                return false;
            }

            if (method is MethodInfo methodInfo && ContainsUnmanagedPointerMarker(DescribeType(methodInfo.ReturnType)))
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (ContainsUnmanagedPointerMarker(DescribeType(parameters[i].ParameterType)))
                {
                    return true;
                }
            }

            return ContainsUnmanagedPointerMarker(method.ToString());
        }

        internal static bool MethodSignatureMightUseReflectionUnsafeInvoke(MethodBase method)
        {
            if (method == null)
            {
                return false;
            }

            if (method is MethodInfo methodInfo && ContainsPointerMarker(DescribeType(methodInfo.ReturnType)))
            {
                return true;
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (ContainsPointerMarker(DescribeType(parameters[i].ParameterType)))
                {
                    return true;
                }
            }

            string methodDescription = method.ToString();
            return ContainsPointerMarker(methodDescription);
        }

        private static string DescribeType(Type type)
        {
            if (type == null)
            {
                return string.Empty;
            }

            var sb = new StringBuilder();
            AppendTypeDescription(sb, type);
            return sb.ToString();
        }

        private static void AppendTypeDescription(StringBuilder sb, Type type)
        {
            if (type == null)
            {
                return;
            }

            if (!string.IsNullOrEmpty(type.FullName))
            {
                sb.Append(type.FullName);
            }

            if (!string.IsNullOrEmpty(type.Name))
            {
                if (sb.Length > 0)
                {
                    sb.Append('|');
                }

                sb.Append(type.Name);
            }

            string text = type.ToString();
            if (!string.IsNullOrEmpty(text))
            {
                if (sb.Length > 0)
                {
                    sb.Append('|');
                }

                sb.Append(text);
            }
        }

        private static bool ContainsPointerMarker(string text)
        {
            if (string.IsNullOrEmpty(text))
            {
                return false;
            }

            if (text.IndexOf("delegate*", StringComparison.OrdinalIgnoreCase) >= 0)
            {
                return true;
            }

            if (text.IndexOf("TypedReference", StringComparison.Ordinal) >= 0)
            {
                return true;
            }

            if (text.IndexOf("System.ReadOnlySpan`", StringComparison.Ordinal) >= 0
                || text.IndexOf("System.Span`", StringComparison.Ordinal) >= 0)
            {
                return true;
            }

            if (text.IndexOf("System.Decimal", StringComparison.Ordinal) >= 0)
            {
                return false;
            }

            return text.IndexOf('*') >= 0;
        }

        private static bool ContainsUnmanagedPointerMarker(string text)
        {
            if (string.IsNullOrEmpty(text))
            {
                return false;
            }

            if (text.IndexOf("delegate*", StringComparison.OrdinalIgnoreCase) >= 0)
            {
                return true;
            }

            return text.IndexOf('*') >= 0;
        }
    }
}
