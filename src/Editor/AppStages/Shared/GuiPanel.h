#pragma once

#include <functional>
#include <vector>

class IGuiPanel
{
public:
	IGuiPanel() = default;
	virtual ~IGuiPanel() = default;

	virtual void render(float deltaSeconds) = 0;
	virtual void update(float deltaSeconds) = 0;
	virtual void dispose() = 0;
	virtual void addUpdateCallback(std::function<void()> callback) = 0;
};

class GuiPanel : public IGuiPanel
{
public:
	GuiPanel() = default;
	virtual ~GuiPanel();

	// IGuiPanel
	virtual void render(float deltaSeconds) override {}
	virtual void update(float deltaSeconds) override;
	virtual void dispose() override;
	virtual void addUpdateCallback(std::function<void()> callback) override;

protected:
	std::vector<std::function<void()>> m_updateCallbacks;
};
