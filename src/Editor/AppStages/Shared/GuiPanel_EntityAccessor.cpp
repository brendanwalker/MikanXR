#include "GuiPanel_EntityAccessor.h"
#include "AssetReferencePropertyMetaData.h"
#include "EnumPropertyMetaData.h"
#include "CommonConfig.h"
#include "MikanVariantTypes.h"
#include "MkGuiStyleManager.h"
#include "StringUtils.h"

#include "imgui.h"
#include "tinyfiledialogs.h"

#include <assert.h>

static const std::string k_defaultStyleName= "default_component_panel";

GuiPanel_EntityAccessor::GuiPanel_EntityAccessor(AppStage* ownerAppStage)
	: GuiPanel(ownerAppStage)
{
	if (MkGuiStyleManager* mgr= getGuiStyleManager())
		m_defaultGuiStyle= mgr->getStyle(k_defaultStyleName);
}

GuiPanel_EntityAccessor::~GuiPanel_EntityAccessor()
{
	// This should have already been cleaned up in dispose()
	assert(m_entityAccessor.lock() == nullptr);
}

bool GuiPanel_EntityAccessor::init(
	const std::string& modelName,
	const std::vector<PropertyDescriptorConstPtr>& propertyDescriptors,
	const std::vector<FunctionDescriptorConstPtr>& functionDescriptors,
	OnConstruct onConstructCallback)
{
	clearEntityAccessor();

	m_modelName= modelName;
	m_propertyDescriptors.clear();
	m_orderedPropertyDescriptors.clear();
	m_functionDescriptors.clear();
	m_onConstructCallback= onConstructCallback;

	if (onConstructCallback && !onConstructCallback())
		return false;

	for (const PropertyDescriptorConstPtr& descriptor : propertyDescriptors)
	{
		const std::string propertyName= descriptor->getName();

		if (descriptor->isUIHidden() && !m_propertyRenderers.contains(propertyName))
			continue;

		m_propertyDescriptors.insert({propertyName, descriptor});
		m_orderedPropertyDescriptors.push_back(descriptor);
	}

	for (const FunctionDescriptorConstPtr& descriptor : functionDescriptors)
	{
		m_functionDescriptors.push_back(descriptor);
	}

	return true;
}

void GuiPanel_EntityAccessor::dispose()
{
	clearEntityAccessor();
	m_propertyDescriptors.clear();
	m_orderedPropertyDescriptors.clear();
	m_functionDescriptors.clear();
	m_propertyRenderers.clear();
	m_onConstructCallback= nullptr;

	GuiPanel::dispose();
}

void GuiPanel_EntityAccessor::clearEntityAccessor()
{
	IEntityAccessorPtr oldEntityAccessor= m_entityAccessor.lock();

	if (oldEntityAccessor)
	{
		CommonConfigPtr oldEntityConfig= oldEntityAccessor->getEntityConfig();
		assert(m_bWasAccessorSet);

		oldEntityAccessor->onDisposed-=
			MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityDisposed);
		oldEntityConfig->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityConfigChanged);

		m_entityAccessor.reset();
		m_bWasAccessorSet= false;
	}
	else
	{
		// If this fires the entity accessor was destroyed without us being notified
		assert(!m_bWasAccessorSet);
	}
}

void GuiPanel_EntityAccessor::setEntityAccessor(IEntityAccessorPtr newEntityAccessor)
{
	IEntityAccessorPtr oldEntityAccessor= m_entityAccessor.lock();

	if (newEntityAccessor != oldEntityAccessor)
	{
		clearEntityAccessor();

		if (newEntityAccessor)
		{
			CommonConfigPtr newEntityConfig= newEntityAccessor->getEntityConfig();

			newEntityAccessor->onDisposed+=
				MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityDisposed);
			newEntityConfig->OnPropertyChanged+=
				MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityConfigChanged);

			m_bWasAccessorSet= true;
		}

		m_entityAccessor= newEntityAccessor;
	}
}

