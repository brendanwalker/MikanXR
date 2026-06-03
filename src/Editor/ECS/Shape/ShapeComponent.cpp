#include "ShapeComponent.h"
#include "IMkTexture.h"
#include "MikanVariantTypes.h"
#include "MikanObject.h"
#include "ProjectManager.h"
#include "PropertyInterface.h"
#include "TextureSourceComponent.h"
#include "TextureSourceQueries.h"
#include "VideoDisplayConstants.h"

// -- ShapeComponentDefinition ------
const std::string ShapeComponentDefinition::k_textureSourceIdPropertyId = "shape_texture_source_id";

ShapeComponentDefinition::ShapeComponentDefinition()
	: TransformComponentDefinition()
{
}

ShapeComponentDefinition::ShapeComponentDefinition(MikanShapeID shapeId)
	: TransformComponentDefinition(shapeId)
{
}

configuru::Config ShapeComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_textureSourceIdPropertyId] = m_textureSourceId;

	return pt;
}

void ShapeComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_textureSourceId = pt.get_or<int>(k_textureSourceIdPropertyId, INVALID_MIKAN_ID);
}

void ShapeComponentDefinition::setTextureSourceId(MikanTextureSourceID id)
{
	if (m_textureSourceId != id)
	{
		m_textureSourceId = id;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_textureSourceIdPropertyId));
	}
}

// -- ShapeComponent ------
ShapeComponent::ShapeComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsUpdate = true;
}

MikanTextureSourceID ShapeComponent::getTextureSourceId() const
{
	return getShapeComponentDefinition()->getTextureSourceId();
}

void ShapeComponent::setTextureSourceId(MikanTextureSourceID id)
{
	getShapeComponentDefinition()->setTextureSourceId(id);
}

void ShapeComponent::update(float deltaSeconds)
{
	// Refresh the cached texture from the referenced texture source component
	const MikanTextureSourceID textureSourceId = getTextureSourceId();
	if (textureSourceId != INVALID_MIKAN_ID)
	{
		ProjectManagerPtr projectManager = getOwnerProjectManager();
		TextureSourceComponentPtr textureSource =
			TextureSourceQueries::getTextureSourceById(projectManager, textureSourceId);

		if (textureSource)
		{
			m_colorTexture = textureSource->getClientColorSourceTexture(
				INVALID_MIKAN_ID,
				eTextureSourceColorType::colorRGBA,
				-1);
		}
		else
		{
			m_colorTexture = nullptr;
		}
	}
	else
	{
		m_colorTexture = nullptr;
	}
}

void ShapeComponent::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	TransformComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);

	// Subclasses override this to respond to property changes (e.g. size change → rebuild mesh)
}

// -- IPropertyInterface ----
void ShapeComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			ShapeComponentDefinition::k_textureSourceIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(INVALID_MIKAN_ID));
}

bool ShapeComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == ShapeComponentDefinition::k_textureSourceIdPropertyId)
	{
		outValue = (int)getTextureSourceId();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool ShapeComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == ShapeComponentDefinition::k_textureSourceIdPropertyId)
	{
		setTextureSourceId((MikanTextureSourceID)inValue.getIntValue());
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}
