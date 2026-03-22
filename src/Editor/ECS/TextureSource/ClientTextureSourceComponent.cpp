#include "ClientSourceManager.h"
#include "ClientTextureSourceComponent.h"
#include "IEditorWindow.h"
#include "MikanTextureSourceTypes.h"
#include "MikanServer.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "TextureSourceRequestHandler.h"
#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

// -- ClientTextureSourceDefinition ------
const std::string ClientTextureSourceDefinition::k_clientSourcePropertyId = "client_source";

ClientTextureSourceDefinition::ClientTextureSourceDefinition()
	: TextureSourceDefinition()
{}

ClientTextureSourceDefinition::ClientTextureSourceDefinition(
	MikanTextureSourceID textureSourceId)
	: TextureSourceDefinition(textureSourceId)
{}

configuru::Config ClientTextureSourceDefinition::writeToJSON()
{
	configuru::Config pt = TextureSourceDefinition::writeToJSON();

	pt["client_source"] = m_clientSource;

	return pt;
}

void ClientTextureSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	TextureSourceDefinition::readFromJSON(pt);

	m_clientSource = pt.get_or<std::string>("client_source", m_clientSource);
}

bool ClientTextureSourceDefinition::readFromInitParams(
	MikanObjectSystem* ownerObjectSystem,
	const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TextureSourceDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues = initParams.getTypedPointer<MikanClientTextureSourceValues>();
	if (componentValues)
	{
		m_clientSource = componentValues->client_source.getValue();
	}

	return true;
}

void ClientTextureSourceDefinition::setClientSource(const std::string& clientSource)
{
	if (clientSource != m_clientSource)
	{
		m_clientSource = clientSource;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_clientSourcePropertyId));
	}
}

// -- ClientTextureSourceComponent -----
ClientTextureSourceComponent::ClientTextureSourceComponent(MikanObjectWeakPtr owner)
	: TextureSourceComponent(owner)
{}

// -- IEntityAccessor ----
rfk::Struct const* ClientTextureSourceComponent::getClientAPIValuesStructType() const
{
	return &MikanClientTextureSourceValues::staticGetArchetype();
}

void ClientTextureSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);
}

IMkTexturePtr ClientTextureSourceComponent::getClientColorSourceTexture(
	MikanCameraID cameraId,
	eTextureSourceColorType textureSourceColorType) const
{
	auto* clientSourceManager = getClientSourceManager();

	if (clientSourceManager != nullptr)
	{
		return clientSourceManager->getClientColorSourceTexture(getClientSourceName(), cameraId, textureSourceColorType);
	}

	return IMkTexturePtr();
}

IMkTexturePtr ClientTextureSourceComponent::getClientDepthSourceTexture(
	MikanCameraID cameraId,
	eTextureSourceDepthType depthTextureType) const
{
	auto* clientSourceManager = getClientSourceManager();

	if (clientSourceManager != nullptr)
	{
		return clientSourceManager->getClientDepthSourceTexture(getClientSourceName(), cameraId, depthTextureType);
	}

	return IMkTexturePtr();
}

// -- IPropertyInterface ----
void ClientTextureSourceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TextureSourceComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			ClientTextureSourceDefinition::k_clientSourcePropertyId, MikanVariantType::STRING));
}

bool ClientTextureSourceComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == ClientTextureSourceDefinition::k_clientSourcePropertyId)
	{
		outValue = getClientTextureSourceDefinition()->getClientSource();
		return true;
	}

	return TextureSourceComponent::getPropertyValue(propertyName, outValue);
}

bool ClientTextureSourceComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == ClientTextureSourceDefinition::k_clientSourcePropertyId)
	{
		std::string devicePath = inValue.getStringValue();
		getClientTextureSourceDefinition()->setClientSource(devicePath);
		return true;
	}

	return TextureSourceComponent::setPropertyValue(propertyName, inValue);
}

ClientSourceManager* ClientTextureSourceComponent::getClientSourceManager() const
{
	return getOwnerEditorWindow()->getClientSourceManager();
}

const std::string& ClientTextureSourceComponent::getClientSourceName() const
{
	return getClientTextureSourceDefinition()->getClientSource();
}

void ClientTextureSourceComponent::showTextureSourceSettings()
{
	ModalDialog_SelectCamera::selectCamera(
		[this](MikanCameraID cameraId) {
		auto* appStage = getOwnerEditorWindow()->pushAppStageOfType<AppStage_TextureSourceSettings>();

		appStage->setSourceCameraId(cameraId);
		appStage->setTextureSourceComponent(getSelfPtr<TextureSourceComponent>());
	});
}