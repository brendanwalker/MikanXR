#pragma once



#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanSpatialAnchorTypes.h"
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

class AnchorObjectSystemConfig : public CommonConfig
{
public:
	AnchorObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddAnchor() const;
	AnchorDefinitionPtr getSpatialAnchorConfig(MikanSpatialAnchorID anchorId) const;
	AnchorDefinitionPtr getSpatialAnchorConfigByName(const std::string& anchorName) const;
	MikanSpatialAnchorID addNewAnchor(MikanSpatialAnchorInfo& anchorInfo);
	MikanSpatialAnchorID addNewAnchor(const std::string& anchorName, const MikanTransform& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

	static const std::string k_anchorVRDevicePathPropertyId;
	std::string anchorVRDevicePath;

	static const std::string k_anchorListPropertyId;
	std::vector<AnchorDefinitionPtr> spatialAnchorList;

	static const std::string k_renderAnchorsPropertyId;
	inline bool getRenderAnchorsFlag() const { return m_bDebugRenderAnchors; }
	void setRenderAnchorsFlag(bool flag);

	MikanSpatialAnchorID nextAnchorId= 0;
	MikanSpatialAnchorID originAnchorId= INVALID_MIKAN_ID;

protected:
	bool m_bDebugRenderAnchors = true;
};

class AnchorObjectSystem : public MikanObjectSystem
{
public:
	static AnchorObjectSystemPtr getSystem() { return s_anchorObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	AnchorObjectSystemConfigConstPtr getAnchorSystemConfigConst() const;
	AnchorObjectSystemConfigPtr getAnchorSystemConfig();

	const AnchorMap& getAnchorMap() const { return m_anchorComponents; }
	AnchorComponentPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const;
	AnchorComponentPtr getSpatialAnchorByName(const std::string& anchorName) const;
	AnchorComponentPtr getOriginSpatialAnchor() const { return m_originAnchor; }
	bool getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const;
	AnchorComponentPtr addNewAnchor(const std::string& anchorName, const GlmTransform& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

protected:
	AnchorComponentPtr createAnchorObject(AnchorDefinitionPtr anchorConfig);
	void disposeAnchorObject(MikanSpatialAnchorID anchorId);

	AnchorMap m_anchorComponents;
	AnchorComponentPtr m_originAnchor;

	static AnchorObjectSystemWeakPtr s_anchorObjectSystem;
};