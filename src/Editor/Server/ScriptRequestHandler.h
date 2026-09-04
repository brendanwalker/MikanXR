#pragma once

#include "IServerRequestHandler.h"
#include "MikanAPITypes.h"
#include "ScriptingFwd.h"

#include <string>
#include <vector>

class ScriptRequestHandler : public IServerRequestHandler
{
public:
	ScriptRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

	// The project's one script context. Binding subscribes to its messages and
	// installs the HTTP trigger routes its scripts declared; unbinding removes them.
	void bindScriptContext(CommonScriptContextPtr scriptContext);
	void unbindScriptContext(CommonScriptContextPtr scriptContext);
	CommonScriptContextPtr getScriptContext() const { return m_scriptContext.lock(); }

	// HTTP trigger routes (e.g. for Stream Deck style integrations). routeName is registered
	// under "/trigger/<routeName>" on the HTTP interprocess message server. Callable from
	// either C++ or (indirectly, via ScriptContext.registerHttpTrigger) Lua scripts.
	bool registerHttpTriggerRoute(const std::string& routeName, const std::string& triggerName);
	void unregisterHttpTriggerRoute(const std::string& routeName);

protected:
	void publishScriptMessageEvent(const std::string& message);
	void invokeScriptTriggerHandler(const ClientRequest& request, ClientResponse& response);
	void invokeScriptMessageHandler(const ClientRequest& request, ClientResponse& response);

	MikanAPIResult invokeScriptTriggerInternal(const std::string& triggerName);

private:
	CommonScriptContextWeakPtr m_scriptContext;
};
