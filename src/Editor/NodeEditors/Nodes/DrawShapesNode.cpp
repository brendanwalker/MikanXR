#include "DrawShapesNode.h"
#include "IconsForkAwesome.h"
#include "CameraComponent.h"
#include "CompositorConstants.h"
#include "IMkGraphicsContext.h"
#include "IMkSceneRenderable.h"
#include "IMkStaticMeshInstance.h"
#include "IMkTexture.h"
#include "IMkMesh.h"
#include "LocText.h"
#include "Logger.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "IMkState.h"
#include "NodeEditorState.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "StringUtils.h"

#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "ShapeComponent.h"
#include "StaticMeshComponent.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "Pins/ArrayPin.h"
#include "Pins/FlowPin.h"
#include "Pins/NodePin.h"

#include "Properties/GraphShapeProperty.h"

#include "imgui.h"

#include <glm/glm.hpp>

// Semantic for MVP in material uniforms
#include "IMkShader.h"

// -- DrawShapesNodeConfig -----
configuru::Config DrawShapesNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["blend_mode"]= k_compositorBlendModeStrings[(int)blendMode];
	pt["depth_test"]= bDepthTest;

	return pt;
}

void DrawShapesNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string blendModeString=
		pt.get_or<std::string>("blend_mode", k_compositorBlendModeStrings[(int)eCompositorBlendMode::blendNormal]);
	blendMode= StringUtils::FindEnumValue<eCompositorBlendMode>(blendModeString, k_compositorBlendModeStrings);

	bDepthTest= pt.get_or<bool>("depth_test", false);
}

// -- DrawShapesNode -----
bool DrawShapesNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const DrawShapesNodeConfig>(nodeConfig);

		m_blendMode= config->blendMode;
		m_bDepthTest= config->bDepthTest;

		return true;
	}

	return false;
}

void DrawShapesNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<DrawShapesNodeConfig>(nodeConfig);

	config->blendMode= m_blendMode;
	config->bDepthTest= m_bDepthTest;

	Node::saveToConfig(nodeConfig);
}

void DrawShapesNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnGraphLoaded-= MakeDelegate(this, &DrawShapesNode::onGraphLoaded);
			m_ownerGraph= nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnGraphLoaded+= MakeDelegate(this, &DrawShapesNode::onGraphLoaded);
			m_ownerGraph= newOwnerGraph;
		}
	}
}

void DrawShapesNode::setShapesPin(ArrayPinPtr inPin) { m_shapesPin= inPin; }

void DrawShapesNode::onGraphLoaded(bool success)
{
	// Don't bother with setup if the graph load failed
	if (!success)
		return;

	// Optionally bind a stencil input pin
	ArrayPinPtr shapesInPin= getFirstPinOfType<ArrayPin>(eNodePinDirection::INPUT);
	if (shapesInPin && shapesInPin->getElementClassName() == GraphShapeProperty::k_propertyClassName)
	{
		setShapesPin(shapesInPin);
	}
}

void DrawShapesNode::onLinkConnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_shapesPin)
	{
		m_shapesPin->copyValueFromSourcePin();
	}
}

void DrawShapesNode::onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_shapesPin)
	{
		m_shapesPin->clearArray();
	}
}

bool DrawShapesNode::evaluateNode(NodeEvaluator& evaluator)
{
	if (!evaluateInputs(evaluator))
		return false;

	auto compositorGraph= std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	CameraComponentPtr cameraComponent= compositorGraph->getBoundCameraComponent();
	if (!cameraComponent)
		return false;

	glm::mat4 vpMatrix;
	if (!cameraComponent->getApertureViewProjectionMatrix(vpMatrix))
		return false;

	// Gather shape properties from the array pin
	std::vector<GraphShapePropertyPtr> shapeProperties;
	if (m_shapesPin)
	{
		for (auto& propPtr : m_shapesPin->getArray())
		{
			auto shapeProp= std::dynamic_pointer_cast<GraphShapeProperty>(propPtr);
			if (shapeProp && shapeProp->getShapeComponent())
				shapeProperties.push_back(shapeProp);
		}
	}

	if (shapeProperties.empty())
	{
		// No shapes — still evaluate output flow
		return true;
	}

	IMkGraphicsContext* graphicsContext= evaluator.getCurrentGraphicsContext();
	MkScopedState mkStateScope= graphicsContext->getMkStateStack().createScopedState("Draw Shapes Node");
	IMkState* mkState= mkStateScope.getStackState();

	// Set blend mode
	switch (m_blendMode)
	{
	case eCompositorBlendMode::blendOff:
		mkState->disableFlag(eMkStateFlagType::blend);
		break;
	case eCompositorBlendMode::blendNormal:
		mkState->enableFlag(eMkStateFlagType::blend);
		mkStateSetBlendFunc(mkState, eMkBlendFunction::SRC_ALPHA, eMkBlendFunction::ONE_MINUS_SRC_ALPHA);
		mkStateSetBlendEquation(mkState, eMkBlendEquation::ADD);
		break;
	case eCompositorBlendMode::blendMultiply:
		mkState->enableFlag(eMkStateFlagType::blend);
		mkStateSetBlendFunc(mkState, eMkBlendFunction::DST_COLOR, eMkBlendFunction::ZERO);
		mkStateSetBlendEquation(mkState, eMkBlendEquation::ADD);
		break;
	}

	// Depth test
	if (m_bDepthTest)
		mkState->enableFlag(eMkStateFlagType::depthTest);
	else
		mkState->disableFlag(eMkStateFlagType::depthTest);

	// Draw each shape
	for (auto& shapeProp : shapeProperties)
	{
		ShapeComponentPtr shape= shapeProp->getShapeComponent();
		if (!shape)
			continue;

		if (shape->hasValidShapeGraph())
		{
			// Delegate rendering to the shape's node graph
			shape->renderShapeGraph(vpMatrix, graphicsContext);
			continue;
		}

		IMkTexturePtr colorTexture= shape->getColorTexture();

		if (auto quadShape= std::dynamic_pointer_cast<QuadShapeComponent>(shape))
		{
			IMkSceneRenderableConstPtr renderable= quadShape->getGlSceneRenderableConst();
			if (renderable)
				drawShapeRenderable(renderable, vpMatrix, colorTexture);
		}
		else if (auto boxShape= std::dynamic_pointer_cast<BoxShapeComponent>(shape))
		{
			IMkSceneRenderableConstPtr renderable= boxShape->getGlSceneRenderableConst();
			if (renderable)
				drawShapeRenderable(renderable, vpMatrix, colorTexture);
		}
		else if (auto modelShape= std::dynamic_pointer_cast<ModelShapeComponent>(shape))
		{
			for (auto& meshComp : modelShape->getTriangulatedMeshes())
			{
				IMkSceneRenderableConstPtr renderable= meshComp->getGlSceneRenderableConst();
				if (renderable)
					drawShapeRenderable(renderable, vpMatrix, colorTexture);
			}
		}
	}

	return true;
}

