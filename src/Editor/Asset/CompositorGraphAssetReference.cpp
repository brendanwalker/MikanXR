#include "CompositorGraphAssetReference.h"
#include "App.h"
#include "CompositorComponent.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MikanObjectSystem.h"
#include "PathUtils.h"
#include "ProjectManager.h"
#include "Windows/CompositorNodeEditorWindow.h"

void CompositorGraphAssetReference::editorOpen()
{
	App* app= App::getInstance();

	// One compositor editor window serves every compositor graph
	CompositorNodeEditorWindow* window= app->getWindowOfType<CompositorNodeEditorWindow>();
	if (window == nullptr)
	{
		window= app->createAppWindow<CompositorNodeEditorWindow>();
	}
	else
	{
		window->getMkWindowContext()->raiseWindow();
	}
	if (window == nullptr)
	{
		MIKAN_LOG_ERROR("CompositorGraphAssetReference::editorOpen") << "Failed to create the compositor editor window";
		return;
	}

	// A compositor already driving this graph is bound so the edit shows live
	const std::string storedPath= PathUtils::makeStoredProjectPath(getInternalAssetPath());
	if (ProjectManagerPtr projectManager= app->getMainWindow()->getProjectManager())
	{
		for (const MikanObjectSystemPtr& system : projectManager->getSystems())
		{
			std::vector<MikanComponentPtr> components;
			if (!system->getComponentList(CompositorComponent::k_componentClassName, components))
				continue;

			for (const MikanComponentPtr& component : components)
			{
				CompositorComponentPtr compositor= std::dynamic_pointer_cast<CompositorComponent>(component);
				if (compositor
					&& PathUtils::makeStoredProjectPath(compositor->getCompositorGraphAssetPath()) == storedPath)
				{
					window->openCompositorComponent(compositor);
					return;
				}
			}
		}
	}

	window->openGraphFile(getInternalAssetPath());
}

// -- CompositorGraphAssetReferenceFactory -----
CompositorGraphAssetReferenceFactory::CompositorGraphAssetReferenceFactory()
	: TypedAssetReferenceFactory<CompositorGraphAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getProjectDirectory() / "compositors" / "").string();
}
