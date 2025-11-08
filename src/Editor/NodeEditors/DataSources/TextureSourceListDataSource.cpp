#include "TextureSourceListDataSource.h"
#include "TextureSourceSystem.h"
#include "TextureSourceComponent.h"

TextureSourceListDataSource::TextureSourceListDataSource(TextureSourceSystemPtr textureSourceSystem)
{
	if (textureSourceSystem != nullptr)
	{
		comboEntrieValues= textureSourceSystem->getTextureSourceComponentList();

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
	return comboEntrieValues[index];
}

const std::string& TextureSourceListDataSource::getEntryDisplayString(int index)
{
	return comboEntrieNames[index];
}
