#pragma once

#include "ComponentFwd.h"
#include "Shared/GuiPanel_EntityAccessor.h"

class AppStage;

class GuiPanel_MikanComponent : public IGuiPanel
{
public:
	GuiPanel_MikanComponent();
	virtual ~GuiPanel_MikanComponent() = default;

	virtual bool init(AppStage* ownerAppStage) = 0;
	virtual void onConstruct() {}

	inline GuiPanel_EntityAccessorPtr getPropertyInterface() const { return m_entityAccessor; }

	MikanComponentPtr getComponent() const;
	virtual bool setComponent(MikanComponentPtr component);
	virtual void onComponentPropertyChanged(
		IEntityAccessorPtr accessorPtr,
		const ConfigPropertyChangeSet& changedPropertySet) {}

	// IGuiPanel
	virtual void render(float deltaSeconds) override;
	virtual void update(float deltaSeconds) override;
	virtual void dispose() override;
	virtual void addUpdateCallback(std::function<void()> callback) override;

protected:
	template <class t_component_type>
	bool initTypedPropertyInterface(AppStage* ownerAppStage)
	{
		const bool bSuccess =
			m_entityAccessor->init<t_component_type>(
				t_component_type::k_componentClassName,
				[this]() -> bool
				{
					onConstruct();
					return true;
				});

		if (bSuccess)
		{
			m_entityAccessor->OnEntityPropertyChanged += MakeDelegate(
				this,
				&GuiPanel_MikanComponent::onComponentPropertyChanged);
		}

		return bSuccess;
	}

	MikanComponentWeakPtr m_component;
	GuiPanel_EntityAccessorPtr m_entityAccessor;
};

using GuiPanel_MikanComponentPtr = std::shared_ptr<GuiPanel_MikanComponent>;
