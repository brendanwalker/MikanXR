#pragma once

#include "CommonConfigFwd.h"
#include "IEntityAccessor.h"
#include "MkGuiDrawUtils.h"
#include "MulticastDelegate.h"
#include "Shared/GuiPanel.h"

#include <functional>
#include <memory>
#include <string>
#include <map>
#include <vector>

class GuiPanel_EntityAccessor : public GuiPanel
{
public:
	GuiPanel_EntityAccessor(AppStage* ownerAppStage);
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

	// Returns true if the property was fully rendered (skip default widget).
	using PropertyRendererCallback = std::function<bool(const PropertyDescriptorConstPtr&)>;

	// Register a custom renderer for a named property.
	void setPropertyRenderer(const std::string& propName, PropertyRendererCallback renderer);

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
	std::map<std::string, PropertyRendererCallback> m_propertyRenderers;
	OnConstruct m_onConstructCallback;
	MkGuiStyleConstPtr m_defaultGuiStyle;
};

using GuiPanel_EntityAccessorPtr = std::shared_ptr<GuiPanel_EntityAccessor>;
