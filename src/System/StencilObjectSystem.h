#pragma once
#include "MikanObjectSystem.h"
#include "MikanClientTypes.h"
#include "StencilObjectSystemConfig.h"

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

class QuadStencilComponent;
using QuadStencilComponentPtr = std::shared_ptr<QuadStencilComponent>;

class BoxStencilComponent;
using BoxStencilComponentPtr = std::shared_ptr<BoxStencilComponent>;

class ModelStencilComponent;
using ModelStencilComponentPtr = std::shared_ptr<ModelStencilComponent>;

class StencilObjectSystem : public MikanObjectSystem
{
public:
	StencilObjectSystem();

	virtual void init() override;

#if 0
	bool canAddStencil() const;
	bool removeStencil(MikanStencilID stencilId);
	eStencilType getStencilType(MikanStencilID stencilId) const;
	bool getStencilName(MikanStencilID stencilId, std::string& outStencilName) const;
	bool getStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	bool setStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanSpatialAnchorID getStencilParentAnchorId(MikanStencilID stencilId) const;

	bool getQuadStencilInfo(MikanStencilID stencilId, MikanStencilQuad& outInfo) const;
	bool getQuadStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	glm::mat4 getQuadStencilWorldTransform(const MikanStencilQuad* stencil) const;
	bool setQuadStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setQuadStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);
	bool updateQuadStencil(const MikanStencilQuad& info);

	bool getBoxStencilInfo(MikanStencilID stencilId, MikanStencilBox& outInfo) const;
	bool getBoxStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	glm::mat4 getBoxStencilWorldTransform(const MikanStencilBox* stencil) const;
	bool setBoxStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setBoxStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);
	bool updateBoxStencil(const MikanStencilBox& info);

	const MikanStencilModelConfig* getModelStencilConfig(MikanStencilID stencilId) const;
	bool getModelStencilInfo(MikanStencilID stencilId, MikanStencilModel& outInfo) const;
	bool getModelStencilWorldTransform(MikanStencilID stencilId, glm::mat4& outXform) const;
	glm::mat4 getModelStencilWorldTransform(const MikanStencilModel* stencil) const;
	bool setModelStencilLocalTransform(MikanStencilID stencilId, const glm::mat4& xform);
	bool setModelStencilWorldTransform(MikanStencilID stencilId, const glm::mat4& xform);
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);
	bool updateModelStencil(const MikanStencilModel& info);
	bool updateModelStencilFilename(MikanStencilID stencilID, const std::filesystem::path& filename);
#endif

protected:
	std::map<MikanStencilID, QuadStencilComponentPtr> QuadStencilComponents;
	std::map<MikanStencilID, BoxStencilComponentPtr> BoxStencilComponents;
	std::map<MikanStencilID, ModelStencilComponentPtr> ModelStencilComponents;
};