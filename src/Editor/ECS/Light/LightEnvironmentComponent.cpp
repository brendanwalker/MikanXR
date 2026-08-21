#include "LightEnvironmentComponent.h"
#include "CameraObjectSystem.h"
#include "IEditorWindow.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "PropertyInterface.h"
#include "SceneLightingCapture/AppStage_SceneLightingCapture.h"
#include "SerializableList.h"

// The flat coefficient list is always exactly this long: 9 SH coefficients
// times 3 color channels.
static constexpr int k_shCoefficientFloatCount= k_shCoefficientCount * 3;

// -- LightEnvironmentDefinition -----
const std::string LightEnvironmentDefinition::k_shCoefficientsPropertyId= "sh_coefficients";
const std::string LightEnvironmentDefinition::k_exposureScalePropertyId= "exposure_scale";
const std::string LightEnvironmentDefinition::k_directionalityPropertyId= "directionality";
const std::string LightEnvironmentDefinition::k_keyLightDirectionPropertyId= "key_light_direction";

LightEnvironmentDefinition::LightEnvironmentDefinition()
	: TransformComponentDefinition()
	, m_shCoefficients((size_t)k_shCoefficientFloatCount, 0.f)
{
}

LightEnvironmentDefinition::LightEnvironmentDefinition(MikanLightID lightId)
	: TransformComponentDefinition(lightId)
	, m_shCoefficients((size_t)k_shCoefficientFloatCount, 0.f)
{
}

void LightEnvironmentDefinition::setSHCoefficients(const std::vector<float>& coefficients)
{
	// Normalize the length here so every reader can index unconditionally.
	m_shCoefficients.assign((size_t)k_shCoefficientFloatCount, 0.f);
	const size_t count= std::min(coefficients.size(), (size_t)k_shCoefficientFloatCount);
	for (size_t i= 0; i < count; ++i)
		m_shCoefficients[i]= coefficients[i];
}

void LightEnvironmentDefinition::setExposureScale(float scale) { m_exposureScale= scale; }

void LightEnvironmentDefinition::setDirectionality(float directionality) { m_directionality= directionality; }

void LightEnvironmentDefinition::setKeyLightDirection(const MikanVector3f& direction)
{
	m_keyLightDirection= direction;
}

SHLightingEnvironment LightEnvironmentDefinition::getLightingEnvironment() const
{
	SHLightingEnvironment environment;

	if ((int)m_shCoefficients.size() >= k_shCoefficientFloatCount)
	{
		for (int i= 0; i < k_shCoefficientCount; ++i)
		{
			environment.coefficients[i]=
				glm::vec3(m_shCoefficients[(size_t)i * 3 + 0], m_shCoefficients[(size_t)i * 3 + 1],
						  m_shCoefficients[(size_t)i * 3 + 2]);
		}
	}

	return environment;
}

void LightEnvironmentDefinition::setLightingEnvironment(const SHLightingEnvironment& environment)
{
	m_shCoefficients.assign((size_t)k_shCoefficientFloatCount, 0.f);
	for (int i= 0; i < k_shCoefficientCount; ++i)
	{
		m_shCoefficients[(size_t)i * 3 + 0]= environment.coefficients[i].r;
		m_shCoefficients[(size_t)i * 3 + 1]= environment.coefficients[i].g;
		m_shCoefficients[(size_t)i * 3 + 2]= environment.coefficients[i].b;
	}

	m_directionality= environment.getDirectionality();

	const glm::vec3 keyDirection= environment.getDominantDirection();
	m_keyLightDirection= {keyDirection.x, keyDirection.y, keyDirection.z};
}

configuru::Config LightEnvironmentDefinition::writeToJSON()
{
	configuru::Config pt= TransformComponentDefinition::writeToJSON();

	std::vector<configuru::Config> coefficients;
	coefficients.reserve(m_shCoefficients.size());
	for (float value : m_shCoefficients)
		coefficients.push_back(configuru::Config(value));
	pt[k_shCoefficientsPropertyId]= coefficients;

	pt[k_exposureScalePropertyId]= m_exposureScale;
	pt[k_directionalityPropertyId]= m_directionality;
	pt["key_light_direction_x"]= m_keyLightDirection.x;
	pt["key_light_direction_y"]= m_keyLightDirection.y;
	pt["key_light_direction_z"]= m_keyLightDirection.z;

	return pt;
}

void LightEnvironmentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_shCoefficients.assign((size_t)k_shCoefficientFloatCount, 0.f);
	if (pt.has_key(k_shCoefficientsPropertyId))
	{
		const configuru::Config& coefficients= pt[k_shCoefficientsPropertyId];
		if (coefficients.is_array())
		{
			int index= 0;
			for (const configuru::Config& value : coefficients.as_array())
			{
				if (index >= k_shCoefficientFloatCount)
					break;
				m_shCoefficients[(size_t)index++]= (float)value.as_float();
			}
		}
	}

	m_exposureScale= pt.get_or<float>(k_exposureScalePropertyId, 1.f);
	m_directionality= pt.get_or<float>(k_directionalityPropertyId, 0.f);
	m_keyLightDirection.x= pt.get_or<float>("key_light_direction_x", 0.f);
	m_keyLightDirection.y= pt.get_or<float>("key_light_direction_y", 0.f);
	m_keyLightDirection.z= pt.get_or<float>("key_light_direction_z", 1.f);
}

