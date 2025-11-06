#include "ClientListDataSource.h"
#include "ClientVideoSourceSystem.h"
#include "ClientVideoSourceComponent.h"

ClientListDataSource::ClientListDataSource(ClientVideoSourceSystemPtr clientVideoSourceSystem)
{
	if (clientVideoSourceSystem != nullptr)
	{
		auto& clientSourcesMap= clientVideoSourceSystem->getClientVideoSourceMap();

		for (auto it = clientSourcesMap.begin(); it != clientSourcesMap.end(); it++)
		{
			ClientVideoSourceComponentPtr clientVideoSourceComponent= it->second.lock();

			comboEntrieValues.push_back(clientVideoSourceComponent);
			comboEntrieNames.push_back(clientVideoSourceComponent->getDevicePath());
		}
	}
}

int ClientListDataSource::getEntryIndex(ClientVideoSourceComponentPtr videoSourceComponent) const
{
	auto it = std::find(comboEntrieValues.begin(), comboEntrieValues.end(), videoSourceComponent);
	if (it != comboEntrieValues.end())
	{
		return static_cast<int>(std::distance(comboEntrieValues.begin(), it));
	}

	return -1;
}

int ClientListDataSource::getEntryCount()
{
	return (int)comboEntrieValues.size();
}

ClientVideoSourceComponentPtr ClientListDataSource::getEntryAtIndex(int index) const
{ 
	return comboEntrieValues[index];
}

const std::string& ClientListDataSource::getEntryDisplayString(int index)
{
	return comboEntrieNames[index];
}
