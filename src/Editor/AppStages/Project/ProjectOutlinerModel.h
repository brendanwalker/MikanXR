#pragma once

#include "ComponentFwd.h"
#include "ObjectSystemFwd.h"
#include "SceneFwd.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

enum class eOutlinerNodeKind : int
{
	INVALID= -1,

	projectRoot= 0,
	folderSources,
	folderMarkers,
	folderTrackingVolumes,
	folderCameras,
	folderLights,
	folderScenes,
	unparentedGroup,
	videoSource,
	textureSource,
	marker,
	trackingVolume,
	trackingMount,
	stage,
	camera,
	compositor,
	stageLight,
	scene,
	sceneActor,

	COUNT
};

class ProjectOutlinerNode;
using ProjectOutlinerNodePtr= std::shared_ptr<ProjectOutlinerNode>;
using ProjectOutlinerNodeConstPtr= std::shared_ptr<const ProjectOutlinerNode>;
using ProjectOutlinerNodeWeakPtr= std::weak_ptr<ProjectOutlinerNode>;

class ProjectOutlinerNode
{
public:
	eOutlinerNodeKind kind= eOutlinerNodeKind::INVALID;
	int componentId= -1;
	// For a folder row: the component id of the row it belongs to (the stage
	// above a Cameras folder); -1 for the top-level folders
	int ownerId= -1;
	std::string systemClassName;
	std::string componentClassName;
	std::string displayName;
	MikanComponentWeakPtr component;
	SelectionComponentWeakPtr selection;
	ProjectOutlinerNodeWeakPtr parent;
	std::vector<ProjectOutlinerNodePtr> children;
};

// Builds the outliner tree over the project's object systems. Pure data: no
// ImGui, rebuilt from scratch each time the object graph changes shape.
class ProjectOutlinerModel
{
public:
	void rebuild(ProjectManagerPtr projectManager);
	void clear();

	ProjectOutlinerNodePtr getRoot() const { return m_root; }
	// The folder of the given kind (and owning row for per-stage folders);
	// null when the kind is not a folder or the owner is gone
	ProjectOutlinerNodePtr findFolderNode(eOutlinerNodeKind kind, int ownerId= -1) const;
	ProjectOutlinerNodePtr findNodeByComponentId(int componentId) const;
	ProjectOutlinerNodePtr findNodeBySelection(SelectionComponentConstPtr selection) const;
	// The scene node at or above the given node; null when the node is outside
	// any scene subtree
	ProjectOutlinerNodePtr findOwningSceneNode(ProjectOutlinerNodePtr node) const;

private:
	ProjectOutlinerNodePtr addComponentNode(ProjectOutlinerNodePtr parentNode, eOutlinerNodeKind kind,
											MikanComponentPtr component, const char* namePrefixKey= nullptr);
	void buildStageSubtree(ProjectManagerPtr projectManager, StageComponentPtr stageComponent,
						   ProjectOutlinerNodePtr parentNode);
	void buildSceneSubtree(SceneComponentPtr sceneComponent, ProjectOutlinerNodePtr parentNode);
	void addSceneActorSubtree(TransformComponentPtr transformComponent, ProjectOutlinerNodePtr parentNode);
	ProjectOutlinerNodePtr addFolderNode(ProjectOutlinerNodePtr parentNode, eOutlinerNodeKind kind,
										 const char* labelKey, int ownerId= -1);
	ProjectOutlinerNodePtr getOrCreateUnparentedGroup();

	ProjectOutlinerNodePtr m_root;
	ProjectOutlinerNodePtr m_unparentedGroup;
	std::map<int, ProjectOutlinerNodeWeakPtr> m_nodeIndex;
	// Folder rows keyed by (kind, ownerId)
	std::map<std::pair<int, int>, ProjectOutlinerNodeWeakPtr> m_folderIndex;
};
