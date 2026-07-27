#pragma once

#include "MikanObjectSystem.h"
#include "ColliderQuery.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "GizmoFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "MikanRendererFwd.h"
#include "SceneFwd.h"

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

// Display units for the ruler/measurement readout (scene is natively meters)
enum class eRulerDisplayUnits : int
{
	meters,
	centimeters,
	millimeters,
};

// How model stencils are drawn in the editor debug view
enum class eStencilDisplayMode : int
{
	solid,     // opaque textured mesh only
	wireframe, // wireframe edges only (lets the background video show through)
	both,      // solid mesh with wireframe overlaid
};

struct EditorSettings
{
	bool bRenderOrigin= true;
	bool bDebugRenderAnchors= true;
	bool bDebugRenderQuadStencils= true;
	bool bDebugRenderBoxStencils= true;
	bool bDebugRenderModelStencils= true;
	bool bDebugRenderQuadShapes= true;
	bool bDebugRenderBoxShapes= true;
	bool bDebugRenderModelShapes= true;
	float cameraSpeed= 1.f;

	// Floor grid + measurement snapping (world units are meters)
	float gridExtent= 10.f;    // total width/depth of the floor grid
	float gridCellSize= 0.5f;  // size of a single grid cell
	float snapIncrement= 0.1f; // ruler / measurement snap increment
	bool bSnapEnabled= false;  // ruler snap baseline; hold Shift while measuring to invert it
	eRulerDisplayUnits rulerDisplayUnits= eRulerDisplayUnits::millimeters; // ruler readout units
	bool bDebugCameraAlignment= false; // draw the MR camera-alignment debug overlay in the compositor view
	eStencilDisplayMode modelStencilDisplayMode= eStencilDisplayMode::both; // how model stencils are drawn
};

class EditorObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	EditorObjectSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator)
		: MikanObjectSystemDefinition(configName, idAllocator)
		, m_editorSettings()
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline const EditorSettings& getEditorSettings() const { return m_editorSettings; }

	static const std::string k_renderOriginFlagPropertyId;
	inline bool getRenderOriginFlag() const { return m_editorSettings.bRenderOrigin; }
	void setRenderOriginFlag(bool flag);

	static const std::string k_renderAnchorsPropertyId;
	inline bool getRenderAnchorsFlag() const { return m_editorSettings.bDebugRenderAnchors; }
	void setRenderAnchorsFlag(bool flag);

	static const std::string k_renderQuadStencilsPropertyId;
	inline bool getRenderQuadStencilsFlag() const { return m_editorSettings.bDebugRenderQuadStencils; }
	void setRenderQuadStencilsFlag(bool flag);

	static const std::string k_renderBoxStencilsPropertyId;
	inline bool getRenderBoxStencilsFlag() const { return m_editorSettings.bDebugRenderBoxStencils; }
	void setRenderBoxStencilsFlag(bool flag);

	static const std::string k_renderModelStencilsPropertyId;
	inline bool getRenderModelStencilsFlag() const { return m_editorSettings.bDebugRenderModelStencils; }
	void setRenderModelStencilsFlag(bool flag);

	static const std::string k_renderQuadShapesPropertyId;
	inline bool getRenderQuadShapesFlag() const { return m_editorSettings.bDebugRenderQuadShapes; }
	void setRenderQuadShapesFlag(bool flag);

	static const std::string k_renderBoxShapesPropertyId;
	inline bool getRenderBoxShapesFlag() const { return m_editorSettings.bDebugRenderBoxShapes; }
	void setRenderBoxShapesFlag(bool flag);

	static const std::string k_renderModelShapesPropertyId;
	inline bool getRenderModelShapesFlag() const { return m_editorSettings.bDebugRenderModelShapes; }
	void setRenderModelShapesFlag(bool flag);

	static const std::string k_cameraSpeedPropertyId;
	float getCameraSpeed() const { return m_editorSettings.cameraSpeed; }
	void setCameraSpeed(float speed);

	static const std::string k_gridExtentPropertyId;
	float getGridExtent() const { return m_editorSettings.gridExtent; }
	void setGridExtent(float extent);

	static const std::string k_gridCellSizePropertyId;
	float getGridCellSize() const { return m_editorSettings.gridCellSize; }
	void setGridCellSize(float cellSize);

	static const std::string k_snapIncrementPropertyId;
	float getSnapIncrement() const { return m_editorSettings.snapIncrement; }
	void setSnapIncrement(float increment);

	static const std::string k_snapEnabledPropertyId;
	bool getSnapEnabled() const { return m_editorSettings.bSnapEnabled; }
	void setSnapEnabled(bool enabled);

	static const std::string k_rulerDisplayUnitsPropertyId;
	eRulerDisplayUnits getRulerDisplayUnits() const { return m_editorSettings.rulerDisplayUnits; }
	void setRulerDisplayUnits(eRulerDisplayUnits units);

	static const std::string k_debugCameraAlignmentPropertyId;
	bool getDebugCameraAlignment() const { return m_editorSettings.bDebugCameraAlignment; }
	void setDebugCameraAlignment(bool enabled);

	static const std::string k_modelStencilDisplayModePropertyId;
	eStencilDisplayMode getModelStencilDisplayMode() const { return m_editorSettings.modelStencilDisplayMode; }
	void setModelStencilDisplayMode(eStencilDisplayMode mode);

