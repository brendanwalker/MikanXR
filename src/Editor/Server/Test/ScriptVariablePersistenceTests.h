#pragma once

// Validates the script variable table behind ScriptDefinition:
//   - every supported type survives a writeToJSON / readFromJSON round trip
//   - the single-line JSON text form round-trips, rejects malformed text
//     without touching the table, and skips an unknown-type entry
//   - ScriptDefinition keeps the script path and the variables together
//   - ScriptComponent's script_variables property re-applies its own text,
//     and both write paths notify exactly that one descriptor name
bool run_script_variable_persistence_tests();
