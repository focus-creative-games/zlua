using System;
using System.Collections.Generic;
using System.IO;
using System.Xml;

namespace ZLua
{
    /// <summary>
    /// Loads and merges MarshalAs XML files (spec marshal/02-MARSHAL-AS §9).
    /// Duplicate target keys are a hard failure.
    /// </summary>
    public static class LuaMarshalAsXmlLoader
    {
        public static List<LuaMarshalAsXmlRule> LoadFromConfiguredPaths(IEnumerable<string> configuredPaths, string projectRoot)
        {
            var files = ExpandToXmlFiles(configuredPaths, projectRoot);
            var rules = new List<LuaMarshalAsXmlRule>();
            var seen = new Dictionary<string, string>(StringComparer.Ordinal);
            foreach (string file in files)
            {
                LoadFile(file, rules, seen);
            }

            return rules;
        }

        public static List<string> ExpandToXmlFiles(IEnumerable<string> configuredPaths, string projectRoot)
        {
            return ExpandToXmlFiles(configuredPaths, projectRoot, "MarshalAs");
        }

        /// <summary>
        /// Shared path expansion for MarshalAs / LuaAlias XML Settings lists.
        /// </summary>
        public static List<string> ExpandToXmlFiles(IEnumerable<string> configuredPaths, string projectRoot, string pathLabel)
        {
            var files = new List<string>();
            if (configuredPaths == null)
            {
                return files;
            }

            var seenFiles = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (string raw in configuredPaths)
            {
                if (string.IsNullOrWhiteSpace(raw))
                {
                    continue;
                }

                string path = raw.Trim();
                if (!Path.IsPathRooted(path))
                {
                    path = Path.GetFullPath(Path.Combine(projectRoot ?? Directory.GetCurrentDirectory(), path));
                }
                else
                {
                    path = Path.GetFullPath(path);
                }

                if (File.Exists(path))
                {
                    if (!string.Equals(Path.GetExtension(path), ".xml", StringComparison.OrdinalIgnoreCase))
                    {
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] " + pathLabel + " XML path is not an .xml file: " + path);
                    }

                    if (seenFiles.Add(path))
                    {
                        files.Add(path);
                    }

                    continue;
                }

                if (Directory.Exists(path))
                {
                    foreach (string file in Directory.GetFiles(path, "*.xml", SearchOption.AllDirectories))
                    {
                        string full = Path.GetFullPath(file);
                        if (seenFiles.Add(full))
                        {
                            files.Add(full);
                        }
                    }

                    continue;
                }

                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] " + pathLabel + " XML path not found: " + path);
            }

