#include "MkMaterialInstance.h"
#include "MkScene.h"
#include "MkScopedState.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "IMkCamera.h"
#include "IMkSceneRenderable.h"
#include "IMkShader.h"
#include "IMkState.h"

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

using namespace std::placeholders;

struct MkDrawCall
{
	std::vector<IMkSceneRenderableConstWeakPtr> instances;
};
using MkDrawCallPtr = std::shared_ptr<MkDrawCall>;
using MkDrawCallConstPtr = std::shared_ptr<const MkDrawCall>;

struct MkSceneImpl
{
	glm::vec4 lightColor;
	glm::vec3 lightDirection;

	class std::map<MkMaterialConstPtr, MkDrawCallPtr> drawCalls;
};

MkScene::MkScene()
	: m_impl(new MkSceneImpl())
{
	m_impl->lightColor= glm::vec4(1.f);
	m_impl->lightDirection= glm::vec3(0.f, 0.f, -1.f);
}

MkScene::~MkScene()
{
	removeAllInstances();
	delete m_impl;
}

void MkScene::setLightColor(const glm::vec4& lightColor)
{
	m_impl->lightColor = lightColor; 
}

const glm::vec4&  MkScene::getLightColor() const
{ 
	return m_impl->lightColor; 
}

void MkScene::setLightDirection(const glm::vec3& lightDirection)
{
	m_impl->lightDirection = lightDirection; 
}

const glm::vec3& MkScene::getLightDirection() const 
{ 
	return m_impl->lightDirection; 
}

void MkScene::addInstance(IMkSceneRenderableConstPtr instance)
{
	MkMaterialConstPtr material= instance->getMaterialInstanceConst()->getMaterial();

	if (m_impl->drawCalls.find(material) == m_impl->drawCalls.end())
	{
		m_impl->drawCalls.insert({material, std::make_shared<MkDrawCall>()});
	}

	m_impl->drawCalls[material]->instances.push_back(instance);
}

void MkScene::removeInstance(IMkSceneRenderableConstPtr instance)
{
	MkMaterialConstPtr material = instance->getMaterialInstanceConst()->getMaterial();

	auto drawCallIter= m_impl->drawCalls.find(material);
	if (drawCallIter != m_impl->drawCalls.end())
	{
		MkDrawCallPtr drawCall = drawCallIter->second;

		// Remove any existing instances from the draw call
		for (auto it = drawCall->instances.begin(); it != drawCall->instances.end(); it++)
		{
			IMkSceneRenderableConstPtr existingInstance= it->lock();

			if (existingInstance == instance)
			{
				drawCall->instances.erase(it);
				break;
			}
		}

		if (drawCall->instances.size() == 0)
		{
			m_impl->drawCalls.erase(drawCallIter);
		}
	}
}

void MkScene::removeAllInstances()
{
	m_impl->drawCalls.clear();
}

void MkScene::render(IMkCameraConstPtr camera, MkStateStack& MkStateStack) const
{
	MkScopedState scopedState= MkStateStack.createScopedState("GlScene");
	IMkState* mkState= scopedState.getStackState();

	// Enable front face culling while drawing the scene
	mkState->enableFlag(eMkStateFlagType::cullFace);
	mkStateSetFrontFace(mkState, eMkFrontFaceMode::CCW);

	// Enable Depth Test while drawing the scene
	mkState->enableFlag(eMkStateFlagType::depthTest);

	// Clear the depth buffer before drawing the scene
	mkStateClearBuffer(mkState, eMkClearFlags::depth);

	for (auto drawCallIter= m_impl->drawCalls.begin(); drawCallIter != m_impl->drawCalls.end(); drawCallIter++)
	{
		MkMaterialConstPtr material = drawCallIter->first;
		MkDrawCallConstPtr drawCall = drawCallIter->second;

		bool bAnyInstancesVisible= false;
		for (auto instanceIter = drawCall->instances.begin();
			 instanceIter != drawCall->instances.end();
			 instanceIter++)
		{
			IMkSceneRenderableConstPtr renderableInstance = instanceIter->lock();

			if (renderableInstance != nullptr && 
				renderableInstance->getVisible() && 
				renderableInstance->canCameraSee(camera))
			{
				bAnyInstancesVisible= true;
				break;
			}
		}

		if (bAnyInstancesVisible)
		{
			// Bind material program.
			// Unbound when materialBinding goes out of scope.
			auto materialBinding = 
				material->bindMaterial(
					// Scene specific material binding callback for camera and scene parameters
					[this, camera](
						IMkShaderPtr program,
						eUniformDataType uniformDataType,
						eUniformSemantic uniformSemantic,
						const std::string& uniformName)
					{
						return materialBindCallback(
							camera, 
							program, uniformDataType, uniformSemantic, uniformName);
					});
			if (materialBinding)
			{
				for (auto instanceIter = drawCall->instances.begin();
					 instanceIter != drawCall->instances.end();
					 instanceIter++)
				{
					IMkSceneRenderableConstPtr renderableInstance = instanceIter->lock();

					if (renderableInstance != nullptr && 
						renderableInstance->getVisible() &&
						renderableInstance->canCameraSee(camera))
					{
						MkMaterialInstanceConstPtr materialInstance = renderableInstance->getMaterialInstanceConst();

						// Bind material instance parameters 
						// Unbound when materialInstanceBinding goes out of scope.
						auto materialInstanceBinding =
							materialInstance->bindMaterialInstance(
								materialBinding,
								[this, camera, renderableInstance](
									IMkShaderPtr program,
									eUniformDataType uniformDataType,
									eUniformSemantic uniformSemantic,
									const std::string& uniformName) 
								{
									return materialInstanceBindCallback(
										camera, renderableInstance, 
										program, uniformDataType, uniformSemantic, uniformName);
								});

						if (materialInstanceBinding)
						{
							// Draw the renderable
							renderableInstance->render();
						}
					}
				}
			}
		}
	}
}

