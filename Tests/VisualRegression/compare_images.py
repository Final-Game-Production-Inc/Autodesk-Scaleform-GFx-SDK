#!/usr/bin/env python3
"""
compare_images.py  -  Per-pixel comparison of two PPM/PNG images.

Reads two image files (PPM P6 binary or PNG 8-bit RGB/RGBA), computes
pixel-level differences, and outputs a JSON result to stdout.

Writes alongside each input image:
  - A .png version of the image (skipped when input is already PNG)
  - A diff PNG (4x amplified, written only when mismatches exist)

Output JSON:
  { pass, rmse, maxDelta, mismatchCount, mismatchPercent, threshold, width, height }

Exit code: 0 = pass, 1 = fail/error.

No external dependencies - only Python stdlib (struct, zlib, math).
"""

import sys
import os
import struct
import zlib
import json
import math
import argparse


# ---------------------------------------------------------------------------
# PPM reader
# ---------------------------------------------------------------------------

def read_ppm(path):
    """Read a P6 binary PPM file. Returns (width, height, pixel_bytes)."""
    with open(path, 'rb') as f:
        data = f.read()

    idx = [0]  # mutable container so nested functions can modify it

    def skip():
        while idx[0] < len(data):
            b = data[idx[0]:idx[0] + 1]
            if b == b'#':
                while idx[0] < len(data) and data[idx[0]:idx[0] + 1] != b'\n':
                    idx[0] += 1
            elif b in (b' ', b'\t', b'\n', b'\r'):
                idx[0] += 1
            else:
                break

    def token():
        skip()
        start = idx[0]
        while idx[0] < len(data) and data[idx[0]:idx[0] + 1] not in (b' ', b'\t', b'\n', b'\r'):
            idx[0] += 1
        return data[start:idx[0]].decode('ascii')

    magic  = token()
    width  = int(token())
    height = int(token())
    maxval = int(token())

    if magic != 'P6':
        raise ValueError(f'Not a P6 PPM: {path!r} (magic={magic!r})')
    if maxval != 255:
        raise ValueError(f'Unsupported maxval {maxval} in {path!r} (only 255 supported)')

    idx[0] += 1  # skip exactly one whitespace byte after maxval
    pixels = data[idx[0]:]
    expected = width * height * 3
    if len(pixels) < expected:
        raise ValueError(
            f'PPM pixel data too short in {path!r}: expected {expected} bytes, got {len(pixels)}'
        )
    return width, height, pixels


# ---------------------------------------------------------------------------
# PNG reader (pure stdlib - struct + zlib, no Pillow required)
# ---------------------------------------------------------------------------

