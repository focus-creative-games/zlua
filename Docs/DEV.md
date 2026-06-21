# 开发计划

## v0.0.1

当前插件中已经包含Windows版本的lua 5.4的dll。本版本的目标是实现Editor下lua与c#之间的无感相互调用。设计要求如下：

- 类比于 P/Invoke、 MonoPInvokeCallback, MarshalAs， novalua 有同样对应的概念 L/Invoke、MonoLuaCallback, LuaMarshalAs
- c# 与 lua 之间交互是高度统一的：
  - c#可以调用 标记为 [LuaInvoke] 的static c#函数，调用lua函数
  - 所有c#类都通过lazy register的方式，使用时自动注册到lua环境。  lua通过 `CSharp.<Module>.<Type>` 可以访问类， 通过`<Type>.XXX` 访问静态成员和函数，通过 `obj:Fun` 调用成员函数和属性，无论它是struct、class、泛型还是array。语义完全等价到c#中以这种方式调用c#函数。没有特殊概念，完全统一
  - c# 与 lua之间的 交互代码，都是自动生成的，对开发者完全感。 在editor下生成c#代码，发布到il2cpp时，生成 c++ 代码。
- 深度集成，启动时就有初始化好的全局CLR和luaState
- 开发者并不需要任何操作luaState的操作。 lua脚本的全局入口为一个 标记为 `[LuaInvoke("app"， "main")]`的函数，即该函数会自动绑定lua的app module模块的main函数。
- 开发者需要提供一个 lua loader 函数，用于加载lua代码。

## v0.0.2

支持发布到win64，backend为il2cpp。 需要达到以下目标：

- 交互函数全是c++代码
- 复用相同签名的交互函数，不像{abc}lua那样，每个字段或者函数都生成一个单独的函数。 就如hybridclr那样，直接生成所有必要的桥接文件，总大小仍然可控。
- 调用 lua接口并不需要通过 LuaDll 里的 extern 调用，而是c++层面直接调用lua api
- 访问类成员变量也是直接c++ 代码中 obj + 偏移 直接访问，不需要调用c#函数。访问类静态成员变量也类似。
- 访问类成员（静态和非静态）函数，也是通过il2cpp的 MethodInfo中的 methodPointer直接调用，不需要经过 c#包装类
- 为托管对象生成lua userData时，在userData中直接记录了object指针。同时在native代码维护了一个 object列表，这个列表被注册到 gc roots。等lua释放userDatas时，再将这个对象从列表中清除。

以上目标需要深度修改 libil2cpp 代码。在 `Packages\com.code-philosophy.novalua\Cpp~\libil2cpp-2022` 目录已经放置了 unity 2022.3.62f3版本的libil2cpp源码。在启动novalua插件
后，需要一个类似hybridclr的安装过程，将unity editor安装目录下的il2cpp复制到项目某个本地目录，然后替换其中的libil2cpp目录为`Packages\com.code-philosophy.novalua\Cpp~\libil2cpp-2022`。 同时在构建过程中设置环境变量UNITY_IL2CPP_PATH为本地il2cpp目录。 具体实现可以参考 hybridclr的实现 `D:\workspace\main\hybridclr_unity\Editor\Installer\InstallerController.cs`。