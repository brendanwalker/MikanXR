#pragma once

#include "Shared/RmlModel.h"

#include <filesystem>
#include <memory>
#include <string>
#include <set>

class RmlModel_PropertyInterface : public RmlModel
{
public:
	template <class t_property_interface>
	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName)
	{
		std::vector<std::string> propertyNames;
		t_property_interface::getPropertyNamesStatic(propertyNames);

		std::vector<std::string> functionNames;
		t_property_interface::getFunctionNamesStatic(propertyNames);

		return init(
			rmlContext,
			modelName,
			propertyNames,
			functionNames);
	}

	bool init(
		Rml::Context* rmlContext,
		const std::string& modelName,
		const std::vector<std::string>& propertyNames = {},
		const std::vector<std::string>& functionNames = {});

	void setPropertyInterface(class IPropertyInterface* propertyInterface);
	void setFunctionInterface(class IFunctionInterface* functionInterface);

protected:
	void onPropertyChanged(const std::string& propertyName);

private:
	class IPropertyInterface* m_propertyInterface = nullptr;
	class IFunctionInterface* m_functionInterface = nullptr;
	std::set<std::string> m_propertyNames;
};

using RmlModel_PropertyInterfacePtr = std::shared_ptr<RmlModel_PropertyInterface>;