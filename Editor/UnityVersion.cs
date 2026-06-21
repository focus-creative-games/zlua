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


using System.Text.RegularExpressions;

namespace NovaLua
{
    public class UnityVersion
    {
        public readonly int major;
        public readonly int minor1;
        public readonly int minor2;
        public readonly bool isTuanjieEngine;

        public override string ToString()
        {
            return $"{major}.{minor1}.{minor2}";
        }

        private static readonly Regex s_unityVersionPat = new Regex(@"(\d+)\.(\d+)\.(\d+)");

        public UnityVersion(string versionStr)
        {
            var matches = s_unityVersionPat.Matches(versionStr);
            Match match = matches[matches.Count - 1];
            major = int.Parse(match.Groups[1].Value);
            minor1 = int.Parse(match.Groups[2].Value);
            minor2 = int.Parse(match.Groups[3].Value);
            isTuanjieEngine = versionStr.Contains("t");
        }
    }
}
