#pragma once

// Validates the ARKit debug channel's pure text-protocol helpers
// (ARKitDebugProtocol):
//   - the handshake accepts a well-formed hello and rejects a malformed one
//   - post-handshake lines classify as log, reply, or unknown
//   - log bodies split into a severity and text, falling back to info
//   - reply counts reject non-numeric, negative, and over-bound values
bool run_arkit_debug_protocol_tests();
