using System.Collections;
using UnityEngine;

namespace ZLua
{
    /// <summary>
    /// Main-thread frame pump: pending Lua registry unrefs (LateUpdate) and
    /// deferred <see cref="LuaAppDomain.Reset"/> (EndOfFrame).
    /// </summary>
    internal sealed class LuaFramePump : MonoBehaviour
    {
        private static LuaFramePump _instance;
        private Coroutine _endOfFrameRoutine;

        internal static void EnsureRegistered()
        {
            if (_instance != null)
            {
                return;
            }

#if UNITY_EDITOR
            // DontDestroyOnLoad is play-mode only; edit-mode / batch -executeMethod skip the GO pump.
            if (!Application.isPlaying)
            {
                return;
            }
#endif

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

        private void OnEnable()
        {
            if (_endOfFrameRoutine == null)
            {
                _endOfFrameRoutine = StartCoroutine(EndOfFrameLoop());
            }
        }

        private void OnDisable()
        {
            if (_endOfFrameRoutine != null)
            {
                StopCoroutine(_endOfFrameRoutine);
                _endOfFrameRoutine = null;
            }
        }

        private void LateUpdate()
        {
            LuaAppDomain.ProcessPendingRefReleases();
        }

        private static IEnumerator EndOfFrameLoop()
        {
            var wait = new WaitForEndOfFrame();
            while (true)
            {
                yield return wait;
                LuaAppDomain.FlushPendingReset();
            }
        }
    }
}
