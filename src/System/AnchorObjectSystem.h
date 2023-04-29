#pragma once



#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

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
	AnchorConfigPtr getSpatialAnchorConfig(MikanSpatialAnchorID anchorId) const;
	AnchorConfigPtr getSpatialAnchorConfigByName(const std::string& anchorName) const;
	MikanSpatialAnchorID addNewAnchor(const std::string& anchorName, const MikanMatrix4f& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

	std::string anchorVRDevicePath;
	std::vector<AnchorConfigPtr> spatialAnchorList;
	MikanSpatialAnchorID nextAnchorId;
	MikanSpatialAnchorID originAnchorId;
	bool debugRenderAnchors;
};


class AnchorObjectSystem : public MikanObjectSystem
{
public:
	AnchorObjectSystem();
	virtual ~AnchorObjectSystem();

	static AnchorObjectSystemPtr getSystem() { return s_anchorObjectSystem.lock(); }

	virtual void init() override;
	virtual void dispose() override;

	AnchorObjectSystemConfigConstPtr getAnchorSystemConfigConst() const;
	AnchorObjectSystemConfigPtr getAnchorSystemConfig();

	const AnchorMap& getAnchorMap() const { return m_anchorComponents; }
	AnchorComponentPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const;
	AnchorComponentPtr getSpatialAnchorByName(const std::string& anchorName) const;
	AnchorComponentPtr getOriginSpatialAnchor() const { return m_originAnchor; }
	bool getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const;
	AnchorComponentPtr addNewAnchor(const std::string& anchorName, const glm::mat4& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

protected:
	AnchorComponentPtr createAnchorObject(AnchorConfigPtr anchorConfig);
	void disposeAnchorObject(MikanSpatialAnchorID anchorId);

	AnchorMap m_anchorComponents;
	AnchorComponentPtr m_originAnchor;

	static AnchorObjectSystemWeakPtr s_anchorObjectSystem;
};