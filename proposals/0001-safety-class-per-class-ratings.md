# CAS-RFC 0001 — Per-class ratings on `safetyClass` (and the missing `x1y1`)

- **Status:** Proposed, awaiting owner decision. Nothing has been edited; no schema file was touched.
- **Type:** Additive (non-breaking) schema change.
- **Author:** drafted 2026-09-20
- **Created:** 2026-09-20
- **Depends on:** nothing. One file changes (`CAS/schemas/capacitor.json`). No PEAS change, no
  cross-module `$ref`, no requirement-side edit (see *Propagation*, below).

> **Note on location.** `CAS/` had no `proposals/` directory before this file. Five sibling
> repos in this workspace do (`MAS/proposals` 18 RFCs, `PEAS`, `SAS`, `CONAS`, `TAS` one each),
> all numbered `NNNN-slug.md` with a `<REPO>-RFC NNNN` title, so this follows the house
> convention rather than inventing one. `safetyClass` is defined only in CAS, so CAS is the
> repo that owns the decision. Move it if you would rather the directory did not exist yet.

## Summary

`safetyClass` cannot describe a capacitor that holds **two** safety certifications, because it
records one class token and the record carries one `ratedVoltage`. Both halves fail:

1. The `class` enum has no value for a part certified **X1 and Y1**, which 61 catalogue rows
   need.
2. More importantly, a dual-class certification carries **a rated voltage per class**, and the
   two differ. TDK prints `X1/440VAC, Y1/400VAC` on one line. A single scalar `ratedVoltage`
   keeps one of them and silently drops the other, with nothing in the row saying which class
   the surviving number belongs to. Adding `x1y1` to the enum does **not** fix this — 573 rows
   already carry the existing dual token `x1y2` and 117 of them are demonstrably storing one
   class's voltage while their own series string names both.

The proposal is therefore a pair: add the missing enum value **and** add an optional
`ratings[]` array that states the rated AC voltage per class.

## Where `safetyClass` is defined, and what it is today

`CAS/schemas/capacitor.json`, at `$defs.part.allOf[1].properties.safetyClass` — an object,
not a bare enum, sitting in the part-identity block beside `technology`, `dielectricCode` and
`qualifiedReliability`:

```json
"safetyClass": {
  "type": ["object", "null"],
  "additionalProperties": false,
  "required": ["class"],
  "properties": {
    "class":     { "type": "string", "enum": ["x1", "x2", "y1", "y2", "y4", "x1y2", "none"] },
    "standard":  { "type": ["string", "null"] },
    "approvals": { "type": ["array", "null"], "items": { "type": "string" } }
  }
}
```

Its own description already argues the case this RFC extends:

> `x1y2` is a single value on purpose, not tidiness: dual-approved parts are real and their
> datasheets say so […] and a single-valued enum would force a false answer on them.

### Propagation — this corrects a premise

Requirement-side enums in this workspace `$ref` the part-side anchor so they cannot drift
(`inputs/designRequirements.json` → `../capacitor.json#/$defs/technology` for
`allowedTechnologies`). **`safetyClass` has no such mirror.** A text sweep of every schema,
doc, example and source file in `PEAS CAS SAS RAS MAS TAS CTAS AAS CONAS CIAS COAS` finds the
identifier in exactly two schema files: `CAS/schemas/capacitor.json` (2 occurrences, both the
definition above) and `COAS/schemas/utils.json` (1 occurrence, an **unrelated** quantity —
`{"enum": ["I","II","III"], "description": "IEC protective-earthing class."}`, a converter
property, not an interference-suppression class). `CAS/schemas/inputs/designRequirements.json`
never mentions it.

So adding an enum member propagates nowhere and needs **one** edit, not two. If a
requirement-side `allowedSafetyClasses` is ever added it must `$ref` this anchor, exactly as
`allowedTechnologies` does.

## Evidence

All counts measured 2026-09-20 against the live catalogues in this checkout
(`TAS/data/capacitors.ndjson`, 251,319 records; `TAS/data/quarantine.ndjson`, 98,690).

**Class census of the live capacitor catalogue** — 11,948 rows carry a `safetyClass` object:

| class | rows |
|---|---|
| `x2` | 6,447 |
| `x1` | 2,789 |
| `y2` | 2,088 |
| `x1y2` | 573 |
| `y1` | 51 |
| absent / `null` | 239,371 |

