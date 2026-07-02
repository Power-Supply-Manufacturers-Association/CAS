# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository nature

CAS defines the Capacitor Agnostic Structure: a JSON Schema 2020-12 specification for describing capacitors used in power electronics. The primary work here is editing JSON Schema files and Markdown docs. The repo ALSO contains a small C++ layer (`src/`, quicktype-generated `CAS.hpp`, a `cas_to_cias` converter with pybind11/embind bindings, Catch2 tests under `tests/test_cas.cpp`, CMake build) and a pytest schema harness (`tests/test_schemas.py`).

## Layout

- `schemas/CAS.json` — top-level wrapper: `{ inputs, capacitor, outputs }` (all three required). Every valid CAS document is also a valid PEAS document (the `capacitor` branch of the PEAS oneOf).
- `schemas/capacitor.json` — the capacitor object: `manufacturerInfo.datasheetInfo` with sections `part`, `electrical`, `thermal`, `mechanical`, `lifetime`, `modelParams`, `factors` (`part`+`electrical`+`mechanical` required; there is NO `business` section — commercial data lives in `distributorsInfo` via PEAS `distributorInfo`). The root allows an **empty seed** (`{}`) for pre-sourcing slots; there is NO passthrough `capacitor` wrapper (removed 2026-07 — do not re-add it).
- `schemas/capacitor.json#/$defs/technology` — THE single technology vocabulary (20 hyphenated values: `ceramic-class-1/2/3`, `aluminum-electrolytic-wet`, `aluminum-electrolytic-polymer`, `aluminum-hybrid-polymer`, `tantalum-*`, `film-*`, `mica`, `thin-film-silicon`, `supercapacitor-*`, `vacuum`, `niobium-oxide`). Both the part-side `technology` field and the requirements-side `allowedTechnologies` `$ref` it — never redefine or copy this enum.
- `schemas/inputs.json` + `schemas/inputs/designRequirements.json` — structured inputs: `operatingPoints[]` (PEAS `twoTerminalOperatingPoint`: conditions + excitation with processed waveforms requiring a magnitude `peak`/`peakToPeak`) and `designRequirements` (allOf over PEAS `designRequirementsBase`, closed with **top-level `unevaluatedProperties: false`** — the sanctioned mixin-closure idiom; never use `additionalProperties: false` on an allOf extension branch, it rejects the base fields).
- `schemas/outputs.json` — per-result blocks (`esrLosses`, `temperature`, …), each sealed with `unevaluatedProperties: false`.
- `schemas/utils.json` — redirect shim (unreferenced by schemas; kept only for the quicktype `-S` input list in CMakeLists).
- `docs/schema.md` — field-by-field reference. Keep in sync with `schemas/capacitor.json`.
- `examples/` — real-part examples (Murata MLCC, Nichicon electrolytic) that validate as CAS AND as PEAS.
- `data/` — reserved for manufacturing **building blocks** (foils, dielectrics, electrolytes). **Finished orderable parts belong in TAS/data/** (230k+ capacitors live there).

## Tests

```bash
pip install pytest jsonschema referencing
pytest tests/test_schemas.py -q          # schema harness (meta, refs, examples, negative cases)

mkdir -p build && cmake -S . -B build -G Ninja && ninja -C build cas_tests
./build/cas_tests                        # Catch2 (never ctest)
```

The pytest harness needs the sibling repos checked out alongside (PSMA workspace layout); cross-repo `$ref`s use absolute `https://psma.com/<repo>/...` URIs resolved via a `referencing.Registry`.

## Schema editing rules

- NEVER change schema files without explicit user permission (workspace-wide rule).
- Closed objects everywhere; allOf mixins over the PEAS bases (`designRequirementsBase`, `datasheetInfo*`, `outputBase`) are sealed with **top-level `unevaluatedProperties: false`**.
- Leaf datasheet fields are typically nullable (`["number","null"]`) so partial datasheet data validates — preserve when adding fields. (`thermal.temperature` is a plain optional `dimensionWithTolerance`, not nullable.)
- All values SI: F not µF, Ω not mΩ; `dissipationFactor` is a fraction, not percent.
- When you add or rename a field in `schemas/capacitor.json`, update `docs/schema.md` and the examples in the same change.

## Siblings

CAS is one of the PEAS component families: MAS (magnetics), CAS (this), SAS (semiconductors), RAS (resistors + varistors), CTAS (control ICs), CONAS (connectors), AAS (analog ICs). Shared primitives (`dimensionWithTolerance`, `curve`, `distributorInfo`, `manufacturerInfo`, the `datasheetInfo*` mixins, application enums) live in `PEAS/schemas/utils.json`. MAS references PEAS utils too these days (the old "MAS is self-contained" claim is stale). The PEAS root object is closed and has no `masVersion`/`masConformance` keys.