def read_png(path):
    """Read an 8-bit RGB or RGBA PNG file. Returns (width, height, rgb_bytes).

    Only supports bit_depth=8, color_type=2 (RGB) or 6 (RGBA).
    RGBA images have their alpha channel stripped to return RGB only.
    """
    with open(path, 'rb') as f:
        data = f.read()

    # Validate PNG signature
    sig = b'\x89PNG\r\n\x1a\n'
    if data[:8] != sig:
        raise ValueError(f'Not a PNG file: {path!r}')

    # Parse chunks
    pos = 8
    ihdr_parsed = False
    width = height = bit_depth = color_type = 0
    idat_chunks = []

    while pos < len(data):
        if pos + 8 > len(data):
            break
        length = struct.unpack('>I', data[pos:pos + 4])[0]
        tag = data[pos + 4:pos + 8]
        chunk_data = data[pos + 8:pos + 8 + length]
        # skip CRC (4 bytes after chunk data)
        pos = pos + 8 + length + 4

        if tag == b'IHDR':
            if len(chunk_data) < 13:
                raise ValueError(f'Truncated IHDR in {path!r}')
            width, height, bit_depth, color_type, comp, filt, interlace = \
                struct.unpack('>IIBBBBB', chunk_data[:13])
            if bit_depth != 8:
                raise ValueError(
                    f'Unsupported bit depth {bit_depth} in {path!r} (only 8 supported)')
            if color_type not in (2, 6):
                raise ValueError(
                    f'Unsupported color type {color_type} in {path!r} '
                    f'(only RGB=2 and RGBA=6 supported)')
            if interlace != 0:
                raise ValueError(f'Interlaced PNGs not supported: {path!r}')
            ihdr_parsed = True
        elif tag == b'IDAT':
            idat_chunks.append(chunk_data)
        elif tag == b'IEND':
            break

    if not ihdr_parsed:
        raise ValueError(f'No IHDR chunk found in {path!r}')
    if not idat_chunks:
        raise ValueError(f'No IDAT chunks found in {path!r}')

    # Decompress concatenated IDAT data
    raw = zlib.decompress(b''.join(idat_chunks))

    # Determine bytes per pixel and stride
    bpp = 3 if color_type == 2 else 4
    stride = width * bpp
    expected_raw = height * (1 + stride)  # 1 filter byte per row
    if len(raw) < expected_raw:
        raise ValueError(
            f'Decompressed data too short in {path!r}: '
            f'expected {expected_raw}, got {len(raw)}')

    # Reverse PNG row filters
    def paeth(a, b, c):
        p = a + b - c
        pa = abs(p - a)
        pb = abs(p - b)
        pc = abs(p - c)
        if pa <= pb and pa <= pc:
            return a
        elif pb <= pc:
            return b
        else:
            return c

    prev_row = bytearray(stride)
    pixels = bytearray(height * stride)

    for y in range(height):
        row_start = y * (1 + stride)
        filter_type = raw[row_start]
        cur_raw = raw[row_start + 1:row_start + 1 + stride]
        cur_row = bytearray(stride)

        if filter_type == 0:  # None
            cur_row[:] = cur_raw
        elif filter_type == 1:  # Sub
            for x in range(stride):
                a = cur_row[x - bpp] if x >= bpp else 0
                cur_row[x] = (cur_raw[x] + a) & 0xFF
        elif filter_type == 2:  # Up
            for x in range(stride):
                cur_row[x] = (cur_raw[x] + prev_row[x]) & 0xFF
        elif filter_type == 3:  # Average
            for x in range(stride):
                a = cur_row[x - bpp] if x >= bpp else 0
                b = prev_row[x]
                cur_row[x] = (cur_raw[x] + ((a + b) >> 1)) & 0xFF
        elif filter_type == 4:  # Paeth
            for x in range(stride):
                a = cur_row[x - bpp] if x >= bpp else 0
                b = prev_row[x]
                c = prev_row[x - bpp] if x >= bpp else 0
                cur_row[x] = (cur_raw[x] + paeth(a, b, c)) & 0xFF
        else:
            raise ValueError(
                f'Unknown PNG filter type {filter_type} at row {y} in {path!r}')

        pixels[y * stride:(y + 1) * stride] = cur_row
        prev_row = cur_row

    # If RGBA, strip alpha to return RGB only
    if color_type == 6:
        rgb = bytearray(width * height * 3)
        for i in range(width * height):
            rgb[i * 3]     = pixels[i * 4]
            rgb[i * 3 + 1] = pixels[i * 4 + 1]
            rgb[i * 3 + 2] = pixels[i * 4 + 2]
        return width, height, bytes(rgb)

    return width, height, bytes(pixels)


# ---------------------------------------------------------------------------
# Image dispatcher
# ---------------------------------------------------------------------------

def read_image(path):
    """Read a PPM or PNG image file. Returns (width, height, rgb_bytes).

    Dispatches to read_ppm or read_png based on file extension.
    """
    ext = os.path.splitext(path)[1].lower()
    if ext == '.ppm':
        return read_ppm(path)
    elif ext == '.png':
        return read_png(path)
    else:
        raise ValueError(f'Unsupported image format {ext!r} for {path!r} (expected .ppm or .png)')


# ---------------------------------------------------------------------------
# PNG writer (pure stdlib - struct + zlib, no Pillow required)
# ---------------------------------------------------------------------------

def write_png(path, width, height, rgb_bytes):
    """Write an RGB PNG file from raw RGB byte data (no external dependencies)."""

    def chunk(tag, data):
        c = tag + data
        return struct.pack('>I', len(data)) + c + struct.pack('>I', zlib.crc32(c) & 0xFFFFFFFF)

    # Apply filter type 0 (None) to each scanline
    row = width * 3
    raw = bytearray()
    for y in range(height):
        raw.append(0)  # filter type byte
        raw.extend(rgb_bytes[y * row:(y + 1) * row])

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)))
        f.write(chunk(b'IDAT', zlib.compress(bytes(raw), 6)))
        f.write(chunk(b'IEND', b''))


