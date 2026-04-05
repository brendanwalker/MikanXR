#pragma once

#include <functional>
#include <vector>

class IGuiPanel
{
public:
	IGuiPanel() = default;
	virtual ~IGuiPanel() = default;

	virtual void onGui() = 0;
	virtual void addDeferredGuiEvent(std::function<void()> callback) = 0;
	virtual void processDeferredGuiEvents() = 0;
	virtual void dispose() = 0;
};

class GuiPanel : public IGuiPanel
{
public:
	GuiPanel() = delete;
	GuiPanel(class AppStage* ownerAppStage);
	virtual ~GuiPanel();

	class AppStage* getOwnerAppStage() const { return m_ownerAppStage; }

	// IGuiPanel
	virtual void onGui() override {}
	virtual void addDeferredGuiEvent(std::function<void()> callback) override;
	virtual void processDeferredGuiEvents() override;
	virtual void dispose() override;

protected:
	class AppStage* m_ownerAppStage = nullptr;
	std::vector<std::function<void()>> m_deferredGuiEvents;
};
