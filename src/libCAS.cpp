// libCAS — emscripten/embind module for the CAS->CIAS converter (JSON-string ABI).
#include <emscripten/bind.h>
#include "CasConverter.hpp"
#include "FidelityJson.hpp"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

static std::string cas_to_cias_json(std::string peasStr, std::string fidelityStr) {
    try {
        auto leaf = CAS::cas_to_cias(json::parse(peasStr),
                                     PEAS::fidelity_from_json(json::parse(fidelityStr)));
        return leaf.dump();
    } catch (const std::exception& e) {
        return std::string("Exception: ") + e.what();
    }
}

EMSCRIPTEN_BINDINGS(cas) {
    emscripten::function("cas_to_cias", &cas_to_cias_json);
}
