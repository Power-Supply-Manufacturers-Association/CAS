#pragma once

// CasConverter — "generate a CIAS element (leaf) from a CAS part".
//
// cas_to_cias(peas, fidelity) returns a CIAS leaf for ONE capacitor:
//   - REQUIREMENTS (ideal): a single ideal capacitor, value from inputs.designRequirements.capacitance.
//   - DATASHEET   (real)  : capacitor from datasheetInfo.electrical.capacitance (or modelParams.cs,
//                           only when fidelity.allowStoredModelParams). ESR (rs / from
//                           dissipationFactor) + ESL (ls) parasitic atoms are the next step (Phase 4).
//   - MKF_MODEL           : throws (MAS-only origin).
//
// Input `peas` is a PEAS capacitor document ({"capacitor": {...}, "inputs": {...}}).
// Convention: a capacitor's two terminals are pins "1" and "2".

#include <nlohmann/json.hpp>
#include <string>
#include "Fidelity.hpp"

namespace CAS {

nlohmann::json cas_to_cias(const nlohmann::json& peas,
                           const PEAS::Fidelity& fidelity,
                           const std::string& name = "capacitor");

} // namespace CAS
