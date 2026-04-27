# CAS Schema Reference

Complete field-by-field documentation for the Capacitor Agnostic Structure schema.

Schema version: JSON Schema 2020-12

---

## Top-Level Structure (CAS.json)

The root CAS document contains three required properties:

| Field | Type | Required | Description |
|---|---|---|---|
| `inputs` | object | Yes | Design requirements and operating points for this capacitor |
| `capacitor` | object | Yes | The capacitor component data (see [capacitor](#capacitor)) |
| `outputs` | object or array | Yes | Computed results (ESR losses, thermal, lifetime, etc.) |

The `inputs` and `outputs` objects are free-form -- their internal structure is not constrained by the CAS schema. This allows tools to attach arbitrary computed data.

---

## capacitor

Reference: `capacitor.json`

The capacitor object has a single required property:

| Field | Type | Required | Description |
|---|---|---|---|
| `manufacturerInfo` | object | Yes | Container for manufacturer-specific information |

### manufacturerInfo

| Field | Type | Required | Description |
|---|---|---|---|
| `datasheetInfo` | object | Yes | Information extracted from the component datasheet |

### datasheetInfo

Contains all eight sections of the capacitor description. **All sections are required.**

| Field | Type | Required | Description |
|---|---|---|---|
| `part` | object | Yes | Basic part identification |
| `electrical` | object | Yes | Electrical characteristics |
| `thermal` | object | Yes | Thermal characteristics |
| `mechanical` | object | Yes | Mechanical dimensions and shape information |
| `business` | object | Yes | Commercial and business information |
| `lifetime` | object | Yes | Lifetime parameters |
| `modelParams` | object | Yes | Model parameters for circuit simulation |
| `factors` | object | Yes | Derating factors for frequency and temperature |

---

## part

Basic part identification. **All fields are required.**

| Field | Type | Nullable | Description |
|---|---|---|---|
| `partNumber` | string | No | Manufacturer part number |
| `series` | string | No | Product series name (e.g., "WCAP-ASLI", "WCAP-CSGP") |
| `technology` | string | No | Capacitor technology type |
| `description` | string | No | Description of the part |
| `case` | string | No | Case or package code (e.g., "1210", "6.3x5.5") |

---

## electrical

Electrical characteristics. **All fields are required.**

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `capacitance` | [dimensionWithTolerance](#dimensionwithtolerance) | No | Farads | Capacitance value with tolerance bounds |
| `capacitanceDriftLongTermPercent` | number | No | % | End-of-life capacitance change (negative = decrease) |
| `capacitanceMinimumLongTerm` | number | No | Farads | Worst-case minimum capacitance after end-of-life, accounting for initial tolerance |
| `ratedVoltage` | number | No | Volts | Maximum continuous DC voltage rating |
| `dissipationFactor` | number | No | % | Dissipation factor (tan delta) |
| `dissipationFactorFrequency` | number | No | Hz | Frequency at which dissipation factor is measured |
| `leakageCurrent` | number | No | Amperes | DC leakage current |
| `insulationResistance` | number | No | Ohms | Insulation resistance between terminals |
| `esr` | number | Yes | Ohms | Equivalent series resistance (ESR) |
| `esrFrequency` | number | Yes | Hz | Frequency at which ESR is measured |
| `rippleCurrent` | number | No | Amperes | Maximum rated ripple current (RMS) |
| `rippleCurrentFrequency` | number | No | Hz | Frequency at which ripple current is specified |
| `rippleCurrentTemperature` | number | Yes | degrees C | Temperature at which ripple current is specified |
| `rippleCurrentFrequencyPoints` | [curve](#curve) | No | -- | Ripple current derating as a function of frequency (X-Y data) |
| `rippleCurrentTemperaturePoints` | [curve](#curve) | No | -- | Ripple current derating as a function of temperature (X-Y data) |
| `thermalResistance` | number | Yes | degrees C/W | Thermal resistance from case to ambient |

**Notes:**
- `rippleCurrentFrequencyPoints` and `rippleCurrentTemperaturePoints` use the [curve](#curve) type. When no curve data is available, use empty arrays for `xData` and `yData`.

---

## thermal

Thermal characteristics. **All fields are required.**

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `temperature` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | degrees C | Operating temperature range. Minimum = lower limit, maximum = upper limit, nominal = reference. |
| `tcc` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | % | Temperature coefficient of capacitance (capacitance change over temperature range). Minimum and maximum define the bounds. |

**Notes:**
- For MLCC Class I (C0G/NP0), `tcc` is typically very small (e.g., +/- 30 ppm/K).
- For MLCC Class II, `tcc` represents the percentage change allowed by the dielectric code (e.g., X7R allows +/- 15%).
- For electrolytic capacitors, `tcc` is often `null` because capacitance vs. temperature behavior is not specified as a simple tolerance.

---

## mechanical

Mechanical dimensions and shape information. Contains two required sub-objects.

### dimensions

Physical dimensions of the component. **All fields are required.** Use `null` for dimensions that do not apply to the form factor (e.g., `diameter` is `null` for chip capacitors; `width` and `length` are `null` for cylindrical capacitors).

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `diameter` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Diameter (cylindrical form factors) |
| `width` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Width / A dimension (rectangular form factors) |
| `length` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Length / B dimension (rectangular form factors) |
| `height` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Height dimension |
| `pitch` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Pin pitch spacing (through-hole form factors) |
| `pinDiameter` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Pin/lead diameter (through-hole form factors) |
| `pinLength` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | meters | Pin/lead length (through-hole form factors) |

### shape

Component shape and assembly information. **All fields are required.**

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `assembly` | string (enum) | No | -- | Assembly / mounting type |
| `shapeType` | string | No | -- | Physical shape of the component |
| `volume` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | cubic meters | Computed component volume |
| `footprint` | [dimensionWithTolerance](#dimensionwithtolerance) | Yes | square meters | PCB footprint area |

#### assembly enum values

| Value | Description |
|---|---|
| `"THT"` | Through-hole technology |
| `"Screw Type"` | Screw terminal mounting |
| `"SMT"` | Surface mount technology |
| `"Snap-In"` | Snap-in mounting (large electrolytic capacitors) |

---

## business

Commercial and supply chain information. **All fields are required.**

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `packaging` | string | No | -- | Packaging format (e.g., "Bulk", "Tape & Reel", "Ammo Pack") |
| `pu` | integer | No | -- | Packaging unit |
| `moq` | integer | No | -- | Minimum order quantity |
| `leadTime` | number | Yes | weeks | Lead time for delivery |
| `stock` | integer | Yes | -- | Available stock quantity |
| `distribution` | string | Yes | -- | Distribution channel |
| `priceCost` | number | No | -- | Cost price |

---

## lifetime

Reliability and lifetime parameters. **All fields are required.** Fields that do not apply to the technology (e.g., Arrhenius parameters for MLCCs) should be set to `null`.

| Field | Type | Nullable | Unit | Description |
|---|---|---|---|---|
| `lifetimeEndurance` | number | Yes | hours | Rated lifetime endurance at maximum temperature and rated ripple current |
| `maxLifetime` | number | Yes | years | Absolute maximum lifetime cap |
| `aexp` | number | Yes | -- | Temperature acceleration exponent (Arrhenius A parameter) |
| `bexp` | number | Yes | -- | Voltage acceleration exponent (Arrhenius B parameter) |
| `deltaT0` | number | Yes | degrees C | Reference temperature delta for the Arrhenius equation |
| `kfactor` | number | Yes | -- | Technology-specific lifetime multiplier |
| `vxfactor` | number | Yes | -- | Voltage stress factor for lifetime calculation |
| `endDefinitionC` | number | Yes | % | End-of-life capacitance change (negative = decrease) |
| `endDefinitionEsr` | number | Yes | % | End-of-life ESR change (positive = increase, e.g., 200 means ESR can increase to 3x initial) |
| `usefulLife` | number | Yes | hours | Useful life duration (more conservative than endurance) |
| `eoUsefulLifeC` | number | Yes | % | End-of-useful-life capacitance change |
| `eoUsefulLifeR` | number | Yes | % | End-of-useful-life resistance change |
| `usefulLifeComment` | string | Yes | -- | Additional comments about useful life conditions |

**Typical values by technology:**

| Technology | lifetimeEndurance | endDefinitionC | endDefinitionEsr |
|---|---|---|---|
| Alum. Electrolytic | 1000--10000 h | -20% | +200% |
| Alum. Polymer | 2000--10000 h | -10% | +100% |
| Hybrid Polymer | 2000--10000 h | -15% | +150% |
| Film Capacitor | 100000 h | -5% | +50% |
| MLCC Class I/II | null (wear-out not applicable) | null | null |

---

## modelParams

Circuit model parameters for SPICE simulation. **All fields are required.** Represents a series RLC circuit with a parallel insulation resistance.

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

Derating factors as functions of frequency and temperature. **All fields are required.** When no derating data is available, use empty arrays.

| Field | Type | Nullable | Description |
|---|---|---|---|
| `rippleCurrentFrequencyFactorFrequency` | number[] | No | Frequency breakpoints for ripple current derating (Hz) |
| `rippleCurrentFrequencyFactorAmplitude` | number[] | No | Derating multipliers at each frequency breakpoint (dimensionless) |
| `rippleCurrentTemperatureFactorTemperature` | number[] | No | Temperature breakpoints for ripple current derating (degrees C) |
| `rippleCurrentTemperatureFactorAmplitude` | number[] | No | Derating multipliers at each temperature breakpoint (dimensionless) |

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

## Utility Types (utils.json)

### dimensionWithTolerance

A physical quantity with minimum, nominal, and maximum values. At least one of the three fields must be present.

| Field | Type | Required | Description |
|---|---|---|---|
| `minimum` | number | At least one required | Minimum value |
| `nominal` | number | At least one required | Nominal (typical) value |
| `maximum` | number | At least one required | Maximum value |

Examples:

```json
{"nominal": 10e-6}
{"minimum": 8e-6, "nominal": 10e-6, "maximum": 12e-6}
{"minimum": -55, "maximum": 125}
```

### curve

X-Y data points for characteristic curves.

| Field | Type | Description |
|---|---|---|
| `xData` | number[] | X-axis values |
| `yData` | number[] | Y-axis values (same length as xData) |

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
