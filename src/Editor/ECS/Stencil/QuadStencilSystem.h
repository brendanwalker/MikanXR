#pragma once

#include "ComponentFwd.h"
#include "MikanStencilTypes.h"
#include "MikanTypedObjectSystem.h"
#include "ObjectSystemConfigFwd.h"
#include "QuadStencilComponent.h"

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class QuadStencilSystemDefinition :
	public MikanTypedObjectSystemDefinition<QuadStencilComponent, QuadStencilDefinition, MikanStencilID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<QuadStencilComponent, QuadStencilDefinition, MikanStencilID>;

	QuadStencilSystemDefinition(const std::string& configName = "QuadStencilSystemDefinition");

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_renderStencilsPropertyId;
	inline bool getRenderStencilsFlag() const { return m_bDebugRenderStencils; }
	void setRenderStencilsFlag(bool flag);

protected:
	bool m_bDebugRenderStencils = true;
};

class QuadStencilSystem :
	public MikanTypedObjectSystem<
		QuadStencilComponent, QuadStencilDefinition,
		MikanStencilID,
		QuadStencilSystem, QuadStencilSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		QuadStencilComponent, QuadStencilDefinition,
		MikanStencilID,
		QuadStencilSystem, QuadStencilSystemDefinition>;

	QuadStencilSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "QuadStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline QuadStencilComponentPtr getQuadStencilById(MikanStencilID stencilId) const {
		return Super::getTypedComponentById(stencilId);
	}
	inline QuadStencilComponentPtr getQuadStencilByName(const std::string& stencilName) const {
		return Super::getTypedComponentByName(stencilName);
	}

	// Helper methods for compatibility
	QuadStencilComponentPtr addNewQuadStencil(const MikanStencilQuadInfo& stencilInfo);
	void getRelevantQuadStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<QuadStencilComponentPtr>& outStencilList) const;

protected:
	static bool isStencilFacingCamera(
		StencilComponentConstPtr stencil,
		const glm::vec3& cameraPosition, const glm::vec3& cameraForward);

	virtual void additionalComponentFactory(
		MikanObjectPtr ownerComponentObject,
		ComponentDefinitionPtr componentDefinition) override;
};
