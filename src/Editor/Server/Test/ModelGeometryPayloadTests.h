#pragma once

// Pins the splice that ServerModelGeometryPayload relies on.
//
// Model render geometry runs to megabytes, so the response is assembled by serializing only the
// MikanResponse header and appending a cached, already serialized render_geometry rather than
// walking every vertex through reflection again. That is only correct because binary serialization
// writes a struct's fields in memory offset order, parents first, with no framing between them.
//
// These tests serialize the whole response the ordinary way and compare it byte for byte against
// the spliced form, for both the stencil and shape response types, so a field added to
// MikanResponse or inserted ahead of render_geometry fails here instead of on the wire.
bool run_model_geometry_payload_tests();