void GuiPanel_EntityAccessor::setPropertyRenderer(
	const std::string& propName,
	PropertyRendererCallback renderer)
{
	m_propertyRenderers[propName]= renderer;
}

void GuiPanel_EntityAccessor::drawPropertiesGui()
{
	drawPropertiesGui({});
}

void GuiPanel_EntityAccessor::drawPropertiesGui(const std::set<std::string>& propertyNames)
{
	IEntityAccessorPtr accessor= m_entityAccessor.lock();
	if (!accessor)
	{
		return;
	}

	// Render auto-generated property widgets
	for (const PropertyDescriptorConstPtr& desc : m_orderedPropertyDescriptors)
	{
		const std::string& propName= desc->getName();
		const std::string uiFieldId= accessor->makePropertyUIIdentifier(propName);

		// If a property name filter is provided, skip properties that aren't in the filter set
		if (!propertyNames.empty() && propertyNames.find(propName) == propertyNames.end())
			continue;

		// Check for a registered custom renderer; if it returns true the property is fully handled
		auto rendererIt= m_propertyRenderers.find(propName);
		if (rendererIt != m_propertyRenderers.end() && rendererIt->second(desc))
			continue;

		const bool isReadOnly= desc->isReadOnly();

		MikanVariant value;
		accessor->getPropertyValue(propName, value);
		const MikanVariantType variantType= value.value_type;

		if (isReadOnly)
		{
			ImGui::BeginDisabled(true);
		}

		bool bValueChanged= false;
		MikanVariant newValue= value;

		if (variantType == MikanVariantType::BOOL)
		{
			bool v= value.getBoolValue();
			if (MkGui::drawCheckBoxProperty(m_defaultGuiStyle, uiFieldId, propName, v))
			{
				newValue= v;
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::INT)
		{
			int v= value.getIntValue();
			const auto* enumMeta= desc->getMetaDataOfType<EnumPropertyMetaData>();
			if (enumMeta)
			{
				bool changed= false;
				if (enumMeta->getDisplayStyle() == eEnumDisplayStyle::RadioButtons)
					changed= MkGui::drawRadioButtonsProperty(m_defaultGuiStyle, uiFieldId, propName, enumMeta->getStrings(), v);
				else
					changed= MkGui::drawEnumComboBoxProperty(m_defaultGuiStyle, uiFieldId, propName, enumMeta->getStrings(), v);
				if (changed)
				{
					newValue= v;
					bValueChanged= true;
				}
			}
			else
			{
				if (MkGui::drawIntProperty(m_defaultGuiStyle, uiFieldId, propName, v))
				{
					newValue= v;
					bValueChanged= true;
				}
			}
		}
		else if (variantType == MikanVariantType::LONG)
		{
			int v= static_cast<int>(value.getLongValue());
			if (MkGui::drawIntProperty(m_defaultGuiStyle, uiFieldId, propName, v))
			{
				newValue= static_cast<long>(v);
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::FLOAT)
		{
			float v= value.getFloatValue();
			if (MkGui::drawFloatProperty(m_defaultGuiStyle, uiFieldId, propName, v))
			{
				newValue= v;
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::DOUBLE)
		{
			float vf= static_cast<float>(value.getDoubleValue());
			if (MkGui::drawFloatProperty(m_defaultGuiStyle, uiFieldId, propName, vf))
			{
				newValue= static_cast<double>(vf);
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::STRING)
		{
			std::string v= value.getStringValue();
			const auto* assetMeta= desc->getMetaDataOfType<AssetReferenceFactoryMetaData>();
			if (assetMeta)
			{
				if (MkGui::drawFilePathProperty(m_defaultGuiStyle, uiFieldId, propName, v))
				{
					const AssetReferenceFactory* factory= assetMeta->getFactory();
					const char* picked= tinyfd_openFileDialog(
						factory->getFileDialogTitle(),
						factory->getDefaultPath(),
						factory->getFilterPatternCount(),
						factory->getFilterPatterns(),
						factory->getFilterDescription(),
						0);
					if (picked && picked[0] != '\0')
					{
						newValue= std::string(picked);
						bValueChanged= true;
					}
				}
			}
			else
			{
				char buf[256];
				strncpy_s(buf, sizeof(buf), v.c_str(), _TRUNCATE);
				if (MkGui::drawStringProperty(m_defaultGuiStyle, uiFieldId, propName, buf, sizeof(buf)))
				{
					newValue= std::string(buf);
					bValueChanged= true;
				}
			}
		}
		else if (variantType == MikanVariantType::VECTOR2F)
		{
			const MikanVector2f& vec= value.getVector2fValue();
			float v[2]= {vec.x, vec.y};
			if (MkGui::drawFloat2Property(m_defaultGuiStyle, uiFieldId, propName, v))
			{
				newValue= MikanVector2f{v[0], v[1]};
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::VECTOR3F)
		{
			const MikanVector3f& vec= value.getVector3fValue();
			float v[3]= {vec.x, vec.y, vec.z};
			if (MkGui::drawFloat3Property(m_defaultGuiStyle, uiFieldId, propName, v))
			{
				newValue= MikanVector3f{v[0], v[1], v[2]};
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::VECTOR4F)
		{
			const MikanVector4f& vec= value.getVector4fValue();
			float v[4]= {vec.x, vec.y, vec.z, vec.w};
			if (MkGui::drawFloat4Property(m_defaultGuiStyle, uiFieldId, propName, v))
			{
				newValue= MikanVector4f{v[0], v[1], v[2], v[3]};
				bValueChanged= true;
			}
		}
		else if (variantType == MikanVariantType::INT_ARRAY)
		{
			const auto& intList= value.getIntArrayValue();
			ImGui::LabelText(propName.c_str(), "[%d items]", (int)intList.size());
			for (int i= 0; i < (int)intList.size(); ++i)
			{
				ImGui::Text("  [%d] %d", i, intList[i]);
			}
		}

		if (isReadOnly)
		{
			ImGui::EndDisabled();
		}

		if (bValueChanged && !isReadOnly && accessor)
		{
			// Capture by value for safe deferred use
			addDeferredGuiEvent([accessor, propName, newValue]() mutable
								{ accessor->setPropertyValue(propName, newValue); });
		}
	}
}

void GuiPanel_EntityAccessor::drawFunctionsGui()
{
	drawFunctionsGui({});
}

void GuiPanel_EntityAccessor::drawFunctionsGui(const std::set<std::string>& functionNames)
{
	IEntityAccessorPtr accessor= m_entityAccessor.lock();
	if (!accessor)
	{
		return;
	}

	// Render function buttons
	if (!m_functionDescriptors.empty())
	{
		ImGui::Separator();
		for (const FunctionDescriptorConstPtr& funcDesc : m_functionDescriptors)
		{
			// If a function is UI hidden, skip it
			if (funcDesc->isUIHidden())
			{
				continue;
			}

			// If a function name filter is provided, skip functions that aren't in the filter set
			if (!functionNames.empty() &&
				functionNames.find(funcDesc->getFunctionName()) == functionNames.end())
			{
				continue;
			}

			if (ImGui::Button(funcDesc->getDisplayName().c_str()))
			{
				const std::string funcName= funcDesc->getFunctionName();
				addDeferredGuiEvent([accessor, funcName]()
									{
					if (accessor)
					{
						accessor->invokeFunction(funcName);
					} });
			}
		}
	}
}

void GuiPanel_EntityAccessor::onGui()
{
	drawPropertiesGui();
	drawFunctionsGui();
}

void GuiPanel_EntityAccessor::onEntityDisposed(const IEntityAccessor* selfPtr)
{
	clearEntityAccessor();
}

void GuiPanel_EntityAccessor::onEntityConfigChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	IEntityAccessorPtr entityAccessor= m_entityAccessor.lock();
	assert(entityAccessor);

	// Forward the change notification
	if (OnEntityPropertyChanged)
	{
		OnEntityPropertyChanged(entityAccessor, changedPropertySet);
	}
}
