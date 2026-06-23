# C# 与 Lua 参数 Marshal 设计规范

本文档为 **参数编组总览**；具体类型规则见分册：

| 文档 | 类型 |
|------|------|
| `TYPE_SYSTEM_SPEC.md` | 类型解析、元表、成员访问 |
| `STRUCT_MARSHAL_SPEC.md` | struct（值类型） |
| `CLASS_MARSHAL_SPEC.md` | class、引用类型、数组元素 |
| `METHOD_OVERLOAD_SPEC.md` | 重载与实参匹配 |

**平台原则：** Mono 与 Il2Cpp 的 Lua 可见编组语义一致；Il2Cpp 侧重零 GC 与生成代码快速路径。

（详细规则待补充。）
