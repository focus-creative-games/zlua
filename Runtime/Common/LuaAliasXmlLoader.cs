using System;
using System.Collections.Generic;
using System.IO;
using System.Xml;

namespace ZLua
{
    /// <summary>
    /// Loads and merges LuaAlias XML files (spec 04-METHOD-OVERLOAD §5.4).
    /// Duplicate (assembly, type, method, signature) keys are a hard failure.
    /// </summary>
    public static class LuaAliasXmlLoader
    {
        public static List<LuaAliasXmlRule> LoadFromConfiguredPaths(IEnumerable<string> configuredPaths, string projectRoot)
        {
            List<string> files;
            try
            {
                files = LuaMarshalAsXmlLoader.ExpandToXmlFiles(configuredPaths, projectRoot, "LuaAlias");
            }
            catch (LuaMarshalAsConfigurationException ex)
            {
                throw new LuaAliasConfigurationException(ex.Message, ex);
            }

            var rules = new List<LuaAliasXmlRule>();
            var seen = new Dictionary<string, string>(StringComparer.Ordinal);
            foreach (string file in files)
            {
                LoadFile(file, rules, seen);
            }

            return rules;
        }

        private static void LoadFile(string filePath, List<LuaAliasXmlRule> rules, Dictionary<string, string> seen)
        {
            var doc = new XmlDocument();
            try
            {
                doc.Load(filePath);
            }
            catch (Exception ex)
            {
                throw new LuaAliasConfigurationException(
                    "[ZLua] Failed to parse LuaAlias XML '" + filePath + "': " + ex.Message, ex);
            }

            XmlElement root = doc.DocumentElement;
            if (root == null || !string.Equals(root.Name, "ZLuaAlias", StringComparison.Ordinal))
            {
                throw new LuaAliasConfigurationException(
                    "[ZLua] LuaAlias XML root must be <ZLuaAlias>: " + filePath);
            }

            string version = root.GetAttribute("version");
            if (!string.Equals(version, "1", StringComparison.Ordinal))
            {
                throw new LuaAliasConfigurationException(
                    "[ZLua] Unsupported LuaAlias XML version '" + version + "' in " + filePath);
            }

            foreach (XmlNode child in root.ChildNodes)
            {
                if (!(child is XmlElement assemblyEl))
                {
                    continue;
                }

                if (!string.Equals(assemblyEl.Name, "Assembly", StringComparison.Ordinal))
                {
                    throw new LuaAliasConfigurationException(
                        "[ZLua] Unexpected element <" + assemblyEl.Name + "> under ZLuaAlias in " + filePath);
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
                        throw new LuaAliasConfigurationException(
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
            List<LuaAliasXmlRule> rules,
            Dictionary<string, string> seen)
        {
            string typeFullName = RequireAttr(typeEl, "fullName", filePath);
            foreach (XmlNode child in typeEl.ChildNodes)
            {
                if (!(child is XmlElement el))
                {
                    continue;
                }

                if (!string.Equals(el.Name, "Method", StringComparison.Ordinal))
                {
                    throw new LuaAliasConfigurationException(
                        "[ZLua] Unexpected element <" + el.Name + "> under Type in LuaAlias XML " + filePath
                        + "; only <Method alias=\"...\"> is allowed.");
                }

                string methodName = RequireAttr(el, "name", filePath);
                string signature = RequireAttr(el, "signature", filePath);
                string alias = RequireAttr(el, "alias", filePath);
                if (string.IsNullOrWhiteSpace(alias))
                {
                    throw new LuaAliasConfigurationException(
                        "[ZLua] Method/@alias must be non-empty in " + filePath);
                }

                foreach (XmlNode nested in el.ChildNodes)
                {
                    if (nested is XmlElement nestedEl)
                    {
                        throw new LuaAliasConfigurationException(
                            "[ZLua] LuaAlias Method must not contain child elements (found <"
                            + nestedEl.Name + ">) in " + filePath);
                    }
                }

                var rule = new LuaAliasXmlRule
                {
                    SourcePath = filePath,
                    AssemblyName = assemblyName,
                    TypeFullName = typeFullName,
                    MethodName = methodName,
                    Signature = signature,
                    Alias = alias.Trim(),
                };

                string key = rule.DuplicateKey;
                if (seen.TryGetValue(key, out string previousPath))
                {
                    throw new LuaAliasConfigurationException(
                        "[ZLua] Duplicate LuaAlias XML rule for " + key
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
                throw new LuaAliasConfigurationException(
                    "[ZLua] Missing or empty @" + attrName + " on <" + el.Name + "> in " + filePath);
            }

            return el.GetAttribute(attrName).Trim();
        }
    }
}
