#pragma once

#include "Shared/RmlModel.h"

#include <filesystem>
#include <memory>
#include <string>

class RmlModel_PropertyInterface : public RmlModel
{
public:

	template <class t_property_interface>
	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName)
	{
		init(
			rmlContext,
			modelName,
			t_property_interface::getPropertyNamesStatic(),
			t_property_interface::getFunctionNamesStatic());
	}

	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName,
		const std::vector<std::string>& propertyNames,
		const std::vector<std::string>& functionNames);

	void setPropertyInterface(class IPropertyInterface* propertyInterface);
	void setFunctionInterface(class IFunctionInterface* functionInterface);

private:
	class IPropertyInterface* m_propertyInterface = nullptr;
	class IFunctionInterface* m_functionInterface = nullptr;
};

using RmlModel_PropertyInterfacePtr = std::shared_ptr<RmlModel_PropertyInterface>;