#pragma once

#include "ObjectFwd.h"
#include "Shared/GuiPanel_EntityAccessor.h"

class GuiPanel_MikanObjectSystem : public IGuiPanel
{
public:
	GuiPanel_MikanObjectSystem() = delete;
	GuiPanel_MikanObjectSystem(class AppStage* ownerAppStage);
	virtual ~GuiPanel_MikanObjectSystem() = default;

	virtual bool init() = 0;
	virtual void onConstruct() {}

	MikanObjectSystemPtr getObjectSystem() const;
	void setObjectSystem(MikanObjectSystemPtr objectSystem);

	// IGuiPanel
	virtual class AppStage* getOwnerAppStage() const override { return m_entityAccessor->getOwnerAppStage(); }
	virtual class MkGuiStyleManager* getGuiStyleManager() const override { return m_entityAccessor->getGuiStyleManager(); }
	virtual void onGui() override;
	virtual void addDeferredGuiEvent(std::function<void()> callback) override;
	virtual void processDeferredGuiEvents() override;
	virtual void dispose() override;

protected:
	template <class t_object_system_type>
	bool initTypedPropertyInterface(AppStage* ownerAppStage)
	{
		const bool bSuccess =
			m_entityAccessor->init<t_object_system_type>(
				t_object_system_type::k_objectSystemClassName,
				[this]() -> bool
				{
					onConstruct();
					return true;
				});

		return bSuccess;
	}

	MikanObjectSystemWeakPtr m_objectSystem;
	GuiPanel_EntityAccessorPtr m_entityAccessor;
};
