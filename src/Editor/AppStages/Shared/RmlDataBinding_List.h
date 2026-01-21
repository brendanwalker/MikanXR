#pragma once

#include "CommonConfigFwd.h"
#include "Logger.h"
#include "MikanComponent.h"
#include "MulticastDelegate.h"
#include "MikanObjectSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared\RmlDataBinding.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

template <typename t_element_type>
class RmlDataBinding_List : public RmlDataBinding
{
public:
	using RmlValueList = Rml::Vector<t_element_type>;
	using FillPropertyList = std::function<void(CommonConfigPtr, RmlValueList&)>;
	using RebuildOnPropertyChangeFunc = std::function<bool(const ConfigPropertyChangeSet&)>;

	RmlDataBinding_List()= default;
	virtual ~RmlDataBinding_List()
	{
		dispose();
	}

	bool init(
		Rml::DataModelConstructor constructor,
		MikanObjectSystemPtr ownerObjectSystem,
		const std::string& listName,
		bool bAddEmptyPrefix= false)
	{
		// Rebuild the list using the system's property interface
		return init(
			constructor,
			ownerObjectSystem->getDefinition(),
			listName,
			[ownerObjectSystem, listName, bAddEmptyPrefix](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
				MikanVariant listPropertyValue;
				if (ownerObjectSystem->getPropertyValue(listName, listPropertyValue))
				{
					outComponentIdList = listPropertyValue.getIntArrayValue();
				}

				if (bAddEmptyPrefix)
				{
					// Add "No Entry" option
					outComponentIdList.insert(outComponentIdList.begin(), INVALID_MIKAN_ID);
				}
			});

		return false;
	}

	bool init(
		Rml::DataModelConstructor constructor,
		CommonConfigPtr ownerConfig,
		const std::string& listName,
		FillPropertyList fillFuntion,
		RebuildOnPropertyChangeFunc rebuildOnPropertyChangeFunc = nullptr)
	{
		m_objectListName = listName;
		m_fillFunction = fillFuntion;
		m_rebuildOnPropertyChangeFunc = rebuildOnPropertyChangeFunc;

		if (!RmlDataBinding::init(constructor))
		{
			return false;
		}

		// Register Data Model Fields
		constructor.Bind(m_objectListName, &m_rmlValueList);

		// Bind the owner config and listen for changes
		setOwnerConfig(ownerConfig);

		return true;
	}

	virtual void dispose() override
	{
		CommonConfigPtr ownerConfig = m_ownerConfig.lock();
		if (ownerConfig)
		{
			// Stop listening for object system changes
			ownerConfig->OnPropertyChanged -=
				MakeDelegate(this, &RmlDataBinding_List<t_element_type>::onConfigMarkedDirty);
		}

		RmlDataBinding::dispose();
	}

	void setOwnerConfig(CommonConfigPtr newOwnerConfig)
	{
		CommonConfigPtr oldOwnerConfig = m_ownerConfig.lock();

		if (oldOwnerConfig != newOwnerConfig)
		{
			if (oldOwnerConfig)
			{
				// Stop listening for object system changes
				oldOwnerConfig->OnPropertyChanged -=
					MakeDelegate(this, &RmlDataBinding_List<t_element_type>::onConfigMarkedDirty);
			}

			if (newOwnerConfig)
			{
				// Start listening for object system changes
				newOwnerConfig->OnPropertyChanged +=
					MakeDelegate(this, &RmlDataBinding_List<t_element_type>::onConfigMarkedDirty);
			}

			m_ownerConfig = newOwnerConfig;

			// Fill in the data model
			rebuildList(true);
		}
	}

	void rebuildList(bool bOwnerChanged= false)
	{
		m_rmlValueList.clear();

		if (m_modelHandle)
		{
			CommonConfigPtr ownerConfig = m_ownerConfig.lock();
			m_fillFunction(ownerConfig, m_rmlValueList);

			m_modelHandle.DirtyVariable(m_objectListName);

			if (OnChanged)
			{
				OnChanged(bOwnerChanged);
			}
		}
	}

	bool fixupSelectedValue(
		const t_element_type inSelectedValue,
		const t_element_type inDefaultValue,
		t_element_type& outSelectedValue) const
	{
		const int listSize = static_cast<int>(m_rmlValueList.size());

		if (listSize > 0)
		{
			auto it= std::find(m_rmlValueList.begin(), m_rmlValueList.end(), inSelectedValue);
			if (it != m_rmlValueList.end())
			{
				outSelectedValue = inSelectedValue;
			}
			else
			{
				outSelectedValue = m_rmlValueList.back();
			}
		}
		else
		{
			outSelectedValue = inDefaultValue;
		}

		return outSelectedValue != inSelectedValue;
	}

	const Rml::Vector<t_element_type>& getRmlValueList() const { return m_rmlValueList; }
	const t_element_type getFirstValue() const { return m_rmlValueList.front(); }
	bool isEmpty() const { return m_rmlValueList.empty(); }
	bool contains(int componentId) const
	{
		return
			std::find(m_rmlValueList.begin(), m_rmlValueList.end(), componentId)
			!= m_rmlValueList.end();
	}



	MulticastDelegate<void(bool bOwnerChanged)> OnChanged;

protected:
	void onConfigMarkedDirty(
		CommonConfigPtr configPtr, 
		const ConfigPropertyChangeSet& changedPropertySet)
	{
		if (m_rebuildOnPropertyChangeFunc)
		{
			if (m_rebuildOnPropertyChangeFunc(changedPropertySet))
			{
				rebuildList(false);
			}
		}
		else if (changedPropertySet.hasPropertyName(m_objectListName))
		{
			rebuildList(false);
		}
	}

private:
	CommonConfigWeakPtr m_ownerConfig;
	std::string m_objectListName;
	FillPropertyList m_fillFunction;
	RebuildOnPropertyChangeFunc m_rebuildOnPropertyChangeFunc;
	RmlValueList m_rmlValueList;
};