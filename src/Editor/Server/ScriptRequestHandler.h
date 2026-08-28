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

	// Scripting Events
	void bindScriptContect(CommonScriptContextPtr scriptContext);
	void unbindScriptContect(CommonScriptContextPtr scriptContext);

	// Snapshot of the live bound script contexts (dead weak references skipped)
	void getBoundScriptContexts(std::vector<CommonScriptContextPtr>& outContexts) const;

	// HTTP trigger routes (e.g. for Stream Deck style integrations). routeName is registered
	// under "/trigger/<routeName>" on the HTTP interprocess message server. Callable from
	// either C++ or (indirectly, via ScriptContext.registerHttpTrigger) Lua scripts.
	bool registerHttpTriggerRoute(const std::string& routeName, const std::string& ownerSystemName, int componentId,
								  const std::string& triggerName);
	void unregisterHttpTriggerRoute(const std::string& routeName);

protected:
	void publishScriptMessageEvent(const std::string& message);
	void invokeComponentScriptTriggerHandler(const ClientRequest& request, ClientResponse& response);
	void invokeScriptMessageHandler(const ClientRequest& request, ClientResponse& response);

	MikanAPIResult invokeComponentScriptTriggerInternal(const std::string& ownerSystemName, int componentId,
														const std::string& triggerName);

private:
	std::vector<CommonScriptContextWeakPtr> m_scriptContexts;
};