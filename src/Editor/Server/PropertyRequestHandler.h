#pragma once

#include "IServerRequestHandler.h"
#include "ScriptingFwd.h"

class PropertyRequestHandler : public IServerRequestHandler
{
public:
	PropertyRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	void setPropertyValueHandler(const ClientRequest& request, ClientResponse& response);
	void getPropertyValueHandler(const ClientRequest& request, ClientResponse& response);
	void setPropertyNotifyModeHandler(const ClientRequest& request, ClientResponse& response);
	void getPropertyDescriptorsHandler(const ClientRequest& request, ClientResponse& response);

private:
	std::vector<CommonScriptContextWeakPtr> m_scriptContexts;
};