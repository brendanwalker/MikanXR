#include "ClientListDataSource.h"
#include "ClientTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"

ClientListDataSource::ClientListDataSource(ClientTextureSourceSystemPtr ClientTextureSourceSystem)
{
	if (ClientTextureSourceSystem != nullptr)
	{
		auto& clientSourcesMap= ClientTextureSourceSystem->getClientTextureSourceMap();

		for (auto it = clientSourcesMap.begin(); it != clientSourcesMap.end(); it++)
		{
			ClientTextureSourceComponentPtr clientTextureSourceComponent= it->second.lock();

			comboEntrieValues.push_back(clientTextureSourceComponent);
			comboEntrieNames.push_back(clientTextureSourceComponent->getClientSourceName());
		}
	}
}

int ClientListDataSource::getEntryIndex(ClientTextureSourceComponentPtr TextureSourceComponent) const
{
	auto it = std::find(comboEntrieValues.begin(), comboEntrieValues.end(), TextureSourceComponent);
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

ClientTextureSourceComponentPtr ClientListDataSource::getEntryAtIndex(int index) const
{ 
	return comboEntrieValues[index];
}

const std::string& ClientListDataSource::getEntryDisplayString(int index)
{
	return comboEntrieNames[index];
}
