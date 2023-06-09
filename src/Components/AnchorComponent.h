#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "SceneComponent.h"
#include "MikanClientTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class AnchorDefinition : public SceneComponentDefinition
{
public:
	AnchorDefinition();
	AnchorDefinition(
		MikanSpatialAnchorID anchorId,
		const std::string& anchorName,
		const MikanTransform& xform);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanSpatialAnchorID getAnchorId() const { return m_anchorId; }
	MikanSpatialAnchorInfo getAnchorInfo() const;

private:
	MikanSpatialAnchorID m_anchorId;
};

class AnchorComponent : public SceneComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline AnchorDefinitionPtr getAnchorDefinition() const
	{
		return std::static_pointer_cast<AnchorDefinition>(m_definition);
	}

	// -- IFunctionInterface ----
	static const std::string k_editAnchorFunctionId;
	static const std::string k_deleteAnchorFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void editAnchor();
	void deleteAnchor();

protected:
	SelectionComponentWeakPtr m_selectionComponent;
};