using UnityEngine;

namespace ZLua
{
    /// <summary>
    /// Flushes queued Lua registry unrefs once per frame on the main thread.
    /// </summary>
    internal sealed class LuaFramePump : MonoBehaviour
    {
        private static LuaFramePump _instance;

        internal static void EnsureRegistered()
        {
            if (_instance != null)
            {
                return;
            }

            var gameObject = new GameObject("[ZLua] FramePump");
            gameObject.hideFlags = HideFlags.HideAndDontSave;
            DontDestroyOnLoad(gameObject);
            _instance = gameObject.AddComponent<LuaFramePump>();
        }

        internal static void Unregister()
        {
            if (_instance == null)
            {
                return;
            }

            GameObject gameObject = _instance.gameObject;
            _instance = null;
            if (gameObject != null)
            {
                Object.Destroy(gameObject);
            }
        }

        private void LateUpdate()
        {
            LuaAppDomain.ProcessPendingRefReleases();
        }
    }
}
