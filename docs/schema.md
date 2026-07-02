# CAS Schema Reference

Complete field-by-field documentation for the Capacitor Agnostic Structure schema.

Schema version: JSON Schema 2020-12

---

## Top-Level Structure (CAS.json)

The root CAS document contains three required properties:

| Field | Type | Required | Description |
|---|---|---|---|
| `inputs` | [inputs](#inputs-inputsjson) | Yes | Design requirements and operating points for this capacitor |
| `capacitor` | [capacitor](#capacitor) | Yes | The capacitor component data |
| `outputs` | array of [outputs](#outputs-outputsjson) | Yes | Computed results per operating point; `outputs[i]` aligns positionally with `inputs.operatingPoints[i]` (may be empty) |

The root object is closed (`additionalProperties: false`). Both `inputs` and `outputs` are fully structured schemas (`inputs.json`, `inputs/designRequirements.json`, `outputs.json`) — they are **not** free-form.

---

## inputs (inputs.json)

Mirrors the MAS inputs structure. Both fields are required; the object is closed.

| Field | Type | Required | Description |
|---|---|---|---|
| `operatingPoints` | array (min 1 item) | Yes | Each item is a PEAS [`twoTerminalOperatingPoint`](https://psma.com/peas/inputs/twoTerminalOperatingPoint.json): `{name?, conditions, excitation}` with `conditions` and `excitation` required. The excitation carries single voltage and current waveforms because a capacitor is two-terminal. |
| `designRequirements` | [designRequirements](#designrequirements-inputsdesignrequirementsjson) | Yes | Requirements the capacitor must satisfy |

### designRequirements (inputs/designRequirements.json)

An `allOf` extension of the PEAS `designRequirementsBase` mixin, closed with top-level `unevaluatedProperties: false`. **Required: `capacitance`.**

Capacitor-specific fields:

| Field | Type | Required | Unit | Description |
|---|---|---|---|---|
| `capacitance` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | F | Required values for the capacitance |
| `ratedVoltage` | number ≥ 0 | No | V | Minimum required rated DC voltage |
| `maximumEsr` | number ≥ 0 | No | Ohm | Maximum allowed equivalent series resistance |
| `minimumRippleCurrent` | number ≥ 0 | No | A RMS | Minimum required rated ripple current |
| `minimumLifetime` | number ≥ 0 | No | hours | Minimum required lifetime under the declared operating points |
| `role` | string (enum) | No | — | `dcLink`, `decoupling`, `outputFilter`, `inputFilter`, `snubber`, `resonant`, `bootstrap`, `holdUp` |
| `application` | string (enum) | No | — | PEAS `capacitorApplication`: `power`, `signal`, `safetyEmcSuppression`, `motorRunStart` |
| `subApplication` | string (enum) | No | — | PEAS `capacitorSubApplication` (e.g. `dcLink`, `lineFilter`, `holdUp`, `decoupling`, `X1`…`Y4`, `motorRun`, `motorStart`) |
| `allowedTechnologies` | array of [technology](#technology-enum-values) | No | — | Subset of acceptable technologies. `$ref`s the **same** enum as the part-side `technology` field (`capacitor.json#/$defs/technology`), so requirement and part vocabularies cannot drift. Absent = all allowed. Min 1 item, unique. |

Inherited from PEAS `designRequirementsBase` (all optional): `name`, `market` (`medical`/`commercial`/`industrial`/`automotive`/`military`/`space`), `topology`, `operatingTemperature` (dimensionWithTolerance, °C), `terminalType` (array of PEAS `connectionType`), `maximumWeight` (kg), `maximumDimensions` (`{width, height, depth}`).

---

## capacitor

Reference: `capacitor.json`

The capacitor object is closed and has two optional properties, constrained by an `anyOf`: either `manufacturerInfo` is present (a sourced part) **or** the object is completely empty (`{}` — a pre-sourcing seed whose requirements live in `inputs.designRequirements`).

| Field | Type | Required | Description |
|---|---|---|---|
| `manufacturerInfo` | object | Yes (unless empty seed) | Container for manufacturer-specific information |
| `distributorsInfo` | array of PEAS `distributorInfo` | No | Where to buy this component — packaging, MOQ, VPE, lead time, stock, cost `{value, currency}`. This is where all commercial data lives; there is **no** `business` section in `datasheetInfo`. |

### manufacturerInfo

**Required: `name`, `datasheetInfo`.** The other fields `$ref` the shared PEAS `manufacturerInfo` field definitions.

| Field | Type | Required | Description |
|---|---|---|---|
| `name` | string | Yes | Manufacturer name |
| `datasheetInfo` | object | Yes | Information extracted from the component datasheet |
| `reference` | string | No | Manufacturer part number |
| `status` | string (enum) | No | `production`, `prototype`, `nrnd`, `obsolete`, `preview` |
| `description` | string | No | Description of the part per its manufacturer |
| `orderCode` | string | No | Manufacturer order code |
| `datasheetUrl` | string (uri) | No | URL to manufacturer datasheet |
| `family` | string | No | Manufacturer product family / product-line name |
| `spiceModel` | object | No | SPICE simulation model for this component |

### datasheetInfo

Contains the sections of the capacitor description. **Required sections: `part`, `electrical`, `mechanical`.**

| Field | Type | Required | Description |
|---|---|---|---|
| `part` | object | Yes | Basic part identification |
| `electrical` | object | Yes | Electrical characteristics |
| `thermal` | object | No | Thermal characteristics |
| `mechanical` | object | Yes | Mechanical dimensions and shape information |
| `lifetime` | object | No | Lifetime parameters |
| `modelParams` | object | No | Model parameters for circuit simulation |
| `factors` | object | No | Derating factors for frequency and temperature |
| `application` | string (nullable) | No | Manufacturer's recommended application / quality-grade categories, comma-joined as printed in the catalogue (e.g. Taiyo Yuden's grade taxonomy: "Auto. (Powertrain/Safety)", "Auto. (Body/Info) & High Reliability", "Telecom-Infrastructure & Industrial", "Medical (International Class. Ⅲ）", "Medical (International Class. I ・ II)", "General Equipment", "Mobile Devices"). Free-form — taxonomies are manufacturer-specific. |
| `provenance` | array | No | Data-provenance trail (see [Provenance](#provenance-data-source-trail)) |

---

## part

Basic part identification. An `allOf` extension of PEAS `datasheetInfoPartBase`. **Required fields: `partNumber`, `technology`.**

| Field | Type | Nullable | Required | Description |
|---|---|---|---|---|
| `partNumber` | string | No | Yes | Manufacturer part number |
| `series` | string | Yes | No | Product series name (null where the part has no distinct series) |
| `technology` | string (enum) | No | Yes | Capacitor technology / chemistry family (see below) |
| `dielectricCode` | string | Yes | No | Standard EIA / MIL dielectric code (e.g. X7R, X5R, C0G, NP0, Y5V, Y5U). Applies to ceramic technologies only; null or absent for non-ceramic chemistries. |
| `description` | string | No | No | Free-text part description as on the datasheet |
| `case` | string | No | No | Case or package code (e.g., "1210", "16x25") |

### technology enum values

A closed 20-value enum (`capacitor.json#/$defs/technology`) so downstream selection logic can dispatch on chemistry without parsing free-form strings. The EIA/MIL dielectric code (X7R, C0G, …) goes in `dielectricCode`, not here.

| Value | Description |
|---|---|
| `aluminum-electrolytic-wet` | Wet aluminum electrolytic |
| `aluminum-electrolytic-polymer` | Solid-polymer aluminum |
| `aluminum-hybrid-polymer` | Hybrid (wet + polymer) aluminum |
| `tantalum-wet` | Wet tantalum |
| `tantalum-mno2` | Solid tantalum with MnO2 cathode |
| `tantalum-polymer` | Solid tantalum with polymer cathode |
| `niobium-oxide` | Niobium-oxide |
| `ceramic-class-1` | Temperature-stable ceramics (C0G/NP0) |
| `ceramic-class-2` | High-K ceramics (X7R/X5R/Y5V) — voltage- and temperature-dependent |
| `ceramic-class-3` | Class-3 ceramics (Y5U etc.) |
| `film-polypropylene` | Metallized/film polypropylene |
| `film-polyester` | Metallized/film polyester (PET) |
| `film-polyphenylene-sulfide` | PPS film |
| `film-paper` | Paper / metallized paper |
| `film-acrylic` | Polymer-multi-layer (PML) vapour-deposited acrylate (e.g. Rubycon PMLCAP) |
| `mica` | Mica |
| `thin-film-silicon` | Deposited SiO2/SiON dielectric on a substrate (Kyocera AVX Accu-P, Murata silicon caps, Vishay RFCS) |
| `supercapacitor-edlc` | Electric double-layer supercapacitor |
| `supercapacitor-hybrid` | Hybrid supercapacitor (e.g. lithium-ion capacitor) |
| `vacuum` | Vacuum capacitor |

---

## electrical

Electrical characteristics. **Required fields: `capacitance`, `ratedVoltage`.** All other fields are optional — `esr` and `rippleCurrent` are rarely published for ceramic/film parts and are enriched from the datasheet by the component-librarian when needed.

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `capacitance` | [dimensionWithTolerance](#dimensionwithtolerance) | No | Farads | Capacitance value with tolerance bounds |
| `capacitanceDriftLongTermPercent` | number | Yes | % | End-of-life capacitance change (decrease) |
| `capacitanceMinimumLongTerm` | number | Yes | Farads | Worst-case minimum capacitance after end-of-life, accounting for initial tolerance |
| `capacitanceBiasPoints` | array | No | — | DC-bias derating curve (dominant for class-2 ceramics): points `{voltage (V), capacitance (F), temperature (°C), acVoltage (V)}` (`voltage`+`capacitance` required per point) |
| `ratedVoltage` | number | No | Volts | Rated voltage |
| `polarized` | boolean | Yes | — | True for polarized chemistries (aluminum electrolytic, tantalum, tantalum-polymer, niobium-oxide, EDLC); false for ceramic, film, mica. Null = unknown — downstream selection should conservatively disallow AC swing. |
| `voltageRatedDcMax` | number | Yes | Volts | Maximum DC rated voltage |
| `dissipationFactor` | number | Yes | fraction | Dissipation factor (tan delta) as a **fraction** (e.g. `0.025` = 2.5%) |
| `dissipationFactorFrequency` | number | Yes | Hz | Frequency of the single-point `dissipationFactor` measurement. Prefer `dissipationFactorPoints` for selection logic; this scalar pair is legacy. |
| `dissipationFactorPoints` | [curve](#curve) | No | — | DF-vs-frequency curve: xData = Hz, yData = tan delta as fraction. Preferred over the scalar when present (DF = ESR·ω·C, so chemistries with low-frequency ESR rise show the same in DF). |
| `qFactor` | number | Yes | — | Quality factor (Q = 1/tan delta). For class-1 (C0G/NP0) MLCCs this is the specified minimum Q at the standard EIA-198 / IEC 60384 condition (Q ≥ 1000 for C ≥ 30 pF, Q ≥ 400 + 20·C[pF] below 30 pF, measured at 1 MHz, 1 Vrms). |
| `qFactorFrequency` | number | Yes | Hz | Frequency at which `qFactor` is specified. Null/absent when the datasheet states only the standard-condition minimum without an explicit frequency. |
| `leakageCurrent` | number | Yes | Amperes | DC leakage current |
| `insulationResistance` | number | Yes | Ohms | Insulation resistance between terminals |
| `esr` | number | Yes | Ohms | Equivalent series resistance (ESR) |
| `esrFrequency` | number | Yes | Hz | Frequency of the single-point `esr` measurement. Prefer `esrPoints` for selection logic; this scalar pair is legacy. |
| `esrPoints` | [curve](#curve) | No | — | ESR-vs-frequency curve: xData = Hz, yData = Ohms. At sub-kHz (mains) frequencies ESR can be 10–100× its 100 kHz datasheet value for class-2 ceramic and polymer chemistries. Preferred over the scalar when present. |
| `rippleCurrent` | number | Yes | Amperes | Ripple current (RMS) |
| `rippleCurrentFrequency` | number | Yes | Hz | Frequency at which ripple current is specified |
| `rippleCurrentTemperature` | number | Yes | °C | Temperature at which ripple current is specified |
| `rippleCurrentFrequencyPoints` | [curve](#curve) | No | — | Ripple current derating vs. frequency (X-Y data) |
| `rippleCurrentTemperaturePoints` | [curve](#curve) | No | — | Ripple current derating vs. temperature (X-Y data) |
| `thermalResistance` | number | Yes | °C/W | Thermal resistance |
| `_esrWarning` | boolean, string | Yes | — | Internal flag/message: ESR value estimated or extrapolated (not from datasheet) |

---

## thermal

Thermal characteristics — an `allOf` extension of PEAS `datasheetInfoThermal`. **All fields are optional** — some catalogue sources do not publish an operating-temperature range, so `temperature` may legitimately be absent at fetch time (same rationale as the `esr`/`rippleCurrent` relaxation in `electrical`).

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `operatingTemperature` | [dimensionWithTolerance](#dimensionwithtolerance) | No | °C | Operating temperature range (inherited from the PEAS base) |
| `temperature` | [dimensionWithTolerance](#dimensionwithtolerance) | No | °C | Operating temperature range of the part. Plain optional dimensionWithTolerance — omit when the datasheet does not state it (an empty object or null is not allowed). Minimum = lower limit, maximum = upper limit, nominal = reference. |
| `tcc` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | % | Temperature coefficient of capacitance (capacitance change over temperature range). Minimum and maximum define the bounds. |

**Notes:**
- For `ceramic-class-1` (C0G/NP0), `tcc` is typically very small (e.g., +/- 30 ppm/K).
- For `ceramic-class-2`, `tcc` represents the percentage change allowed by the dielectric code (e.g., X7R allows +/- 15%).
- For electrolytic capacitors, `tcc` is often `null` because capacitance vs. temperature behavior is not specified as a simple tolerance.

---

## mechanical

Mechanical dimensions and shape information. **Required: `shape`.** `dimensions` is optional — some catalogue sources publish only the case/shape code without explicit measurements (e.g. Murata RDE/RCE radial-lead series), so it may legitimately be absent at fetch time.

### dimensions

Physical dimensions of the component. **All fields are optional and nullable.** Use `null` (or omit) for dimensions that do not apply to the form factor (e.g., `diameter` for chip capacitors; `width` and `length` for cylindrical capacitors).

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `diameter` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Diameter (cylindrical form factors) |
| `width` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Width / A dimension (rectangular form factors) |
| `length` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Length / B dimension (rectangular form factors) |
| `height` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Height dimension |
| `thickness` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Thickness / T dimension. Chip-style (MLCC) datasheets specify the body as L x W x T, where T is the dimension perpendicular to the mounting plane; unlike the EIA case code (which fixes only L x W), T varies per part with layer count. Kept separate from `height` to stay faithful to datasheet nomenclature. |
| `pitch` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Pin pitch spacing (through-hole form factors) |
| `pinDiameter` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Pin/lead diameter (through-hole form factors) |
| `pinLength` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Pin/lead length (through-hole form factors) |

### shape

Component shape and assembly information. **Required: `assembly`, `shapeType`.**

| Field | Type | Nullable | Required | Unit | Description |
|---|---|---|---|---|---|
| `assembly` | string (enum) | No | Yes | — | Assembly / mounting type |
| `shapeType` | string | No | Yes | — | Physical shape of the component |
| `volume` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | No | cubic meters | Computed component volume |
| `footprint` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | No | square meters | PCB footprint area |

#### assembly enum values

| Value | Description |
|---|---|
| `"THT"` | Through-hole technology |
| `"Screw Type"` | Screw terminal mounting |
| `"SMT"` | Surface mount technology |
| `"Snap-In"` | Snap-in mounting (large electrolytic capacitors) |

---

## lifetime

Reliability and lifetime parameters — an `allOf` extension of PEAS `datasheetInfoLifetime` with capacitor-specific end-of-life definitions. **All fields are optional and nullable.** Fields that do not apply to the technology (e.g., Arrhenius parameters for MLCCs) should be `null` or absent.

Inherited from the PEAS base:

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `lifetimeEndurance` | number | Yes | hours | Rated endurance life at rated conditions (voltage, temperature) |
| `maxLifetime` | number | Yes | years | Maximum expected service lifetime |
| `aexp` | number | Yes | — | Voltage stress exponent for the lifetime model |
| `bexp` | number | Yes | — | Temperature stress coefficient (Arrhenius exponent) |
| `deltaT0` | number | Yes | °C | Reference temperature differential ΔT0 for the lifetime model |
| `kfactor` | number | Yes | — | K factor (voltage derating exponent or scaling constant) |
| `vxfactor` | number | Yes | — | Vx factor for extended voltage lifetime model |
| `usefulLife` | number | Yes | hours | Useful life (IEC 62863 / IEC 61709: time to specified end-of-life criteria) |

CAS-specific additions:

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `endDefinitionC` | number | Yes | % | End-of-life capacitance change (decrease) |
| `endDefinitionEsr` | number | Yes | % | End-of-life ESR change (increase; e.g., 200 means ESR can increase to 3x initial) |
| `eoUsefulLifeC` | number | Yes | % | End-of-useful-life capacitance change (decrease) |
| `eoUsefulLifeR` | number | Yes | % | End-of-useful-life resistance change (increase) |
| `usefulLifeComment` | string | Yes | — | Additional comments about useful life conditions |

**Typical values by technology:**

| Technology | lifetimeEndurance | endDefinitionC | endDefinitionEsr |
|---|---|---|---|
| `aluminum-electrolytic-wet` | 1000--10000 h | 20% | 200% |
| `aluminum-electrolytic-polymer` | 2000--10000 h | 10% | 100% |
| `aluminum-hybrid-polymer` | 2000--10000 h | 15% | 150% |
| `film-*` | 100000 h | 5% | 50% |
| `ceramic-class-1/2` | null (wear-out not applicable) | null | null |

---

## modelParams

Circuit model parameters for SPICE simulation. **All fields are optional and nullable.** Represents a series RLC circuit with a parallel insulation resistance.

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `rs` | number | Yes | Ohms | Series resistance (ESR) |
| `cs` | number | Yes | Farads | Series capacitance |
| `ls` | number | Yes | Henries | Series inductance (ESL) |
| `riso` | number | Yes | Ohms | Insulation/isolation resistance (parallel path) |

**Circuit topology:**

```
     Terminal A                          Terminal B
         o                                  o
         |                                  |
         +---[Rs]---[Ls]---+---[Cs]---+-----+
         |                             |
         +----------[Riso]------------+
```

- Rs, Ls, and Cs are in series between the terminals
- Riso is in parallel across the terminals
- Typical Riso values: 1e8 to 1e12 Ohms (electrolytic to ceramic)

---

## factors

Derating factors as functions of frequency and temperature. **All fields are optional.**

| Field | Type | Description |
|---|---|---|
| `rippleCurrentFrequencyFactorFrequency` | number[] | Frequency breakpoints for ripple current derating (Hz) |
| `rippleCurrentFrequencyFactorAmplitude` | number[] | Derating multipliers at each frequency breakpoint (dimensionless) |
| `rippleCurrentTemperatureFactorTemperature` | number[] | Temperature breakpoints for ripple current derating (degrees C) |
| `rippleCurrentTemperatureFactorAmplitude` | number[] | Derating multipliers at each temperature breakpoint (dimensionless) |

**Usage:** To find the derated ripple current at a given frequency and temperature:

```
I_ripple_derated = I_ripple_rated * factor_freq(f_sw) * factor_temp(T_amb)
```

where `factor_freq` and `factor_temp` are obtained by interpolating the respective arrays.

**Example (aluminum electrolytic):**

```json
{
  "rippleCurrentFrequencyFactorFrequency": [120, 1000, 10000, 100000],
  "rippleCurrentFrequencyFactorAmplitude": [0.5, 0.65, 0.85, 1.0],
  "rippleCurrentTemperatureFactorTemperature": [60, 85, 105],
  "rippleCurrentTemperatureFactorAmplitude": [1.5, 1.0, 0.0]
}
```

This means:
- At 120 Hz, ripple current capability is 50% of the rated value
- At 100 kHz, ripple current capability is 100% (the reference frequency)
- At 60 degrees C, the capacitor can handle 150% of rated ripple (because it runs cooler than rated temperature)
- At 105 degrees C (maximum rated), derating reaches 0% (no ripple allowed at max temp)

---

## outputs (outputs.json)

Computed results for **one operating point**; the top-level CAS document holds an **array** of these, aligned positionally with `inputs.operatingPoints`. Each named block is independently optional so partial computations validate; every block is an `allOf` over the PEAS `outputBase` provenance shell — `origin` (`manufacturer`/`measurement`/`simulation`) and `methodUsed` are **required** in every block — and is sealed with `unevaluatedProperties: false`.

| Block | Required field(s) inside | Description |
|---|---|---|
| `esrLosses` | `totalLosses` (W) | Power dissipated by the ESR under the ripple current; optional `esr`, `rippleCurrentRms`, `lossesPerHarmonic`, `conditions` |
| `temperature` | `hotSpotTemperature` (°C) | Self-heating; optional `ambientTemperature`, `temperatureRise` (K), `thermalResistance` (K/W) |
| `effectiveCapacitance` | `effectiveCapacitance` (dimensionWithTolerance, F) | Capacitance after DC-bias and temperature derating; optional `deratingFactor` (0..1], `ratedCapacitance`, `conditions` |
| `impedance` | `impedanceMatrix` | \|Z\| and phase vs. frequency (array of PEAS `complexMatrixAtFrequency`, 1x1 for a two-terminal cap); optional `selfResonantFrequency` (Hz), `esrAtSelfResonance` (Ohm), `esl` (H) |
| `rippleVoltage` | — | Ripple voltage across the terminals: `peakToPeak` (V), `rms` (V), `perHarmonic` |
| `lifetime` | `predictedHours` (dimensionWithTolerance, h) | Predicted lifetime under this stress; optional `predictedYears`, `accelerationFactor`, `ratedLifetime`, `endOfLifeReached`, `endOfLifeMode` (`capacitanceDrop`/`esrRise`/`openCircuit`/`shortCircuit`/`electrolyteDryOut`/`dielectricBreakdown`), `conditions` |
| `reliability` | — | `fit` (failures / 10^9 device-hours), `mtbf` (h), `confidenceLevel` (0..1] |
| `insulation` | — | Insulation coordination figures for film safety / Y-class capacitors (`$ref` to PEAS `outputs/insulationCoordination.json`) |

---

## Utility Types

The canonical shared primitives live in `PEAS/schemas/utils.json` (referenced by absolute `$id` `https://psma.com/peas/utils.json`). `CAS/schemas/utils.json` is only a redirect shim to the PEAS file.

### dimensionWithTolerance

A physical quantity with minimum, nominal, and maximum values. At least one of `minimum`/`nominal`/`maximum` must be present. Closed object.

| Field | Type | Required | Description |
|---|---|---|---|
| `minimum` | number | At least one of min/nom/max | Minimum value |
| `nominal` | number | At least one of min/nom/max | Nominal (typical) value |
| `maximum` | number | At least one of min/nom/max | Maximum value |
| `excludeMinimum` | boolean | No | True if the minimum value is excluded from the range (default false) |
| `excludeMaximum` | boolean | No | True if the maximum value is excluded from the range (default false) |
| `unit` | string | No | Optional SI unit string (e.g. `'F'`, `'V'`, `'m'`); when absent, the unit is implied by the field name |

Examples:

```json
{"nominal": 10e-6}
{"minimum": 8e-6, "nominal": 10e-6, "maximum": 12e-6}
{"minimum": -55, "maximum": 125}
```

### curve

X-Y data points for characteristic curves, optionally labelled and annotated with measurement conditions.

| Field | Type | Description |
|---|---|---|
| `xData` | number[] | X-axis values |
| `yData` | number[] | Y-axis values (same length as xData) |
| `xLabel` | string | Human-readable X-axis label incl. unit (e.g. `"Frequency [Hz]"`) |
| `yLabel` | string | Human-readable Y-axis label incl. unit (e.g. `"ESR [Ω]"`) |
| `conditions` | object | Measurement conditions, e.g. `{"temperature": 25, "voltage": 5}` |

Example:

```json
{
  "xData": [100, 1000, 10000, 100000],
  "yData": [0.5, 0.7, 1.0, 1.2]
}
```

### numberArray

A simple array of numbers.

```json
[120, 1000, 10000, 100000]
```

Used internally by the `factors` section for derating curve breakpoints.

## Provenance (data-source trail)

Every `datasheetInfo` carries an optional `provenance` array recording where its data
came from. Optional and closed, so records without it remain valid. Each entry:

| field | meaning |
|---|---|
| `source` | `manufacturerDatasheet` · `manufacturerParametric` · `manufacturerDatabase` · `distributor` · `librarianEnrichment` · `scrape` · `manual` (required) |
| `sourceName` | human-readable source, e.g. `"TI parametric API"`, `"WE - Passive Components.mdb"`, `"DigiKey"` |
| `sourceUrl` | URL the value came from (optional) |
| `retrievedDate` | `YYYY-MM-DD` (optional) |
| `fields` | which `datasheetInfo` fields this source supplied — for mixed-source records (optional) |

It is a **list**: a record may combine sources (e.g. specs from the datasheet, a rated
voltage from a distributor, a missing field back-filled by librarian enrichment). The
canonical definition lives in `PEAS/schemas/utils.json#/$defs/provenance`.
