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

namespace ZLua.Utils
{
    /// <summary>
    /// Nested C#→Lua pcall errors cannot <c>throw</c> on Unity Mono (stack capture during
    /// an outer <c>lua_pcall</c> SIGSEGVs). Stash the message for the Lua C callback Wrap to
    /// convert via <c>lua_error</c>.
    /// </summary>
    internal static class NestedLuaCallPendingError
    {
        [System.ThreadStatic]
        private static string s_message;

        internal static void Set(string message)
        {
            s_message = string.IsNullOrEmpty(message)
                ? "lua pcall failed (empty error object)"
                : message;
        }

        internal static bool HasPending => s_message != null;

        internal static bool TryTake(out string message)
        {
            message = s_message;
            s_message = null;
            return message != null;
        }
    }
}
