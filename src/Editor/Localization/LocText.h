#pragma once

#include <string>

// Call-site helpers over LocalizationManager, safe without one (headless
// tools): every helper degrades to returning the key itself.
//
// Usage per string kind:
//	locText("section.key")        Text/TextWrapped/tooltips and validated
//	                              printf format strings ("...Fmt" keys)
//	locLabel("section.key")       any interactive widget label (Button,
//	                              Checkbox, MenuItem, combo, slider): the ID
//	                              is the key, not the translation
//	locWindowTitle("windows.key") ImGui::Begin / popup titles and every
//	                              by-name window reference (SetWindowFocus,
//	                              OpenPopup)
//	locFormat("section.keyFmt")   printf-expansion into a std::string, for
//	                              labels assembled at runtime
const char* locText(const char* key);
const char* locLabel(const char* key);
const char* locWindowTitle(const char* key);
std::string locFormat(const char* key, ...);

// Key for a label that is generated from a descriptor id rather than written at
// a call site (the property and function databases). Prefers a per-class
// override "<entityClassName>.<descriptorId>" when one is defined, otherwise the
// shared "<sharedSection>.<descriptorId>". Which one wins does not vary by
// language, so callers resolve once and keep the key, then pass it to locText
// every frame.
//
// Returns an empty string when neither key is defined, which tells the caller to
// keep whatever it displayed before rather than render a key. The label coverage
// guard in the localization test makes that empty result unreachable for any
// descriptor that always draws a generic widget.
std::string locResolveDescriptorKey(const std::string& entityClassName, const std::string& sharedSection,
									const std::string& descriptorId);