void DrawShapesNode::drawShapeRenderable(IMkSceneRenderableConstPtr renderable, const glm::mat4& vpMatrix,
										 IMkTexturePtr colorTexture)
{
	MkMaterialInstancePtr matInst= renderable->getMaterialInstance();
	if (!matInst)
		return;

	MkMaterialConstPtr material= matInst->getMaterial();
	if (!material)
		return;

	// Compute MVP without storing it on the material instance.
	// Storing it via setMat4BySemantic would persist in mat4Sources, which runs
	// AFTER the callback in bindMaterialInstance and would overwrite the scene
	// camera MVP that MkScene::materialInstanceBindCallback correctly sets.
	const glm::mat4 mvpMatrix= vpMatrix * renderable->getModelMatrix();

	// Bind the color texture to the diffuse semantic
	if (colorTexture)
	{
		matInst->setTextureBySemantic(eUniformSemantic::diffuseTexture, colorTexture);
	}

	// Bind material and draw, injecting MVP transiently via callback so it is
	// never written into mat4Sources on the shared material instance.
	if (auto materialBinding= material->bindMaterial())
	{
		BindUniformCallback mvpCallback= [&mvpMatrix](IMkShaderPtr program, eUniformDataType dataType,
													  eUniformSemantic semantic,
													  const std::string& uniformName) -> eUniformBindResult
		{
			if (semantic == eUniformSemantic::modelViewProjectionMatrix)
			{
				return program->setMatrix4x4Uniform(uniformName, mvpMatrix) ? eUniformBindResult::bound
																			: eUniformBindResult::error;
			}
			return eUniformBindResult::unbound;
		};

		if (auto matInstBinding= matInst->bindMaterialInstance(materialBinding, mvpCallback))
		{
			auto staticMeshInst= std::dynamic_pointer_cast<const IMkStaticMeshInstance>(renderable);
			if (staticMeshInst)
			{
				IMkMeshConstPtr mesh= staticMeshInst->getMesh();
				if (mesh)
					mesh->drawElements();
			}
		}
	}
}

FlowPinPtr DrawShapesNode::getOutputFlowPin() const { return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT); }

void DrawShapesNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.drawShapesHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		// Blend Mode
		const std::string blendModeItems=
			std::string(locText("nodes.blendOff")) + '\0' + locText("nodes.blendOn") + '\0';
		int iBlendMode= (int)m_blendMode;
		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "drawShapesNodeBlendMode", locText("nodes.blendMode"),
											  blendModeItems.c_str(), iBlendMode))
		{
			m_blendMode= (eCompositorBlendMode)iBlendMode;
		}

		// Depth Test
		MkGui::drawCheckBoxProperty(propertyStyle, "drawShapesNodeDepthTest", locText("nodes.depthTest"), m_bDepthTest);
	}
}

// -- DrawShapesNodeFactory -----
NodePtr DrawShapesNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<DrawShapesNode>(NodeFactory::createNode(editorState));

	// Flow in
	FlowPinPtr flowInPin= node->addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);

	// Shapes array in
	ArrayPinPtr shapesInPin= node->addPin<ArrayPin>("shapes", eNodePinDirection::INPUT);
	shapesInPin->setElementClassName(GraphShapeProperty::k_propertyClassName);
	shapesInPin->setHasDefaultValue(true); // unconnected == no shapes
	node->m_shapesPin= shapesInPin;

	// Flow out
	FlowPinPtr flowOutPin= node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	autoConnectInputPin(editorState, flowInPin);
	autoConnectOutputPin(editorState, flowOutPin);
	autoConnectInputPin(editorState, shapesInPin);

	return node;
}

const char* DrawShapesNode::editorGetHeaderIcon() const { return ICON_FK_CUBES; }
