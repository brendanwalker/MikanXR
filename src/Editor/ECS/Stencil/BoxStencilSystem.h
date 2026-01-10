#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanStencilTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

using BoxStencilMap = std::map<MikanStencilID, BoxStencilComponentWeakPtr>;

class BoxStencilSystemDefinition : public MikanObjectSystemDefinition
{
public:
	BoxStencilSystemDefinition(const std::string& configName = "BoxStencilSystemDefinition")
		: MikanObjectSystemDefinition(configName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_boxStencilListPropertyId;
	const std::vector<BoxStencilDefinitionPtr>& getBoxStencilList() const { return m_boxStencilList; }
	BoxStencilDefinitionConstPtr getBoxStencilConfigConst(MikanStencilID stencilId) const;
	BoxStencilDefinitionPtr getBoxStencilConfig(MikanStencilID stencilId);
	BoxStencilDefinitionPtr allocateBoxStencilDefinition(const struct MikanStencilBoxInfo& boxInfo);
	bool addBoxStencilDefinition(BoxStencilDefinitionPtr boxDefinitionPtr);
	bool removeBoxStencilDefinition(MikanStencilID stencilId);

	static const std::string k_renderStencilsPropertyId;
	inline bool getRenderStencilsFlag() const { return m_bDebugRenderStencils; }
	void setRenderStencilsFlag(bool flag);

protected:
	std::vector<BoxStencilDefinitionPtr> m_boxStencilList;
	MikanStencilID m_nextStencilId = 0;
	bool m_bDebugRenderStencils = true;
};

class BoxStencilSystem : public MikanObjectSystem
{
public:
	BoxStencilSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "BoxStencilSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	BoxStencilSystemDefinitionConstPtr getBoxStencilSystemDefinitionConst() const;
	BoxStencilSystemDefinitionPtr getBoxStencilSystemDefinition();

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;
	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const BoxStencilMap& getBoxStencilMap() const { return m_boxStencilComponents; }
	BoxStencilComponentPtr getBoxStencilById(MikanStencilID stencilId) const;
	BoxStencilComponentPtr getBoxStencilByName(const std::string& stencilName) const;
	BoxStencilComponentPtr addNewBoxStencil(const MikanStencilBoxInfo& stencilInfo);
	bool removeBoxStencil(MikanStencilID stencilId);
	void getRelevantBoxStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<BoxStencilComponentPtr>& outStencilList) const;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	static bool isStencilFacingCamera(
		StencilComponentConstPtr stencil,
		const glm::vec3& cameraPosition, const glm::vec3& cameraForward);

	BoxStencilComponentPtr createBoxStencilObject(BoxStencilDefinitionPtr boxConfig);
	bool disposeBoxStencilObject(MikanStencilID stencilId);

	BoxStencilMap m_boxStencilComponents;
};