            files.Sort(StringComparer.OrdinalIgnoreCase);
            return files;
        }

        private static void LoadFile(string filePath, List<LuaMarshalAsXmlRule> rules, Dictionary<string, string> seen)
        {
            var doc = new XmlDocument();
            try
            {
                doc.Load(filePath);
            }
            catch (Exception ex)
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Failed to parse MarshalAs XML '" + filePath + "': " + ex.Message, ex);
            }

            XmlElement root = doc.DocumentElement;
            if (root == null || !string.Equals(root.Name, "ZLuaMarshalAs", StringComparison.Ordinal))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] MarshalAs XML root must be <ZLuaMarshalAs>: " + filePath);
            }

            string version = root.GetAttribute("version");
            if (!string.Equals(version, "1", StringComparison.Ordinal))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Unsupported MarshalAs XML version '" + version + "' in " + filePath);
            }

            foreach (XmlNode child in root.ChildNodes)
            {
                if (!(child is XmlElement assemblyEl))
                {
                    continue;
                }

                if (!string.Equals(assemblyEl.Name, "Assembly", StringComparison.Ordinal))
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] Unexpected element <" + assemblyEl.Name + "> under ZLuaMarshalAs in " + filePath);
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
                        throw new LuaMarshalAsConfigurationException(
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
            List<LuaMarshalAsXmlRule> rules,
            Dictionary<string, string> seen)
        {
            string typeFullName = RequireAttr(typeEl, "fullName", filePath);
            foreach (XmlNode child in typeEl.ChildNodes)
            {
                if (!(child is XmlElement el))
                {
                    continue;
                }

                switch (el.Name)
                {
                    case "MarshalAs":
                        AddRule(rules, seen, BuildRule(
                            filePath, assemblyName, typeFullName, LuaMarshalAsXmlTargetKind.Type,
                            memberName: null, methodName: null, signature: null, paramIndex: -1, el));
                        break;
                    case "Field":
                        AddRule(rules, seen, BuildRule(
                            filePath, assemblyName, typeFullName, LuaMarshalAsXmlTargetKind.Field,
                            memberName: RequireAttr(el, "name", filePath), methodName: null, signature: null, paramIndex: -1,
                            RequireSingleMarshalAs(el, filePath)));
                        break;
                    case "Property":
                        AddRule(rules, seen, BuildRule(
                            filePath, assemblyName, typeFullName, LuaMarshalAsXmlTargetKind.Property,
                            memberName: RequireAttr(el, "name", filePath), methodName: null, signature: null, paramIndex: -1,
                            RequireSingleMarshalAs(el, filePath)));
                        break;
                    case "Method":
                        ParseMethod(filePath, assemblyName, typeFullName, el, rules, seen);
                        break;
                    default:
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] Unexpected element <" + el.Name + "> under Type in " + filePath);
                }
            }
        }

        private static void ParseMethod(
            string filePath,
            string assemblyName,
            string typeFullName,
            XmlElement methodEl,
            List<LuaMarshalAsXmlRule> rules,
            Dictionary<string, string> seen)
        {
            string methodName = RequireAttr(methodEl, "name", filePath);
            string signature = RequireAttr(methodEl, "signature", filePath);

            foreach (XmlAttribute attr in methodEl.Attributes)
            {
                if (attr.Name == "name" || attr.Name == "signature")
                {
                    continue;
                }

                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Unsupported Method attribute '@" + attr.Name + "' in " + filePath);
            }

            foreach (XmlNode child in methodEl.ChildNodes)
            {
                if (!(child is XmlElement el))
                {
                    continue;
                }

                switch (el.Name)
                {
                    case "Param":
                    {
                        if (el.HasAttribute("name"))
                        {
                            throw new LuaMarshalAsConfigurationException(
                                "[ZLua] Param must use @index (not @name) in " + filePath);
                        }

                        if (!el.HasAttribute("index")
                            || !int.TryParse(el.GetAttribute("index"), out int index)
                            || index < 0)
                        {
                            throw new LuaMarshalAsConfigurationException(
                                "[ZLua] Param/@index must be a non-negative integer in " + filePath);
                        }

                        AddRule(rules, seen, BuildRule(
                            filePath, assemblyName, typeFullName, LuaMarshalAsXmlTargetKind.Param,
                            memberName: null, methodName: methodName, signature: signature, paramIndex: index,
                            RequireSingleMarshalAs(el, filePath)));
                        break;
                    }
                    case "Return":
                        AddRule(rules, seen, BuildRule(
                            filePath, assemblyName, typeFullName, LuaMarshalAsXmlTargetKind.Return,
                            memberName: null, methodName: methodName, signature: signature, paramIndex: -1,
                            RequireSingleMarshalAs(el, filePath)));
                        break;
                    case "MarshalAs":
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] Method-level MarshalAs is not allowed; use Param/Return in " + filePath);
                    default:
                        throw new LuaMarshalAsConfigurationException(
                            "[ZLua] Unexpected element <" + el.Name + "> under Method in " + filePath);
                }
            }
        }

        private static XmlElement RequireSingleMarshalAs(XmlElement parent, string filePath)
        {
            XmlElement found = null;
            foreach (XmlNode child in parent.ChildNodes)
            {
                if (!(child is XmlElement el))
                {
                    continue;
                }

                if (!string.Equals(el.Name, "MarshalAs", StringComparison.Ordinal))
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] Unexpected element <" + el.Name + "> under <" + parent.Name + "> in " + filePath);
                }

                if (found != null)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] Duplicate <MarshalAs> under <" + parent.Name + "> in " + filePath);
                }

                found = el;
            }

            if (found == null)
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Missing <MarshalAs> under <" + parent.Name + "> in " + filePath);
            }

            return found;
        }

        private static LuaMarshalAsXmlRule BuildRule(
            string filePath,
            string assemblyName,
            string typeFullName,
            LuaMarshalAsXmlTargetKind kind,
            string memberName,
            string methodName,
            string signature,
            int paramIndex,
            XmlElement marshalAsEl)
        {
            string typeName = RequireAttr(marshalAsEl, "type", filePath);
            if (string.Equals(typeName, "OpaqueLightUserData", StringComparison.Ordinal))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Obsolete MarshalAs type 'OpaqueLightUserData' in " + filePath
                    + "; use 'OpaqueValue'.");
            }

            if (string.Equals(typeName, "ParamsTable", StringComparison.Ordinal))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Removed MarshalAs type 'ParamsTable' in " + filePath
                    + "; params T[] uses default szarray rules (single stack slot: userdata / table / nil).");
            }

            if (!Enum.TryParse(typeName, ignoreCase: false, out LuaMarshalType marshalType))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Unknown MarshalAs type '" + typeName + "' in " + filePath);
            }

            string membersAttr = marshalAsEl.HasAttribute("members") ? marshalAsEl.GetAttribute("members") : null;
            string[] members = SplitMembers(membersAttr);
            if ((marshalType == LuaMarshalType.Table || marshalType == LuaMarshalType.UnpackedValues)
                && (members == null || members.Length == 0))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] MarshalAs type '" + marshalType + "' requires @members in " + filePath);
            }

            return new LuaMarshalAsXmlRule
            {
                SourcePath = filePath,
                AssemblyName = assemblyName,
                TypeFullName = typeFullName,
                Kind = kind,
                MemberName = memberName,
                MethodName = methodName,
                Signature = signature,
                ParamIndex = paramIndex,
                MarshalType = marshalType,
                Members = members,
            };
        }

        private static string[] SplitMembers(string membersAttr)
        {
            if (string.IsNullOrWhiteSpace(membersAttr))
            {
                return null;
            }

            string[] parts = membersAttr.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < parts.Length; i++)
            {
                parts[i] = parts[i].Trim();
            }

            return parts;
        }

        private static void AddRule(
            List<LuaMarshalAsXmlRule> rules,
            Dictionary<string, string> seen,
            LuaMarshalAsXmlRule rule)
        {
            string key = rule.DuplicateKey;
            if (seen.TryGetValue(key, out string previousPath))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Duplicate MarshalAs XML rule for key '" + key + "'.\n"
                    + "  first: " + previousPath + "\n"
                    + "  conflict: " + rule.SourcePath);
            }

            seen.Add(key, rule.SourcePath);
            rules.Add(rule);
        }

        private static string RequireAttr(XmlElement el, string name, string filePath)
        {
            string value = el.GetAttribute(name);
            if (string.IsNullOrWhiteSpace(value))
            {
                throw new LuaMarshalAsConfigurationException(
                    "[ZLua] Missing @" + name + " on <" + el.Name + "> in " + filePath);
            }

            return value.Trim();
        }
    }
}
