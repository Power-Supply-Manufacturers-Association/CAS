cmake_minimum_required(VERSION 3.15)
project(CAS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

include(FetchContent)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG tags/v3.11.3)
FetchContent_MakeAvailable(json)

# --- quicktype(schemas) -> CAS.hpp ---
find_program(QUICKTYPE quicktype REQUIRED)
set(CAS_HPP "${CMAKE_CURRENT_BINARY_DIR}/CAS.hpp")
set(PEAS_SCHEMAS "${CMAKE_CURRENT_SOURCE_DIR}/../PEAS/schemas")
add_custom_command(
    OUTPUT "${CAS_HPP}"
    COMMAND "${QUICKTYPE}" -l c++ -s schema
            "${CMAKE_CURRENT_SOURCE_DIR}/schemas/CAS.json"
            -S "${CMAKE_CURRENT_SOURCE_DIR}/schemas/capacitor.json"
            -S "${CMAKE_CURRENT_SOURCE_DIR}/schemas/inputs.json"
            -S "${CMAKE_CURRENT_SOURCE_DIR}/schemas/inputs/designRequirements.json"
            -S "${CMAKE_CURRENT_SOURCE_DIR}/schemas/outputs.json"
            -S "${CMAKE_CURRENT_SOURCE_DIR}/schemas/utils.json"
            -S "${PEAS_SCHEMAS}/utils.json"
            -S "${PEAS_SCHEMAS}/inputs/twoTerminalOperatingPoint.json"
            -S "${PEAS_SCHEMAS}/inputs/operatingConditions.json"
            -S "${PEAS_SCHEMAS}/inputs/operatingPointExcitation.json"
            -S "${PEAS_SCHEMAS}/outputs/outputBase.json"
            -S "${PEAS_SCHEMAS}/outputs/measurementCondition.json"
            -S "${PEAS_SCHEMAS}/outputs/complexMatrixAtFrequency.json"
            -S "${PEAS_SCHEMAS}/outputs/insulationCoordination.json"
            -S "${PEAS_SCHEMAS}/outputs/lossElementPerHarmonic.json"
            -o "${CAS_HPP}"
            --namespace CAS --source-style single-source --type-style pascal-case
            --member-style underscore-case --enumerator-style upper-underscore-case --no-boost
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/schemas/CAS.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/schemas/capacitor.json"
            "${CMAKE_CURRENT_SOURCE_DIR}/schemas/inputs/designRequirements.json"
    COMMENT "Generating CAS.hpp via quicktype"
    VERBATIM)
add_custom_target(cas_generated_header DEPENDS "${CAS_HPP}")

add_library(CAS STATIC src/CasConverter.cpp)
add_dependencies(CAS cas_generated_header)
target_include_directories(CAS PUBLIC
    src
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../PEAS/src")
target_link_libraries(CAS PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(CAS PRIVATE "${json_SOURCE_DIR}/include/nlohmann")

if (NOT EMSCRIPTEN)
    option(CAS_BUILD_TESTS "Build CAS unit tests" ON)
    if (CAS_BUILD_TESTS)
        enable_testing()
        FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.8.1)
        FetchContent_MakeAvailable(Catch2)
        add_executable(cas_tests tests/test_cas.cpp)
        target_link_libraries(cas_tests PRIVATE CAS Catch2::Catch2WithMain)
        add_test(NAME cas_tests COMMAND cas_tests)
    endif()

    option(CAS_BUILD_PYBIND "Build the PyCAS pybind11 module" ON)
    if (CAS_BUILD_PYBIND)
        FetchContent_Declare(pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11.git GIT_TAG v2.13.6)
        FetchContent_Declare(pybind11_json
            GIT_REPOSITORY https://github.com/pybind/pybind11_json.git)
        FetchContent_MakeAvailable(pybind11 pybind11_json)
        pybind11_add_module(PyCAS src/bindings.cpp)
        add_dependencies(PyCAS cas_generated_header)
        target_include_directories(PyCAS PRIVATE
            src "${CMAKE_CURRENT_SOURCE_DIR}/../PEAS/src"
            "${pybind11_json_SOURCE_DIR}/include")
        target_link_libraries(PyCAS PRIVATE CAS)
    endif()
else()
    add_executable(libCAS src/libCAS.cpp)
    add_dependencies(libCAS cas_generated_header)
    target_include_directories(libCAS PRIVATE
        src "${CMAKE_CURRENT_SOURCE_DIR}/../PEAS/src")
    target_link_libraries(libCAS PRIVATE CAS)
    set_target_properties(libCAS PROPERTIES SUFFIX ".wasm.js")
    target_compile_options(libCAS PRIVATE -fwasm-exceptions)
    target_link_options(libCAS PRIVATE
        -fwasm-exceptions -sWASM=1 -sALLOW_MEMORY_GROWTH -sEXPORT_ES6=1
        -sMODULARIZE=1 -sENVIRONMENT=web,node --bind)
endif()
