#include "TextureSourceListDataSource.h"
#include "TextureSourceQueries.h"
#include "TextureSourceComponent.h"

TextureSourceListDataSource::TextureSourceListDataSource(ProjectManagerPtr projectManager)
{
	if (projectManager != nullptr)
	{
		comboEntrieValues = TextureSourceQueries::getTextureSourceComponentList(projectManager);

		for (TextureSourceComponentPtr textureSourceComponent : comboEntrieValues)
		{
			comboEntrieNames.push_back(textureSourceComponent->getName());
		}
	}
}

int TextureSourceListDataSource::getEntryIndex(TextureSourceComponentPtr TextureSourceComponent) const
{
	auto it = std::find(comboEntrieValues.begin(), comboEntrieValues.end(), TextureSourceComponent);
	if (it != comboEntrieValues.end())
	{
		return static_cast<int>(std::distance(comboEntrieValues.begin(), it));
	}

	return -1;
}

int TextureSourceListDataSource::getEntryCount()
{
	return (int)comboEntrieValues.size();
}

TextureSourceComponentPtr TextureSourceListDataSource::getEntryAtIndex(int index) const
{ 
	return (index >= 0 && index < (int)comboEntrieNames.size()) ? comboEntrieValues[index] : TextureSourceComponentPtr();
}

const std::string& TextureSourceListDataSource::getEntryDisplayString(int index)
{
	return comboEntrieNames[index];
}
