#include "App.h"
#include "GlCamera.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "GlScene.h"
#include "GlShaderCache.h"
#include "Renderer.h"

#include <algorithm>

GlScene::GlScene()
{
}

GlScene::~GlScene()
{
	// Clean up all draw calls
	for (auto it = m_drawCalls.begin(); it != m_drawCalls.end(); it++)
	{
		delete it->second;
	}
}

void GlScene::addInstance(IGlSceneRenderableConstPtr instance)
{
	GlMaterialConstPtr material= instance->getMaterialInstanceConst()->getMaterial();

	if (m_drawCalls.find(material) == m_drawCalls.end())
	{
		m_drawCalls.insert({material, new GlDrawCall});
	}

	m_drawCalls[material]->instances.push_back(instance);
}

void GlScene::removeInstance(IGlSceneRenderableConstPtr instance)
{
	GlMaterialConstPtr material = instance->getMaterialInstanceConst()->getMaterial();

	auto drawCallIter= m_drawCalls.find(material);
	if (drawCallIter != m_drawCalls.end())
	{
		GlDrawCall* drawCall = drawCallIter->second;
		auto& instances= drawCall->instances;

		instances.erase(
			std::remove(instances.begin(), instances.end(), instance),
			instances.end());

		if (instances.size() == 0)
		{
			m_drawCalls.erase(drawCallIter);
			delete drawCall;
		}
	}
}

void GlScene::render() const
{
	GlCamera *camera= App::getInstance()->getRenderer()->getCurrentCamera();
	if (camera == nullptr)
		return;

	glm::mat4 VPMatrix = camera->getViewProjectionMatrix();

	for (auto drawCallIter= m_drawCalls.begin(); drawCallIter != m_drawCalls.end(); drawCallIter++)
	{
		GlMaterialConstPtr material = drawCallIter->first;
		const GlDrawCall* drawCall = drawCallIter->second;

		// Bind material program (unbound when materialBinding goes out of scope)
		auto materialBinding= material->bindMaterial();
		if (materialBinding)
		{			
			for(auto instanceIter= drawCall->instances.begin(); 
				instanceIter != drawCall->instances.end(); 
				instanceIter++)
			{
				IGlSceneRenderableConstPtr renderableInstance= *instanceIter;

				if (renderableInstance->getVisible())
				{
					GlMaterialInstancePtr materialInstance= renderableInstance->getMaterialInstance();

					// Bind material instance parameters (unbound when materialInstanceBinding goes out of scope)
					auto materialInstanceBinding=  materialInstance->bindMaterialInstance(materialBinding);
					if (materialInstanceBinding)
					{
						// Set the per-mesh-instance ModelViewProjection matrix transform on the shader program
						const glm::mat4 mvpMatrix = VPMatrix * renderableInstance->getModelMatrix();
						materialInstance->setMat4BySemantic(eUniformSemantic::modelViewProjectionMatrix, mvpMatrix);

						// Draw the renderable
						renderableInstance->render();
					}
				}
			}
		}
	}
}