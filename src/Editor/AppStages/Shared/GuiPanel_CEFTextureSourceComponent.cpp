#include "AppStage.h"
#include "Shared/GuiPanel_CEFTextureSourceComponent.h"
#include "CEFTextureSourceComponent.h"
#include "MkGuiDrawUtils.h"

bool GuiPanel_CEFTextureSourceComponent::init() { return initTypedPropertyInterface<CEFTextureSourceComponent>(); }

void GuiPanel_CEFTextureSourceComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// URL property: text input field
	m_entityAccessor->setPropertyRenderer(
		CEFTextureSourceDefinition::k_urlPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto comp= getCEFTextureSourceComponent();
			if (!comp)
				return false;

			const std::string& currentUrl= comp->getCEFTextureSourceDefinition()->getUrl();
			char buf[2048];
			strncpy_s(buf, sizeof(buf), currentUrl.c_str(), _TRUNCATE);

			if (MkGui::drawStringProperty(m_defaultGuiStyle,
										  comp->makePropertyUIIdentifier(CEFTextureSourceDefinition::k_urlPropertyId),
										  "URL", buf, sizeof(buf)))
			{
				std::string newUrl(buf);
				addDeferredGuiEvent([comp, newUrl]() { comp->getCEFTextureSourceDefinition()->setUrl(newUrl); });
			}
			return true;
		});

	// Width property: int input
	m_entityAccessor->setPropertyRenderer(
		CEFTextureSourceDefinition::k_widthPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto comp= getCEFTextureSourceComponent();
			if (!comp)
				return false;

			int w= comp->getCEFTextureSourceDefinition()->getWidth();

			if (MkGui::drawIntProperty(m_defaultGuiStyle,
									   comp->makePropertyUIIdentifier(CEFTextureSourceDefinition::k_widthPropertyId),
									   "Width", w))
			{
				addDeferredGuiEvent([comp, w]() { comp->getCEFTextureSourceDefinition()->setWidth(w); });
			}
			return true;
		});

	// Height property: int input
	m_entityAccessor->setPropertyRenderer(
		CEFTextureSourceDefinition::k_heightPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto comp= getCEFTextureSourceComponent();
			if (!comp)
				return false;

			int h= comp->getCEFTextureSourceDefinition()->getHeight();

			if (MkGui::drawIntProperty(m_defaultGuiStyle,
									   comp->makePropertyUIIdentifier(CEFTextureSourceDefinition::k_heightPropertyId),
									   "Height", h))
			{
				addDeferredGuiEvent([comp, h]() { comp->getCEFTextureSourceDefinition()->setHeight(h); });
			}
			return true;
		});
}

CEFTextureSourceComponentPtr GuiPanel_CEFTextureSourceComponent::getCEFTextureSourceComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<CEFTextureSourceComponent>(component);
	}
	return nullptr;
}
