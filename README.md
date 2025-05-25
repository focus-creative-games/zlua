# zlua

zlua是一个**极致高效**、**富有创新**、稳定、敏捷的充分满足商业游戏项目需求的现代Unity lua脚本方案。

## 为什么选择 zlua

Unity平台已经有革命性的原生C#热更新方案[HybridCLR](https://github.com/focus-creative-games/hybridclr)，也有很成熟的xlua、tolua
之类的lua方案，为何我们还需要一个新的lua解决方案？因为现存的所有lua方案都存在以下问题：

- 与il2cpp运行时交互非常低效的痛点问题。
- 与il2cpp运行时交互产生大量GC的问题。
- 生成巨量wrapper文件。不仅低效还显著增加了代码大小。
- 未优化字段、简单property访问这种最最常用的功能。精心优化可以提升10倍以上性能。
- 对CLR的特性支持不全。例如对泛型类型、泛型函数以及对ref、out、in之类的函数参数支持不佳。
- 要么已经数年不维护，要么维护频率极低，基本丧失积极维护的动力，需要一个更敏捷的紧跟Unity和团结引擎版本变化的方案。

## 特性

### 运行时

- 数倍甚至十倍以上优化了双向调用的性能。首次清晰引入 `[LuaInvoke]`、`[LuaCallback]`、 `[LuaMarshalAs]`的概念。
- 深度优化双向调用过程产生的GC，通过针对il2cpp的优化可以无感优化掉绝大多数struct和string之类的GC问题。
- 更高效的wrapper生成，所有相同签名函数只需生成一个wrapper函数，极大缩减wrapper数量，同时提供更简洁和灵活的Wrapper配置。
- 优化il2cpp和lua循环引用引发的内存泄露。
- 支持全方面的CLR与lua运行时的相互调用。例如支持数组、泛型类型、泛型函数以及对ref、out、in之类的函数参数。
- 支持字段访问优化及简单property(类似`int Value {get; set;}`)优化，至少**提升10倍**以上性能。
- 清晰优雅的代码实现、良好的模块。
- 支持luajit 及 lua 5.3-5.4版本。
- 尽可能使用luajit深度优化调用c#函数的性能。
- 尽可能使用light C function优化调用c#函数的性能。
- 支持协程。
- 维护大量常见的第三库。
- 支持调试。
- 支持profiler。
- **TODO**
  - aot hotfix。依赖hybridclr。
  - 原生Lua MonoBehaviour支持。依赖hybridclr。

### Editor

- Editor内无任何生成。
- 支持Editor 内热重载。
- 支持console lua日志跳转到源码。

### 其他

- 完善全面的测试工程。

## 支持的版本和平台

- 支持luajit(5.1)及lua 5.3+版本。
- 支持 unity 2022+版本及团结引擎（2021及更早的版本后面再支持）。
- 支持 mono、il2cpp backend。
- 支持il2cpp支持的所有平台（含webgl、微信小游戏及团结引擎支持的鸿蒙和车机平台）。

## 许可证

zlua 采用 MIT 许可证发布，欢迎自由使用、修改和分发。

## 联系我们

如有问题、建议或错误报告，请在用以下方式联系我们：

- GitHub 上提交 Issue
- 邮件联系维护者：`zlua#code-philosophy.com`
- QQ群 **zlua交流群**： 824793773
- discord频道 `https://discord.gg/htmr44jW6A`
