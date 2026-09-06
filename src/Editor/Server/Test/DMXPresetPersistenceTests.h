#pragma once

// Validates DMXPresetDefinition, the persisted per-fixture channel bytes:
//   - fixtures of different lengths survive a writeToJSON / readFromJSON round trip
//   - the preset_data property text re-applies onto a second component and
//     rejects malformed text without touching the table
//   - a channel range write grows a slice with zeros and notifies exactly
//     preset_data, and a same-value write does not notify
bool run_dmx_preset_persistence_tests();
