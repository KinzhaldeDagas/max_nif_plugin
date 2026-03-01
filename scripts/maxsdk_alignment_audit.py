#!/usr/bin/env python3
"""Audit Max NIF plugin source includes against the local 3dsmaxtrain SDK include tree.

Usage:
    python scripts/maxsdk_alignment_audit.py
"""

from __future__ import annotations

import os
import pathlib
import re
from collections import Counter

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
SDK_INCLUDE_ROOT = REPO_ROOT / "3dsmaxtrain" / "include"
SOURCE_EXTS = {".h", ".hpp", ".c", ".cc", ".cxx", ".cpp"}
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<\"]([^>\"]+)[>\"]')


def iter_source_files() -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for path in REPO_ROOT.rglob("*"):
        if path.suffix.lower() not in SOURCE_EXTS:
            continue
        parts = set(path.parts)
        if ".git" in parts or "3dsmaxtrain" in parts:
            continue
        files.append(path)
    return files


def build_sdk_header_index() -> dict[str, str]:
    index: dict[str, str] = {}
    for path in SDK_INCLUDE_ROOT.rglob("*.h"):
        rel = path.relative_to(SDK_INCLUDE_ROOT).as_posix()
        index[path.name.lower()] = rel
    return index


def main() -> None:
    if not SDK_INCLUDE_ROOT.exists():
        raise SystemExit(f"Missing SDK include root: {SDK_INCLUDE_ROOT}")

    sdk_index = build_sdk_header_index()
    project_includes: Counter[str] = Counter()
    sdk_found: Counter[str] = Counter()
    sdk_missing: Counter[str] = Counter()

    for source_file in iter_source_files():
        for line in source_file.read_text(errors="ignore").splitlines():
            match = INCLUDE_RE.match(line)
            if not match:
                continue
            include = match.group(1)
            if include.endswith(('.inl', '.hpp', '.hh')):
                continue
            header = os.path.basename(include).lower()
            if not header.endswith(".h"):
                continue

            project_includes[header] += 1
            if header in sdk_index:
                sdk_found[header] += 1
            else:
                sdk_missing[header] += 1

    print("# Max SDK alignment audit")
    print(f"Total source files scanned: {len(iter_source_files())}")
    print(f"Unique .h includes: {len(project_includes)}")
    print(f"Unique headers found in 3dsmaxtrain/include: {len(sdk_found)}")
    print(f"Unique headers not found in 3dsmaxtrain/include: {len(sdk_missing)}")

    print("\nTop headers resolved by local SDK include:")
    for header, count in sdk_found.most_common(20):
        print(f"  {count:>4}  {header:<24} -> {sdk_index[header]}")

    print("\nTop headers not resolved by local SDK include:")
    for header, count in sdk_missing.most_common(20):
        print(f"  {count:>4}  {header}")


if __name__ == "__main__":
    main()
