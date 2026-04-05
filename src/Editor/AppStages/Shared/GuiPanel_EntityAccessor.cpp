#include "GuiPanel_EntityAccessor.h"
#include "CommonConfig.h"
#include "MikanVariantTypes.h"

#include "imgui.h"

#include <assert.h>

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

	m_modelName = modelName;
	m_propertyDescriptors.clear();
	m_orderedPropertyDescriptors.clear();
	m_functionDescriptors.clear();
	m_onConstructCallback = onConstructCallback;

	for (const PropertyDescriptorConstPtr& descriptor : propertyDescriptors)
	{
		if (descriptor->isUIHidden())
			continue;

		m_propertyDescriptors.insert({ descriptor->getName(), descriptor });
		m_orderedPropertyDescriptors.push_back(descriptor);
	}

	for (const FunctionDescriptorConstPtr& descriptor : functionDescriptors)
	{
		m_functionDescriptors.push_back(descriptor);
	}

	if (onConstructCallback && !onConstructCallback())
		return false;

	return true;
}

void GuiPanel_EntityAccessor::dispose()
{
	clearEntityAccessor();
	m_propertyDescriptors.clear();
	m_orderedPropertyDescriptors.clear();
	m_functionDescriptors.clear();
	m_onConstructCallback = nullptr;

	GuiPanel::dispose();
}

void GuiPanel_EntityAccessor::clearEntityAccessor()
{
	IEntityAccessorPtr oldEntityAccessor = m_entityAccessor.lock();

	if (oldEntityAccessor)
	{
		CommonConfigPtr oldEntityConfig = oldEntityAccessor->getEntityConfig();
		assert(m_bWasAccessorSet);

		oldEntityAccessor->onDisposed -=
			MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityDisposed);
		oldEntityConfig->OnPropertyChanged -=
			MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityConfigChanged);

		m_entityAccessor.reset();
		m_bWasAccessorSet = false;
	}
	else
	{
		// If this fires the entity accessor was destroyed without us being notified
		assert(!m_bWasAccessorSet);
	}
}

void GuiPanel_EntityAccessor::setEntityAccessor(IEntityAccessorPtr newEntityAccessor)
{
	IEntityAccessorPtr oldEntityAccessor = m_entityAccessor.lock();

	if (newEntityAccessor != oldEntityAccessor)
	{
		clearEntityAccessor();

		if (newEntityAccessor)
		{
			CommonConfigPtr newEntityConfig = newEntityAccessor->getEntityConfig();

			newEntityAccessor->onDisposed +=
				MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityDisposed);
			newEntityConfig->OnPropertyChanged +=
				MakeDelegate(this, &GuiPanel_EntityAccessor::onEntityConfigChanged);

			m_bWasAccessorSet = true;
		}

		m_entityAccessor = newEntityAccessor;
	}
}

void GuiPanel_EntityAccessor::onGui()
{
	IEntityAccessorPtr accessor = m_entityAccessor.lock();
	if (!accessor)
	{
		return;
	}

	// Render auto-generated property widgets
	for (const PropertyDescriptorConstPtr& desc : m_orderedPropertyDescriptors)
	{
		const std::string& propName = desc->getName();
		const bool isReadOnly = desc->isReadOnly();

		MikanVariant value;
		accessor->getPropertyValue(propName, value);
		const MikanVariantType variantType = value.value_type;

		if (isReadOnly)
		{
			ImGui::BeginDisabled(true);
		}

		bool bValueChanged = false;
		MikanVariant newValue = value;

		if (variantType == MikanVariantType::BOOL)
		{
			bool v = value.getBoolValue();
			if (ImGui::Checkbox(propName.c_str(), &v))
			{
				newValue = v;
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::INT)
		{
			int v = value.getIntValue();
			if (ImGui::InputInt(propName.c_str(), &v))
			{
				newValue = v;
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::LONG)
		{
			int v = static_cast<int>(value.getLongValue());
			if (ImGui::InputInt(propName.c_str(), &v))
			{
				newValue = static_cast<long>(v);
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::FLOAT)
		{
			float v = value.getFloatValue();
			if (ImGui::InputFloat(propName.c_str(), &v))
			{
				newValue = v;
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::DOUBLE)
		{
			double v = value.getDoubleValue();
			float vf = static_cast<float>(v);
			if (ImGui::InputFloat(propName.c_str(), &vf))
			{
				newValue = static_cast<double>(vf);
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::STRING)
		{
			std::string v = value.getStringValue();
			char buf[256];
			strncpy_s(buf, sizeof(buf), v.c_str(), _TRUNCATE);
			if (ImGui::InputText(propName.c_str(), buf, sizeof(buf)))
			{
				newValue = std::string(buf);
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::VECTOR2F)
		{
			const MikanVector2f& vec = value.getVector2fValue();
			float v[2] = { vec.x, vec.y };
			if (ImGui::InputFloat2(propName.c_str(), v))
			{
				MikanVector2f newVec{ v[0], v[1] };
				newValue = newVec;
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::VECTOR3F)
		{
			const MikanVector3f& vec = value.getVector3fValue();
			float v[3] = { vec.x, vec.y, vec.z };
			if (ImGui::InputFloat3(propName.c_str(), v))
			{
				MikanVector3f newVec{ v[0], v[1], v[2] };
				newValue = newVec;
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::VECTOR4F)
		{
			const MikanVector4f& vec = value.getVector4fValue();
			float v[4] = { vec.x, vec.y, vec.z, vec.w };
			if (ImGui::InputFloat4(propName.c_str(), v))
			{
				MikanVector4f newVec{ v[0], v[1], v[2], v[3] };
				newValue = newVec;
				bValueChanged = true;
			}
		}
		else if (variantType == MikanVariantType::INT_ARRAY)
		{
			const std::vector<int>& intList = value.getIntArrayValue();
			ImGui::LabelText(propName.c_str(), "[%d items]", (int)intList.size());
			for (int i = 0; i < (int)intList.size(); ++i)
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
			addDeferredGuiEvent([accessor, propName, newValue]() mutable {
				accessor->setPropertyValue(propName, newValue);
			});
		}
	}

	// Render function buttons
	if (!m_functionDescriptors.empty())
	{
		ImGui::Separator();
		for (const FunctionDescriptorConstPtr& funcDesc : m_functionDescriptors)
		{
			if (ImGui::Button(funcDesc->getDisplayName().c_str()))
			{
				const std::string funcName = funcDesc->getFunctionName();
				addDeferredGuiEvent([accessor, funcName]() {
					if (accessor)
					{
						accessor->invokeFunction(funcName);
					}
				});
			}
		}
	}
}

void GuiPanel_EntityAccessor::onEntityDisposed(const IEntityAccessor* selfPtr)
{
	clearEntityAccessor();
}

void GuiPanel_EntityAccessor::onEntityConfigChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	IEntityAccessorPtr entityAccessor = m_entityAccessor.lock();
	assert(entityAccessor);

	// Forward the change notification
	if (OnEntityPropertyChanged)
	{
		OnEntityPropertyChanged(entityAccessor, changedPropertySet);
	}
}
