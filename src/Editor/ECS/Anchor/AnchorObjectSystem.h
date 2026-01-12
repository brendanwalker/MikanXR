#pragma once

#include "AnchorComponent.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MikanTypedObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GlmTransform;

class AnchorObjectSystemDefinition :
	public MikanTypedObjectSystemDefinition<AnchorComponent, AnchorDefinition, MikanSpatialAnchorID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<AnchorComponent, AnchorDefinition, MikanSpatialAnchorID>;

	AnchorObjectSystemDefinition(const std::string& configName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_renderAnchorsPropertyId;
	inline bool getRenderAnchorsFlag() const { return m_bDebugRenderAnchors; }
	void setRenderAnchorsFlag(bool flag);

protected:
	bool m_bDebugRenderAnchors = true;
};

class AnchorObjectSystem :
	public MikanTypedObjectSystem<
		AnchorComponent, AnchorDefinition,
		MikanSpatialAnchorID,
		AnchorObjectSystem, AnchorObjectSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		AnchorComponent, AnchorDefinition,
		MikanSpatialAnchorID,
		AnchorObjectSystem, AnchorObjectSystemDefinition>;

	AnchorObjectSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "AnchorObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline AnchorComponentPtr getSpatialAnchorById(MikanSpatialAnchorID anchorId) const {
		return Super::getTypedComponentById(anchorId);
	}
	inline AnchorComponentPtr getSpatialAnchorByName(const std::string& anchorName) const {
		return Super::getTypedComponentByName(anchorName);
	}
	bool getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	virtual void additionalComponentFactory(
		MikanObjectPtr ownerComponentObject,
		ComponentDefinitionPtr componentDefinition) override;
};