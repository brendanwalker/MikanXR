#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "ComponentProperty.h"
#include "ComponentFwd.h"
#include "ObjectFwd.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class SceneComponent;
using SceneComponentWeakPtr = std::weak_ptr<SceneComponent>;

class AnchorComponent : public MikanComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	void setSpatialAnchor(const MikanSpatialAnchorInfo& anchor);
	void setAnchorXform(const glm::mat4& xform);
	void setAnchorName(const std::string& newAnchorName);

	MikanSpatialAnchorID getAnchorId() const { return AnchorId; }
	const glm::mat4 getAnchorXform() const { return AnchorXform; }
	const std::string& getAnchorName() const { return AnchorName; }

protected:
	COMPONENT_PROPERTY(MikanSpatialAnchorID, AnchorId);
	COMPONENT_PROPERTY(glm::mat4, AnchorXform);
	COMPONENT_PROPERTY(std::string, AnchorName);

	SceneComponentWeakPtr m_sceneComponent;
	void updateSceneComponentTransform();
};