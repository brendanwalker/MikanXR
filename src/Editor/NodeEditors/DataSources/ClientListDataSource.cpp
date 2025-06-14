#include "ClientListDataSource.h"
#include "ClientSourceManager.h"

ClientListDataSource::ClientListDataSource()
{
	auto* clientSourceManager = ClientSourceManager::getInstance();

	if (clientSourceManager != nullptr)
	{
		auto& clientSources= clientSourceManager->getClientSources();

		for (auto it = clientSources.getMap().begin(); it != clientSources.getMap().end(); it++)
		{
			ClientSourceManager::ClientSource* clientSource= it->second;

			comboEntries.push_back(clientSource->clientId);
		}
	}
}