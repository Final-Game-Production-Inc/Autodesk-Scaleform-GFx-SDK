#!/usr/bin/env python3
"""
run_all.py  -  Visual regression test orchestrator.

Runs capture executables for each renderer, saves logs alongside captures,
then compares frame pairs and writes a results summary.

Output structure:
  Output/
    Captures/<renderer>/<TestName>/frame_NNNN.ppm + .png
    Logs/<renderer>/<TestName>.log
    Diff/<ref>_vs_<test>/<TestName>/frame_NNNN_diff.ppm + .png
    results.json

Usage:
  run_all.py
      [--manifest TestManifest.json]
      [--data-dir  <path to Bin/Data>]
      [--bin-dir   <path to capture executables>]
      [--output-dir <output root>]
      [--threshold 8]
      [--skip-renderer <name>]   # may be repeated, e.g. --skip-renderer d3d11
      [--only-compare]           # skip all captures, only run pixel comparisons

Renderer / comparison configuration comes from TestManifest.json (see
'renderers' and 'comparisons' keys). Defaults to D3D11 as reference and
Vulkan as test when those keys are absent.
"""

import sys
import os
import json
import subprocess
import argparse
import datetime

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
EXE_EXT    = '.exe' if sys.platform == 'win32' else ''

# Defaults used when the manifest does not specify renderers/comparisons
DEFAULT_RENDERERS = {
    'd3d11':  {'exe': 'CaptureD3D11'},
    'vulkan': {'exe': 'CaptureVulkan'},
}
DEFAULT_COMPARISONS = [{'reference': 'd3d11', 'test': 'vulkan'}]


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def run_capture(exe_path, swf_path, output_dir, frames, width, height, log_path,
                timeout=15):
    """
    Run a capture executable and save its combined stdout+stderr to log_path.
    Returns True if the process exited 0.
    """
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(os.path.dirname(log_path), exist_ok=True)

    frames_str = ','.join(str(f) for f in frames)
    cmd = [
        exe_path, swf_path, output_dir,
        '--frames', frames_str,
        '--width',  str(width),
        '--height', str(height),
    ]
    print(f'  Running: {" ".join(cmd)}')

    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                errors='replace', timeout=timeout)
    except subprocess.TimeoutExpired:
        print(f'  [TIMEOUT] Process killed after {timeout}s')
        with open(log_path, 'w', encoding='utf-8') as f:
            f.write(f'Command: {" ".join(cmd)}\n')
            f.write(f'Exit code: TIMEOUT ({timeout}s)\n')
            f.write(f'Timestamp: {datetime.datetime.now().isoformat()}\n')
        return False

    # Echo stdout live; print stderr to our stderr (validation messages, errors)
    if result.stdout:
        print(result.stdout, end='')
    if result.stderr:
        print(result.stderr, end='', file=sys.stderr)

    # Save combined log
    with open(log_path, 'w', encoding='utf-8') as f:
        f.write(f'Command: {" ".join(cmd)}\n')
        f.write(f'Exit code: {result.returncode}\n')
        f.write(f'Timestamp: {datetime.datetime.now().isoformat()}\n\n')
        if result.stdout:
            f.write('=== STDOUT ===\n')
            f.write(result.stdout)
            f.write('\n')
        if result.stderr:
            f.write('=== STDERR ===\n')
            f.write(result.stderr)
            f.write('\n')

    return result.returncode == 0


