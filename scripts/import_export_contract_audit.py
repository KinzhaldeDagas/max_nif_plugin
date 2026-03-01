#!/usr/bin/env python3
"""Quick static contract checks for importer/exporter entrypoint semantics."""
from pathlib import Path
import re

text = {
    "imp": Path("NifImport/MaxNifImport.cpp").read_text(errors="ignore"),
    "nif": Path("NifExport/NifExport.cpp").read_text(errors="ignore"),
    "kf": Path("NifExport/KfExport.cpp").read_text(errors="ignore"),
}

checks = [
    ("Importer ExtCount exposes NIF/KF/KFM", lambda: "return 3;" in text["imp"]),
    (
        "Importer rejects unsupported extensions",
        lambda: re.search(r"else\s*\{\s*return FALSE;\s*\}", text["imp"]) is not None,
    ),
    ("NIF export returns internal result", lambda: "result = DoExportInternal" in text["nif"]),
    ("NIF export internal success returns TRUE", lambda: re.search(r"DoExportInternal[\s\S]*return TRUE;", text["nif"]) is not None),
    ("KF export parses requested nif version", lambda: "ParseVersionString(T2AString(Exporter::mNifVersion))" in text["kf"]),
]

failed = False
for label, fn in checks:
    ok = fn()
    print(("PASS: " if ok else "FAIL: ") + label)
    failed = failed or (not ok)

raise SystemExit(1 if failed else 0)
