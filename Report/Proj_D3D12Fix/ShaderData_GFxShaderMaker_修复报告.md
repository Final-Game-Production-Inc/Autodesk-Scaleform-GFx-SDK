# ShaderData.xml 与 GFxShaderMaker（D3D12）修复报告

| 项 | 值 |
|---|---|
| 范围 | D3D12 着色器数据生成链：`ShaderData.xml`（母版） + `GFxShaderMaker`（生成器） |
| 工程 | `Apps\Tools\GFxShaderMaker`、`Src\Render\ShaderData.xml` |
| 产物 | `Src\Render\D3D12\D3D12_ShaderDescs.h/.cpp`（以及 Vulkan / D3D1x / GL 各平台同名文件） |
| 对比基线 | `Src\Render\ShaderDataISSUE.xml`、`GFxShaderMaker.Platforms\Platform_D3D12_ISSUES.cs`（修复前快照） |
| 结论 | 两处修复：**① 生成器改用 dxc（DXIL / SM 6.x）**；**② 着色器母版里把模糊滤镜的循环半径 clamp 到 ±32**。改完重新生成，D3D12/Vulkan 的 shader 数据才与 HAL 契约对齐 |

---

## 1. 先理清这条链

```
ShaderData.xml            ← 着色器母版：ShaderGroup / ShaderFeature(Flavor) / ShaderSource(HLSL 片段)
      │
      ▼  GFxShaderMaker.exe -platform D3D12 -xml ShaderData.xml -type Binary
      │      ├─ Platform_D3D12.cs       平台插件（编译器定位、输入布局、二进制 extern）
      │      └─ ShaderVersion_D3D12.cs  版本插件（SM 6.x、RootSignature、cbuffer 布局）
      ▼
D3D12_ShaderDescs.h/.cpp  ← DXIL 字节码数组 + 描述符/常量/顶点属性描述表
      │
      ▼
D3D12 HAL 运行时（根签名 CBV b0/b1 + SRV t0..t7 + Sampler s0..s7）
```

**关键点**：生成器输出的是"运行时契约"——根签名槽位、顶点属性格式、uniform 偏移。HAL 写错会被校验层抓到；**生成器写错，HAL 就永远对不上**，而且症状很像"渲染 bug"。这次两个问题都在生成链上。

---

## 2. 修复一：D3D12 插件用错了编译器（fxc → dxc）

### 2.1 修复前（`Platform_D3D12_ISSUES.cs`）

没有重写 `LocateShaderCompiler()`，直接继承 `Platform_D3DCommon` 的实现 —— 那个实现是**找 `fxc.exe`**（`DXSDK_DIR` / Windows Kits 的 fxc），选项也是 fxc 风格：

```csharp
// ISSUES：继承基类 -> fxc.exe
case "Debug":     return "/Zi";
case "Release":   return "/Zi /O3";
case "Shipping":  return "/O3 /Qstrip_reflect /Qstrip_debug";
```

但 D3D12 版本插件要求的是 **SM 6.0**：

```csharp
// ShaderVersion_D3D12.cs（未变）
Fragment => "ps_6_0", Vertex => "vs_6_0", ...
RootSignatureAttribute => "[RootSignature(ScaleformRS)]\n";
```

`fxc` 最高只到 `vs_5_1/ps_5_1`，且**不认识 `[RootSignature(...)]` 属性**（那是 DXC 的 HLSL 特性）。也就是说修复前的 D3D12 生成**根本跑不通**：要么在 profile 上直接报错，要么产出与 HAL 期望（DXIL + 根签名）不匹配的字节码。

### 2.2 修复后（`Platform_D3D12.cs`）

新增编译器定位重写 + 把选项改成 dxc 语法：

```csharp
protected override string LocateShaderCompiler()
{
    // D3D12 needs DXIL (SM 6.x), so use dxc.exe from the Windows SDK.
    string kits = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86),
                               "Windows Kits", "10", "bin");
    if (Directory.Exists(kits))
    {
        string dxc = Directory.GetFiles(kits, "dxc.exe", SearchOption.AllDirectories)
            .Where(p => p.IndexOf("\\x64\\", StringComparison.OrdinalIgnoreCase) >= 0)
            .OrderByDescending(p => p).FirstOrDefault();
        if (!string.IsNullOrEmpty(dxc)) { Console.WriteLine("Using dxc.exe: {0}\n", dxc); return dxc; }
    }
    throw new Exception("Could not locate dxc.exe in the Windows SDK (Windows Kits\\10\\bin\\*\\x64).");
}

protected override string D3DFXCExtraOptions => 配置 switch {
    "Debug"/"DebugOpt" => "-Zi",
    "Release"          => "-Zi -O3",
    "Shipping"         => "-O3 -Qstrip_reflect -Qstrip_debug",
};
```

### 2.3 差异清单（逐行对比结果）

