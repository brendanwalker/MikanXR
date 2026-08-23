#pragma once

// Validates the automation server's variant text coercion
// (AutomationVariantText::variantToText / textToVariant):
//   - scalar, string, and math types round-trip through their text form
//   - quaternions parse and print in w x y z field order
//   - malformed numbers, wrong component counts, and out-of-range integers
//     report parse errors
//   - array types refuse a set with an unsupported-type error
bool run_automation_variant_text_tests();
