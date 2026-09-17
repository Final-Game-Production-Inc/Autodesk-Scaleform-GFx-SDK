#!/usr/bin/env python3
"""
compare_reference.py  -  Compare committed reference images (no GPU needed).

Reads TestManifest.json and compares D3D11 vs Vulkan reference PNGs from
the Reference/ directory. Intended for CI where no GPU is available.

Can also compare live Vulkan captures against D3D11 reference when run
after CaptureVulkan with --vulkan-dir pointing to fresh captures.

Usage:
  compare_reference.py [--reference-dir Reference]
                       [--vulkan-dir Reference/vulkan]
                       [--threshold 8]
                       [--manifest TestManifest.json]

Exit code: 0 = all pass, 1 = any failure.
"""

import sys
import os
import json
import math
import argparse

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)
from compare_images import read_image, write_png


def compare_pair(ref_path, test_path, diff_path, threshold):
    """Compare two images directly. Returns result dict."""
    rw, rh, rpx = read_image(ref_path)
    tw, th, tpx = read_image(test_path)

    if rw != tw or rh != th:
        return {
            'pass': False,
            'error': f'Dimension mismatch: reference={rw}x{rh}, test={tw}x{th}',
            'rmse': 0, 'maxDelta': 0, 'mismatchCount': 0,
        }

    w, h = rw, rh
    n = w * h
    sum_sq = 0.0
    max_delta = 0
    mismatch_count = 0
    diff_data = bytearray(n * 3) if diff_path else None

    for i in range(n):
        o = i * 3
        dr = abs(int(rpx[o])     - int(tpx[o]))
        dg = abs(int(rpx[o + 1]) - int(tpx[o + 1]))
        db = abs(int(rpx[o + 2]) - int(tpx[o + 2]))
        s = dr + dg + db
        sum_sq += dr * dr + dg * dg + db * db
        if s > max_delta:
            max_delta = s
        if s > threshold:
            mismatch_count += 1
        if diff_data is not None:
            diff_data[o]     = min(255, dr * 4)
            diff_data[o + 1] = min(255, dg * 4)
            diff_data[o + 2] = min(255, db * 4)

    rmse = math.sqrt(sum_sq / (n * 3))
    passed = mismatch_count == 0 or rmse <= threshold / 3.0

    if diff_data is not None and mismatch_count > 0:
        os.makedirs(os.path.dirname(os.path.abspath(diff_path)), exist_ok=True)
        write_png(diff_path, w, h, bytes(diff_data))

    return {
        'pass':            passed,
        'rmse':            round(rmse, 4),
        'maxDelta':        max_delta,
        'mismatchCount':   mismatch_count,
        'mismatchPercent': round(mismatch_count / n * 100, 4),
        'threshold':       threshold,
        'width':           w,
        'height':          h,
    }


def main():
    ap = argparse.ArgumentParser(description='Compare reference images (no GPU needed)')
    ap.add_argument('--manifest', default='',
                    help='Path to TestManifest.json')
    ap.add_argument('--reference-dir', default='',
                    help='Root reference directory (default: Reference/)')
    ap.add_argument('--vulkan-dir', default='',
                    help='Vulkan captures directory (default: <reference-dir>/vulkan)')
    ap.add_argument('--threshold', type=int, default=8,
                    help='Per-pixel mismatch threshold (default: 8)')
    ap.add_argument('--diff-dir', default='',
                    help='Output directory for diff images (default: Output/Diff)')
    ap.add_argument('--exclude', nargs='*', default=[],
                    help='Test names to exclude from comparison')
    args = ap.parse_args()

    manifest_path = args.manifest or os.path.join(SCRIPT_DIR, 'TestManifest.json')
    reference_dir = args.reference_dir or os.path.join(SCRIPT_DIR, 'Reference')
    vulkan_dir    = args.vulkan_dir or os.path.join(reference_dir, 'vulkan')
    d3d11_dir     = os.path.join(reference_dir, 'd3d11')
    diff_dir      = args.diff_dir or os.path.join(SCRIPT_DIR, 'Output', 'Diff')

    if not os.path.exists(manifest_path):
        print(f'ERROR: Manifest not found: {manifest_path}', file=sys.stderr)
        sys.exit(1)

    with open(manifest_path) as f:
        manifest = json.load(f)

    tests          = manifest.get('tests', [])
    default_frames = manifest.get('defaultFrames', [1, 10, 30, 60])

    all_results  = []
    passed_count = 0
    total_frames = 0
    failed_frames = 0

    exclude_set = set(args.exclude)
    skipped_count = 0
    for test in tests:
        name   = test['name']
        frames = sorted(test.get('frames', default_frames))

        if name in exclude_set:
            print(f'\n=== {name} === [SKIP — excluded]')
            skipped_count += 1
            continue

        print(f'\n=== {name} ===')
        test_pass = True

        for frame in frames:
            frame_str = f'{frame:04d}'
            ref_png   = os.path.join(d3d11_dir, name, f'frame_{frame_str}.png')
            vk_png    = os.path.join(vulkan_dir, name, f'frame_{frame_str}.png')
            diff_png  = os.path.join(diff_dir, 'd3d11_vs_vulkan', name, f'frame_{frame_str}_diff.png')

            total_frames += 1

            if not os.path.exists(ref_png):
                print(f'    Frame {frame}: [FAIL] missing D3D11 reference: {ref_png}')
                test_pass = False
                failed_frames += 1
                continue
            if not os.path.exists(vk_png):
                print(f'    Frame {frame}: [FAIL] missing Vulkan image: {vk_png}')
                test_pass = False
                failed_frames += 1
                continue

            try:
                cmp = compare_pair(ref_png, vk_png, diff_png, args.threshold)
            except Exception as e:
                cmp = {'pass': False, 'error': str(e), 'rmse': '?', 'mismatchCount': '?'}
                print(f'      compare error: {e}')

            if 'error' in cmp:
                print(f'      error: {cmp["error"]}')

            ok = cmp.get('pass', False)
            if not ok:
                test_pass = False
                failed_frames += 1

            status   = 'PASS' if ok else 'FAIL'
            rmse     = cmp.get('rmse', '?')
            mismatch = cmp.get('mismatchCount', '?')
            print(f'    Frame {frame}: [{status}]  rmse={rmse}  mismatch={mismatch}px')

        if test_pass:
            passed_count += 1
        print(f"  {name}: [{'PASS' if test_pass else 'FAIL'}]")
        all_results.append({'name': name, 'pass': test_pass})

    tested = len(tests) - skipped_count
    overall = passed_count == tested
    print(f'\n{"=" * 40}')
    print(f"  OVERALL: {'PASS' if overall else 'FAIL'}  ({passed_count}/{tested} tests, {skipped_count} skipped)")
    if failed_frames > 0:
        print(f"  Failed frames: {failed_frames}/{total_frames}")
    print(f'{"=" * 40}')
    sys.exit(0 if overall else 1)


if __name__ == '__main__':
    main()