| 项 | ISSUES | FIXED |
|---|---|---|
| `using System.IO;` | 无 | 有（找 dxc 需要） |
| `LocateShaderCompiler()` | 继承基类 → **fxc.exe** | 重写 → **dxc.exe（Windows Kits\10\bin\**\x64）** |
| 编译选项前缀 | `/Zi`、`/O3`、`/Qstrip_*`（fxc） | `-Zi`、`-O3`、`-Qstrip_*`（dxc） |
| 找不到编译器时 | 报 "Could not locate fxc.exe" | 报 "Could not locate dxc.exe in the Windows SDK" |
| 行数 | 193 | 214 |

共 **27 行差异**，全部围绕"用哪个编译器、怎么传参数"。

---

## 3. 修复二：模糊滤镜的动态循环没有上界（`ShaderData.xml`）

### 3.1 修复前（`ShaderDataISSUE.xml`）

`Box1`（一维模糊）与 `Box2`（二维模糊）的循环边界**直接来自运行时 uniform `fsize`**：

```hlsl
// Box1
float  i = 0;
for (i = -fsize.x; i <= fsize.x; i++)     // 迭代次数 = 2*fsize.x+1，无上限
{{

// Box2（嵌套）
float2 i = float2(0, 0);
for (i.x = -fsize.x; i.x <= fsize.x; i.x++)
{
  for (i.y = -fsize.y; i.y <= fsize.y; i.y++)
  {                                       // 迭代次数 = (2fx+1)*(2fy+1)，平方级
```

而 `fsize` 是 HAL 每帧按 SWF 的滤镜参数算出来的（`Src\Render\Render_Shader.h:825-866`）：

```cpp
float blurx = Alg::Max(1.0f, floorf(TwipsToPixels(params.BlurX)));
float blury = Alg::Max(1.0f, floorf(TwipsToPixels(params.BlurY)));
fsize[0] = (blurx-1)*0.5f;   // 半宽
fsize[1] = (blury-1)*0.5f;
```

**后果**：`Box2` 的采样次数 = `(2fx+1) × (2fy+1)`。SWF 里一个 100px 的 Glow/Blur 就是 ≈ 100×100 = **10000 次纹理采样/像素**；作用在 1088×512 的整屏临时 RT 上就是十亿级采样 → 单帧几秒甚至更久 → 触发 GPU 看门狗（TDR）/ `DEVICE_HUNG` / 设备移除，表现就是"滤镜一开就卡死/黑屏/崩溃"。

> 注：`fsize` 的来源是 SWF 的滤镜参数（BlurX/BlurY，单位 twips），不是常量；参数越大循环越长，属于"由内容决定的炸弹"。

### 3.2 修复后（`ShaderData.xml`）

在进入循环**之前**把半径夹紧（`ShaderData.xml:735`、`:752`）：

```hlsl
// Box1
float  fsizeX = clamp(fsize.x, -32.0, 32.0);
for (i = -fsizeX; i <= fsizeX; i++)

// Box2
float2 fsizeXY = clamp(fsize.xy, float2(-32.0,-32.0), float2(32.0,32.0));
for (i.x = -fsizeXY.x; i.x <= fsizeXY.x; i.x++)
{
  for (i.y = -fsizeXY.y; i.y <= fsizeXY.y; i.y++)
```

### 3.3 差异清单

两个文件行数 1227 vs 1225，**只有 8 行不同**，且全部是上面这处 clamp：

| 文件 | 行 | 内容 |
|---|---|---|
| ISSUES | Box1 | `for (i = -fsize.x; i <= fsize.x; i++)` |
| ISSUES | Box2 | `for (i.x = -fsize.x; ...)` / `for (i.y = -fsize.y; ...)` |
| FIXED | Box1 | `float fsizeX = clamp(fsize.x, -32.0, 32.0);` + `for (i = -fsizeX; ...)` |
| FIXED | Box2 | `float2 fsizeXY = clamp(fsize.xy, ...)` + 两层循环都用 `fsizeXY` |

### 3.4 为什么是 ±32

- 一维：`2*32+1 = 65` taps，仍是合理的可分离模糊半径；
- 二维：`65×65 = 4225` taps，已经偏重，但**有上界**，不会再有"十万级采样"；
- 与 Scaleform 参考实现的量级一致；超出这个半径时视觉差异已经可以忽略，但性能会指数级恶化。

---

## 4. 重新生成与产物

### 4.1 命令

```bat
cd Src\Render
..\..\Bin\Tools\GFxShaderMaker\GFxShaderMaker.exe -platform D3D12 ^
    -xml ShaderData.xml -outdir D3D12\ -config Debug -type Binary
```

