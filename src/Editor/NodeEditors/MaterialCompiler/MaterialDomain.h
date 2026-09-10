#pragma once

#include "MkVertexConstants.h"
#include "ShaderValueType.h"

#include <string>
#include <vector>

// What a material draws. Compositor materials shade the fullscreen layer quad,
// shape materials shade shape renderables under a model-view-projection.
enum class eMaterialDomain : int
{
	INVALID= -1,

	compositor,
	shape,

	COUNT
};

// The vertex layout a material is compiled against. Each preset is the exact
// attribute list of one family of meshes, so a material compiled for a preset
// links against those meshes and no others.
enum class eMaterialVertexPreset : int
{
	INVALID= -1,

	P2T, // vec2 position, vec2 texCoord: the compositor layer quad
	PT,  // vec3 position, vec2 texCoord: quad and box shapes
	PNT, // vec3 position, vec3 normal, vec2 texCoord: model shapes

	COUNT
};

struct MaterialVertexAttribute
{
	std::string name;
	eVertexDataType dataType;
	eVertexSemantic semantic;
	eShaderValueType valueType;
};

namespace MaterialDomainUtils
{
const std::string& domainToString(eMaterialDomain domain);
eMaterialDomain domainFromString(const std::string& name);

const std::string& presetToString(eMaterialVertexPreset preset);
eMaterialVertexPreset presetFromString(const std::string& name);

const std::vector<eMaterialVertexPreset>& getAllowedPresets(eMaterialDomain domain);
eMaterialVertexPreset getDefaultPreset(eMaterialDomain domain);
bool isPresetAllowed(eMaterialDomain domain, eMaterialVertexPreset preset);

// Attributes in layout location order
const std::vector<MaterialVertexAttribute>& getPresetAttributes(eMaterialVertexPreset preset);
const MaterialVertexAttribute* findPresetAttribute(eMaterialVertexPreset preset, eVertexSemantic semantic);

// Recover domain and preset from a hand-authored .mat's attribute list: a vec2
// position is the compositor quad, a vec3 position is a shape mesh
eMaterialDomain inferDomain(const std::vector<MaterialVertexAttribute>& attributes);
eMaterialVertexPreset inferPreset(const std::vector<MaterialVertexAttribute>& attributes);
} // namespace MaterialDomainUtils
