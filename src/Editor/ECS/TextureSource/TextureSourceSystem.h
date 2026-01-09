#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTextureSourceTypes.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "TextureSourceSystemConfig.h"

#include <filesystem>
#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

class TextureSourceSystem : public MikanObjectSystem
{
public:
	TextureSourceSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static TextureSourceSystemPtr getSystem() { return s_TextureSourceSystem.lock(); }

	inline static const std::string k_objectSystemClassName = "TextureSourceSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;
	virtual MikanComponentPtr getComponentById(int componentId) const override;

	TextureSourceSystemConfigConstPtr getTextureSourceSystemConfigConst() const;
	TextureSourceSystemConfigPtr getTextureSourceSystemConfig();

	TextureSourceComponentList getTextureSourceComponentList() const;
	TextureSourceIdList getTextureSourceIdList() const;
	TextureSourceComponentPtr getTextureSourceById(MikanTextureSourceID TextureSourceId) const;
	eTextureSourceType getTextureSourceType(MikanTextureSourceID TextureSourceId) const;
	bool removeTextureSource(MikanTextureSourceID TextureSourceId);

	ClientTextureSourceSystemPtr getClientTextureSourceSystem() const { return m_clientTextureSourceSystem; }
	SpoutTextureSourceSystemPtr getSpoutTextureSourceSystem() const { return m_spoutTextureSourceSystem; }

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

private:
	ClientTextureSourceSystemPtr m_clientTextureSourceSystem;
	SpoutTextureSourceSystemPtr m_spoutTextureSourceSystem;

	static TextureSourceSystemWeakPtr s_TextureSourceSystem;
};
