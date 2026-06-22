// PyCAS — pybind11 module exposing the CAS->CIAS converter.
#include <pybind11/pybind11.h>
#include <pybind11_json/pybind11_json.hpp>
#include "CasConverter.hpp"
#include "FidelityJson.hpp"

namespace py = pybind11;
using json = nlohmann::json;

PYBIND11_MODULE(PyCAS, m) {
    m.doc() = "CAS (capacitor) -> CIAS leaf converter";
    m.def("cas_to_cias",
          [](const json& peas, const json& fidelity, const std::string& name) {
              return CAS::cas_to_cias(peas, PEAS::fidelity_from_json(fidelity), name);
          },
          py::arg("peas"), py::arg("fidelity"), py::arg("name") = "capacitor",
          "Convert a CAS capacitor PEAS document to a CIAS leaf.");
}
