"""CAS schema tests (pytest + jsonschema + referencing), cloned from the RAS harness.

Run from the CAS repo root with the sibling repos checked out alongside:

    pip install pytest jsonschema referencing
    pytest tests/ -q
"""
import copy
import json
from pathlib import Path

import pytest
from jsonschema import Draft202012Validator
from referencing import Registry, Resource
from referencing.jsonschema import DRAFT202012

REPO = Path(__file__).resolve().parents[1]
WORKSPACE = REPO.parent
SIBLINGS = ["PEAS", "MAS", "CAS", "SAS", "RAS", "AAS", "CTAS", "CONAS", "TBAS", "TAS", "CIAS", "COAS"]
CAS_SCHEMAS = sorted(REPO.glob("schemas/**/*.json"))


def build_registry():
    resources = []
    for repo in SIBLINGS:
        sdir = WORKSPACE / repo / "schemas"
        if not sdir.is_dir():
            continue
        for path in sdir.rglob("*.json"):
            doc = json.loads(path.read_text())
            if "$id" in doc:
                resources.append((doc["$id"], Resource.from_contents(doc, default_specification=DRAFT202012)))
    return Registry().with_resources(resources)


@pytest.fixture(scope="session")
def registry():
    return build_registry()


@pytest.fixture(scope="session")
def cas_validator(registry):
    return Draft202012Validator(json.loads((REPO / "schemas/CAS.json").read_text()), registry=registry)


@pytest.fixture(scope="session")
def peas_validator(registry):
    return Draft202012Validator(
        json.loads((WORKSPACE / "PEAS/schemas/peas.json").read_text()), registry=registry)


@pytest.fixture()
def mlcc_doc():
    return json.loads((REPO / "examples/01_mlcc_grm32_1uF.json").read_text())


def assert_valid(validator, doc):
    errs = sorted(validator.iter_errors(doc), key=lambda e: list(e.path))
    assert not errs, "expected valid, got errors:\n" + "\n".join(
        f"  - {e.message} @ {list(e.absolute_path)}" for e in errs)


def assert_invalid(validator, doc):
    assert list(validator.iter_errors(doc)), "expected invalid, got no errors"


# --- hygiene -------------------------------------------------------------------

@pytest.mark.parametrize("path", CAS_SCHEMAS, ids=lambda p: str(p.relative_to(REPO)))
def test_meta_validates(path):
    Draft202012Validator.check_schema(json.loads(path.read_text()))


def test_every_ref_resolves(registry):
    def walk(node, out):
        if isinstance(node, dict):
            if "$ref" in node:
                out.append(node["$ref"])
            for v in node.values():
                walk(v, out)
        elif isinstance(node, list):
            for v in node:
                walk(v, out)
    for path in CAS_SCHEMAS:
        doc = json.loads(path.read_text())
        refs = []
        walk(doc, refs)
        resolver = registry.resolver(base_uri=doc["$id"])
        for ref in refs:
            resolver.lookup(ref)


# --- examples ------------------------------------------------------------------

@pytest.mark.parametrize("example", sorted((REPO / "examples").glob("*.json")), ids=lambda p: p.name)
def test_example_validates(cas_validator, example):
    assert_valid(cas_validator, json.loads(example.read_text()))


@pytest.mark.parametrize("example", sorted((REPO / "examples").glob("*.json")), ids=lambda p: p.name)
def test_example_is_valid_peas(peas_validator, example):
    assert_valid(peas_validator, json.loads(example.read_text()))


# --- H6: base fields usable, junk rejected --------------------------------------

def test_design_requirements_accepts_base_fields(mlcc_doc):
    # market/name come from PEAS designRequirementsBase; before the 2026-07 fix the
    # extension branch's additionalProperties:false rejected every one of them.
    pass  # presence in the example (name, market) is asserted by test_example_validates


def test_design_requirements_rejects_unknown_key(cas_validator, mlcc_doc):
    mlcc_doc["inputs"]["designRequirements"]["junkKey"] = 1
    assert_invalid(cas_validator, mlcc_doc)


# --- H7: one technology vocabulary ----------------------------------------------

def test_allowed_technologies_shares_part_enum(cas_validator, mlcc_doc):
    mlcc_doc["inputs"]["designRequirements"]["allowedTechnologies"] = ["tantalum-polymer"]
    assert_valid(cas_validator, mlcc_doc)


def test_allowed_technologies_rejects_retired_vocabulary(cas_validator, mlcc_doc):
    mlcc_doc["inputs"]["designRequirements"]["allowedTechnologies"] = ["MLCC Class I"]
    assert_invalid(cas_validator, mlcc_doc)


def test_requirement_and_part_enums_cannot_drift(registry):
    # allowedTechnologies items are a $ref into capacitor.json#/$defs/technology —
    # assert the ref (not a copied enum) so drift is structurally impossible.
    dr = json.loads((REPO / "schemas/inputs/designRequirements.json").read_text())
    items = None
    def find(node):
        nonlocal items
        if isinstance(node, dict):
            if "allowedTechnologies" in node.get("properties", {}):
                items = node["properties"]["allowedTechnologies"]["items"]
            for v in node.values():
                find(v)
        elif isinstance(node, list):
            for v in node:
                find(v)
    find(dr)
    assert items == {"$ref": "../capacitor.json#/$defs/technology"}


# --- wrapper removal + seed ------------------------------------------------------

def test_double_nested_wrapper_rejected(cas_validator, mlcc_doc):
    mlcc_doc["capacitor"] = {"capacitor": {"anything": 1}}
    assert_invalid(cas_validator, mlcc_doc)


def test_empty_capacitor_seed_valid(cas_validator, mlcc_doc):
    mlcc_doc["capacitor"] = {}
    assert_valid(cas_validator, mlcc_doc)


# --- thermal.temperature typed ----------------------------------------------------

def test_thermal_temperature_garbage_rejected(cas_validator, mlcc_doc):
    mlcc_doc["capacitor"]["manufacturerInfo"]["datasheetInfo"]["thermal"] = {"temperature": "hot"}
    assert_invalid(cas_validator, mlcc_doc)


def test_thermal_temperature_dimtol_valid(cas_validator, mlcc_doc):
    mlcc_doc["capacitor"]["manufacturerInfo"]["datasheetInfo"]["thermal"] = {
        "temperature": {"minimum": -55.0, "maximum": 125.0}}
    assert_valid(cas_validator, mlcc_doc)


# --- outputs sealed ---------------------------------------------------------------

def test_output_block_rejects_unknown_key(registry):
    outputs = json.loads((REPO / "schemas/outputs.json").read_text())
    v = Draft202012Validator(outputs, registry=registry)
    doc = {"esrLosses": {"origin": "simulation", "methodUsed": "x", "typoField": 1}}
    errs = [e for e in v.iter_errors(doc)]
    assert errs, "typo'd key inside an output block must be rejected"