（Vulkan 同形：`-platform Vulkan -outdir Vulkan\`。`-type Binary` 表示把编译好的字节码写成 C 数组。）

### 4.2 时间线（文件系统证据）

| 时间 | 事件 |
|---|---|
| 10/25 23:56–23:58 | `ShaderVersion_D3D12.cs` / `Platform_D3DCommon.cs` / `ShaderVersion_D3DCommon.cs` 就位 |
| 10/26 09:01 | `Platform_D3D12.cs` 加 dxc 支持；工具重建（`Bin\Tools\GFxShaderMaker\*.exe` 09:01:32） |
| 10/26 09:50 | `ShaderData.xml` 加 blur clamp |
| 10/26 09:52 | **重新生成** `D3D12_ShaderDescs.cpp/.h`（09:52:04 / 09:52:05） |
| 10/28 10:35 | 用同一份 XML 重新生成 `Vulkan_ShaderDescs.*`（10:35:28 / 10:35:29） |

> 也就是说：**改 XML 必须重新跑所有平台**。这次 D3D12 与 Vulkan 都重生了；D3D1x / GL 的数据仍是 10/13 的旧产物（未含 clamp），若以后在这两个后端上跑大半径模糊，同样的循环爆炸会复现。

### 4.3 生成器输出的"运行时契约"（HAL 必须对齐的部分）

| 生成物 | 值 | HAL 侧对应 |
|---|---|---|
| 根签名 | `CBV(b0,VS) / CBV(b1,PS) / SRV(t0..t7) / Sampler(s0..s7)` | D3D12 HAL 必须**每次 draw 铺满 8 个 SRV / 8 个 sampler**（否则就是之前 id=654/1361 那类问题） |
| 顶点位置格式 | `DXGI_FORMAT_R32G32_FLOAT`（`VET_Pos`） | 顶点数据必须是 F32 位置 → HAL 的 `MapVertexFormat` 要把 S16 提升为 F32，且**偏移补偿必须正确** |
| INSTANCE / FACTOR 语义 | 映射到 `COLOR` 槽 + `VET_Instance`/`VET_Argument` | 批量/实例化顶点流 |
| `D3D12_INPUT_ELEMENT_DESC[]` | 每个 shader 一份（`D3D12Attributes`） | PSO 创建时直接使用 |
| 二进制 | `extern const BYTE pBinary_*/pBinary_*_size` | 与 `D3D12_ShaderDescs.cpp` 的数组一一对应 |

---

## 5. 验证

| 项 | 结果 |
|---|---|
| 生成器可用性 | 修复前：D3D12 走 fxc，SM 6.0 / `[RootSignature]` 无法编译；修复后：dxc 正常产出 DXIL |
| 数据一致性 | `D3D12_ShaderDescs.cpp/.h` 于 09:52 重新生成，与 09:50 的 XML 对应 |
| 渲染 | 9-1-A 小提琴 DropShadow 正常；J.swf 矢量按钮/文字正常；游戏宫02 谱面正常（`verify_d3d12\*.png`） |
| 稳定性 | 设备不再 `DEVICE_HUNG`；`exit=0`；live objects = 0 |
| 风险提示 | D3D12 最终只开 `Filter_Shadow`（单 pass），**多 pass 的 Box1/Box2 大半径路径在 D3D12 上尚未被实际跑到**；Vulkan 侧（同一份 XML）在 10/28 重生成后跑过多 pass 滤镜，clamp 的价值主要在那里体现 |

---

## 6. 经验

1. **生成器是"隐式契约"的源头。** HAL 的错误会被校验层抓住；生成器的错误（编译器选错、循环无界、格式不符）只会表现为"渲染玄学"。移植新后端时，**先把生成器跑通，再谈 HAL**。
2. **fxc 与 dxc 不可混用。** D3D12 用 SM 6.x + `[RootSignature]`，必须 dxc（DXIL）；fxc 只到 SM 5.1，且不认该属性。选项风格也不同（`/Zi` vs `-Zi`）。
3. **动态循环必须在着色器里 clamp。** 循环次数由内容（SWF 参数）决定时，等于把 GPU 的生死交给美术资源；clamp 是唯一可靠的护栏。
4. **改 XML 必须全平台重生成**，并把生成物一并提交；只生成当前后端，会在别的后端留下"旧逻辑"。
5. **保留 ISSUE 快照是好习惯。** 这次能给出"8 行差异 / 27 行差异"的精确结论，全靠 `ShaderDataISSUE.xml` 与 `Platform_D3D12_ISSUES.cs` 这两份对照基线。

---

## 附录 · 对照文件

| 修复前 | 修复后 | 差异量 |
|---|---|---|
| `Src\Render\ShaderDataISSUE.xml` | `Src\Render\ShaderData.xml` | 8 行（blur clamp） |
| `GFxShaderMaker.Platforms\Platform_D3D12_ISSUES.cs` | `GFxShaderMaker.Platforms\Platform_D3D12.cs` | 27 行（dxc + 选项） |
| `D3D12_ShaderDescs.h.dxbc.bak`（9/14） | `D3D12_ShaderDescs.h/.cpp`（10/26 09:52） | 整份 DXIL 重生成 |
