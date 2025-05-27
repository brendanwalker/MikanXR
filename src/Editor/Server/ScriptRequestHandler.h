#pragma once

#include "IServerRequestHandler.h"
#include "ScriptingFwd.h"

class ScriptRequestHandler : public IServerRequestHandler
{
public:
	ScriptRequestHandler(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

	// Scripting Events
	void bindScriptContect(CommonScriptContextPtr scriptContext);
	void unbindScriptContect(CommonScriptContextPtr scriptContext);

protected:
	void publishScriptMessageEvent(const std::string& message);
	void invokeScriptMessageHandler(const ClientRequest& request, ClientResponse& response);

private:
	std::vector<CommonScriptContextWeakPtr> m_scriptContexts;
};