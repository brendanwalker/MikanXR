#pragma once

#include "CommonConfigFwd.h"
#include "IEntityAccessor.h"
#include "MulticastDelegate.h"
#include "Shared/GuiPanel.h"

#include <memory>
#include <string>
#include <map>
#include <vector>

class GuiPanel_EntityAccessor : public GuiPanel
{
public:
	GuiPanel_EntityAccessor(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	virtual ~GuiPanel_EntityAccessor();

	using OnConstruct = std::function<bool()>;

	template <class t_property_interface>
	bool init(const std::string& modelName, OnConstruct onConstructCallback = {})
	{
		std::vector<PropertyDescriptorConstPtr> propertyDescriptors;
		t_property_interface::getPropertyDescriptors(propertyDescriptors);

		std::vector<FunctionDescriptorConstPtr> functionDescriptors;
		t_property_interface::getFunctionDescriptors(functionDescriptors);

		return init(modelName, propertyDescriptors, functionDescriptors, onConstructCallback);
	}

	bool init(
		const std::string& modelName,
		const std::vector<PropertyDescriptorConstPtr>& propertyDescriptors,
		const std::vector<FunctionDescriptorConstPtr>& functionDescriptors,
		OnConstruct onConstructCallback = {});

	// IGuiPanel
	virtual void onGui() override;
	virtual void dispose() override;

	void clearEntityAccessor();
	void setEntityAccessor(IEntityAccessorPtr newEntityAccessor);
	inline IEntityAccessorPtr getEntityAccessor() const { return m_entityAccessor.lock(); }

	MulticastDelegate<void(IEntityAccessorPtr accessorPtr, const ConfigPropertyChangeSet& changedPropertySet)> OnEntityPropertyChanged;

protected:
	void onEntityDisposed(const IEntityAccessor* selfPtr);
	void onEntityConfigChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

private:
	bool m_bWasAccessorSet = false;
	IEntityAccessorWeakPtr m_entityAccessor;
	std::string m_modelName;
	std::map<std::string, PropertyDescriptorConstPtr> m_propertyDescriptors;
	std::vector<PropertyDescriptorConstPtr> m_orderedPropertyDescriptors;
	std::vector<FunctionDescriptorConstPtr> m_functionDescriptors;
	OnConstruct m_onConstructCallback;
};

using GuiPanel_EntityAccessorPtr = std::shared_ptr<GuiPanel_EntityAccessor>;
