# class 类型 Marshal 设计规范

本文档描述 **引用类型**（class、interface、string、array、delegate 实例等）在 Lua 与 C# 之间的编组规则，与 `TYPE_SYSTEM_SPEC.md` §3（class 实例 userdata）、`MARSHAL_SPEC.md` 衔接。

**平台原则：** Mono 与 Il2Cpp 的 Lua 可见语义一致；class 实例默认 **GCHandle + userdata**（Il2Cpp：`ObjectRegistry`）。

---

## 1. 默认 by-value 规则（概要）

| 方向 | Lua 形态 | 说明 |
|------|----------|------|
| **C# → Lua** | class **userdata** | `PushConstructorInstance` / `ObjectRegistry` |
| **Lua → C#** | userdata / `nil` | `TryGetBoxedTarget`；须可赋值给目标类型 |
| **string** | `string` / `nil` | 非 userdata |
| **array** | array userdata | 见 `TYPE_SYSTEM_SPEC.md` §7 |

详细字段、方法访问见 `TYPE_SYSTEM_SPEC.md` §3。

---

## 2. `ref` / `out` / `in` 引用类型形参（Lua → C#）

总览见 `MARSHAL_SPEC.md` §3。本节补充 **引用类型** 特有规则。

### 2.1 Pop 行为

| Lua 实参 | 行为 |
|----------|------|
| class / string / array **userdata** 或 `string` / `nil` | 按 **by-val** 读入引用（对象 ID） |
| 其他可转换形态 | 拷贝进 **临时 ref 槽**（与值类型拷贝分支相同） |

**均不报错**（与值类型 §3.1 一致）。

### 2.2 写回：仅「共享引用」，无 rebind

| C# 侧操作 | Lua 侧 |
|-----------|--------|
| `refParam = otherObject`（**重新绑定** ref） | **不回写**；Lua 变量 / userdata 仍指向原对象 |
| 对 **可变对象** 原地修改（`ref StringBuilder`、`ref List<T>`、改数组元素） | **可见**（同一托管对象） |
| `ref string` 赋新字符串 | **不回写**（string 不可变，语义等同 rebind） |

```lua
local s = "hello"
CS.Demo.ChangeString(s)   -- void ChangeString(ref string s) { s = "world"; }
-- s 仍为 "hello"

local sb = StringBuilder("hi")
CS.Demo.Append(sb, "!")   -- void Append(ref StringBuilder sb, string t) { sb.Append(t); }
-- sb 内容已变（共享引用）
```

### 2.3 与 `zlua.new_ref` 的关系

`zlua.new_ref` **仅接受值类型**（`LIB_SPEC.md` §6）。**不要**对 class / string 使用 `new_ref`；`ref MyClass` 直接传 userdata 或 `nil` 即可。

---

## 3. 相关文档

| 文档 | 内容 |
|------|------|
| `MARSHAL_SPEC.md` §3 | ref/in/out 总规则 |
| `STRUCT_MARSHAL_SPEC.md` §6.2 | 值类型 StructUserData / ref |
| `FUNCTION_MARSHAL_SPEC.md` | delegate 形参（无 ref） |
