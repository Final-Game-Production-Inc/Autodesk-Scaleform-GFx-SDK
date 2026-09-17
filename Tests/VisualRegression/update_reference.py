#!/usr/bin/env python3
"""
update_reference.py  -  Refresh reference images from latest captures.

Run this after making Vulkan renderer changes and verifying correctness.
Copies PNG captures from Output/Captures/ to Reference/ for all tests
listed in TestManifest.json.

Usage:
  update_reference.py [--renderer d3d11|vulkan|both]
                      [--captures-dir Output/Captures]
                      [--reference-dir Reference]
                      [--manifest TestManifest.json]

Typical workflow:
  1. Make Vulkan code changes
  2. Rebuild (build_tests.ps1)
  3. Run captures (run_all.py)
  4. Verify results look correct (inspect Output/Captures/)
  5. Run: python update_reference.py --renderer vulkan
  6. Commit updated Reference/vulkan/ PNGs
"""

import sys
import os
import shutil
import json
import argparse

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def main():
    ap = argparse.ArgumentParser(description='Update reference images from captures')
    ap.add_argument('--renderer', default='both', choices=['d3d11', 'vulkan', 'both'],
                    help='Which renderer reference to update (default: both)')
    ap.add_argument('--captures-dir', default='',
                    help='Captures directory (default: Output/Captures)')
    ap.add_argument('--reference-dir', default='',
                    help='Reference directory (default: Reference)')
    ap.add_argument('--manifest', default='',
                    help='Path to TestManifest.json')
    args = ap.parse_args()

    manifest_path = args.manifest or os.path.join(SCRIPT_DIR, 'TestManifest.json')
    captures_dir  = args.captures_dir or os.path.join(SCRIPT_DIR, 'Output', 'Captures')
    reference_dir = args.reference_dir or os.path.join(SCRIPT_DIR, 'Reference')

    with open(manifest_path) as f:
        manifest = json.load(f)

    tests          = manifest.get('tests', [])
    default_frames = manifest.get('defaultFrames', [1, 10, 30, 60])

    renderers = ['d3d11', 'vulkan'] if args.renderer == 'both' else [args.renderer]

    total_copied = 0
    total_missing = 0

    for renderer in renderers:
        print(f'\n=== Updating {renderer} reference ===')
        for test in tests:
            name   = test['name']
            frames = sorted(test.get('frames', default_frames))

            src_dir = os.path.join(captures_dir, renderer, name)
            dst_dir = os.path.join(reference_dir, renderer, name)

            if not os.path.isdir(src_dir):
                print(f'  [SKIP] {name}: capture dir not found: {src_dir}')
                total_missing += 1
                continue

            os.makedirs(dst_dir, exist_ok=True)
            copied = 0
            for frame in frames:
                png_name = f'frame_{frame:04d}.png'
                src = os.path.join(src_dir, png_name)
                dst = os.path.join(dst_dir, png_name)
                if os.path.exists(src):
                    shutil.copy2(src, dst)
                    copied += 1
                else:
                    print(f'  [WARN] {name}: missing {png_name}')
                    total_missing += 1

            total_copied += copied

    print(f'\nDone: {total_copied} files copied, {total_missing} missing')
    if total_missing > 0:
        print('Run run_all.py first to generate captures, then retry.')
        sys.exit(1)


if __name__ == '__main__':
    main()
