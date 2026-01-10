#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanStencilTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

using QuadStencilMap = std::map<MikanStencilID, QuadStencilComponentWeakPtr>;

class QuadStencilSystemDefinition : public MikanObjectSystemDefinition
{
public:
	QuadStencilSystemDefinition(const std::string& configName = "QuadStencilSystemDefinition")
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_quadStencilListPropertyId;
	const std::vector<QuadStencilDefinitionPtr>& getQuadStencilList() const { return m_quadStencilList; }
	QuadStencilDefinitionConstPtr getQuadStencilConfigConst(MikanStencilID stencilId) const;
	QuadStencilDefinitionPtr getQuadStencilConfig(MikanStencilID stencilId);
	QuadStencilDefinitionPtr allocateQuadStencilDefinition(const struct MikanStencilQuadInfo& quadInfo);
	bool addQuadStencilDefinition(QuadStencilDefinitionPtr quadDefinitionPtr);
	bool removeQuadStencilDefinition(MikanStencilID stencilId);

	static const std::string k_renderStencilsPropertyId;
	inline bool getRenderStencilsFlag() const { return m_bDebugRenderStencils; }
	void setRenderStencilsFlag(bool flag);

protected:
	std::vector<QuadStencilDefinitionPtr> m_quadStencilList;
	MikanStencilID m_nextStencilId = 0;
	bool m_bDebugRenderStencils = true;
};

class QuadStencilSystem : public MikanObjectSystem
{
public:
	QuadStencilSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "QuadStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	QuadStencilSystemDefinitionConstPtr getQuadStencilSystemDefinitionConst() const;
	QuadStencilSystemDefinitionPtr getQuadStencilSystemDefinition();

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;
	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const QuadStencilMap& getQuadStencilMap() const { return m_quadStencilComponents; }
	QuadStencilComponentPtr getQuadStencilById(MikanStencilID stencilId) const;
	QuadStencilComponentPtr getQuadStencilByName(const std::string& stencilName) const;
	QuadStencilComponentPtr addNewQuadStencil(const MikanStencilQuadInfo& stencilInfo);
	bool removeQuadStencil(MikanStencilID stencilId);
	void getRelevantQuadStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<QuadStencilComponentPtr>& outStencilList) const;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	static bool isStencilFacingCamera(
		StencilComponentConstPtr stencil,
		const glm::vec3& cameraPosition, const glm::vec3& cameraForward);

	QuadStencilComponentPtr createQuadStencilObject(QuadStencilDefinitionPtr quadConfig);
	bool disposeQuadStencilObject(MikanStencilID stencilId);

	QuadStencilMap m_quadStencilComponents;
};
