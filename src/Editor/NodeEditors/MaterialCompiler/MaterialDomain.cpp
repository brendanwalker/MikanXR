#include "MaterialDomain.h"
#include "MikanShaderConfig.h"

namespace
{
const std::string k_domainNames[(int)eMaterialDomain::COUNT]= {"compositor", "shape"};
const std::string k_presetNames[(int)eMaterialVertexPreset::COUNT]= {"P2T", "PT", "PNT"};
const std::string k_invalidDomainName= "INVALID";

const std::vector<eMaterialVertexPreset> k_compositorPresets= {eMaterialVertexPreset::P2T};
const std::vector<eMaterialVertexPreset> k_shapePresets= {eMaterialVertexPreset::PT, eMaterialVertexPreset::PNT};
const std::vector<eMaterialVertexPreset> k_noPresets= {};

// Attribute names match the hand-written shaders in resources/shaders so a
// compiled material reads like one
const std::vector<MaterialVertexAttribute> k_p2tAttributes= {
	{"aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position, eShaderValueType::float2},
	{"aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord, eShaderValueType::float2}};
const std::vector<MaterialVertexAttribute> k_ptAttributes= {
	{"aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position, eShaderValueType::float3},
	{"aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord, eShaderValueType::float2}};
const std::vector<MaterialVertexAttribute> k_pntAttributes= {
	{"aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position, eShaderValueType::float3},
	{"aNormal", eVertexDataType::datatype_vec3, eVertexSemantic::normal, eShaderValueType::float3},
	{"aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord, eShaderValueType::float2}};
const std::vector<MaterialVertexAttribute> k_noAttributes= {};

bool attributesMatchPreset(const std::vector<MaterialVertexAttribute>& attributes, eMaterialVertexPreset preset)
{
	const std::vector<MaterialVertexAttribute>& presetAttributes= MaterialDomainUtils::getPresetAttributes(preset);
	if (attributes.size() != presetAttributes.size())
		return false;

	for (size_t index= 0; index < attributes.size(); ++index)
	{
		if (attributes[index].dataType != presetAttributes[index].dataType
			|| attributes[index].semantic != presetAttributes[index].semantic)
		{
			return false;
		}
	}

	return true;
}
} // namespace

namespace MaterialDomainUtils
{
const std::string& domainToString(eMaterialDomain domain)
{
	const int index= (int)domain;
	return (index > (int)eMaterialDomain::INVALID && index < (int)eMaterialDomain::COUNT) ? k_domainNames[index]
																						  : k_invalidDomainName;
}

const char* materialExtension(eMaterialDomain domain)
{
	return domain == eMaterialDomain::shape ? k_shapeMaterialExtension : k_compositorMaterialExtension;
}

const char* materialFolderName(eMaterialDomain domain)
{
	return domain == eMaterialDomain::shape ? "shape_materials" : "compositor_materials";
}

eMaterialDomain domainFromMaterialExtension(const std::string& extension)
{
	if (extension == k_compositorMaterialExtension)
		return eMaterialDomain::compositor;
	if (extension == k_shapeMaterialExtension)
		return eMaterialDomain::shape;

	return eMaterialDomain::INVALID;
}

bool isMaterialFileExtension(const std::string& extension)
{
	return extension == k_compositorMaterialExtension || extension == k_shapeMaterialExtension
		   || extension == k_legacyMaterialExtension;
}

eMaterialDomain resolveMaterialDomain(const MikanShaderConfig& config)
{
	if (!config.domain.empty())
	{
		return domainFromString(config.domain);
	}

	std::vector<MaterialVertexAttribute> attributes;
	for (const GlVertexAttributeConfigPtr& attribConfig : config.vertexAttributes)
	{
		// Inference only reads the semantic and data type
		attributes.push_back(
			{attribConfig->name, attribConfig->dataType, attribConfig->semantic, eShaderValueType::INVALID});
	}

	return inferDomain(attributes);
}

eMaterialDomain domainFromString(const std::string& name)
{
	for (int index= 0; index < (int)eMaterialDomain::COUNT; ++index)
	{
		if (k_domainNames[index] == name)
			return (eMaterialDomain)index;
	}

	return eMaterialDomain::INVALID;
}

const std::string& presetToString(eMaterialVertexPreset preset)
{
	const int index= (int)preset;
	return (index > (int)eMaterialVertexPreset::INVALID && index < (int)eMaterialVertexPreset::COUNT)
			   ? k_presetNames[index]
			   : k_invalidDomainName;
}

eMaterialVertexPreset presetFromString(const std::string& name)
{
	for (int index= 0; index < (int)eMaterialVertexPreset::COUNT; ++index)
	{
		if (k_presetNames[index] == name)
			return (eMaterialVertexPreset)index;
	}

	return eMaterialVertexPreset::INVALID;
}

const std::vector<eMaterialVertexPreset>& getAllowedPresets(eMaterialDomain domain)
{
	switch (domain)
	{
	case eMaterialDomain::compositor:
		return k_compositorPresets;
	case eMaterialDomain::shape:
		return k_shapePresets;
	default:
		return k_noPresets;
	}
}

eMaterialVertexPreset getDefaultPreset(eMaterialDomain domain)
{
	const std::vector<eMaterialVertexPreset>& presets= getAllowedPresets(domain);
	return presets.empty() ? eMaterialVertexPreset::INVALID : presets.front();
}

bool isPresetAllowed(eMaterialDomain domain, eMaterialVertexPreset preset)
{
	for (eMaterialVertexPreset allowed : getAllowedPresets(domain))
	{
		if (allowed == preset)
			return true;
	}

	return false;
}

const std::vector<MaterialVertexAttribute>& getPresetAttributes(eMaterialVertexPreset preset)
{
	switch (preset)
	{
	case eMaterialVertexPreset::P2T:
		return k_p2tAttributes;
	case eMaterialVertexPreset::PT:
		return k_ptAttributes;
	case eMaterialVertexPreset::PNT:
		return k_pntAttributes;
	default:
		return k_noAttributes;
	}
}

const MaterialVertexAttribute* findPresetAttribute(eMaterialVertexPreset preset, eVertexSemantic semantic)
{
	for (const MaterialVertexAttribute& attribute : getPresetAttributes(preset))
	{
		if (attribute.semantic == semantic)
			return &attribute;
	}

	return nullptr;
}

eMaterialDomain inferDomain(const std::vector<MaterialVertexAttribute>& attributes)
{
	for (const MaterialVertexAttribute& attribute : attributes)
	{
		if (attribute.semantic == eVertexSemantic::position)
		{
			return attribute.dataType == eVertexDataType::datatype_vec2 ? eMaterialDomain::compositor
																		: eMaterialDomain::shape;
		}
	}

	return eMaterialDomain::INVALID;
}

eMaterialVertexPreset inferPreset(const std::vector<MaterialVertexAttribute>& attributes)
{
	for (int index= 0; index < (int)eMaterialVertexPreset::COUNT; ++index)
	{
		const eMaterialVertexPreset preset= (eMaterialVertexPreset)index;
		if (attributesMatchPreset(attributes, preset))
			return preset;
	}

	return eMaterialVertexPreset::INVALID;
}
} // namespace MaterialDomainUtils
