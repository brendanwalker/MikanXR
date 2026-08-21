#pragma once

// Validates LightEnvironmentDefinition, the persisted scene lighting probe:
//   - the 27 SH coefficients survive a writeToJSON / readFromJSON round trip
//   - a short, over-long, or absent coefficient list is normalized to exactly
//     27 entries so downstream indexing is unconditional
//   - setLightingEnvironment derives directionality and key direction
//   - the exposure scale multiplies through to the environment the renderer
//     consumes
//
// The values-struct / descriptor / getPropertyValue contract is covered
// separately by ClientApiPropertySchemaTests.
bool run_light_environment_persistence_tests();
