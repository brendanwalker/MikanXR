#include "App.h"
#include "GlCamera.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "GlScene.h"
#include "GlShaderCache.h"
#include "GlViewport.h"

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

void GlScene::render(GlCameraConstPtr camera) const
{
	for (auto drawCallIter= m_drawCalls.begin(); drawCallIter != m_drawCalls.end(); drawCallIter++)
	{
		GlMaterialConstPtr material = drawCallIter->first;
		GlDrawCallConstPtr drawCall = drawCallIter->second;

		bool bAnyInstancesVisible= false;
		for (auto instanceIter = drawCall->instances.begin();
			 instanceIter != drawCall->instances.end();
			 instanceIter++)
		{
			IGlSceneRenderableConstPtr renderableInstance = instanceIter->lock();

			if (renderableInstance->getVisible())
			{
				bAnyInstancesVisible= true;
				break;
			}
		}

		if (bAnyInstancesVisible)
		{
			// Bind material program.
			// Unbound when materialBinding goes out of scope.
			auto materialBinding = material->bindMaterial(shared_from_this(), camera);
			if (materialBinding)
			{
				for (auto instanceIter = drawCall->instances.begin();
					 instanceIter != drawCall->instances.end();
					 instanceIter++)
				{
					IGlSceneRenderableConstPtr renderableInstance = instanceIter->lock();

					if (renderableInstance != nullptr && renderableInstance->getVisible())
					{
						GlMaterialInstanceConstPtr materialInstance = renderableInstance->getMaterialInstanceConst();

						// Bind material instance parameters 
						// Unbound when materialInstanceBinding goes out of scope.
						auto materialInstanceBinding =
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
}