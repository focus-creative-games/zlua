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

#if UNITY_EDITOR
using System;
using UnityEditor.Build;
using UnityEditor.Build.Reporting;

namespace ZLua.BuildProcessors
{

    public class MsvcStdextWorkaround : IPreprocessBuildWithReport
    {
        const string kWorkaroundFlag = "/D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS";

        public int callbackOrder => 0;

        public void OnPreprocessBuild(BuildReport report)
        {
            var clEnv = Environment.GetEnvironmentVariable("_CL_");

            if (string.IsNullOrEmpty(clEnv))
            {
                Environment.SetEnvironmentVariable("_CL_", kWorkaroundFlag);
            }
            else if (!clEnv.Contains(kWorkaroundFlag))
            {
                clEnv += " " + kWorkaroundFlag;
                Environment.SetEnvironmentVariable("_CL_", clEnv);
            }
        }
    }
}

#endif // UNITY_EDITOR
