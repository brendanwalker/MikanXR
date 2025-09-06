#include "RmlDataBinding_ComponentList.h"
#include "MikanComponent.h"
#include "MulticastDelegate.h"
#include "MikanObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlDataBinding_ComponentList::~RmlDataBinding_ComponentList()
{
	dispose();
}

bool RmlDataBinding_ComponentList::init(
	Rml::DataModelConstructor constructor,
	CommonConfigPtr ownerConfig,
	const std::string& objectListName,
	FillComponentIdList gatherFuntion)
{
	m_objectListName = objectListName;
	m_fillFunction = gatherFuntion;

	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	// Register Data Model Fields
	constructor.Bind(m_objectListName, &m_componentIdList);

	// Bind the owner config and listen for changes
	setOwnerConfig(ownerConfig);

	return true;
}

void RmlDataBinding_ComponentList::setOwnerConfig(CommonConfigPtr newOwnerConfig)
{
	CommonConfigPtr oldOwnerConfig= m_ownerConfig.lock();

	if (oldOwnerConfig != newOwnerConfig)
	{
		if (oldOwnerConfig)
		{
			// Stop listening for object system changes
			oldOwnerConfig->OnMarkedDirty -=
				MakeDelegate(this, &RmlDataBinding_ComponentList::onConfigMarkedDirty);
		}

		if (newOwnerConfig)
		{
			// Start listening for object system changes
			newOwnerConfig->OnMarkedDirty +=
				MakeDelegate(this, &RmlDataBinding_ComponentList::onConfigMarkedDirty);
		}

		m_ownerConfig = newOwnerConfig;

		// Fill in the data model
		rebuildComponentIdList(true);
	}
}

void RmlDataBinding_ComponentList::dispose()
{
	setOwnerConfig(CommonConfigPtr());

	RmlDataBinding::dispose();
}

bool RmlDataBinding_ComponentList::contains(int componentId) const
{
	return
		std::find(m_componentIdList.begin(), m_componentIdList.end(), componentId) 
		!= m_componentIdList.end();
}

void RmlDataBinding_ComponentList::onConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(m_objectListName))
	{
		rebuildComponentIdList(false);
	}
}

void RmlDataBinding_ComponentList::rebuildComponentIdList(bool bOwnerChanged)
{
	m_componentIdList.clear();

	CommonConfigPtr ownerConfig = m_ownerConfig.lock();
	if (!ownerConfig)
	{
		m_fillFunction(ownerConfig, m_componentIdList);
	}

	if (OnChanged)
	{
		OnChanged(bOwnerChanged);
	}

	m_modelHandle.DirtyVariable(m_objectListName);
}