#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "TransformComponent.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class AnchorDefinition : public TransformComponentDefinition
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

private:
	MikanSpatialAnchorID m_anchorId;
};

class AnchorComponent : public TransformComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline AnchorDefinitionPtr getAnchorDefinition() const
	{
		return std::static_pointer_cast<AnchorDefinition>(m_definition);
	}

	void extractAnchorInfoForClientAPI(struct MikanSpatialAnchorInfo& outAnchorInfo) const;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);

	// -- IRmlFunctionInterface ----
	static const std::string k_editAnchorFunctionId;
	static const std::string k_deleteAnchorFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

	void editAnchor();
	void deleteAnchor();

protected:
	SelectionComponentWeakPtr m_selectionComponent;
};