bool LightEnvironmentDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
													const Serialization::PolymorphicObjectPtr& initParams)
{
	return TransformComponentDefinition::readFromInitParams(ownerObjectSystem, initParams);
}

// -- LightEnvironmentComponent -----
LightEnvironmentComponent::LightEnvironmentComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
}

SHLightingEnvironment LightEnvironmentComponent::getScaledLightingEnvironment() const
{
	LightEnvironmentDefinitionPtr definition= getLightEnvironmentDefinition();
	if (!definition)
		return SHLightingEnvironment();

	SHLightingEnvironment environment= definition->getLightingEnvironment();
	const float scale= definition->getExposureScale();
	for (int i= 0; i < k_shCoefficientCount; ++i)
		environment.coefficients[i]*= scale;

	return environment;
}

rfk::Struct const* LightEnvironmentComponent::getClientAPIValuesStructType() const
{
	return &MikanLightEnvironmentComponentValues::staticGetArchetype();
}

void LightEnvironmentComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		LightEnvironmentDefinition::k_shCoefficientsPropertyId, MikanVariantType::FLOAT_ARRAY));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(LightEnvironmentDefinition::k_exposureScalePropertyId,
																  MikanVariantType::FLOAT));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		LightEnvironmentDefinition::k_directionalityPropertyId, MikanVariantType::FLOAT));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		LightEnvironmentDefinition::k_keyLightDirectionPropertyId, MikanVariantType::VECTOR3F));
}

bool LightEnvironmentComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	LightEnvironmentDefinitionPtr definition= getLightEnvironmentDefinition();

	if (propertyName == LightEnvironmentDefinition::k_shCoefficientsPropertyId)
	{
		if (definition)
		{
			Serialization::List<float> coefficients;
			for (float value : definition->getSHCoefficients())
				coefficients.push_back(value);
			outValue= coefficients;
			return true;
		}
	}
	else if (propertyName == LightEnvironmentDefinition::k_exposureScalePropertyId)
	{
		if (definition)
		{
			outValue= definition->getExposureScale();
			return true;
		}
	}
	else if (propertyName == LightEnvironmentDefinition::k_directionalityPropertyId)
	{
		if (definition)
		{
			outValue= definition->getDirectionality();
			return true;
		}
	}
	else if (propertyName == LightEnvironmentDefinition::k_keyLightDirectionPropertyId)
	{
		if (definition)
		{
			outValue= definition->getKeyLightDirection();
			return true;
		}
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool LightEnvironmentComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	LightEnvironmentDefinitionPtr definition= getLightEnvironmentDefinition();

	if (propertyName == LightEnvironmentDefinition::k_shCoefficientsPropertyId)
	{
		if (definition)
		{
			const Serialization::List<float>& source= inValue.getFloatArrayValue();
			std::vector<float> coefficients;
			coefficients.reserve(source.size());
			for (size_t i= 0; i < source.size(); ++i)
				coefficients.push_back(source[i]);
			definition->setSHCoefficients(coefficients);
			return true;
		}
	}
	else if (propertyName == LightEnvironmentDefinition::k_exposureScalePropertyId)
	{
		if (definition)
		{
			definition->setExposureScale(inValue.getFloatValue());
			return true;
		}
	}
	else if (propertyName == LightEnvironmentDefinition::k_directionalityPropertyId)
	{
		if (definition)
		{
			definition->setDirectionality(inValue.getFloatValue());
			return true;
		}
	}
	else if (propertyName == LightEnvironmentDefinition::k_keyLightDirectionPropertyId)
	{
		if (definition)
		{
			definition->setKeyLightDirection(inValue.getVector3fValue());
			return true;
		}
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string LightEnvironmentComponent::k_captureSceneLightingFunctionId= "capture_scene_lighting";

void LightEnvironmentComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(k_captureSceneLightingFunctionId, "Capture Scene Lighting"));
}

bool LightEnvironmentComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == LightEnvironmentComponent::k_captureSceneLightingFunctionId)
	{
		captureSceneLighting();
		return true;
	}

	return TransformComponent::invokeFunction(functionName);
}

void LightEnvironmentComponent::captureSceneLighting()
{
	// The estimate is only meaningful in world space if the frame it was
	// captured from has a known camera pose, so the camera is chosen up front
	// rather than guessed at.
	auto parentCamera= std::dynamic_pointer_cast<CameraComponent>(getParentTransformComponent());
	if (parentCamera != nullptr)
	{
		auto* captureStage= getOwnerEditorWindow()->pushAppStageOfType<AppStage_SceneLightingCapture>();
		if (captureStage != nullptr)
		{
			captureStage->setSourceCamera(parentCamera);
			captureStage->setTargetProbe(getSelfPtr<LightEnvironmentComponent>());
		}
		return;
	}
	else
	{
		MIKAN_LOG_WARNING("LightEnvironmentComponent::captureSceneLighting")
			<< "Called on a probe that is not parented to a camera. "
			<< "The scene lighting capture tool requires a camera to be selected first.";
	}
}