private:
	EditorSettings m_editorSettings;
};

class EditorObjectSystem : public MikanObjectSystem
{
public:
	EditorObjectSystem(ProjectManagerPtr ownerObjectSystem)
		: MikanObjectSystem(ownerObjectSystem)
	{
	}

	inline static const std::string k_objectSystemClassName= "EditorObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;

	void customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera);

	// Draws the orthographic ruler/measurement overlay (no-op unless a measurement
	// exists and the supplied camera is orthographic).
	void renderRuler(IMkGraphicsContext* graphicsContext, MikanViewportPtr targetViewport);

	EditorObjectSystemDefinitionConstPtr getEditorSystemConfigConst() const;
	EditorObjectSystemDefinitionPtr getEditorSystemConfig();
	const EditorSettings& getEditorSettings() const { return getEditorSystemConfigConst()->getEditorSettings(); }

	virtual MikanComponentPtr getComponentById(int componentId) const override;
	virtual bool getComponentList(const std::string& componentClassName,
								  std::vector<MikanComponentPtr>& outComponentList) const override;
	virtual bool getComponentIdList(const std::string& componentClassName,
									std::vector<int>& outComponentIdList) const override;

	void bindViewport(MikanViewportWeakPtr viewportWeakPtr);
	void unbindViewport(MikanViewportWeakPtr viewportWeakPtr);
	void clearViewports();

	void setObjectSystemSelectionFilter(const std::set<const MikanObjectSystem*>& objectSystemFilter);
	SelectionComponentPtr getSelection() const { return m_selectedComponentWeakPtr.lock(); }
	SelectionComponentPtr getSelectedSceneActor() const;
	void setSelection(SelectionComponentPtr newComponentPtr);
	MulticastDelegate<void()> OnSelectionChanged;

	inline MikanObjectPtr getGizmoObject() const { return m_gizmoObjectWeakPtr.lock(); }
	MikanViewportPtr getPrimaryViewport() const;
	MikanCameraPtr getPrimaryCamera() const;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;
	virtual void registerFunctionDescriptors(MikanFunctionDatabasePtr functionDatabase) override;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static const std::string k_selectedLanguagePropertyId;
	static const std::string k_availableLanguageListPropertyId;
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

protected:
	std::vector<MikanViewportWeakPtr> m_viewports;

	std::set<const MikanObjectSystem*> m_objectSystemSelectionFilter;
	ColliderRaycastHitResult m_lastestRaycastResult;
	SelectionComponentWeakPtr m_hoverComponentWeakPtr;
	ColliderComponentWeakPtr m_hoverColliderWeakPtr;
	SelectionComponentWeakPtr m_selectedComponentWeakPtr;

	MikanObjectWeakPtr m_gizmoObjectWeakPtr;
	GizmoTransformComponentWeakPtr m_gizmoComponentWeakPtr;

	// App Events
	void onAppStageEntered(class AppStage* oldAppStage, class AppStage* newAppStage);

	// Object System Events
	void onSceneActivated(SceneComponentPtr newScene);
	void onSceneDeactivated(SceneComponentPtr oldScene);
	void onActorDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component);

	// Keyboard Events
	void onDeletePressed();

	// Viewport Events
	void onMouseExited();
	void onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
	void onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button);
	void onSelectionChanged(SelectionComponentPtr oldComponentPtr, SelectionComponentPtr newComponentPtr);

	// Helpers
	void createSceneTransformGizmo(SceneComponentPtr ownerScene);
	void disposeSceneTransformGizmo();
	void createGizmoBoxCollider(MikanObjectPtr gizmoObjectPtr, const std::string& name, const glm::vec3& center,
								const glm::vec3& halfExtents, const int priority);
	void createGizmoDiskCollider(MikanObjectPtr gizmoObjectPtr, const std::string& name, const glm::vec3& center,
								 const glm::vec3& normal, const float radius, const int priority);
	SelectionComponentPtr findClosestSelectionTarget(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
													 ColliderRaycastHitResult& outRaycastResult) const;
	void clearHoveredComponent();
	void clearSelectedComponent();

	// Ruler / measurement tool (orthographic only)
	glm::vec3 projectAndSnapMeasurePoint(const glm::vec3& worldPoint) const;
	// Effective snap state: the configured baseline, inverted while Shift is held.
	bool isRulerSnapActive(MikanViewportPtr targetViewport) const;

	bool m_hasMeasurement= false;
	bool m_isMeasuring= false;
	glm::vec3 m_measureStart= glm::vec3(0.f);
	glm::vec3 m_measureEnd= glm::vec3(0.f);
};