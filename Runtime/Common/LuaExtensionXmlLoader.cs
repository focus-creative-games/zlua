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

using System;
using System.Collections.Generic;
using System.Xml;

namespace ZLua
{
    /// <summary>
    /// Loads and merges LuaExtension XML files (spec 13-EXTENSION-METHODS §2.2).
    /// Duplicate (target, extension-class) keys are a hard failure.
    /// </summary>
    public static class LuaExtensionXmlLoader
    {
        public static List<LuaExtensionXmlRule> LoadFromConfiguredPaths(
            IEnumerable<string> configuredPaths,
            string projectRoot)
        {
            List<string> files;
            try
            {
                files = LuaMarshalAsXmlLoader.ExpandToXmlFiles(configuredPaths, projectRoot, "LuaExtension");
            }
            catch (LuaMarshalAsConfigurationException ex)
            {
                throw new LuaExtensionConfigurationException(ex.Message, ex);
            }

            var rules = new List<LuaExtensionXmlRule>();
            var seen = new Dictionary<string, string>(StringComparer.Ordinal);
            foreach (string file in files)
            {
                LoadFile(file, rules, seen);
            }

            return rules;
        }

        private static void LoadFile(string filePath, List<LuaExtensionXmlRule> rules, Dictionary<string, string> seen)
        {
            var doc = new XmlDocument();
            try
            {
                doc.Load(filePath);
            }
            catch (Exception ex)
            {
                throw new LuaExtensionConfigurationException(
                    "[ZLua] Failed to parse LuaExtension XML '" + filePath + "': " + ex.Message, ex);
            }

            XmlElement root = doc.DocumentElement;
            if (root == null || !string.Equals(root.Name, "ZLuaExtensions", StringComparison.Ordinal))
            {
                throw new LuaExtensionConfigurationException(
                    "[ZLua] LuaExtension XML root must be <ZLuaExtensions>: " + filePath);
            }

            string version = root.GetAttribute("version");
            if (!string.Equals(version, "1", StringComparison.Ordinal))
            {
                throw new LuaExtensionConfigurationException(
                    "[ZLua] Unsupported LuaExtension XML version '" + version + "' in " + filePath);
            }

            foreach (XmlNode child in root.ChildNodes)
            {
                if (!(child is XmlElement assemblyEl))
                {
                    continue;
                }

                if (!string.Equals(assemblyEl.Name, "Assembly", StringComparison.Ordinal))
                {
                    throw new LuaExtensionConfigurationException(
                        "[ZLua] Unexpected element <" + assemblyEl.Name + "> under ZLuaExtensions in " + filePath);
                }

                string assemblyName = RequireAttr(assemblyEl, "name", filePath);
                foreach (XmlNode typeNode in assemblyEl.ChildNodes)
                {
                    if (!(typeNode is XmlElement typeEl))
                    {
                        continue;
                    }

                    if (!string.Equals(typeEl.Name, "Type", StringComparison.Ordinal))
                    {
                        throw new LuaExtensionConfigurationException(
                            "[ZLua] Unexpected element <" + typeEl.Name + "> under Assembly in " + filePath);
                    }

                    ParseType(filePath, assemblyName, typeEl, rules, seen);
                }
            }
        }

        private static void ParseType(
            string filePath,
            string assemblyName,
            XmlElement typeEl,
            List<LuaExtensionXmlRule> rules,
            Dictionary<string, string> seen)
        {
            string typeFullName = RequireAttr(typeEl, "fullName", filePath);
            foreach (XmlNode child in typeEl.ChildNodes)
            {
                if (!(child is XmlElement el))
                {
                    continue;
                }

                if (!string.Equals(el.Name, "Extension", StringComparison.Ordinal))
                {
                    throw new LuaExtensionConfigurationException(
                        "[ZLua] Unexpected element <" + el.Name + "> under Type in LuaExtension XML " + filePath
                        + "; only <Extension assembly=\"...\" fullName=\"...\"> is allowed.");
                }

                string extensionAssembly = RequireAttr(el, "assembly", filePath);
                string extensionFullName = RequireAttr(el, "fullName", filePath);

                foreach (XmlNode nested in el.ChildNodes)
                {
                    if (nested is XmlElement nestedEl)
                    {
                        throw new LuaExtensionConfigurationException(
                            "[ZLua] LuaExtension Extension must not contain child elements (found <"
                            + nestedEl.Name + ">) in " + filePath);
                    }
                }

                var rule = new LuaExtensionXmlRule
                {
                    SourcePath = filePath,
                    TargetAssemblyName = assemblyName,
                    TargetTypeFullName = typeFullName,
                    ExtensionAssemblyName = extensionAssembly,
                    ExtensionTypeFullName = extensionFullName,
                };

                string key = rule.DuplicateKey;
                if (seen.TryGetValue(key, out string previousPath))
                {
                    throw new LuaExtensionConfigurationException(
                        "[ZLua] Duplicate LuaExtension XML rule for " + key
                        + "\n  first: " + previousPath
                        + "\n  again: " + filePath);
                }

                seen[key] = filePath;
                rules.Add(rule);
            }
        }

        private static string RequireAttr(XmlElement el, string attrName, string filePath)
        {
            if (!el.HasAttribute(attrName) || string.IsNullOrWhiteSpace(el.GetAttribute(attrName)))
            {
                throw new LuaExtensionConfigurationException(
                    "[ZLua] Missing or empty @" + attrName + " on <" + el.Name + "> in " + filePath);
            }

            return el.GetAttribute(attrName).Trim();
        }
    }
}
