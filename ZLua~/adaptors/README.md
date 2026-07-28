# ZLua 迁移适配（xLua / toLua / SLua）

为从 **xLua / toLua / SLua** 迁到 ZLua 的工程提供 **Lua→C# 类型访问路径** 兼容。

权威契约：`docs/spec/12-MIGRATION-ADAPTORS.md`（zlua-doc）。

## 目录

| 路径 | 用途 |
|------|------|
| `adaptor.lua` | **唯一** Lua 适配实现 |
| `xlua/ExportTypes.cs` | 从 xLua 白名单生成清单（`top_namespace = "CS"`） |
| `tolua/ExportTypes.cs` | 从 `CustomSettings` 生成（命名空间链，如 `UnityEngine.GameObject`） |
| `slua/ExportTypes.cs` | 从 SLua Custom 导出配置生成 |

## 使用步骤

1. 将对应方案的 `ExportTypes.cs` 复制到 **旧工程** Editor 目录。
2. 菜单 **ZLua / ExportTypes**，生成 `Assets/ZLua/{方案}_export_types.lua`。
3. 将生成的清单与根目录 `adaptor.lua` 复制到 **ZLua 工程** Lua 源码目录。
4. 入口：

```lua
local export_types = require "xlua_export_types"
local adaptor = require "adaptor"
adaptor.init(export_types)
```

## 清单形状（摘要）

```lua
return {
  top_namespace = "CS",  -- 可选；toLua/SLua 省略 → _G
  types = {
    ["UnityEngine.CoreModule"] = {
      { full_name = "UnityEngine.GameObject" },  -- 与 export 路径相同时可省略 export_name
      { full_name = "Ns.Outer+Inner", export_name = "Ns.Outer.Inner" },
    },
  },
}
```

- 省略 `export_name` → 视为等于 `full_name`。
- `export_name` **不含** `.` → `init` 急切挂到根。
- `export_name` **含** `.` → 命名空间表 + `__index` 惰性解析。

## 能力边界

- **仅** 类型路径重定向到 `CSharp[asm][full_name]`。
- **不** 兼容 C#→Lua、泛型 `List(Int32)` 写法等。
- toLua 按运行时 `BeginModule` 路径导出（**不是** 仅全局短名）。

本目录在 `ZLua~` 下，**不会**被 Unity 编译进包；须手动复制后使用。
