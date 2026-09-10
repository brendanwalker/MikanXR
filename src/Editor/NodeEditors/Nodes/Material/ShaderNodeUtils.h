#pragma once

#include "IMkGuiStyle.h"
#include "MaterialCompiler/ShaderValueType.h"

#include <string>

// Helpers shared by the material graph nodes: value formatting for the canvas,
// identifier validation for uniform and function names, and the property
// sheet editor for a typed float value.
namespace ShaderNodeUtils
{
// "1.00" for a float1, "(1.00, 0.50)" for wider vectors, empty for non float types
std::string formatValueText(eShaderValueType type, const ShaderValueDefault& value);

// Letters, digits and underscores, not starting with a digit, non-empty
bool isValidIdentifier(const std::string& name);

// A float editor sized to the type, or a color editor when bIsColor. Returns true when edited.
bool drawValueProperty(MkGuiStyleConstPtr style, const std::string& fieldName, const std::string& label,
					   eShaderValueType type, bool bIsColor, ShaderValueDefault& inoutValue);

// Null separated combo items naming float1 through float4, in eShaderValueType order
const std::string& getFloatTypeComboItems();
// Combo index of a float type (float1 is 0), -1 for anything else
int getFloatTypeComboIndex(eShaderValueType type);
eShaderValueType getFloatTypeFromComboIndex(int index);
} // namespace ShaderNodeUtils
