# ZLua 迁移适配（xLua / toLua / SLua）

为从 **xLua / toLua / SLua** 迁到 ZLua 的工程提供 **Lua→C# 类型访问路径** 兼容。

权威契约：仓库文档 `docs/spec/12-MIGRATION-ADAPTORS.md`（zlua-doc）。

## 目录

| 目录 | 用途 |
|------|------|
| `xlua/` | `adaptor.lua`（`CS.*`）+ `ExportTypes.cs` |
| `tolua/` | `adaptor.lua`（全局短名）+ `ExportTypes.cs` |
| `slua/` | `adaptor.lua`（`Namespace.Type`）+ `ExportTypes.cs` |

## 使用步骤

1. 将对应方案的 `ExportTypes.cs` 复制到 **旧工程** 的 Editor 目录。
2. 菜单 **ZLua / ExportTypes**，生成 `Assets/ZLua/{方案}_export_types.lua`（路径可在脚本顶部常量修改）。
3. 将生成的 `*_export_types.lua` 与同方案的 `adaptor.lua` 复制到 **ZLua 工程** 的 Lua 源码目录（须能被 moduleLoader `require`）。
4. 入口脚本：

```lua
local export_types = require "xlua_export_types"  -- 或 tolua_ / slua_
local adaptor = require "adaptor"
adaptor.init(export_types)
```

## 能力边界

- **仅** 解决类型表如何按旧路径拿到（重定向到 `CSharp[asm][fullName]`）。
- **不** 兼容 C#→Lua、泛型 `List(Int32)` 写法、Event 赋值语法等。
- **惰性**：带命名空间的类型只在 `ns.Name` 访问时才 `CSharp[...]` 绑定；无命名空间（及 toLua 短名）在 `init` 时注册。
- 嵌套类型：`Outer+Inner` 在挂载路径中按 `Outer.Inner` 处理。
- 开放泛型 / 含 `[` 的 FullName（闭合泛型实例等）在导出时跳过并告警。

## 注意

- 同一工程只复制 **一个** 方案的 `ExportTypes.cs`，避免重复菜单。
- 本目录位于 `ZLua~` 下，**不会**被 Unity 编译进包；必须手动复制后使用。
