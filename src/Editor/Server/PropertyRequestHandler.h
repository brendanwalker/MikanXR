#pragma once

#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "IServerRequestHandler.h"
#include "ObjectSystemConfigFwd.h"

class PropertyRequestHandler : public IServerRequestHandler
{
public:
	PropertyRequestHandler(class MikanServer* owner)
		: IServerRequestHandler(owner)
	{
	}

	virtual bool startup(class MainWindow* mainWindow) override;
	virtual void shutdown() override;

protected:
	// Project lifecycle (the ProjectConfig instance is replaced on every
	// project load, so the property-change subscription must follow it)
	void onProjectLoaded(ProjectManagerPtr projectManager);
	void onProjectPreUnload(ProjectManagerPtr projectManager);
	void bindProjectConfig(ProjectConfigPtr projectConfig);
	void unbindProjectConfig();

	// Property Events
	void onProjectConfigChanged(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void onObjectSystemDefinitionChanged(MikanObjectSystemDefinitionPtr systemDefinition,
										 const std::string& propertyName);
	void onComponentDefinitionChanged(MikanComponentDefinitionPtr componentDefinition, const std::string& propertyName);

	// Property Request Handlers
	void setPropertyValueHandler(const ClientRequest& request, ClientResponse& response);
	void getPropertyValueHandler(const ClientRequest& request, ClientResponse& response);
	void getComponentValuesHandler(const ClientRequest& request, ClientResponse& response);
	void getComponentListHandler(const ClientRequest& request, ClientResponse& response);
	void getSystemValuesHandler(const ClientRequest& request, ClientResponse& response);
	void createSystemObjectHandler(const ClientRequest& request, ClientResponse& response);
	void destroySystemObjectHandler(const ClientRequest& request, ClientResponse& response);
	void setPropertyNotifyModeHandler(const ClientRequest& request, ClientResponse& response);
	void getPropertyDescriptorsHandler(const ClientRequest& request, ClientResponse& response);

private:
	ProjectConfigWeakPtr m_projectConfig;
};