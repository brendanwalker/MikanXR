#include "ShapeGraphAssetReference.h"
#include "App.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MikanObjectSystem.h"
#include "PathUtils.h"
#include "ProjectManager.h"
#include "BoxShapeComponent.h"
#include "ModelShapeComponent.h"
#include "QuadShapeComponent.h"
#include "ShapeComponent.h"
#include "Windows/ShapeNodeEditorWindow.h"

void ShapeGraphAssetReference::editorOpen()
{
	App* app= App::getInstance();

	// One shape editor window serves every shape graph
	ShapeNodeEditorWindow* window= app->getWindowOfType<ShapeNodeEditorWindow>();
	if (window == nullptr)
	{
		window= app->createAppWindow<ShapeNodeEditorWindow>();
	}
	else
	{
		window->getMkWindowContext()->raiseWindow();
	}
	if (window == nullptr)
	{
		MIKAN_LOG_ERROR("ShapeGraphAssetReference::editorOpen") << "Failed to create the shape editor window";
		return;
	}

	// A shape already driving this graph is bound so the edit shows live. Shape
	// components come in several classes, so every system is asked for each.
	const std::string storedPath= PathUtils::makeStoredProjectPath(getInternalAssetPath());
	if (ProjectManagerPtr projectManager= app->getMainWindow()->getProjectManager())
	{
		const std::string shapeClassNames[]= {QuadShapeComponent::k_componentClassName,
											  BoxShapeComponent::k_componentClassName,
											  ModelShapeComponent::k_componentClassName};
		for (const MikanObjectSystemPtr& system : projectManager->getSystems())
		{
			for (const std::string& className : shapeClassNames)
			{
				std::vector<MikanComponentPtr> components;
				if (!system->getComponentList(className, components))
					continue;

				for (const MikanComponentPtr& component : components)
				{
					ShapeComponentPtr shape= std::dynamic_pointer_cast<ShapeComponent>(component);
					if (shape
						&& PathUtils::makeStoredProjectPath(shape->getShapeComponentDefinition()->getShapeGraphPath())
							   == storedPath)
					{
						window->openShapeComponent(shape);
						return;
					}
				}
			}
		}
	}

	window->openGraphFile(getInternalAssetPath());
}

// -- ShapeGraphAssetReferenceFactory -----
ShapeGraphAssetReferenceFactory::ShapeGraphAssetReferenceFactory()
	: TypedAssetReferenceFactory<ShapeGraphAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getProjectDirectory() / "shapes" / "").string();
}