**(a) 61 rows need a class the enum does not have.** 60 live rows plus 1 in quarantine are
KEMET C700 discs whose **series string itself spells both certifications** while
`safetyClass` is absent:

| series, verbatim | rows | example part |
|---|---|---|
| `C700KJN SFTY X1-440 Y1-400` | 29 | `C722U101KSYDBA7301` |
| `C700KJN SFTY X1-760 Y1-500` | 17 | `C721U102MSWDBA7317` |
| `C700KJN SFTY X1-440 Y1-250` | 14 | `C721U102MTWDBA7317` |

The user's figure of 61 is confirmed (60 + 1 quarantined). Note what the series names also
carry: two voltages each, 440/400, 760/500, 440/250.

**(b) The voltage half of the gap is live and already materialised.** 318 live TDK rows hold
`ratedVoltage: 440` with `safetyClass` absent (series `CS` 159, `CD` 102, series literally
`"TBD"` 57). TDK's own product page for one of them prints, verbatim:

```
CD11ZU2GA332MYGKA
  Capacitance      3.3nF ±20%
  Rated Voltage    X1/440VAC, Y1/400VAC
  Withstanding Voltage   4kVAC
```
(<https://product.tdk.com/en/search/capacitor/ceramic/lead-disc/info?part_no=CD11ZU2GA332MYGKA>)

A stratified sample of **73 of those 318** was fetched from `product.tdk.com` (3 per part-number
prefix, all 200 OK):

- 20 parts print `X1/440VAC, Y1/400VAC` — e.g. `CD11ZU2GA332MYNKA`, `CD12ZU2GA472MYVKA`,
  `CD45SL2GA470JYGKA`, `CD70-B2GA151KYVKA`.
- 21 parts print `X1/440VAC, Y2/300VAC` — e.g. `CS45-B2GA151K-GKA`, `CS70-B2GA331KYGKA`,
  `CS65-B2GA221KANKA`, `CS14-F2GA103MYVKA`.
- 32 parts show an **empty** rated-voltage cell on TDK's page. These are exactly the 57
  series-`"TBD"` rows: every one of the 32 is a part number ending in `S` (bulk/lead-length
  variant), and every one of the 41 that published a pair does not. That correlation is
  perfect in the sample (41/41 vs 32/32). Flagged, **not acted on** — where our 440 V figure
  for those 57 rows came from is a separate data question, not a schema question.

So in the probed sample **every TDK safety disc whose page publishes a rated voltage publishes
two classes with two different voltages**, and the pattern varies (Y1/400 vs Y2/300) — which
argues for a structured shape, not for a second enum member.

**(c) The enum alone demonstrably does not fix the voltage.** 117 rows are *already* classed
`x1y2`, and their series string names both ratings while `ratedVoltage` holds only one:

| series, verbatim | rows | stored `ratedVoltage` | the rating that is lost |
|---|---|---|---|
| `C700KJY SFTY X1-400 Y2-250` | 66 | `400` | Y2 250 V |
| `C700KJY SFTY X1-440 Y2-300` | 51 | `440` | X1's twin, Y2 300 V |

And across the whole `x1y2` cohort the stored voltage is not even consistently the X figure:
191 KEMET rows at 1000 V, 206 at 300 V (`R41T`/`R41D`/`R41B`/`R41P`), 66 at 400 V, 51 at
440 V, 48 at 250 V, 11 Würth `WCAP-FTY2` at 1000 V. Nothing in any of those rows says which
class the number belongs to. That is the defect the 318/400 V duplicate pair reproduced at
import time: two rows of the same part, both read correctly, one taking the X rating and one
the Y.

**(d) The Delta "X2 + Y2 + Y2" assembly could not be found, and is a different problem.**
Searched every `TAS/data/*.ndjson` (21 catalogues) for a manufacturer field containing
"delta", case-insensitive: **zero rows**. The only EMI-filter records are Molex (36) and
Amphenol CS (1) in `connectors.ndjson` and Laird (18 + 2 quarantined) in `magnetics.ndjson` —
none of them Delta, none of them carrying X/Y class tokens. Not one capacitor record in the
catalogue mentions both `X2` and `Y2`. **The 5-part figure is unverified**; the parts are not
in the live catalogues visible from this checkout.

On the merits it is a separate problem regardless, and this RFC says so plainly rather than
forcing one solution: *a three-capacitor filter is not one capacitor*. "X2 + Y2 + Y2" is
multiplicity of **components**, not multiplicity of **certifications** held by one component.
The workspace already models that — a CIAS brick is `components[]`, each entry a named
reference designator plus its own PEAS document, so the assembly is three capacitor records
(one X2, two Y2) wired by nets inside a brick in `TAS/data/circuits.ndjson`. Stretching
`safetyClass` to carry repeated classes would encode a bill of materials in a certification
field and would still not say which capacitor sits where. See *Alternatives*, item 6.

## A third defect, found by pulling all 318 TDK parts: the value is in the wrong field

Revision 2 treated this as "one scalar cannot hold two class ratings". A full per-part pull of
the 318 TDK `2GA` discs shows the scalar is also the WRONG QUANTITY, and that the two problems
have to be solved together.

What TDK publishes, per part, for the 261 it still rates:

| TDK's own `Rated Voltage` field | parts |
|---|---|
| `X1/440VAC, Y2/300VAC` | 159 |
| `X1/440VAC, Y1/400VAC` | 102 |

**Both numbers are VAC. Not one of the 261 is single-rated.** The catalogue stores 440 for all
of them, and that 440 is consistently the X rating — 261/261, no exceptions — so the cohort is
at least internally consistent.

But `capacitor.json` already distinguishes these quantities and the data ignores it:

- `electrical.voltageRatedAcMax` — *"Maximum continuous AC rated voltage in Volts RMS … the
  headline rating of an interference-suppression (X/Y class) capacitor — 275 / 300 / 305 / 310
  / 440 V~ — and is a DIFFERENT quantity from `ratedVoltage` / `voltageRatedDcMax`"*. It names
  440 V~ explicitly, and it is optional and nullable.
- `electrical.ratedVoltage` — described only as *"Rated voltage in Volts"*, but grouped with
  the DC quantity by the sentence above, and **`required` alongside `capacitance`**.

So 261 live rows carry an AC figure in the field the schema's own text distinguishes FROM the
AC one, while the field that exists for exactly this number sits empty. And because
`ratedVoltage` is required, an AC-only part has **no legal way to omit it** — the data is not
merely sloppy, the schema leaves it nowhere correct to go.

### The 400 V was not the other half of the pair

Worth recording, because it nearly became a "fix". 318 deleted duplicates all held 400.0 — a
constant, not a per-part value. It matches the published Y rating on only 102 of 261; for the
other 159 the Y rating is 300 VAC and 400 appears nowhere in TDK's rating. It behaves like a
uniform decode of the `2G` order-code field (generic TDK/JIS code 2G = 400 V **DC**) applied
across the family. Writing it would have put a number TDK does not publish onto 159 safety
capacitors — worse than the half-truth already there.

### What this adds to the proposal

`ratings[]` fixes the multiplicity. It does not by itself answer:

1. **What `ratedVoltage` holds for an AC-only part.** Options: keep it required and let it hold
   the X-class AC figure (status quo, and the schema's own text argues against it); relax it to
   optional when `voltageRatedAcMax` is present; or require exactly one of the two. Relaxing a
   required field is the one part of this proposal that is NOT purely additive.
2. **Whether the 261 rows should be migrated** to put 440 in `voltageRatedAcMax`, which is a
   data change following the schema decision rather than preceding it.

Both are recorded here rather than decided.

## What IEC 60384-14 actually constrains — and what it does not

This section was added after the first draft, because the standard settles a question the
draft had left to judgement: whether a subclass token implies a voltage.

**It does not, for X — and the reason is structural.** The X subclasses are defined by the
peak impulse the capacitor must withstand in service, NOT by its rated voltage
(Vishay *General Technical Information*, doc 26529, reproducing IEC 60384-14):

| Sub-class | Peak pulse in service | Peak impulse before endurance test |
|---|---|---|
| X1 | > 2.5 kV, ≤ 4.0 kV | 4 kV (C ≤ 1 µF) |
| X2 | ≤ 2.5 kV | 2.5 kV (C ≤ 1 µF) |
| X3 | ≤ 1.2 kV | none |

There is no rated-voltage column. So `x1` constrains impulse withstand and says nothing
about whether the part is 400 V or 440 V or 760 V.

**It does, for Y** — the Y subclasses are defined by the insulation they bridge *and* a
rated-voltage band:

| Sub-class | Insulation bridged | Rated voltage | Peak impulse |
|---|---|---|---|
| Y1 | Double or reinforced | ≤ 500 Vac | 8 kV |
| Y2 | Basic or supplementary | ≥ 150 Vac, ≤ 300 Vac | 5 kV |
| Y4 | Basic or supplementary | < 150 Vac | 2.5 kV |

(Editions differ: one secondary source gives Y1 as "Reinforced, ≤ 250 V". The ≤ 500 Vac
figure is the one printed in the Vishay table above and is consistent with the live data —
see the `X1-760 Y1-500` cohort below. Where they disagree the schema should take the looser
bound, since a schema that rejects a real certified part is worse than one that admits a
wrong one, which the ingest gate can still catch.)

**Vendor evidence that the pair genuinely varies.** The same X1 token appears at three
different voltages and the same Y1 token at three more, across real datasheets:

- jbcapacitors JYA: `Class X1, 440VAC, Class Y1, 400VAC` (suffix CD) and
  `Class X1, 400VAC, Class Y2, 300VAC` (suffix CE)
- KEMET C700 series, the vendor's own order-code suffix table:
  `Q = 440V/X1, 250Vac/Y1` · `R = 400Vac/X1, 250Vac/Y2` · `S = 440Vac/X1, 400Vac/Y1`
- KEMET series summary: `KJN: Y1 (250Vac/400Vac) / X1 (440Vac)` and
  `KJY: Y2 (250Vac) / X1 (400Vac)`

That last one is decisive for this repo: the 61 X1+Y1 rows found in the catalogue are
C700KJN discs, and KEMET itself publishes **one series carrying two different Y1 voltages**.
No mapping from class token to voltage can exist, not even per series.

### Consequence for enforcing agreement in-schema

The decision to enforce `class` ↔ `ratings` agreement in the schema is adopted, but it can
only mean two of the three things it might have meant:

1. **Token agreement — enforceable.** `class: "x1y1"` must be accompanied by exactly the
   ratings `{x1, y1}`; `x1y2` by `{x1, y2}`; a single-class token by that one class. This is
   pure bookkeeping and belongs in the schema.
2. **Y voltage bands — enforceable.** `y1` implies `ratedVoltageAc ≤ 500`, `y2` implies
   `150 ≤ ratedVoltageAc ≤ 300`, `y4` implies `< 150`. Straight from the table.
3. **X voltage — NOT enforceable, and must not be attempted.** The standard sets no
   rated-voltage bound for X1/X2/X3. Inventing one would reject real parts: the live
   catalogue already holds `X1-760`, and X1 legitimately appears at 400 V and 440 V on the
   same vendor's shelf.

A schema that constrained X voltages would look stricter and be wrong. That asymmetry is
not an omission; it is what the standard says.

## Why the quantity belongs here

`safetyClass` is part identity, and the per-class rated voltage is part of the same
certificate: IEC 60384-14 grants a class *at a stated voltage*, so class and voltage are one
fact, not two. Putting the voltage beside the class — rather than adding more scalars under
`electrical` — keeps the pairing inseparable and self-describing.

It stays in **CAS**, not PEAS. The workspace rule is that a type needed by several modules
belongs in PEAS and a type needed by one stays in that module. Only CAS has interference
suppression classes; COAS's identically-named field is the IEC protective-earthing class and
shares nothing but the spelling. Hoisting would create a PEAS def with exactly one consumer,
and PEAS may never `$ref` back into CAS anyway.

`electrical.ratedVoltage` and `electrical.voltageRatedAcMax` are untouched. They remain the
headline scalar for indexing and filtering; `ratings[]` is the authoritative per-class
statement when present.

## Proposed change

One file: `CAS/schemas/capacitor.json`, at
`$defs.part.allOf[1].properties.safetyClass.properties`.

```diff
 "class": {
-  "description": "The safety class as certified. … `x1y2` is a single value on purpose …",
+  "description": "The safety class as certified. … `x1y2` and `x1y1` are single values on
+   purpose … When `ratings` is present, `class` is the SUMMARY token for the same fact and
+   must agree with it: a single-entry `ratings` uses that class's own token, a two-entry
+   `ratings` uses the combined token (`x1y2`, `x1y1`). `none` means the sheet states no
+   safety-class approval.",
   "type": "string",
-  "enum": ["x1", "x2", "y1", "y2", "y4", "x1y2", "none"]
+  "enum": ["x1", "x2", "y1", "y2", "y4", "x1y2", "x1y1", "none"]
 },
+"ratings": {
+  "description": "The certification stated per class: IEC 60384-14 grants a class AT a rated
+   voltage, and a dual-approved part carries a DIFFERENT voltage for each class — TDK prints
+   `X1/440VAC, Y1/400VAC` on one line, KEMET names `X1-440 Y2-300` in the series itself.
+   One entry per class held, each with the AC voltage that class is granted at. Items use
+   single-class tokens only: a rating belongs to one class by construction, so the combined
+   tokens are not legal here. Omit when the datasheet states a class but no per-class
+   voltage; `electrical.voltageRatedAcMax` / `electrical.ratedVoltage` are unchanged and keep
+   the headline scalar. Repeating a class to describe an assembly of several capacitors is
+   NOT what this field is for — that is a CIAS brick of several capacitor parts.",
+  "type": "array",
+  "minItems": 1,
+  "items": {
+    "type": "object",
+    "additionalProperties": false,
+    "required": ["class", "ratedVoltageAc"],
+    "properties": {
+      "class": {
+        "description": "The single class this rating is granted for.",
+        "type": "string",
+        "enum": ["x1", "x2", "y1", "y2", "y4"]
+      },
+      "ratedVoltageAc": {
+        "description": "Rated AC voltage in Volts RMS for THIS class, at the datasheet's
+         reference frequency (50/60 Hz unless the sheet says otherwise).",
+        "type": "number",
+        "exclusiveMinimum": 0
+      },
+      "standard": {
+        "description": "The standard this class is granted under, verbatim as printed, when it
+         differs per class. Omit when the part-level `standard` covers both.",
+        "type": ["string", "null"]
+      }
+    }
+  }
+}
```

Nothing else changes: `class` stays **required**, `standard` and `approvals` are untouched, no
field becomes required, and `additionalProperties: false` on the `safetyClass` object stays.

What the schema cannot check, stated rather than assumed: that `class` agrees with `ratings`
(a `class: "x1"` beside a two-entry `ratings` validates), and that a class is not repeated
(`uniqueItems` compares whole objects, so `x1@440` and `x1@250` are "unique"). Both are
one-line checks for the ingest gate, in the same category as CIAS's unresolvable component
URIs. If you would rather have them enforced in-schema, an `if`/`then` per combined token
would do it at the cost of seven branches; I did not propose that, and will add it on request.

## Compatibility

Additive and non-breaking. Every currently valid document stays valid:

- `ratings` is a new optional property inside an object that is already
  `additionalProperties: false`, so the object only becomes *less* restrictive.
- `x1y1` is a new enum member; no existing value is removed or renamed.
- No new required field. `class` remains required exactly as before.
- All 11,948 rows that carry a `safetyClass` object today validate unchanged, and the 239,371
  rows with no class are unaffected.
- Consumers that read `.safetyClass.class` keep finding a token on every classed row — which
  is the reason `class` stays required rather than being displaced by the array.
- The requirement side is untouched because nothing on the requirement side references
  `safetyClass` (see *Propagation*).

No migration is forced. Backfilling the 61 X1/Y1 rows and the 318 TDK rows is a **separate
change set** and is not performed in advance of this decision.

## Alternatives considered

1. **Add `x1y1` to the enum and nothing else.** Rejected. It records that a part holds both
   certifications while still forcing one of the two rated voltages to be discarded, with
   nothing saying which one survived. This is not speculative: 117 rows already classed
   `x1y2` are in exactly that state today, and the stored voltage across the `x1y2` cohort
   ranges over 250/300/400/440/1000 V with no per-class attribution. The bare enum addition
   would make 61 more rows *look* correct while the same information keeps being lost.

2. **Two scalars, `voltageRatedAcMaxX` and `voltageRatedAcMaxY`, under `electrical`.**
   Rejected. It hard-codes "at most one X and one Y", puts class semantics into field names
   where no enum governs them, and separates the voltage from the certificate it belongs to —
   a reader of `safetyClass` would have to know to look somewhere else for half the fact.

3. **Replace the `safetyClass` object with an array of per-class objects.** Cleanest in
   isolation, rejected on compatibility: it invalidates all 11,948 rows carrying the object
   form and every consumer reading `.safetyClass.class`, for a gain the optional `ratings[]`
   already delivers.

4. **Hoist `safetyClass` (or the new rating type) into PEAS.** Rejected by the dependency
   rule: PEAS is the root and a type used by one module stays in that module. Only CAS has
   this quantity; COAS's `safetyClass` is the IEC protective-earthing class `I|II|III` and is
   a different fact with the same name.

5. **Store an X1/Y1 part as two records, one per class.** Rejected. One orderable part has
   one part number; two rows claiming it is the duplicate-pair failure mode that produced the
   318 rows at 440 V and their now-deleted 400 V twins — both read correctly from the same
   datasheet, and the catalogue could not tell which was which.

6. **Let `ratings` repeat a class, so the X2 + Y2 + Y2 assembly becomes one capacitor record.**
   Rejected, and this is the "these are two different problems" answer. Repeating `y2` would
   say "this capacitor is twice certified Y2", which is false; the truth is that there are two
   Y capacitors. A multi-capacitor filter is a CIAS brick whose `components[]` are three
   capacitor parts with their own part numbers, classes and connections — the shape the
   workspace already uses for exactly this. Nothing in this RFC should be read as making
   `safetyClass` able to describe an assembly.

7. **Leave it and keep the classification in the series string.** This is the status quo: the
   fact is present in the catalogue as prose (`C700KJN SFTY X1-440 Y1-400`) where no query can
   filter on it, and 379 rows (61 + 318) carry no class at all. It also violates the house
   rule that a name field holds names, not data.

## Implementation

If accepted:

1. One edit to `CAS/schemas/capacitor.json` as diffed above.
2. `CAS/docs/schema.md` and `CAS/README.md` — both already document `safetyClass` and quote
   the `x1y2` rationale (2 mentions each); both gain `x1y1` and the `ratings` field in the same
   change, per the workspace rule that schema, docs and examples move together.
3. `CAS/examples/` — the existing `03_film_x2_safety_mkp.json` stays as the single-class case;
   one example gains a dual-class part so the shape is exercised by a document, not only by a
   test.
4. The check that actually measures the change, rather than one that would pass either way: a
   **real** row from the live catalogue (`C721U102MTWDBA7317`, series
   `C700KJN SFTY X1-440 Y1-250`) given `class: "x1y1"` and
   `ratings: [{class: "x1", ratedVoltageAc: 440}, {class: "y1", ratedVoltageAc: 250}]`,
   validated against `capacitor.json` with the full sibling registry — **valid after the
   change and invalid before it**, both asserted. Plus negatives: a combined token inside a
   `ratings` item must be rejected, and a `ratings` entry without `ratedVoltageAc` must be
   rejected. `pytest CAS/tests/ -q` green.
5. Data lands separately and only after acceptance: the 61 X1/Y1 rows, and the 318 TDK rows
   whose per-class pairs are re-read from the vendor (including the question of where the
   57 series-`"TBD"` rows' 440 V came from, which this RFC flags and does not answer).


## Amendments after implementation (2026-09-21)

**Enforcement was implemented, reversing the "Proposed change" section above.** That section
deferred class/ratings agreement to the ingest gate and offered in-schema `if`/`then` "on
request". The owner requested it. Implemented as eight branches on `safetyClass`: one per
single-class token (the ratings must be exactly that class), one each for `x1y2` and `x1y1`
(exactly those two classes), and `none` (no `ratings` at all). Per-item bands enforce the Y
classes only — Y1 at most 500 V, Y2 150 to 300 V, Y4 below 150 V — and **no X class carries a
voltage bound**, because IEC 60384-14 defines X sub-classes by impulse withstand, not voltage.

**A null trap, found by building it.** `safetyClass` is `["object","null"]`, and `required` and
`properties` pass *vacuously* on a non-object. An unguarded `if` therefore evaluates TRUE on
`null`, and the `none` branch's `then: {not: {required: [ratings]}}` then fails — so every null
`safetyClass` in the catalogue would have been rejected. Each `if` carries `"type": "object"`.
Proven load-bearing: removing it makes a null `safetyClass` invalid.

**A description corrected.** The `class` description gave Y4 as "250 V rated". IEC 60384-14 as
reproduced in Vishay doc 26529 gives Y4 as basic/supplementary, rated below 150 V. With the Y4
band now enforced, leaving 250 V in the text would have had the schema contradict itself. Zero
live rows are Y4, so the correction changes no data.

**Counter-checks run.** A real KEMET C700 disc (`C722U101KSYDBA7301`, series `X1-440 Y1-400`) with
`x1y1` and ratings `X1@440, Y1@400` is valid after and REJECTED before. Eight fixtures behave as
specified, including `x1` at 760 V accepted (no X band) and a null `safetyClass` accepted. All
253,832 live capacitors remain valid under the new schema, and 0 of them carried `ratings[]`
beforehand, so no existing row could be affected by the new constraints.

**Still open:** 261 TDK rows hold an AC rating in required `ratedVoltage` while
`voltageRatedAcMax` sits empty. Moving them needs `ratedVoltage` relaxed for AC-only parts,
which is not purely additive and is left for a separate decision. No data was migrated into
`ratings[]` by this change.

## Amendments after the second implementation (2026-09-23)

**The deferred, non-additive half landed.** `electrical.ratedVoltage` is no longer
unconditionally required. `required` keeps `capacitance` alone, and an `anyOf` beside it
demands **at least one of** `ratedVoltage` or a *numeric* `voltageRatedAcMax`. Three
properties of that shape are deliberate:

1. **It is scoped, not a blanket relaxation.** A record carrying neither field is still
   rejected — asserted before and after the change, so the negative is known to be measuring
   something.
2. **`voltageRatedAcMax: null` does not satisfy it.** The AC branch pins
   `voltageRatedAcMax` to `type: "number"`, because null means "no AC rating published" and
   would otherwise leave a part with no rated voltage at all while passing.
3. **Nothing currently valid becomes invalid.** Every record that validated before carries
   `ratedVoltage`, which is branch one verbatim. Confirmed across all 253,832 live
   capacitors.

The `anyOf` sits beside `additionalProperties: false` rather than inside an allOf branch, so
no branch has to re-declare the base's properties; the AC branch's lone `properties` entry
constrains a type and does not participate in the closure.

`CAS/docs/schema.md` moved with it (the electrical section's "Required fields" sentence and
the `ratedVoltage` row).

**Option 1 of "What this adds to the proposal" is therefore settled**, and it was settled the
way the schema's own text argued for: `ratedVoltage` is the DC quantity, an AC-only part puts
its figure in `voltageRatedAcMax`, and the schema now lets it.

### The 261 TDK rows were migrated

Re-derived per part from TDK — not from the series split recorded above — using two
independent TDK sources that agree on all 261 parts:

- the TDK Meister product database (`TstDB.tmdb`), specification types `301000350` and
  `301000910` (identical rating strings on every part) cross-checked against `301000900`,
  which stores the same fact as two short-form rows (`X1/440` + `Y1/400`);
- the TDK Product Center detail page for each exact order code.

Both readings agree part-for-part, 261/261, and reproduce the split the earlier pull found:
**159 `X1/440VAC, Y2/300VAC` and 102 `X1/440VAC, Y1/400VAC`.** Per row: 440 moved from
`ratedVoltage` (now removed) into `voltageRatedAcMax`, and `safetyClass` — previously absent
on all 261 — was populated with the combined token plus a two-entry `ratings[]` carrying each
class at its own voltage. No row's pair had to be inferred from a sibling, so none was left
behind.

Note what the migration did NOT write: the generic order-code field `2G` decodes to 400 V DC
and is present in the Meister row as a numeric spec, exactly as this proposal predicted. It
was not used for anything.

### What did not survive contact with the schema

**The `x1` first, `y*` second ordering of `ratings[]` is convention, not constraint.** The
schema's agreement branches check the SET of classes, so `[y2, x1]` validates identically.
The migration writes the vendor's own printed order; nothing enforces it.

**`standard` was left unset on every migrated row.** TDK's per-class rating strings name the
class and the voltage and no standard, and the part-level `standard` was equally absent. The
field exists for the case where a certificate names one; inventing `IEC 60384-14` from the
class token would be exactly the inference the `safetyClass` description forbids.

### A consequence outside the schema, surfaced not silenced

The TAS C++ physics validator scores capacitor completeness against a core-field manifest of
`{capacitance, ratedVoltage}` with a 0.60 floor. With `ratedVoltage` legitimately gone, all
261 migrated rows drop from completeness 1.0 to 0.5 and raise `GEN_SPARSE` (SUSPICIOUS) —
rows that were `Ok` before the migration. The data is not sparse; the manifest predates the
schema change and does not know the AC spelling of "this part states a rated voltage". The
validator already has the idiom for this (`"dcResistance|dcResistances"` on magnetics), so the
fix is one manifest entry, `"ratedVoltage|voltageRatedAcMax"` — but it is a C++ change to a
fabrication detector, so it is reported here rather than made.
