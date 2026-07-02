// Minimal dependency-free unit tests for cas_to_cias.
#include "CasConverter.hpp"
#include "Fidelity.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using nlohmann::json;
using PEAS::Fidelity;

#include <catch2/catch_test_macros.hpp>
#define CHECK_MSG(cond, ...) do { INFO(__VA_ARGS__); CHECK(cond); } while (0)

static double leaf_capacitance(const json& leaf) {
    return leaf.at("components").at(0).at("data").at("capacitor").at("manufacturerInfo")
               .at("datasheetInfo").at("electrical").at("capacitance").at("nominal").get<double>();
}

TEST_CASE("CAS cas_to_cias", "[cas]") {
    // ideal: value from designRequirements.capacitance
    json idealDoc = json::parse(R"({
        "inputs": { "designRequirements": { "capacitance": { "nominal": 1e-7 }, "ratedVoltage": 50 } }
    })");
    json leaf = CAS::cas_to_cias(idealDoc, Fidelity(Fidelity::Origin::REQUIREMENTS));
    CHECK_MSG(leaf.at("ports").size() == 2, "ideal: 2 ports");
    CHECK_MSG(leaf.at("components").at(0).at("name") == "C", "ideal: component C");
    CHECK_MSG(leaf_capacitance(leaf) == 1e-7, "ideal: value from designRequirements (1e-7)");

    // real: value from electrical.capacitance, not modelParams (stored OFF)
    json realDoc = json::parse(R"({
        "capacitor": { "manufacturerInfo": { "name": "Murata", "datasheetInfo": {
            "part": { "partNumber": "GRM188", "technology": "ceramic-class-2" },
            "electrical": { "capacitance": { "nominal": 2.2e-6 }, "ratedVoltage": 25 },
            "mechanical": { "shape": { "assembly": "SMT", "shapeType": "chip" } },
            "modelParams": { "cs": 2.3e-6 } } } }
    })");
    json realLeaf = CAS::cas_to_cias(realDoc, Fidelity(Fidelity::Origin::DATASHEET));
    CHECK_MSG(leaf_capacitance(realLeaf) == 2.2e-6, "real: value from electrical.capacitance (2.2u)");

    json storedLeaf = CAS::cas_to_cias(realDoc, Fidelity(Fidelity::Origin::DATASHEET, true));
    CHECK_MSG(leaf_capacitance(storedLeaf) == 2.3e-6, "real+allowStored: value from modelParams.cs (2.3u)");

    bool threw = false;
    try { CAS::cas_to_cias(realDoc, Fidelity(Fidelity::Origin::MKF_MODEL)); }
    catch (const std::exception&) { threw = true; }
    CHECK_MSG(threw, "MKF_MODEL origin throws for CAS");
}

TEST_CASE("CAS real capacitor with ESR -> multi-atom (C + series R)", "[cas][real][esr]") {
    json doc = json::parse(R"({
        "capacitor": { "manufacturerInfo": { "name": "Nichicon", "datasheetInfo": {
            "part": { "partNumber": "UPW1V101", "technology": "aluminum-electrolytic-wet" },
            "electrical": { "capacitance": { "nominal": 1e-4 }, "ratedVoltage": 35, "esr": 0.085 },
            "mechanical": { "shape": { "assembly": "THT", "shapeType": "radial" } } } } }
    })");
    json leaf = CAS::cas_to_cias(doc, Fidelity(Fidelity::Origin::DATASHEET));

    REQUIRE(leaf.at("components").size() == 2);
    CHECK_MSG(leaf.at("components").at(0).at("name") == "C", "atom0 = C");
    CHECK_MSG(leaf.at("components").at(1).at("name") == "Resr", "atom1 = Resr (the ESR)");
    double esr = leaf.at("components").at(1).at("data").at("resistor").at("manufacturerInfo")
                     .at("datasheetInfo").at("electrical").at("resistance").at("nominal").get<double>();
    CHECK_MSG(esr == 0.085, "ESR resistor value = 0.085 Ohm");

    // C and ESR share an internal node (series), and ports 1/2 are still exposed.
    bool series = false;
    for (const auto& conn : leaf.at("connections")) {
        bool c2 = false, r1 = false, port = false;
        for (const auto& ep : conn.at("endpoints")) {
            if (ep.contains("component") && ep.at("component") == "C"    && ep.at("pin") == "2") c2 = true;
            if (ep.contains("component") && ep.at("component") == "Resr" && ep.at("pin") == "1") r1 = true;
            if (ep.contains("port")) port = true;
        }
        if (c2 && r1 && !port) series = true;
    }
    CHECK_MSG(series, "C and ESR wired in series via an internal node");

    // ESR-less real cap stays a single C atom (no invented parasitic).
    json noEsr = json::parse(R"({
        "capacitor": { "manufacturerInfo": { "name": "TDK", "datasheetInfo": {
            "part": { "partNumber": "C3216", "technology": "ceramic-class-1" },
            "electrical": { "capacitance": { "nominal": 1e-7 }, "ratedVoltage": 50 },
            "mechanical": { "shape": { "assembly": "SMT", "shapeType": "chip" } } } } }
    })");
    CHECK_MSG(CAS::cas_to_cias(noEsr, Fidelity(Fidelity::Origin::DATASHEET)).at("components").size() == 1,
              "ESR-less real cap = single C atom");
}
