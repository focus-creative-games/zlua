# zlua

zlua是一个高效、稳定、优雅的特殊为il2cpp优化的Unity lua脚本方案。

## 为什么选择 zlua

Unity平台已经有革命性的原生C#热更新方案[HybridCLR](https://github.com/focus-creative-games/hybridclr)，也有很成熟的xlua、tolua
之类的lua方案，为何我们还需要一个新的lua解决方案？有以下原因：

- 现有的lua方案都存在与il2cpp运行时交互低效的问题，通过针对il2cpp的优化可以数保提升交互性能
- 现有的lua方案都存在与il2cpp运行时交互容易产生GC的问题，现存的方案有提供针对值类型的优化方案，但有较多限制，通过针对il2cpp的优化可以无感优化掉绝大多数这类GC问题
- 现有的lua方案要么已经数年不维护，要么维护频繁极低，基本丧失积极维护的动力，需要一个更敏捷的紧跟Unity和团结引擎版本变化的方案

zlua目标提供一个更高效、更稳定的满足商业游戏项目的现代化的Unity lua脚本方案。
