# Phase 3/4 modernization checklist

This checklist continues execution after Phase 1 (build/include hygiene) and Phase 2 (import/export contract fixes).

## Phase 3 — Modifier/material modernization

Reference samples:
- `3dsmaxtrain/samples/modifiers/*`
- `3dsmaxtrain/samples/materials/*`

### NifProps/Modifier
- [ ] Audit parameter block declarations and rollup wiring against modern modifier sample patterns.
- [ ] Confirm notification/refresh behavior does not rely on deprecated callbacks.
- [ ] Validate BSDismember/BSSubIndex modifier data lifetime and undo/redo correctness.

### NifProps/Shader and material interop
- [ ] Compare shader class descriptor flags and class IDs with sample material plugin patterns.
- [ ] Validate texture slot routing (diffuse/normal/spec/glow) and fallback behavior.
- [ ] Verify user-prop metadata round-trip for game-specific shader flags.

## Phase 4 — Runtime integration hardening

Reference samples:
- `3dsmaxtrain/samples/utilities/*`
- `3dsmaxtrain/samples/gup/*`

### Plugin lifecycle
- [ ] Review plugin initialization/deinitialization ordering in each DLL entry.
- [ ] Ensure callbacks/notifications are unregistered on teardown.
- [ ] Verify no scene-owned pointers are kept past reset/new scene/load operations.

### Smoke workflow validation
- [ ] Load plugin set in 3ds Max.
- [ ] Import NIF (mesh + skin + material).
- [ ] Export NIF and KF.
- [ ] Reset scene, reload plugin, repeat import/export once.
- [ ] Record warnings/errors and regressions by subsystem (`NifImport`, `NifExport`, `NifProps`, `NifCommon`).

## Immediate code-level drifts fixed while continuing phases

- [x] Export entry points now return failure on exceptions/cancel instead of always reporting success.
- [x] Importer now fails on unsupported file extensions instead of silently succeeding.
- [x] KF export now guards against missing app-settings selection before dereference.
