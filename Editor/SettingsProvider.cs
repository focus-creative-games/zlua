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

using UnityEditor;
using UnityEngine.UIElements;

namespace ZLua
{
    public class SettingsProvider : UnityEditor.SettingsProvider
    {

        private static SettingsProvider s_provider;

        [SettingsProvider]
        public static UnityEditor.SettingsProvider CreateMyCustomSettingsProvider()
        {
            if (s_provider == null)
            {
                s_provider = new SettingsProvider();
            }
            return s_provider;
        }


        private SerializedObject _serializedObject;
        public SettingsProvider() : base("Project/ZLua", SettingsScope.Project)
        {
        }

        public override void OnActivate(string searchContext, VisualElement rootElement)
        {
            InitGUI();
            using (var so = new SerializedObject(Settings.Instance))
            {
                keywords = GetSearchKeywordsFromSerializedObject(so);
            }
        }

        public override void OnDeactivate()
        {
            base.OnDeactivate();
            Settings.Save();
        }

        private void InitGUI()
        {
            var setting = Settings.Instance;
            _serializedObject?.Dispose();
            _serializedObject = new SerializedObject(setting);
        }

        public override void OnGUI(string searchContext)
        {
            if (_serializedObject == null || !_serializedObject.targetObject)
            {
                InitGUI();
            }
            _serializedObject.Update();
            EditorGUI.BeginChangeCheck();

            // Draw all serialized fields on Settings so new members do not require Provider changes.
            using (var prop = _serializedObject.GetIterator())
            {
                if (prop.NextVisible(true))
                {
                    do
                    {
                        if (prop.name == "m_Script")
                        {
                            continue;
                        }

                        EditorGUILayout.PropertyField(prop, true);
                    }
                    while (prop.NextVisible(false));
                }
            }

            if (EditorGUI.EndChangeCheck())
            {
                _serializedObject.ApplyModifiedProperties();
                Settings.Save();
            }
        }
    }
}