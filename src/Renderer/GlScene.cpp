#include "App.h"
#include "GlCamera.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "GlScene.h"
#include "GlStaticMeshInstance.h"
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

void GlScene::addInstance(const GlStaticMeshInstance* instance)
{
	const GlMaterial *material= instance->getMaterialInstance()->getMaterial();

	if (m_drawCalls.find(material) == m_drawCalls.end())
	{
		m_drawCalls.insert({material, new GlDrawCall});
	}

	m_drawCalls[material]->instances.push_back(instance);
}

void GlScene::removeInstance(const GlStaticMeshInstance* instance)
{
	const GlMaterial* material = instance->getMaterialInstance()->getMaterial();

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
		const GlMaterial* material = drawCallIter->first;
		const GlDrawCall* drawCall = drawCallIter->second;

		if (material->bindMaterial())
		{			
			for(auto instanceIter= drawCall->instances.begin(); 
				instanceIter != drawCall->instances.end(); 
				instanceIter++)
			{
				const GlStaticMeshInstance* instance= *instanceIter;

				if (instance->getVisible())
				{
					// Set the ModelViewProjection matrix transform on the shader program
					const glm::mat4 mvpMatrix = VPMatrix * instance->getModelMatrix();
					material->getProgram()->setMatrix4x4Uniform(
						eUniformSemantic::modelViewProjectionMatrix, mvpMatrix);

					// Apply other material instance parameters (color, etc)
					instance->getMaterialInstance()->applyMaterialInstanceParameters();

					instance->render();
				}
			}
			
			material->unbindMaterial();
		}
	}
}