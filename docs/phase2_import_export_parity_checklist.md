# Phase 2 parity checklist: NifImport/NifExport vs 3dsmaxtrain samples

Reference samples reviewed:
- `3dsmaxtrain/samples/import_export/asciiexp/*`
- `3dsmaxtrain/samples/import_export/3dsexp*`
- `3dsmaxtrain/samples/igame/export/*`

## SceneImport / SceneExport class contracts

- [x] **Importer extension contract fixed**: `MaxNifImport::ExtCount()` now matches supported extensions returned by `Ext()` and `DoImport()` (`NIF`, `KF`, `KFM`).
- [x] **Exporter extension contract already consistent**: `NifExport` and `KfExport` `ExtCount()/Ext()` pairings are consistent.
- [x] **SupportsOptions implemented**: exporters provide `SupportsOptions(...)` and accept selected-export mode.

## Option dialogs and I/O settings lifecycle

- [x] Import/export settings initialized through `AppSettings::Initialize(...)` before main work.
- [x] Exporters load persisted config, run options dialogs when prompts are enabled, then write settings back to registry/root node.
- [x] **Error return semantics tightened for import**: `MaxNifImport::DoImport()` now returns failure (`FALSE`) on handled exceptions rather than success.

## Material/texture path behavior

- [x] Import side still resolves files through resolver helpers (`FindImage`, `FindMaterial`, `FindFile*`).
- [x] Export side still supports configurable texture prefix and app-specific settings via dialog/config system.
- [ ] Deferred: align path canonicalization/wrapper usage with latest SDK utility classes where available.

## API drift fixes applied in this phase

- [x] **Extension exposure drift**: fixed importer `ExtCount` mismatch.
- [x] **Version parsing drift in KF export**: `KfExport::DoExport()` now parses `Exporter::mNifVersion` before validation (`IsSupportedVersion(...)`).
- [x] **Failure signaling drift**: importer now signals failure on caught exceptions.

## Follow-up candidates (next PRs)

1. Add small contract checks for extension/count parity in import/export entrypoints.
2. Audit importer/exporter message handling against SDK sample patterns for prompt/progress edge cases.
3. Validate resolver behavior across mixed relative/absolute texture roots with representative NIF assets.
