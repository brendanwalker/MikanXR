#pragma once

#include "CommonConfig.h"
#include "RmlPropertyInterface.h"
#include "RmlFunctionInterface.h"
#include "Shared/RmlModel.h"
#include "RmlFwd.h"

#include <memory>
#include <string>
#include <map>

class RmlModel_PropertyInterface : public RmlModel
{
public:
	using OnConstruct = std::function<bool(Rml::DataModelConstructor&)>;

	template <class t_property_interface>
	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName,
		OnConstruct onContructCallback = {})
	{
		std::vector<RmlPropertyDescriptorConstPtr> propertyNames;
		t_property_interface::getRmlPropertyDescriptors(propertyNames);

		std::vector<RmlFunctionDescriptorConstPtr> functionNames;
		t_property_interface::getRmlFunctionDescriptors(functionNames);

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
		const std::vector<RmlPropertyDescriptorConstPtr>& propertyDescriptors,
		const std::vector<RmlFunctionDescriptorConstPtr>& functionDescriptors,
		OnConstruct onContructCallback);

	void setPropertyInterface(
		IRmlPropertyInterfacePtr propertyInterface,
		CommonConfigPtr propertyChangeEventSource = CommonConfigPtr());
	void setFunctionInterface(IRmlFunctionInterfacePtr functionInterface);

protected:
	void onPropertiesChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

private:
	CommonConfigWeakPtr m_propertyChangeEventSource;
	IRmlPropertyInterfaceWeakPtr m_propertyInterface;
	IRmlFunctionInterfaceWeakPtr m_functionInterface;
	std::map<std::string, RmlPropertyDescriptorConstPtr> m_propertyDescriptors;
};