def ppm_to_png_path(ppm_path):
    return os.path.splitext(ppm_path)[0] + '.png'


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(
        description='Compare two PPM/PNG images pixel by pixel; writes PNG diff output.'
    )
    ap.add_argument('reference',    help='Reference image path (PPM or PNG)')
    ap.add_argument('test',         help='Test image path (PPM or PNG)')
    ap.add_argument('--diff',       default='', help='Output diff image path (PPM or PNG)')
    ap.add_argument('--threshold',  type=int, default=8,
                    help='Per-pixel sum-of-channels threshold for mismatch (default: 8)')
    args = ap.parse_args()

    try:
        rw, rh, rpx = read_image(args.reference)
    except Exception as e:
        print(json.dumps({'pass': False, 'error': f'Failed to read reference: {e}'}))
        sys.exit(1)

    try:
        tw, th, tpx = read_image(args.test)
    except Exception as e:
        print(json.dumps({'pass': False, 'error': f'Failed to read test: {e}'}))
        sys.exit(1)

    if rw != tw or rh != th:
        print(json.dumps({
            'pass':  False,
            'error': f'Dimension mismatch: reference={rw}x{rh}, test={tw}x{th}',
        }))
        sys.exit(1)

    w, h  = rw, rh
    n     = w * h
    sum_sq        = 0.0
    max_delta     = 0
    mismatch_count = 0
    diff_data     = bytearray(n * 3) if args.diff else None

    for i in range(n):
        o  = i * 3
        dr = abs(int(rpx[o])     - int(tpx[o]))
        dg = abs(int(rpx[o + 1]) - int(tpx[o + 1]))
        db = abs(int(rpx[o + 2]) - int(tpx[o + 2]))
        s  = dr + dg + db
        sum_sq += dr * dr + dg * dg + db * db
        if s > max_delta:
            max_delta = s
        if s > args.threshold:
            mismatch_count += 1
        if diff_data is not None:
            diff_data[o]     = min(255, dr * 4)
            diff_data[o + 1] = min(255, dg * 4)
            diff_data[o + 2] = min(255, db * 4)

    rmse         = math.sqrt(sum_sq / (n * 3))
    mismatch_pct = round(mismatch_count / n * 100, 4)
    passed       = mismatch_count == 0 or rmse <= args.threshold / 3.0

    # Write PNG versions of reference and test for easy viewing (skip if already PNG)
    ref_ext = os.path.splitext(args.reference)[1].lower()
    test_ext = os.path.splitext(args.test)[1].lower()
    if ref_ext != '.png':
        write_png(ppm_to_png_path(args.reference), w, h, bytes(rpx[:n * 3]))
    if test_ext != '.png':
        write_png(ppm_to_png_path(args.test),      w, h, bytes(tpx[:n * 3]))

    # Write diff output when there are any mismatches
    if diff_data is not None and mismatch_count > 0:
        diff_dir = os.path.dirname(os.path.abspath(args.diff))
        os.makedirs(diff_dir, exist_ok=True)
        diff_ext = os.path.splitext(args.diff)[1].lower()
        if diff_ext == '.png':
            # Write PNG diff directly
            write_png(args.diff, w, h, bytes(diff_data))
        else:
            # Write PPM diff + a PNG copy alongside
            with open(args.diff, 'wb') as f:
                f.write(f'P6\n{w} {h}\n255\n'.encode('ascii'))
                f.write(bytes(diff_data))
            write_png(ppm_to_png_path(args.diff), w, h, bytes(diff_data))

    result = {
        'pass':            passed,
        'rmse':            round(rmse, 4),
        'maxDelta':        max_delta,
        'mismatchCount':   mismatch_count,
        'mismatchPercent': mismatch_pct,
        'threshold':       args.threshold,
        'width':           w,
        'height':          h,
    }
    print(json.dumps(result))
    sys.exit(0 if passed else 1)


if __name__ == '__main__':
    main()
