# Visual Regression Tests

Pixel-level comparison of D3D11 and Vulkan renderers for the Scaleform GFx 4.2 SDK.

Each test renders the same SWF through both renderers at fixed frame numbers and
compares the output images. Differences are reported as RMSE and mismatch pixel
counts; amplified diff images are written for any failing frames.

## Prerequisites

- Python 3.6+
- Visual Studio 2022 with C++ workload (for building capture tools)
- Vulkan SDK with `VK_SDK_PATH` set (for CaptureVulkan)

## Quick Start

**1. Build the capture tools** (first time only, Windows):

```powershell
cd Tests\VisualRegression
.\build_tests.ps1
```

This produces `Bin\x64\Msvc10\Tests\CaptureD3D11.exe` and `CaptureVulkan.exe`.

**2. Run the full test suite:**

```bash
python Tests/VisualRegression/run_all.py
```

**3. Re-run comparisons without re-capturing** (e.g. after tweaking the threshold):

```bash
python Tests/VisualRegression/run_all.py --only-compare
```

## Output Structure

All generated output lives in `Tests/VisualRegression/Output/` (gitignored):

```
Output/
  Captures/<renderer>/<TestName>/
      frame_0001.ppm          # raw pixel data (used for comparison)
      frame_0001.png          # PNG copy for human inspection
      ...
  Logs/<renderer>/<TestName>.log
      # Full stdout + stderr from the capture executable
  Diff/<ref>_vs_<test>/<TestName>/
      frame_0001_diff.ppm     # amplified (4×) difference image
      frame_0001_diff.png     # PNG version
      ...
  results.json                # machine-readable summary of all tests
```

## Command-Line Options

### run_all.py

| Option | Default | Description |
|--------|---------|-------------|
| `--manifest` | `TestManifest.json` | Path to the test manifest |
| `--data-dir` | `Bin/Data` | Root of SWF data files |
| `--bin-dir` | `Bin/x64/Msvc10/Tests` | Directory with capture executables |
| `--output-dir` | `Output` | Root output directory |
| `--threshold` | `8` | Per-pixel sum-of-channels mismatch threshold |
| `--skip-renderer NAME` | — | Skip capturing with this renderer (repeatable) |
| `--only-compare` | — | Skip all captures; only run comparisons |

### compare_images.py

Can also be used standalone:

```bash
python Tests/VisualRegression/compare_images.py ref.ppm test.ppm \
    --diff diff.ppm --threshold 8
```

Outputs JSON to stdout and writes PNG versions of all three images.

## Adding Tests

Edit `TestManifest.json`. Each entry needs a `name` and a `swf` path relative to
`Bin/Data`:

```json
{ "name": "MyWidget", "swf": "AS2/Samples/MyWidget.swf" }
```

Optional per-test overrides: `"frames": [1, 5, 10]`, `"resolution": {"width": 800, "height": 600}`.

## Configuring Renderers

The `renderers` and `comparisons` keys in `TestManifest.json` control which
executables are used. The defaults compare D3D11 (reference) against Vulkan (test):

```json
"renderers": {
    "d3d11":  { "exe": "CaptureD3D11" },
    "vulkan": { "exe": "CaptureVulkan" }
},
"comparisons": [
    { "reference": "d3d11", "test": "vulkan" }
]
```

To add a second renderer or swap reference/test, add entries here without touching
`run_all.py`.

## Files

| File | Purpose |
|------|---------|
| `CaptureD3D11.cpp` | Headless D3D11 frame capture tool |
| `CaptureVulkan.cpp` | Headless Vulkan frame capture tool |
| `compare_images.py` | Per-pixel PPM comparison; writes PNG output |
| `run_all.py` | Test orchestrator |
| `build_tests.ps1` | Builds capture executables (Windows / MSBuild) |
| `TestManifest.json` | Test suite definition |
| `.gitignore` | Excludes `Output/` from version control |
