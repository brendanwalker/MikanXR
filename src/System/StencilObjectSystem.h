#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "StencilObjectSystemConfig.h"

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

using QuadStencilMap = std::map<MikanStencilID, QuadStencilComponentWeakPtr>;
using BoxStencilMap = std::map<MikanStencilID, BoxStencilComponentWeakPtr>;
using ModelStencilMap = std::map<MikanStencilID, ModelStencilComponentWeakPtr>;

class StencilObjectSystem : public MikanObjectSystem
{
public:
	StencilObjectSystem();
	virtual ~StencilObjectSystem();

	static StencilObjectSystemPtr getSystem() { return s_stencilObjectSystem.lock(); }

	virtual void init() override;
	virtual void dispose() override;

	StencilObjectSystemConfigConstPtr getStencilSystemConfigConst() const;
	StencilObjectSystemConfigPtr getStencilSystemConfig();

	StencilComponentPtr getStencilById(MikanStencilID stencilId) const;
	eStencilType getStencilType(MikanStencilID stencilId) const;
	bool getStencilWorldTransform(MikanStencilID parentStencilId, glm::mat4& outXform) const;

	const QuadStencilMap& getQuadStencilMap() const { return m_quadStencilComponents; }
	QuadStencilComponentPtr getQuadStencilById(MikanStencilID stencilId) const;
	QuadStencilComponentPtr getQuadStencilByName(const std::string& stencilName) const;
	QuadStencilComponentPtr addNewQuadStencil(const MikanStencilQuad& stencilInfo);
	bool removeQuadStencil(MikanStencilID stencilId);
	void getRelevantQuadStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<QuadStencilComponentPtr>& outStencilList) const;

	const BoxStencilMap& getBoxStencilMap() const { return m_boxStencilComponents; }
	BoxStencilComponentPtr getBoxStencilById(MikanStencilID stencilId) const;
	BoxStencilComponentPtr getBoxStencilByName(const std::string& stencilName) const;
	BoxStencilComponentPtr addNewBoxStencil(const MikanStencilBox& stencilInfo);
	bool removeBoxStencil(MikanStencilID stencilId);
	void getRelevantBoxStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<BoxStencilComponentPtr>& outStencilList) const;

	const ModelStencilMap& getModelStencilMap() const { return m_modelStencilComponents; }
	ModelStencilComponentPtr getModelStencilById(MikanStencilID stencilId) const;
	ModelStencilComponentPtr getModelStencilByName(const std::string& stencilName) const;
	ModelStencilComponentPtr addNewModelStencil(const MikanStencilModel& stencilInfo);
	bool removeModelStencil(MikanStencilID stencilId);
	void getRelevantModelStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		std::vector<ModelStencilComponentPtr>& outStencilList) const;

protected:
	QuadStencilComponentPtr createQuadStencilObject(QuadStencilConfigPtr quadConfig);
	void disposeQuadStencilObject(MikanStencilID stencilId);

	BoxStencilComponentPtr createBoxStencilObject(BoxStencilConfigPtr boxConfig);
	void disposeBoxStencilObject(MikanStencilID stencilId);

	ModelStencilComponentPtr createModelStencilObject(ModelStencilConfigPtr modelConfig);
	void disposeModelStencilObject(MikanStencilID stencilId);

	QuadStencilMap m_quadStencilComponents;
	BoxStencilMap m_boxStencilComponents;
	ModelStencilMap m_modelStencilComponents;

	static StencilObjectSystemWeakPtr s_stencilObjectSystem;
};