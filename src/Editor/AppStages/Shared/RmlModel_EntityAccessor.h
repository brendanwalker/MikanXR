#pragma once

#include "CommonConfig.h"
#include "IEntityAccessor.h"
#include "Shared/RmlModel.h"
#include "MulticastDelegate.h"
#include "RmlFwd.h"

#include <memory>
#include <string>
#include <map>

class RmlModel_EntityAccessor : public RmlModel
{
public:
	virtual ~RmlModel_EntityAccessor();

	using OnConstruct = std::function<bool(Rml::DataModelConstructor&)>;

	template <class t_property_interface>
	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName,
		OnConstruct onContructCallback = {})
	{
		std::vector<PropertyDescriptorConstPtr> propertyNames;
		t_property_interface::getPropertyDescriptors(propertyNames);

		std::vector<FunctionDescriptorConstPtr> functionNames;
		t_property_interface::getFunctionDescriptors(functionNames);

		return init(
			rmlContext,
			modelName,
			propertyNames,
			functionNames,
			onContructCallback);
	}

	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName,
		const std::vector<PropertyDescriptorConstPtr>& propertyDescriptors,
		const std::vector<FunctionDescriptorConstPtr>& functionDescriptors,
		OnConstruct onContructCallback);

	void clearEntityAccessor();
	void setEntityAccessor(IEntityAccessorPtr newEntityAccessor);

	MulticastDelegate<void(IEntityAccessorPtr accessorPtr, const ConfigPropertyChangeSet& changedPropertySet)> OnEntityPropertyChanged;

protected:
	void onEntityDisposed(const IEntityAccessor* selfPtr);
	void onEntityConfigChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

private:
	bool m_bWasAccessorSet = false;
	IEntityAccessorWeakPtr m_entityAccessor;
	std::map<std::string, PropertyDescriptorConstPtr> m_propertyDescriptors;
};