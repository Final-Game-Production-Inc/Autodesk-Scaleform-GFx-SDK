# Proj_D3D12Fix · D3D12 后端修复归档

本目录归档 D3D12 渲染后端的移植修复成果（2026-10-27 那一轮），原始逐条记录在仓库根的 `CheckPoint.txt`。

| 文件 | 内容 |
|---|---|
| [D3D12_最终修复报告.md](D3D12_最终修复报告.md) | 现象 P1–P8、根因 R1–R10（含证据链）、修复清单、验证数据与截图、遗留与经验 |
| [ShaderData_GFxShaderMaker_修复报告.md](ShaderData_GFxShaderMaker_修复报告.md) | 着色器生成链：fxc→dxc（DXIL/SM6）、ShaderData.xml 的 blur 循环 clamp、重生成流程与契约对照 |

## 一句话结论

`D3D12::Texture` 析构函数**没有把自己从 `TextureManager::Textures` 链表摘除**，导致悬垂链表 → Scaleform 堆被写坏 → "顺利通过"黄色方块、各处随机 `0xC0000005`、退出崩溃、26 个 live object 都只是它的不同表现。修好这一条后，其余问题（设备挂起、描述符、顶点格式、视频、音频）都已独立闭环。

## 关键数字

| 指标 | 修复前 | 修复后 |
|---|---|---|
| Debug Layer 报错 | 6 类 | 0 |
| D3D12 live objects | 26 | 0 |
| 游戏宫02 稳定性 | 3/3 崩 | 3/3 RUNNING |
| 退出码 | 2173（0x87D） | 0 |
| 视频帧率 | 卡顿/绿屏 | 60.0fps 稳定 |

## 来源

- `CheckPoint.txt`：`2026/10/27 10:12`（阶段总结）、`11:55–20:35`（逐条记录）、`2026/10/28 01:05`（同源缺陷在 Vulkan 的复现）
- 截图：`..\verify_d3d12\*.png`
- dump：`dumps\GFxPlayer_D3D12_Debug_Static.exe.*.dmp`

