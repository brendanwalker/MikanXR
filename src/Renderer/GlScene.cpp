#include "App.h"
#include "GlCamera.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "GlScene.h"
#include "GlShaderCache.h"
#include "GlViewport.h"
#include "Renderer.h"

#include <algorithm>

GlScene::GlScene()
	: m_lightColor(glm::vec4(1.f))
	, m_lightDirection(glm::vec3(0.f, 0.f, -1.f))
{
}

GlScene::~GlScene()
{
	// Clean up all draw calls
	m_drawCalls.clear();
}

void GlScene::addInstance(IGlSceneRenderableConstPtr instance)
{
	GlMaterialConstPtr material= instance->getMaterialInstanceConst()->getMaterial();

	if (m_drawCalls.find(material) == m_drawCalls.end())
	{
		m_drawCalls.insert({material, std::make_shared<GlDrawCall>()});
	}

	m_drawCalls[material]->instances.push_back(instance);
}

void GlScene::removeInstance(IGlSceneRenderableConstPtr instance)
{
	GlMaterialConstPtr material = instance->getMaterialInstanceConst()->getMaterial();

	auto drawCallIter= m_drawCalls.find(material);
	if (drawCallIter != m_drawCalls.end())
	{
		GlDrawCallPtr drawCall = drawCallIter->second;

		// Remove any existing instances from the draw call
		for (auto it = drawCall->instances.begin(); it != drawCall->instances.end(); it++)
		{
			IGlSceneRenderableConstPtr existingInstance= it->lock();

			if (existingInstance == instance)
			{
				drawCall->instances.erase(it);
				break;
			}
		}

		if (drawCall->instances.size() == 0)
		{
			m_drawCalls.erase(drawCallIter);
		}
	}
}

void GlScene::render() const
{
	GlCameraPtr camera= App::getInstance()->getRenderer()->getRenderingViewport()->getCurrentCamera();
	if (camera == nullptr)
		return;

	for (auto drawCallIter= m_drawCalls.begin(); drawCallIter != m_drawCalls.end(); drawCallIter++)
	{
		GlMaterialConstPtr material = drawCallIter->first;
		GlDrawCallConstPtr drawCall = drawCallIter->second;

		// Bind material program (unbound when materialBinding goes out of scope)
		auto materialBinding= material->bindMaterial(shared_from_this(), camera);
		if (materialBinding)
		{			
			for(auto instanceIter= drawCall->instances.begin(); 
				instanceIter != drawCall->instances.end(); 
				instanceIter++)
			{
				IGlSceneRenderableConstPtr renderableInstance= instanceIter->lock();

				if (renderableInstance != nullptr && renderableInstance->getVisible())
				{
					GlMaterialInstancePtr materialInstance= renderableInstance->getMaterialInstance();

					// Bind material instance parameters (unbound when materialInstanceBinding goes out of scope)
					auto materialInstanceBinding= 
						materialInstance->bindMaterialInstance(
							materialBinding, 
							renderableInstance);

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