eUniformBindResult MkScene::materialBindCallback(
	IMkCameraConstPtr camera,
	IMkShaderPtr program,
	eUniformDataType uniformDataType,
	eUniformSemantic uniformSemantic,
	const std::string& uniformName) const
{
	eUniformBindResult bindResult= eUniformBindResult::unbound;

	switch (uniformSemantic)
	{
	case eUniformSemantic::lightColorRGB:
		{
			bindResult =
				program->setVector3Uniform(uniformName, getLightColor())
				? eUniformBindResult::bound
				: eUniformBindResult::error;
		} break;
	case eUniformSemantic::lightDirection:
		{
			bindResult =
				program->setVector3Uniform(uniformName, getLightDirection())
				? eUniformBindResult::bound
				: eUniformBindResult::error;
		} break;
	case eUniformSemantic::viewMatrix:
		{
			if (camera != nullptr)
			{
				bindResult =
					program->setMatrix4x4Uniform(uniformName, camera->getViewMatrix())
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			else
			{
				bindResult = eUniformBindResult::error;
			}
		} break;
	case eUniformSemantic::projectionMatrix:
		{
			if (camera != nullptr)
			{
				bindResult =
					program->setMatrix4x4Uniform(uniformName, camera->getProjectionMatrix())
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			else
			{
				bindResult = eUniformBindResult::error;
			}
		} break;
	}

	return bindResult;
}

eUniformBindResult MkScene::materialInstanceBindCallback(
	IMkCameraConstPtr camera,
	IMkSceneRenderableConstPtr renderableInstance,
	IMkShaderPtr program,
	eUniformDataType uniformDataType,
	eUniformSemantic uniformSemantic,
	const std::string& uniformName) const
{
	eUniformBindResult bindResult= eUniformBindResult::unbound;

	switch (uniformSemantic)
	{
		case eUniformSemantic::modelMatrix:
			{
				const glm::mat4 modelMat = renderableInstance->getModelMatrix();

				bindResult= 
					program->setMatrix4x4Uniform(uniformName, modelMat)
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			break;
		case eUniformSemantic::normalMatrix:
			{
				const glm::mat4 normalMat = renderableInstance->getNormalMatrix();

				bindResult= 
					program->setMatrix4x4Uniform(uniformName, normalMat)
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			break;
		case eUniformSemantic::modelViewProjectionMatrix:
			{
				if (camera != nullptr)
				{
					const glm::mat4 viewProjMat = camera->getViewProjectionMatrix();
					const glm::mat4 modelMat = renderableInstance->getModelMatrix();
					const glm::mat4 modelViewProjMatrix = viewProjMat * modelMat;

					bindResult= 
						program->setMatrix4x4Uniform(uniformName, modelViewProjMatrix)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
				}
				else
				{
					bindResult = eUniformBindResult::error;
				}
			}
			break;
	}

	return bindResult;
}