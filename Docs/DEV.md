# 开发计划

## v1.0

仅测试 Unity 2022.3.62f3 + Lua 5.4

- lua调用c#
  - [x] 访问class、struct类
  - [x] 访问成员变量和静态成员变量
  - [x] 调用函数和静态函数
  - [ ] 解决函数重载问题
  - 访问Property，普通无参property和带参property (如index访问器)
  - 访问Event
  - 访问泛型类
  - 调用泛型函数
  - 支持函数重载
  - 访问Array之类的对象
- C#调用lua
  - LuaInvoke
- Marshal
  - 常规bool,byte, long, float, double 等常规类型
  - string类型
  - class类型
  - value类型
  - array类型
  - 特殊类型：指针、函数指针、TypedReference
  - in/out/ref 类型
  - LuaMarshalAs规则
- 性能优化
  - 优化成员和属性访问
  - 将简单的int X {get; set;}之类的Property访问重构为字段访问
  - 优化函数调用
  - 桥接函数优化
  - Marshal优化
- 测试项目
- 性能报告
- 其他
  - 编译 lua 5.4.8 for macOS

## v2.0

- 支持 Unity 2021+ LTS、支持团结引擎
- 支持 Lua 5.1、Lua 5.3+、LuaJit、Luau
- Editor方面的优化？ 暂时没想好
- 额外的功能，如 LuaMonoBehaviour支持？暂时也没想好
- xlua、tolua迁移指南

## v3.0

基于Lua 5.3+ 版本的 LuaJIT
