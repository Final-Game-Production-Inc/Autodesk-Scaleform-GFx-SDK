# artifacts · 证据目录

本目录存放实验产出的**可复现证据**，按里程碑分子目录：

```
artifacts\
  baseline\      T02 冻结的基线截图（D3D11 与 Vulkan-滤镜关，同状态同分辨率）
  swap\          E1 交换链终帧回读（swap_f<frame>.bmp）
  rt\            E3 每个滤镜 pass 的临时 RT 回读（rt_f<frame>_p<pass>_t<type>.bmp）
  bisect\        E2 类型二分矩阵（每种组合一张截图 + 结果表）
  logs\          soak 的 stderr/stdout 全文
  reports\       M1 判定报告、M2 根因报告、M3 回归报告
```

约定：

1. 每个文件名带 **帧号** 或 **时间戳**，且与实验台账（03 §7）记录一致；
2. 每份证据必须能对应一个 **exe 时间戳**（写进报告或文件名）；
3. 二进制证据（BMP/PNG/dump）不入文档目录，只在本目录；
4. 里程碑结束时把关键证据（≤ 5 个）复制进报告，其余保留在本目录。
