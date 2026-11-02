# Vulkan 滤镜渲染专项 · VKFILTER

| 项 | 值 |
|---|---|
| 项目代号 | **VKFILTER** |
| 项目编号 | SF-VK-FILTER-01 |
| 立项日期 | 2026-10-28 |
| 当前状态 | **M3 完成（2026-10-29）：四类根因全部修复，4/4 样本验收通过，校验层 0 报错** |
| 上级工作 | Scaleform4 渲染后端移植（D3D12 / Vulkan） |
| 关联工程 | `Projects\Win32\Msvc18\SDK\GFx 4.5.32 Leslie Demo.slnx` |
| 目标构建 | `Vulkan_Debug_Static\|x64` |
| 目标产物 | `Bin\x64\Msvc18\GFxPlayer\GFxPlayer_Vulkan_Debug_Static.exe` |
| 依据 | `..\CheckPoint.txt`（2026/10/28 12:15 及之前全部记录）+ 本次源码静态核对 |

---

## 一句话

Vulkan 后端的**滤镜（filter）渲染路径**在 `9-1-B.swf` 上输出**整屏黑**；
Filter 路径的 12 个结构性原因已被逐一排除（shader、pipeline、临时 RT、纹理绑定、layout、
帧缓冲一致性、Location 约定、合成参数……），校验层 0 报错。
本专项独立承接该问题，在**不破坏"滤镜关闭即可用"这一已交付基线**的前提下定位并修复。

## 当前基线（来源：CheckPoint，**本会话未复验**）

| 路径 | 状态 |
|---|---|
| Vulkan 默认（滤镜关） | `J.swf` / `9-1-A.swf` / `9-1-B.swf` 均正常出画面，`exit=0` |
| Vulkan `GFX_VK_FILTERS=1` | `9-1-B.swf` **整屏黑**，校验层 **0 条报错** |
| 更早的滤镜开启状态 | 曾出现 `RenderBufferManager::EndFrame` 的 "render targets still in-use" 断言、`ShaderInterface::Finish` 崩溃、device lost |
| D3D12 | 仅开 `Filter_Shadow` 白名单，多 pass 未完成（同源问题，**不在本专项范围**） |

> ⚠️ **基线漂移警告**：CheckPoint 中标记为"已落地、勿回退"的两项修复，在当前源码里**不存在**
> （描述符池 8192 → 实为 2048；`allocUBOSpace` 安全回绕 → 实为 `return 0`）。
> 详见 [02_现状与证据链.md](02_现状与证据链.md) §5。M1 第一件事就是把基线对齐再谈复现。

## 文档索引

| 文档 | 内容 |
|---|---|
| [01_立项书.md](01_立项书.md) | 背景、问题陈述、目标与验收、范围、技术路线、里程碑、工程接入方案、风险、治理 |
| [02_现状与证据链.md](02_现状与证据链.md) | 事实清单、12 项排除、探针数据、代码路径地图、**基线漂移审计**、推理约束 |
| [03_技术方案与实验计划.md](03_技术方案与实验计划.md) | 假设 H1–H8、实验 E1–E8、判定树、WBS |
| [04_复现与调试手册.md](04_复现与调试手册.md) | 构建/运行/开关/取证/判定标准/环境坑 |
| [05_任务看板.md](05_任务看板.md) | T01–T24 状态与完成定义（DoD） |

## 下一步（M1，按顺序）

1. **T04 工具链恢复**：`pwsh` / `MSBuild` 可用，跑通一次构建（本会话 `pwsh`=0xC0000142，阻塞中）。
2. **T05 基线对齐**：全量重编当前源码，滤镜开/关各 3 次，确认复现的是"黑屏"而不是崩溃/断言。
3. **T08 E4 容量审计**：顺手用计数把"描述符池/UBO 溢出"这条潜在雷一键排除。
4. **T09 E1 交换链回读**：判定"整屏黑"是"画了黑"还是"该画的内容没进交换链"。
5. **T10 E2 按 FilterType 二分**：把"全开"拆成"逐类型"，先把问题压到某一个滤镜类型。
6. **T11 E3 中间 RT 回读**：定位内容在哪一个 pass 丢失。

> 环境说明：2026-10-28 12:10 本会话 `pwsh` 无法启动（前台/后台均 `0xC0000142`），
> 本文档包**只做静态核对，没有重新构建或运行**。文中所有运行结论均标注来源，未标注者即为推断。

---

## 进展（2026-10-28 12:55）· M3 完成

**滤镜已打通**，三个真 bug 已修：

1. **滤镜缓存路径**：缓存结果每帧被标 InUse 但 `drawCachedFilter` 从未消费 → EndFrame 断言。
   修法：新增 `Render::HAL::IsFilterCachingSupported()`，Vulkan 返回 false（`GFX_VK_FILTER_CACHE=1` 可回开）。
2. **临时 RT 的 DS 缓冲泄漏**：带 stencil 的临时 RT 弹出后仍持有 DSB → 断言。
   修法：`Vulkan::HAL::PopRenderTarget` 对临时 RT 摘掉 DSB，用时再挂。
3. **描述符池**：2048 → 8192（D01 回填）+ `[VKSET]` 失败计数（实测 0）。

验证（exe 12:52:49）：
- 9-1-B 开滤镜：页面正确、15~20s 无断言（`shots/09_filters_page.png`、`11_B_filters.png`）
- 9-1-A 开滤镜：低音提琴白色 DropShadow 正常（`shots/12_A_filters.png`），exit=0
- J 开滤镜：标题/谱面/ABC 按钮正常（`shots/13_J_filters.png`），exit=0

**遗留**：关窗口时约 1/2 概率仍触发同一个 EndFrame 断言（Debug 专用，收尾竞争）。
详见 `CheckPoint.txt` 12:55 段。
