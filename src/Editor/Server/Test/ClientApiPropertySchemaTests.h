#pragma once

// Validates that every component / object-system "client API values struct"
// is consistent with the property descriptors exposed for that same class.
//
// For each entity that exposes a client API values struct, this test:
//   1. Walks the reflected fields of the values struct (the data the client
//      receives from a ComponentGetValuesRequest / SystemGetValuesRequest) and
//      computes the MikanVariantType the server-side serializer
//      (ServerEntitySerializer) will require for each field.
//   2. Gathers the static property descriptors the class advertises.
//   3. Asserts every values-struct field has a matching descriptor of the
//      exact required variant type.
//
// This catches the recurring "client API field type / name does not match the
// exposed property descriptor" bug class without needing a live editor session.
bool run_client_api_property_schema_tests();
