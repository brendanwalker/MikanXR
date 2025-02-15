#pragma once

#include <string>

class IRemoteControllableAppStage
{
public:
	virtual ~IRemoteControllableAppStage() {}
};

class RemoteControllableAppStageFactory
{
public:
	virtual std::string getAppStageName() = 0;
	virtual IRemoteControllableAppStage* pushAppStage() = 0;
};