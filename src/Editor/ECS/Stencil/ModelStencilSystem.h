#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanStencilTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

using ModelStencilMap = std::map<MikanStencilID, ModelStencilComponentWeakPtr>;

class ModelStencilSystemDefinition : public MikanObjectSystemDefinition
{
public:
	ModelStencilSystemDefinition(const std::string& configName = "ModelStencilSystemDefinition")
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_modelStencilListPropertyId;
	const std::vector<ModelStencilDefinitionPtr>& getModelStencilList() const { return m_modelStencilList; }
	ModelStencilDefinitionConstPtr getModelStencilConfigConst(MikanStencilID stencilId) const;
	ModelStencilDefinitionPtr getModelStencilConfig(MikanStencilID stencilId);
	ModelStencilDefinitionPtr allocateModelStencilDefinition(const struct MikanStencilModelInfo& modelInfo);
	bool addModelStencilDefinition(ModelStencilDefinitionPtr modelDefinitionPtr);
	bool removeModelStencilDefinition(MikanStencilID stencilId);

	static const std::string k_renderStencilsPropertyId;
	inline bool getRenderStencilsFlag() const { return m_bDebugRenderStencils; }
	void setRenderStencilsFlag(bool flag);

protected:
	std::vector<ModelStencilDefinitionPtr> m_modelStencilList;
	MikanStencilID m_nextStencilId = 0;
	bool m_bDebugRenderStencils = true;
};

class ModelStencilSystem : public MikanObjectSystem
{
public:
	ModelStencilSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "ModelStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	ModelStencilSystemDefinitionConstPtr getModelStencilSystemDefinitionConst() const;
	ModelStencilSystemDefinitionPtr getModelStencilSystemDefinition();

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;
	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const ModelStencilMap& getModelStencilMap() const { return m_modelStencilComponents; }
	ModelStencilComponentPtr getModelStencilById(MikanStencilID stencilId) const;
	ModelStencilComponentPtr getModelStencilByName(const std::string& stencilName) const;
	ModelStencilComponentPtr addNewModelStencil(const MikanStencilModelInfo& stencilInfo);
	bool removeModelStencil(MikanStencilID stencilId);
	void getRelevantModelStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<ModelStencilComponentPtr>& outStencilList) const;
	void setRenderStencilsFlag(bool flag);

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	static bool isStencilFacingCamera(
		StencilComponentConstPtr stencil,
		const glm::vec3& cameraPosition, const glm::vec3& cameraForward);

	ModelStencilComponentPtr createModelStencilObject(ModelStencilDefinitionPtr modelConfig);
	bool disposeModelStencilObject(MikanStencilID stencilId);

	ModelStencilMap m_modelStencilComponents;
};
