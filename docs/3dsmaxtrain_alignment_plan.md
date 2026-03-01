# 3dsmaxtrain include/samples analysis and NifTools alignment plan

## Scope analyzed

- SDK-style headers in `3dsmaxtrain/include` (API surface snapshot for Max 2026 era).
- Reference plugin implementations in `3dsmaxtrain/samples`, with emphasis on:
  - `samples/import_export` (closest to `NifImport` + `NifExport`)
  - `samples/materials` (closest to `NifProps/Shader` and texture material handling)
  - `samples/modifiers` + `samples/EditableModifierCommon` (closest to `NifProps/Modifier`)
  - `samples/utilities` + `samples/gup` (runtime services, plugin package manager behavior)

## Quick findings

1. The repository already has a Max 2026 property sheet (`Build/Max2026.props`) wired to `$(MAX_SDK_DIR)`, so the build entry point for modern SDKs exists.
2. The codebase still carries legacy compatibility scaffolding (`stdafx.h`, VC80 project files, old config names), so alignment should focus on *incremental modernization* rather than broad rewrites.
3. Running `scripts/maxsdk_alignment_audit.py` on current source produced:
   - 144 source files scanned
   - 298 unique header includes
   - 52 headers that map directly to `3dsmaxtrain/include`
   - 246 headers not found in the local SDK snapshot (expected: many are project-local/Niflib/system headers)
4. Core Max SDK touchpoints are concentrated around importer/exporter and modifier APIs (`max.h`, `iparamb2.h`, `istdplug.h`, `modstack.h`, `notify.h`, etc.), which matches expected migration risk areas.

## Alignment strategy

### Phase 1 — Build and include hygiene (lowest risk, immediate value)

- Keep `Build/Max2026.props` as the canonical include/lib gateway and avoid hardcoded SDK paths in project files.
- Normalize includes to canonical case and path style used by `3dsmaxtrain/include` where possible (`max.h`, `macrorec.h`, etc.).
- Remove/guard stale SDK branches that predate x64-only Max targets when they affect compile paths for 2026.

**Reference samples**
- `3dsmaxtrain/samples/import_export/asciiexp`
- `3dsmaxtrain/samples/import_export/3dsexp`

### Phase 2 — Import/export API parity pass

- Compare `NifImport/*` and `NifExport/*` callback usage against modern sample patterns:
  - `SceneImport`/`SceneExport` class contracts
  - option dialogs and I/O settings lifecycle
  - material/texture path behavior
- Create a parity checklist per module (import, export, animation, collision).
- Fix API drifts first where signatures or ownership changed (e.g., string/path wrappers, interface acquisition/release, notifications).

**Reference samples**
- `3dsmaxtrain/samples/import_export/*`
- `3dsmaxtrain/samples/igame/export`

### Phase 3 — Modifier/material modernization

- Audit `NifProps/Modifier/*` and `NifProps/Shader/*` against:
  - `samples/modifiers/*`
  - `samples/materials/*`
- Prioritize points where Max SDK introduced stricter type safety or updated UI parameter handling.
- Align custom UI resources and parameter blocks with sample conventions to reduce future breakage.

### Phase 4 — Runtime integration hardening

- Validate startup/runtime behavior against utility/GUP sample patterns:
  - plugin registration and class descriptors
  - deferred initialization and notifications
  - safe scene callbacks and teardown
- Add smoke tests/scripts for “load plugin → import nif → export nif → unload/reload plugin” workflows.

## Suggested execution backlog

1. **Create a machine-readable compatibility matrix** (`module`, `source file`, `SDK API used`, `sample reference`, `status`).
2. **Run and store the audit regularly**:
   - `python scripts/maxsdk_alignment_audit.py`
3. **Triage by risk**:
   - High: import/export entrypoints, scene graph traversal, skin/bone interfaces.
   - Medium: material adapters, collision helpers.
   - Low: legacy build artifacts and warning cleanups.
4. **Land small PRs** grouped by subsystem (`NifImport`, `NifExport`, `NifProps`, `NifCommon`) to keep validation tight.

## Deliverables produced in this change

- `scripts/maxsdk_alignment_audit.py` — repeatable include-coverage audit against `3dsmaxtrain/include`.
- `docs/3dsmaxtrain_alignment_plan.md` — phased modernization plan tied to SDK sample directories.
