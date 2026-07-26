#include "AppStage.h"
#include "Shared/GuiPanel_ARKitVideoSourceComponent.h"
#include "ARKitVideoSourceComponent.h"

bool GuiPanel_ARKitVideoSourceComponent::init() { return initTypedPropertyInterface<ARKitVideoSourceComponent>(); }

void GuiPanel_ARKitVideoSourceComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// JBU sigma/confidence sliders, bounded to sensible ranges instead of the
	// generic renderer's unbounded drag-float (see this class's header comment).
	// Definition setters already clamp too, so this is belt-and-suspenders, not the
	// only thing standing between a bad value and the kernel.
	m_entityAccessor->setPropertyRenderer(
		ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto arkitComp= getARKitVideoSourceComponent();
			if (!arkitComp)
				return false;

			ARKitVideoSourceDefinitionPtr definition= arkitComp->getARKitVideoSourceDefinition();
			float value= definition->getJbuSigmaSpatial();
			if (MkGui::drawFloatSliderProperty(
					m_defaultGuiStyle,
					arkitComp->makePropertyUIIdentifier(ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId),
					"JBU Sigma Spatial", value, 0.1f, 128.0f, 0.1f, 128.0f))
			{
				addDeferredGuiEvent([definition, value]() { definition->setJbuSigmaSpatial(value); });
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto arkitComp= getARKitVideoSourceComponent();
			if (!arkitComp)
				return false;

			ARKitVideoSourceDefinitionPtr definition= arkitComp->getARKitVideoSourceDefinition();
			float value= definition->getJbuSigmaColor();
			if (MkGui::drawFloatSliderProperty(
					m_defaultGuiStyle,
					arkitComp->makePropertyUIIdentifier(ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId),
					"JBU Sigma Color", value, 0.1f, 255.0f, 0.1f, 255.0f))
			{
				addDeferredGuiEvent([definition, value]() { definition->setJbuSigmaColor(value); });
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto arkitComp= getARKitVideoSourceComponent();
			if (!arkitComp)
				return false;

			ARKitVideoSourceDefinitionPtr definition= arkitComp->getARKitVideoSourceDefinition();
			float value= definition->getJbuConfWeightLow();
			if (MkGui::drawFloatSliderProperty(
					m_defaultGuiStyle,
					arkitComp->makePropertyUIIdentifier(ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId),
					"JBU Conf Weight (Low)", value, 0.0f, 1.0f, 0.0f, 1.0f))
			{
				addDeferredGuiEvent([definition, value]() { definition->setJbuConfWeightLow(value); });
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto arkitComp= getARKitVideoSourceComponent();
			if (!arkitComp)
				return false;

			ARKitVideoSourceDefinitionPtr definition= arkitComp->getARKitVideoSourceDefinition();
			float value= definition->getJbuConfWeightMedium();
			if (MkGui::drawFloatSliderProperty(
					m_defaultGuiStyle,
					arkitComp->makePropertyUIIdentifier(ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId),
					"JBU Conf Weight (Medium)", value, 0.0f, 1.0f, 0.0f, 1.0f))
			{
				addDeferredGuiEvent([definition, value]() { definition->setJbuConfWeightMedium(value); });
			}
			return true;
		});

	// Person-segmentation edge strength (1.0 == no gating, 0.0 == hard crisp silhouette).
	// seg_gating_enabled (bool) is left to the generic renderer, like depth_streaming_enabled.
	m_entityAccessor->setPropertyRenderer(
		ARKitVideoSourceDefinition::k_segEdgeStrengthPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto arkitComp= getARKitVideoSourceComponent();
			if (!arkitComp)
				return false;

			ARKitVideoSourceDefinitionPtr definition= arkitComp->getARKitVideoSourceDefinition();
			float value= definition->getSegEdgeStrength();
			if (MkGui::drawFloatSliderProperty(
					m_defaultGuiStyle,
					arkitComp->makePropertyUIIdentifier(ARKitVideoSourceDefinition::k_segEdgeStrengthPropertyId),
					"Seg Edge Strength", value, 0.0f, 1.0f, 0.0f, 1.0f))
			{
				addDeferredGuiEvent([definition, value]() { definition->setSegEdgeStrength(value); });
			}
			return true;
		});
}

ARKitVideoSourceComponentPtr GuiPanel_ARKitVideoSourceComponent::getARKitVideoSourceComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<ARKitVideoSourceComponent>(component);
	}
	return nullptr;
}

void GuiPanel_ARKitVideoSourceComponent::drawCompactGui()
{
	GuiPanel_EntityAccessorPtr entityAccessor= getPropertyInterface();

	static const std::set<std::string> compactProperties= {
		MikanComponentDefinition::k_componentNamePropertyId,
		ARKitVideoSourceDefinition::k_basePortPropertyId,
	};
	static const std::set<std::string> compactFunctions= {
		ARKitVideoSourceComponent::k_showVideoSourceSettingsFunctionId};
	entityAccessor->drawPropertiesGui(compactProperties);
	entityAccessor->drawFunctionsGui(compactFunctions);
}
