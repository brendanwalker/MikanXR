#include "WriteDepthNode.h"
#include "IconsForkAwesome.h"
#include "CameraComponent.h"
#include "IMkGraphicsContext.h"
#include "IMkShaderCache.h"
#include "IMkState.h"
#include "IMkTexture.h"
#include "IMkTriangulatedMesh.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkStateModifiers.h"
#include "MkStateStack.h"
#include "NodeEditorState.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "Pins/FlowPin.h"
#include "Pins/TexturePin.h"

#include <glm/glm.hpp>

// -- WriteDepthNode -----
TexturePinPtr WriteDepthNode::getDepthTexturePin() const
{
	return getFirstPinOfType<TexturePin>(eNodePinDirection::INPUT);
}

bool WriteDepthNode::evaluateNode(NodeEvaluator& evaluator)
{
	if (!evaluateInputs(evaluator))
		return false;

	TexturePinPtr depthTexturePin= getDepthTexturePin();
	IMkTexturePtr depthTexture= depthTexturePin ? depthTexturePin->getValue() : IMkTexturePtr();
	if (!depthTexture)
	{
		evaluator.addError(NodeEvaluationError(eNodeEvaluationErrorCode::missingInput, "No depth texture", this));
		return false;
	}

	// The same aperture projection DepthMaskNode bakes into its linear depth, so the near and far
	// planes the shader recovers from it match the ones the linear values were measured against.
	// Unflipped, because the flip only touches the y row and the shader reads the z row.
	auto compositorGraph= std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	CameraComponentPtr cameraComponent= compositorGraph->getBoundCameraComponent();
	glm::mat4 projectionMatrix;
	if (!cameraComponent || !cameraComponent->getApertureProjectionMatrix(projectionMatrix))
	{
		evaluator.addError(NodeEvaluationError(eNodeEvaluationErrorCode::evaluationError, "No bound camera", this));
		return false;
	}

	IMkGraphicsContext* graphicsContext= evaluator.getCurrentGraphicsContext();
	if (!m_materialInstance)
	{
		MkMaterialConstPtr material=
			graphicsContext->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_LINEAR_TO_HARDWARE_DEPTH);
		if (!material)
		{
			evaluator.addError(NodeEvaluationError(eNodeEvaluationErrorCode::evaluationError,
												   "Missing linear to hardware depth material", this));
			return false;
		}

		m_materialInstance= createMkMaterialInstance(material);
	}

	// Depth writes only happen with the depth test enabled. The working depth buffer is cleared to far
	// each frame, so the default LESS test lets every nearer source depth through and merges a second
	// write to the nearer of the two.
	MkScopedState mkStateScope= graphicsContext->getMkStateStack().createScopedState("Write Depth Node");
	IMkState* mkState= mkStateScope.getStackState();
	mkState->enableFlag(eMkStateFlagType::depthTest);
	mkState->disableFlag(eMkStateFlagType::blend);
	mkState->disableFlag(eMkStateFlagType::stencilTest);
	mkStateSetDepthMask(mkState, true);
	mkStateSetColorMask(mkState, glm::bvec4(false));

	MkMaterialConstPtr material= m_materialInstance->getMaterial();
	if (auto materialBinding= material->bindMaterial())
	{
		m_materialInstance->setTextureBySemantic(eUniformSemantic::depthTexture, depthTexture);
		m_materialInstance->setMat4BySemantic(eUniformSemantic::projectionMatrix, projectionMatrix);

		if (auto materialInstanceBinding= m_materialInstance->bindMaterialInstance(materialBinding))
		{
			compositorGraph->getLayerMesh()->drawElements();
		}
	}

	return true;
}

FlowPinPtr WriteDepthNode::getOutputFlowPin() const { return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT); }

const char* WriteDepthNode::editorGetHeaderIcon() const { return ICON_FK_ARROW_CIRCLE_DOWN; }

// -- WriteDepthNodeFactory -----
NodePtr WriteDepthNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<WriteDepthNode>(NodeFactory::createNode(editorState));

	// Flow in
	FlowPinPtr flowInPin= node->addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);

	// Linear depth texture in
	TexturePinPtr depthTextureInPin= node->addPin<TexturePin>("depthTexture", eNodePinDirection::INPUT);

	// Flow out
	FlowPinPtr flowOutPin= node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	autoConnectInputPin(editorState, flowInPin);
	autoConnectOutputPin(editorState, flowOutPin);
	autoConnectInputPin(editorState, depthTextureInPin);

	return node;
}
