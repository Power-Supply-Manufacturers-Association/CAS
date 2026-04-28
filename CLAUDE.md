# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository nature

CAS is a **schema-only repository** — no source code, no build, no test runner. It defines the Capacitor Agnostic Structure: a JSON Schema 2020-12 specification for describing capacitors used in power electronics. Work here is editing JSON Schema files and Markdown docs, not writing application code.

## Layout

- `schemas/CAS.json` — top-level wrapper: `{ inputs, capacitor, outputs }`. Every valid CAS document is also a valid EAS document.
- `schemas/capacitor.json` — the capacitor object. Wrapped in `manufacturerInfo.datasheetInfo`, which contains the eight required sections: `part`, `electrical`, `thermal`, `mechanical`, `business`, `lifetime`, `modelParams`, `factors`.
- `schemas/utils.json` — **redirect shim** to `../../EAS/schemas/utils.json`. Shared types (`dimensionWithTolerance`, `curve`, `numberArray`, `distributorInfo`, `manufacturerInfo` base) live in EAS, not here.
- `docs/schema.md` — field-by-field reference. Keep in sync with `schemas/capacitor.json`.
- `data/capacitors.ndjson` — manufacturing **building blocks** only (foils, dielectrics, electrolytes). **Finished orderable parts belong in TAS, not CAS.**

## EAS / MAS layout (sibling repos)

This repo is one of four siblings under the EAS umbrella. Expected checkout layout (verified at `/home/alf/PSMA/`):

```
PSMA/
  EAS/   schemas/{eas.json, utils.json}              -- parent + shared utils
  MAS/   schemas/{MAS.json, inputs.json, magnetic.json, outputs.json, utils.json, inputs/, magnetic/, outputs/, conformance/}
  CAS/   schemas/{CAS.json, capacitor.json, utils.json (shim)}
  (SAS, RAS — referenced by EAS but not present here yet)
```

**EAS as parent:** `EAS/schemas/eas.json` is the universal wrapper. It declares `inputs`/`outputs` and a top-level `oneOf` over `magnetic` (→ MAS), `capacitor` (→ CAS, via relative path `../../CAS/schemas/capacitor.json`), `semiconductor`, `resistor`. So a valid CAS document must also satisfy that branch of EAS.

**EAS utils unify CAS/RAS/SAS only — not MAS.** `EAS/schemas/utils.json` explicitly states *"MAS remains self-contained and does not reference this file."* It provides shared primitives (`dimensionWithTolerance`, `curve`, `numberArray`, `connectionType`, `distributorInfo`, `substituteInfo`) **and** the `datasheetInfo*` mixins (`Thermal`, `Mechanical`, `Business`) plus base `manufacturerInfo`. CAS's `schemas/utils.json` is a shim to this file, and `capacitor.json`'s `manufacturerInfo` is `allOf [EAS base, capacitor-specific datasheetInfo]`. Do not duplicate these into CAS — extend the EAS base.

**MAS does its own thing.** `MAS/schemas/MAS.json` adds `masVersion` (SemVer) and `masConformance` (class A/B/C) at the top level, makes `outputs` an array, and uses its own `MAS/schemas/utils.json`. When borrowing MAS conventions for CAS, copy the *shape*, not the references.

## Schema editing rules

- The capacitor object exposes `manufacturerInfo`, `distributorsInfo`, and a passthrough `capacitor` wrapper (used by some DB entries). The root requires `anyOf: [manufacturerInfo, capacitor]` — preserve this when editing.
- All eight `datasheetInfo` sections are required at the schema level. Many leaf fields are nullable (`["number", "null"]`) so partial datasheet data still validates — keep that pattern when adding fields.
- Technology enum values are fixed strings with spaces and punctuation: `"MLCC Class I"`, `"MLCC Class II"`, `"Alum. Electrolytic"`, `"Alum. Polymer"`, `"Hybrid Polymer"`, `"Film Capacitor"`. Do not normalize them.
- When you add or rename a field in `schemas/capacitor.json`, update the matching row in `docs/schema.md` and any example in `README.md` in the same change.

## `inputs` is currently unconstrained — and that is the main schema gap vs. MAS

`CAS/schemas/CAS.json` declares `inputs` as **required but free-form** (`type: object`, no properties, no `$ref`). The README describes inputs conceptually ("Voltage/Current, Frequency, Temperature, Lifetime req.") but nothing in the schema enforces it. By contrast, MAS has a fully structured `inputs.json` that future CAS work should mirror in shape:

- **MAS `inputs.operatingPoints[]`** — each item = `{conditions, excitationsPerWinding[]}`.
  - `conditions` (`operatingConditions.json`): `ambientTemperature` (required), `ambientRelativeHumidity`, and a `cooling` `oneOf` over `naturalConvection` / `forcedConvection` (requires `velocity`) / `heatsink` (requires `thermalResistance`) / `coldPlate` (requires `maximumTemperature`).
  - `excitationsPerWinding[]` (`operatingPointExcitation.json`): per-port `{frequency, current, voltage, magneticFluxDensity, magneticFieldStrength, magnetizingCurrent}` with a rich `signalDescriptor` (waveform label enum: triangular, sinusoidal, rectangular, flybackPrimary, …). `anyOf` requires either `(frequency+current+voltage)` or `(frequency+magneticFluxDensity)`.
- **MAS `inputs.designRequirements`** (`designRequirements.json`) — required `magnetizingInductance` + `turnsRatios`; optional: `minimumImpedance[]`, `leakageInductance[]`, `strayCapacitance[]`, `isolationSides[]`, `operatingTemperature`, full `insulation` block (altitude, CTI `groupI..IIIB`, pollutionDegree `PD1..4`, overvoltageCategory `I..IV`, insulationType, mainSupplyVoltage, standards enum `IEC 60664-1 / 61558-1 / 60335-1 / 62368-1`), `market` (medical/commercial/industrial/military/space), `topology` (24-value enum), `application`, `subApplication`, `wiringTechnology`, `maximumWeight`, `maximumDimensions`, `terminalType[]`.
- **MAS `inputs.converterInformation.supportedTopologies`** — optional sub-schemas per topology (`flyback`, `buck`, `boost`, `pushPull`, `forward`, `dualActiveBridge`, `llcResonant`, `cllcResonant`, `phaseShiftFullBridge`, `currentTransformer`, `isolatedBuck`, `isolatedBuckBoost`); each declares its own required fields (e.g. `buck.json` requires `inputVoltage`, `diodeVoltageDrop`, `operatingPoints`).

When asked to formalize CAS `inputs`, the natural mapping is: capacitor `operatingPoints[]` (ambient + per-terminal voltage/ripple-current `signalDescriptor`), `designRequirements` (capacitance, ratedVoltage, ripple current, lifetime hours, max ESR, max dimensions, market), and a similar `converterInformation` topology hook reusing the converter list. Until then, treat `inputs`/`outputs` payloads as tool-private — no validation will catch shape errors.

## Sibling schemas

CAS is part of the EAS family: MAS (magnetics), CAS (this), SAS (semiconductors), RAS (resistors). Cross-domain primitives, the `datasheetInfo*` mixins, and the `manufacturerInfo` base live in `EAS/schemas/utils.json` and are inherited by CAS/RAS/SAS — MAS is intentionally self-contained.