def run_compare(compare_script, ref_ppm, tst_ppm, diff_ppm, threshold):
    """
    Run compare_images.py and parse its JSON output.
    Returns a dict with at least 'pass' key.
    """
    cmd = [
        sys.executable, compare_script,
        ref_ppm, tst_ppm,
        '--diff',      diff_ppm,
        '--threshold', str(threshold),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    try:
        return json.loads(result.stdout)
    except Exception:
        stderr_snippet = result.stderr[:300] if result.stderr else '(no stderr)'
        return {'pass': False, 'error': f'compare_images.py failed: {stderr_snippet}'}


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description='Visual regression test runner')
    ap.add_argument('--manifest',       default='',
                    help='Path to TestManifest.json')
    ap.add_argument('--data-dir',       default='',
                    help='Root data directory containing SWF files')
    ap.add_argument('--bin-dir',        default='',
                    help='Directory containing capture executables')
    ap.add_argument('--output-dir',     default='',
                    help='Root output directory')
    ap.add_argument('--threshold',      type=int, default=8,
                    help='Per-pixel mismatch threshold (default: 8)')
    ap.add_argument('--skip-renderer',  action='append', default=[], metavar='NAME',
                    help='Skip capture for this renderer (repeatable)')
    ap.add_argument('--only-compare',   action='store_true',
                    help='Skip all captures; only run pixel comparisons')
    ap.add_argument('--timeout',        type=int, default=15,
                    help='Per-capture timeout in seconds (default: 15)')
    args = ap.parse_args()

    # Resolve paths
    manifest_path  = args.manifest   or os.path.join(SCRIPT_DIR, 'TestManifest.json')
    data_dir       = args.data_dir   or os.path.join(SCRIPT_DIR, '..', '..', 'Bin', 'Data')
    bin_dir        = args.bin_dir    or os.path.join(SCRIPT_DIR, '..', '..', 'Bin', 'x64', 'Msvc10', 'Tests')
    output_dir     = args.output_dir or os.path.join(SCRIPT_DIR, 'Output')
    compare_script = os.path.join(SCRIPT_DIR, 'compare_images.py')

    data_dir   = os.path.realpath(data_dir)
    bin_dir    = os.path.realpath(bin_dir)
    output_dir = os.path.realpath(output_dir)

    # Load manifest
    if not os.path.exists(manifest_path):
        print(f'ERROR: Manifest not found: {manifest_path}', file=sys.stderr)
        sys.exit(1)
    with open(manifest_path) as f:
        manifest = json.load(f)

    renderers      = manifest.get('renderers',          DEFAULT_RENDERERS)
    comparisons    = manifest.get('comparisons',         DEFAULT_COMPARISONS)
    default_res    = manifest.get('defaultResolution',   {'width': 1024, 'height': 768})
    default_frames = manifest.get('defaultFrames',       [1, 10, 30, 60])
    tests          = manifest.get('tests',               [])

    # Build exe map: renderer_name -> full path to executable
    exe_map = {
        name: os.path.join(bin_dir, cfg['exe'] + EXE_EXT)
        for name, cfg in renderers.items()
    }

    skip_set = set(args.skip_renderer)
    if args.only_compare:
        skip_set = set(renderers.keys())

    # Determine which renderers are actually referenced in comparisons
    active_renderers = {c['reference'] for c in comparisons} | {c['test'] for c in comparisons}

    all_results  = []
    passed_count = 0

    for test in tests:
        name   = test['name']
        swf    = test['swf']
        frames = sorted(test.get('frames', default_frames))
        res    = test.get('resolution', default_res)
        width  = res['width']
        height = res['height']

        swf_path = os.path.join(data_dir, swf)
        print(f'\n=== {name} ===')

        # ---- Capture phase ----
        capture_dirs = {}   # renderer_name -> output directory for this test
        for rname in active_renderers:
            rdir = os.path.join(output_dir, 'Captures', rname, name)
            capture_dirs[rname] = rdir

            if rname in skip_set:
                continue

            exe = exe_map.get(rname)
            if not exe:
                print(f'  [WARNING] Unknown renderer "{rname}" - skipping capture')
                continue
            if not os.path.exists(exe):
                print(f'  [WARNING] Executable not found: {exe} - skipping capture')
                continue

            log_path = os.path.join(output_dir, 'Logs', rname, f'{name}.log')
            print(f'  [{rname}] Capturing...')
            run_capture(exe, swf_path, rdir, frames, width, height, log_path,
                        timeout=args.timeout)

        # ---- Compare phase ----
        test_pass    = True
        frame_results = []

        for cmp_pair in comparisons:
            ref_name   = cmp_pair['reference']
            tst_name   = cmp_pair['test']
            pair_label = f'{ref_name}_vs_{tst_name}'
            ref_dir    = capture_dirs.get(ref_name, '')
            tst_dir    = capture_dirs.get(tst_name, '')
            diff_dir   = os.path.join(output_dir, 'Diff', pair_label, name)

            for f in frames:
                frame_str = f'{f:04d}'
                ref_ppm   = os.path.join(ref_dir,  f'frame_{frame_str}.ppm')
                tst_ppm   = os.path.join(tst_dir,  f'frame_{frame_str}.ppm')
                diff_ppm  = os.path.join(diff_dir, f'frame_{frame_str}_diff.ppm')

                if not os.path.exists(ref_ppm) or not os.path.exists(tst_ppm):
                    missing = []
                    if not os.path.exists(ref_ppm): missing.append(f'{ref_name}')
                    if not os.path.exists(tst_ppm): missing.append(f'{tst_name}')
                    print(f'    Frame {f} [{pair_label}]: [SKIP] missing: {", ".join(missing)}')
                    continue

                os.makedirs(diff_dir, exist_ok=True)
                cmp = run_compare(compare_script, ref_ppm, tst_ppm, diff_ppm, args.threshold)
                ok  = cmp.get('pass', False)
                if not ok:
                    test_pass = False

                status   = 'PASS' if ok else 'FAIL'
                rmse     = cmp.get('rmse', '?')
                mismatch = cmp.get('mismatchCount', '?')
                print(f'    Frame {f} [{pair_label}]: [{status}]  rmse={rmse}  mismatch={mismatch}px')
                frame_results.append({'frame': f, 'comparison': pair_label, **cmp})

        if test_pass:
            passed_count += 1
        print(f"  {name}: [{'PASS' if test_pass else 'FAIL'}]")
        all_results.append({'name': name, 'pass': test_pass, 'frames': frame_results})

    # ---- Write results.json ----
    overall = passed_count == len(tests)
    summary = {
        'timestamp': datetime.datetime.now().strftime('%Y-%m-%dT%H:%M:%S'),
        'threshold': args.threshold,
        'passed':    passed_count,
        'total':     len(tests),
        'overall':   'PASS' if overall else 'FAIL',
        'tests':     all_results,
    }
    os.makedirs(output_dir, exist_ok=True)
    results_path = os.path.join(output_dir, 'results.json')
    with open(results_path, 'w') as f:
        json.dump(summary, f, indent=2)

    print(f'\nResults written to: {results_path}')
    print(f'\n{"=" * 38}')
    print(f"  OVERALL: {'PASS' if overall else 'FAIL'}  ({passed_count} / {len(tests)} tests passed)")
    print(f'{"=" * 38}')
    sys.exit(0 if overall else 1)


if __name__ == '__main__':
    main()
