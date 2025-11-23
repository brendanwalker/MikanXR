#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GlmTransform;

using AnchorMap = std::map<MikanSpatialAnchorID, AnchorComponentWeakPtr>;

class AnchorObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	AnchorObjectSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddAnchor() const;
	AnchorDefinitionPtr getSpatialAnchorConfig(MikanSpatialAnchorID anchorId) const;
	AnchorDefinitionPtr getSpatialAnchorConfigByName(const std::string& anchorName) const;
	MikanSpatialAnchorID addNewAnchor(MikanSceneID ownerSceneId);
	MikanSpatialAnchorID addNewAnchor(MikanSceneID ownerSceneId, const std::string& anchorName, const struct MikanTransform& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

	static const std::string k_anchorVRDevicePathPropertyId;
	std::string anchorVRDevicePath;

	static const std::string k_anchorListPropertyId;
	std::vector<AnchorDefinitionPtr> spatialAnchorList;

	static const std::string k_renderAnchorsPropertyId;
	inline bool getRenderAnchorsFlag() const { return m_bDebugRenderAnchors; }
	void setRenderAnchorsFlag(bool flag);

	MikanSpatialAnchorID nextAnchorId= 0;

protected:
	bool m_bDebugRenderAnchors = true;
};

class AnchorObjectSystem : public MikanObjectSystem
{
public:
	AnchorObjectSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static AnchorObjectSystemPtr getSystem() { return s_anchorObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getAnchorSystemConfigConst();
	}
	AnchorObjectSystemConfigConstPtr getAnchorSystemConfigConst() const;
	AnchorObjectSystemConfigPtr getAnchorSystemConfig();

	const AnchorMap& getAnchorMap() const { return m_anchorComponents; }
	AnchorComponentPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const;
	AnchorComponentPtr getSpatialAnchorByName(const std::string& anchorName) const;
	bool getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const;
	AnchorComponentPtr addNewAnchor(MikanStageID ownerStageId);
	AnchorComponentPtr addNewAnchor(MikanStageID ownerStageId, const std::string& anchorName, const class GlmTransform& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

protected:
	AnchorComponentPtr createAnchorObject(AnchorDefinitionPtr anchorConfig);
	void disposeAnchorObject(MikanSpatialAnchorID anchorId);

	AnchorMap m_anchorComponents;

	static AnchorObjectSystemWeakPtr s_anchorObjectSystem;
};