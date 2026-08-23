#pragma once

#include "PropertyInterface.h"
#include "AssetReference.h"

class AssetReferenceFactoryMetaData : public PropertyMetaData
{
public:
	AssetReferenceFactoryMetaData(std::shared_ptr<AssetReferenceFactory> factory)
		: m_factory(std::move(factory))
	{
	}

	const AssetReferenceFactory* getFactory() const { return m_factory.get(); }

private:
	std::shared_ptr<AssetReferenceFactory> m_factory;
};
