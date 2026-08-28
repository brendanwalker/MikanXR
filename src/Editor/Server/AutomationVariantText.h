#pragma once

#include "MikanVariantTypes.h"

#include <string>
#include <vector>

/// Text coercion between automation command tokens and MikanVariant values,
/// used by the automation server's property commands.
/// Scalar and math types convert both ways. Array and map types format for
/// reads only; a set replies that the type is unsupported.
namespace AutomationVariantText
{
/// Format a variant's value as one line of text.
/// Math types print their components space separated in field order
/// (quaternions as w x y z, matrices as 16 floats x0..w3).
std::string variantToText(const MikanVariant& value);

/// Parse value tokens into a variant of the given type.
/// Math types consume one numeric token per component; strings join all
/// tokens with single spaces (a quoted command token arrives as one token).
/// @returns false (with outError set) on a parse failure or unsupported type
bool textToVariant(MikanVariantType dataType, const std::vector<std::string>& valueTokens, MikanVariant& outValue,
				   std::string& outError);
} // namespace AutomationVariantText
