#pragma once



#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
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
	AnchorConfigPtr getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) const;
	AnchorConfigPtr getSpatialAnchorInfoByName(const std::string& anchorName) const;
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

	static AnchorObjectSystem* getSystem() { return s_anchorObjectSystem; }

	virtual void init() override;
	virtual void dispose() override;

	const AnchorMap& getAnchorMap() const { return m_anchorComponents; }
	AnchorComponentWeakPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const;
	AnchorComponentWeakPtr getSpatialAnchorByName(const std::string& anchorName) const;
	AnchorComponentPtr addNewAnchor(const std::string& anchorName, const glm::mat4& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

protected:
	AnchorObjectSystemConfigConstPtr getAnchorConfigConst() const;
	AnchorObjectSystemConfigPtr getAnchorConfig();

	AnchorComponentPtr createAnchorObject(const MikanSpatialAnchorInfo& anchorInfo);
	void disposeAnchorObject(MikanSpatialAnchorID anchorId);

	AnchorMap m_anchorComponents;

	static AnchorObjectSystem* s_anchorObjectSystem;
};