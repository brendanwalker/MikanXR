#pragma once

#include "AnchorObjectSystemConfig.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"

#include <map>
#include <memory>

#include <glm/glm.hpp>

class MikanAnchorComponent;
using MikanAnchorComponentPtr = std::shared_ptr<MikanAnchorComponent>;
using MikanAnchorComponentWeakPtr = std::weak_ptr<MikanAnchorComponent>;

class AnchorObjectSystem : public MikanObjectSystem
{
public:
	AnchorObjectSystem();
	virtual ~AnchorObjectSystem();

	static AnchorObjectSystem* getSystem() { return s_anchorObjectSystem; }

	virtual void init() override;
	virtual void dispose() override;

	MikanAnchorComponentWeakPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const;
	MikanAnchorComponentWeakPtr getSpatialAnchorByName(const std::string& anchorName) const;
	MikanAnchorComponentPtr addNewAnchor(const std::string& anchorName, const glm::mat4& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

protected:
	const AnchorObjectSystemConfig& getAnchorConfigConst() const;
	AnchorObjectSystemConfig& getAnchorConfig();

	MikanAnchorComponentPtr createAnchorObject(const MikanSpatialAnchorInfo& anchorInfo);
	void disposeAnchorObject(MikanSpatialAnchorID anchorId);

	std::map<MikanSpatialAnchorID, MikanAnchorComponentWeakPtr> m_anchorComponents;

	static AnchorObjectSystem* s_anchorObjectSystem;
};