---@meta
-- Mikan scripting API — object system classes and globals.
-- This file is a LuaLS definition file; it contains no executable code.
-- Keep in sync with: src/Editor/ECS/ object system bindLuaFunctions implementations.

------------------------------------------------------------------------
-- CameraObjectSystem
------------------------------------------------------------------------

---@class CameraObjectSystem
local CameraObjectSystem = {}

---@param id integer
---@return CameraComponent
function CameraObjectSystem:getCameraById(id) end

---@param name string
---@return CameraComponent
function CameraObjectSystem:getCameraByName(name) end

---@return integer
function CameraObjectSystem:getCameraCount() end

---@param index integer Zero-based index
---@return CameraComponent
function CameraObjectSystem:getCameraAtIndex(index) end

------------------------------------------------------------------------
-- SceneObjectSystem
------------------------------------------------------------------------

---@class SceneObjectSystem
local SceneObjectSystem = {}

--- Get the currently active scene.
---@return SceneComponent
function SceneObjectSystem:getCurrentScene() end

---@param id integer
---@return SceneComponent
function SceneObjectSystem:getSceneById(id) end

---@param name string
---@return SceneComponent
function SceneObjectSystem:getSceneByName(name) end

---@return integer
function SceneObjectSystem:getSceneCount() end

---@param index integer Zero-based index
---@return SceneComponent
function SceneObjectSystem:getSceneAtIndex(index) end

------------------------------------------------------------------------
-- DMXObjectSystem
------------------------------------------------------------------------

---@class DMXObjectSystem
local DMXObjectSystem = {}

---@return integer
function DMXObjectSystem:getSpotLightCount() end

---@param index integer Zero-based index
---@return RGBSpotLightComponent
function DMXObjectSystem:getSpotLightAtIndex(index) end

---@return integer
function DMXObjectSystem:getPixelGridCount() end

---@param index integer Zero-based index
---@return RGBPixelGridComponent
function DMXObjectSystem:getPixelGridAtIndex(index) end

------------------------------------------------------------------------
-- AnchorObjectSystem
------------------------------------------------------------------------

---@class AnchorObjectSystem
local AnchorObjectSystem = {}

---@param id integer
---@return AnchorComponent
function AnchorObjectSystem:getAnchorById(id) end

---@param name string
---@return AnchorComponent
function AnchorObjectSystem:getAnchorByName(name) end

---@return integer
function AnchorObjectSystem:getAnchorCount() end

---@param index integer Zero-based index
---@return AnchorComponent
function AnchorObjectSystem:getAnchorAtIndex(index) end

------------------------------------------------------------------------
-- CompositorObjectSystem
------------------------------------------------------------------------

---@class CompositorObjectSystem
local CompositorObjectSystem = {}

---@param id integer
---@return CompositorComponent
function CompositorObjectSystem:getCompositorById(id) end

---@param name string
---@return CompositorComponent
function CompositorObjectSystem:getCompositorByName(name) end

---@return integer
function CompositorObjectSystem:getCompositorCount() end

---@param index integer Zero-based index
---@return CompositorComponent
function CompositorObjectSystem:getCompositorAtIndex(index) end

--- Get the number of compositors belonging to a stage.
---@param stageId integer
---@return integer
function CompositorObjectSystem:getCompositorCountForStage(stageId) end

--- Get the compositor for a stage at the given index.
---@param stageId integer
---@param index integer Zero-based index
---@return CompositorComponent
function CompositorObjectSystem:getCompositorForStageAtIndex(stageId, index) end

------------------------------------------------------------------------
-- ModelStencilSystem — global singleton injected by ComponentScriptContext
------------------------------------------------------------------------

---@class ModelStencilSystem
ModelStencilSystem = {}

---@param id integer
---@return ModelStencilComponent
function ModelStencilSystem:getModelStencilById(id) end

---@param name string
---@return ModelStencilComponent
function ModelStencilSystem:getModelStencilByName(name) end

---@return integer
function ModelStencilSystem:getModelStencilCount() end

---@param index integer Zero-based index
---@return ModelStencilComponent
function ModelStencilSystem:getModelStencilAtIndex(index) end

------------------------------------------------------------------------
-- BoxStencilSystem — global singleton injected by ComponentScriptContext
------------------------------------------------------------------------

---@class BoxStencilSystem
BoxStencilSystem = {}

---@param id integer
---@return BoxStencilComponent
function BoxStencilSystem:getBoxStencilById(id) end

---@param name string
---@return BoxStencilComponent
function BoxStencilSystem:getBoxStencilByName(name) end

---@return integer
function BoxStencilSystem:getBoxStencilCount() end

---@param index integer Zero-based index
---@return BoxStencilComponent
function BoxStencilSystem:getBoxStencilAtIndex(index) end

------------------------------------------------------------------------
-- QuadStencilSystem — global singleton injected by ComponentScriptContext
------------------------------------------------------------------------

---@class QuadStencilSystem
QuadStencilSystem = {}

---@param id integer
---@return QuadStencilComponent
function QuadStencilSystem:getQuadStencilById(id) end

---@param name string
---@return QuadStencilComponent
function QuadStencilSystem:getQuadStencilByName(name) end

---@return integer
function QuadStencilSystem:getQuadStencilCount() end

---@param index integer Zero-based index
---@return QuadStencilComponent
function QuadStencilSystem:getQuadStencilAtIndex(index) end
