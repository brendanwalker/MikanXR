#pragma once

#include "CommonConfigFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared\RmlDataBinding.h"

class RmlDataBinding_ComponentList : public RmlDataBinding
{
public:
	using ComponentIdList = Rml::Vector<int>;
	using FillComponentIdList = std::function<void(CommonConfigPtr, ComponentIdList&)>;

	RmlDataBinding_ComponentList()= default;
	virtual ~RmlDataBinding_ComponentList();

	bool init(
		Rml::DataModelConstructor constructor,
		CommonConfigPtr ownerConfig,
		const std::string& listName,
		FillComponentIdList fillFuntion);
	void setOwnerConfig(CommonConfigPtr newOwnerConfig);
	virtual void dispose() override;

	const Rml::Vector<int>& getComponentIdList() const { return m_componentIdList; }
	bool isEmpty() const { return m_componentIdList.empty(); }
	bool contains(int componentId) const;

	MulticastDelegate<void(bool bOwnerChanged)> OnChanged;

protected:
	void onConfigMarkedDirty(
		CommonConfigPtr configPtr, 
		const ConfigPropertyChangeSet& changedPropertySet);
	void rebuildComponentIdList(bool bOwnerChanged);

private:
	CommonConfigWeakPtr m_ownerConfig;
	std::string m_objectListName;
	FillComponentIdList m_fillFunction;
	ComponentIdList m_componentIdList;
};

using RmlDataBinding_ComponentListPtr = std::shared_ptr<RmlDataBinding_ComponentList>;