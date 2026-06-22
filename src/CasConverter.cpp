#include "CasConverter.hpp"
#include "CAS.hpp"        // quicktype-generated typed structs (build/CAS.hpp)
#include "Dimension.hpp"  // PEAS::resolve_dimensional_values

#include <stdexcept>
#include <optional>

namespace CAS {

using nlohmann::json;

namespace {

// Real series ESR (Ohm), from the datasheet — nullopt if the part has no ESR spec (e.g. a film cap
// with negligible ESR). Not invented when absent (no-fallback): absent => no ESR atom emitted.
std::optional<double> resolve_esr(const json& peas, const PEAS::Fidelity& fidelity) {
    if (!fidelity.is_real()) return std::nullopt;                 // ideal cap = pure C, no parasitics
    auto capacitor = peas.at("capacitor").get<Capacitor>();
    if (!capacitor.get_manufacturer_info()) return std::nullopt;
    const auto& datasheet = capacitor.get_manufacturer_info()->get_datasheet_info();
    if (fidelity.allowStoredModelParams) {
        if (auto mp = datasheet.get_model_params()) {
            if (mp->get_rs()) return *mp->get_rs();               // fitted series R
        }
    }
    return datasheet.get_electrical().get_esr();                  // scalar ESR (optional)
}

// The chosen capacitance value (F), per the fidelity origin.
double resolve_capacitance(const json& peas, const PEAS::Fidelity& fidelity) {
    using Origin = PEAS::Fidelity::Origin;
    PEAS::reject_mkf_model(fidelity, "CAS");

    if (fidelity.origin == Origin::REQUIREMENTS) {
        if (!peas.contains("inputs") || !peas.at("inputs").contains("designRequirements"))
            throw std::runtime_error("CAS ideal: inputs.designRequirements missing");
        auto dr = peas.at("inputs").at("designRequirements").get<DesignRequirements>();
        return PEAS::resolve_dimensional_values(dr.get_capacitance());
    }

    // DATASHEET (real): physical electrical.capacitance by default; stored modelParams.cs only when allowed.
    if (!peas.contains("capacitor"))
        throw std::runtime_error("CAS real: 'capacitor' component missing from PEAS document");
    auto capacitor = peas.at("capacitor").get<Capacitor>();
    if (!capacitor.get_manufacturer_info())
        throw std::runtime_error("CAS real: capacitor.manufacturerInfo missing");
    const auto& datasheet = capacitor.get_manufacturer_info()->get_datasheet_info();

    if (fidelity.allowStoredModelParams) {
        if (auto mp = datasheet.get_model_params()) {
            if (mp->get_cs()) return *mp->get_cs();
        }
    }
    return PEAS::resolve_dimensional_values(datasheet.get_electrical().get_capacitance());
}

// A minimal ideal-capacitor PEAS document carrying just the value — the leaf atom the CIAS
// converter reads (capacitor...electrical.capacitance.nominal) to emit a C card.
json make_capacitor_atom(double c) {
    json electrical;
    electrical["capacitance"]["nominal"] = c;
    electrical["ratedVoltage"] = 0.0;

    json datasheet;
    datasheet["part"]["partNumber"] = "ideal";
    datasheet["part"]["technology"] = "film-pp";
    datasheet["electrical"] = electrical;
    datasheet["mechanical"]["shape"]["assembly"] = "SMT";
    datasheet["mechanical"]["shape"]["shapeType"] = "chip";

    json atom;
    atom["capacitor"]["manufacturerInfo"]["name"] = "ideal";
    atom["capacitor"]["manufacturerInfo"]["datasheetInfo"] = datasheet;
    return atom;
}

// A parasitic resistor atom (the CIAS converter reads resistor...electrical.resistance to emit an R
// card). Used for a real capacitor's series ESR.
json make_resistor_atom(double r) {
    json atom;
    auto& mfg = atom["resistor"]["manufacturerInfo"];
    mfg["name"] = "parasitic";
    mfg["datasheetInfo"]["part"]["partNumber"] = "esr";
    mfg["datasheetInfo"]["electrical"]["resistance"]["nominal"] = r;
    return atom;
}

json pin_port_net(const std::string& net, const std::string& comp,
                  const std::string& pin, const std::string& port) {
    json endpoints = json::array();
    endpoints.push_back(json{{"component", comp}, {"pin", pin}});
    endpoints.push_back(json{{"port", port}});
    return json{{"name", net}, {"endpoints", endpoints}};
}

} // namespace

json cas_to_cias(const json& peas, const PEAS::Fidelity& fidelity, const std::string& name) {
    const double c = resolve_capacitance(peas, fidelity);
    const std::optional<double> esr = resolve_esr(peas, fidelity);

    json ports = json::array({json{{"name", "1"}}, json{{"name", "2"}}});
    json components = json::array({json{{"name", "C"}, {"data", make_capacitor_atom(c)}}});
    json connections = json::array();

    if (esr && *esr > 0.0) {
        // Real capacitor = C in series with its ESR: port1 -> C -> esr_node -> Resr -> port2.
        // (ESL would add a series L atom here too; the CAS schema has no scalar ESL — only
        // modelParams.ls when allowStoredModelParams — so it is deferred.)
        components.push_back(json{{"name", "Resr"}, {"data", make_resistor_atom(*esr)}});
        connections.push_back(pin_port_net("1", "C", "1", "1"));
        connections.push_back(json{{"name", "esr_node"}, {"endpoints", json::array({
            json{{"component", "C"}, {"pin", "2"}}, json{{"component", "Resr"}, {"pin", "1"}}})}});
        connections.push_back(pin_port_net("2", "Resr", "2", "2"));
    } else {
        // Ideal (or ESR-less) capacitor = single C atom across the two ports.
        connections.push_back(pin_port_net("1", "C", "1", "1"));
        connections.push_back(pin_port_net("2", "C", "2", "2"));
    }

    json leaf;
    leaf["name"] = name;
    leaf["ports"] = ports;
    leaf["components"] = components;
    leaf["connections"] = connections;
    return leaf;
}

} // namespace CAS
