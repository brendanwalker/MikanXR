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
using QuadStencilComponentWeakPtr = std::weak_ptr<QuadStencilComponent>;

class BoxStencilComponent;
using BoxStencilComponentPtr = std::shared_ptr<BoxStencilComponent>;
using BoxStencilComponentWeakPtr = std::weak_ptr<BoxStencilComponent>;

class ModelStencilComponent;
using ModelStencilComponentPtr = std::shared_ptr<ModelStencilComponent>;
using ModelStencilComponentWeakPtr = std::weak_ptr<ModelStencilComponent>;

class StencilObjectSystem : public MikanObjectSystem
{
public:
	StencilObjectSystem();
	virtual ~StencilObjectSystem();

	static StencilObjectSystem* getSystem() { return s_stencilObjectSystem; }

	virtual void init() override;
	virtual void dispose() override;

	QuadStencilComponentWeakPtr getQuadStencilById(MikanStencilID stencilId) const;
	QuadStencilComponentWeakPtr getQuadStencilByName(const std::string& stencilName) const;
	QuadStencilComponentPtr addNewQuadStencil(const MikanStencilQuad& stencilInfo);
	bool removeQuadStencil(MikanStencilID stencilId);

protected:
	QuadStencilComponentPtr createQuadStencilObject(const MikanStencilQuad& stencilInfo);
	void disposeQuadStencilObject(MikanStencilID stencilId);

	BoxStencilComponentPtr createBoxStencilObject(const MikanStencilBox& stencilInfo);
	void disposeBoxStencilObject(MikanStencilID stencilId);

	ModelStencilComponentPtr createModelStencilObject(const MikanStencilModel& stencilInfo);
	void disposeModelStencilObject(MikanStencilID stencilId);

	const StencilObjectSystemConfig& getStencilConfigConst() const;
	StencilObjectSystemConfig& getStencilConfig();

	std::map<MikanStencilID, QuadStencilComponentWeakPtr> m_quadStencilComponents;
	std::map<MikanStencilID, BoxStencilComponentWeakPtr> m_boxStencilComponents;
	std::map<MikanStencilID, ModelStencilComponentWeakPtr> m_modelStencilComponents;

	static StencilObjectSystem* s_stencilObjectSystem;
};