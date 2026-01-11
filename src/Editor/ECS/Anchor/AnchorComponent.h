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

	static const std::string k_ownerStageIdPropertyId;
	inline MikanStageID getOwnerStageId() const { return m_stageId; }
	void setOwnerStageId(MikanStageID stageId);

private:
	MikanSpatialAnchorID m_anchorId;
	MikanStageID m_stageId;
};

class AnchorComponent : public TransformComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline static const std::string k_componentClassName = "AnchorComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline AnchorDefinitionPtr getAnchorDefinition() const
	{
		return std::static_pointer_cast<AnchorDefinition>(m_definition);
	}
	StageComponentConstPtr getOwnerStageComponent() const;

	void extractAnchorInfoForClientAPI(struct MikanSpatialAnchorInfo& outAnchorInfo) const;

	// -- IFunctionInterface ----
	static const std::string k_editAnchorFunctionId;
	static const std::string k_deleteAnchorFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void editAnchor();
	void deleteAnchor();

protected:
	SelectionComponentWeakPtr m_selectionComponent;
};