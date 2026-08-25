#pragma once

// Guards the property notification contract the transaction recorder and the
// websocket property events both depend on:
//   - a successful setPropertyValue on a writable descriptor must fire the
//     definition's OnPropertyChanged (catches setters that skip
//     notifyPropertyChanged)
//   - every notified property name must resolve to one of the class's
//     descriptors (catches misspelled notification names)
// Known gaps are listed in an explicit allowlist inside the test; shrinking
// that list over time is the payoff.
bool run_property_notification_guard_tests();
