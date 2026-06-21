using NovaLua;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace NovaLlua
{
    public class PreserveTypes : MonoBehaviour
    {
        // Start is called before the first frame update
        void Start()
        {
            LuaIl2CppAppDomain.Initialize(null);
        }
    }
}
