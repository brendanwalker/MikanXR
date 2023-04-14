#pragma once

#include "AnchorObjectSystemConfig.h"
#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"

#include <map>
#include <memory>

#include <glm/glm.hpp>

class AnchorObjectSystem : public MikanObjectSystem
{
public:
	AnchorObjectSystem();
	virtual ~AnchorObjectSystem();

	static AnchorObjectSystem* getSystem() { return s_anchorObjectSystem; }

	virtual void init() override;
	virtual void dispose() override;

	AnchorComponentWeakPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const;
	AnchorComponentWeakPtr getSpatialAnchorByName(const std::string& anchorName) const;
	AnchorComponentPtr addNewAnchor(const std::string& anchorName, const glm::mat4& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

protected:
	const AnchorObjectSystemConfig& getAnchorConfigConst() const;
	AnchorObjectSystemConfig& getAnchorConfig();

	AnchorComponentPtr createAnchorObject(const MikanSpatialAnchorInfo& anchorInfo);
	void disposeAnchorObject(MikanSpatialAnchorID anchorId);

	std::map<MikanSpatialAnchorID, AnchorComponentWeakPtr> m_anchorComponents;

	static AnchorObjectSystem* s_anchorObjectSystem;
};