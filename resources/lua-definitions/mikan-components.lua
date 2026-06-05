---@meta
-- Mikan scripting API — component class hierarchy.
-- This file is a LuaLS definition file; it contains no executable code.
-- Keep in sync with: src/Editor/ECS/ component bindLuaFunctions implementations.

------------------------------------------------------------------------
-- MikanComponent — base class for all Mikan components
------------------------------------------------------------------------

---@class MikanComponent
---@field name string Component name (read/write)
---@field className string Component class name (read-only)
local MikanComponent = {}

--- Get the CameraObjectSystem singleton.
---@return CameraObjectSystem
function MikanComponent:getCameraSystem() end

--- Get the SceneObjectSystem singleton.
---@return SceneObjectSystem
function MikanComponent:getSceneSystem() end

--- Get the DMXObjectSystem singleton.
---@return DMXObjectSystem
function MikanComponent:getDMXSystem() end

--- Get the AnchorObjectSystem singleton.
---@return AnchorObjectSystem
function MikanComponent:getAnchorSystem() end

--- Get the CompositorObjectSystem singleton.
---@return CompositorObjectSystem
function MikanComponent:getCompositorSystem() end

------------------------------------------------------------------------
-- TransformComponent : MikanComponent
------------------------------------------------------------------------

---@class TransformComponent : MikanComponent
---@field relativePosition Vec3f Position relative to parent (read/write)
---@field relativeRotation Vec3f Euler rotation in degrees relative to parent (read/write)
---@field relativeScale Vec3f Scale relative to parent (read/write)
local TransformComponent = {}

------------------------------------------------------------------------
-- SceneComponent : TransformComponent
------------------------------------------------------------------------

---@class SceneComponent : TransformComponent
---@field parentStageId integer ID of the parent stage (read-only)
---@field displayCompositorId integer ID of the display compositor (read-only)
local SceneComponent = {}

--- Switch the scene's display to its compositor output.
function SceneComponent:showCompositorOutput() end

--- Get the parent StageComponent.
---@return StageComponent
function SceneComponent:getParentStage() end

--- Get the number of output compositors for this scene.
---@return integer
function SceneComponent:getOutputCompositorCount() end

--- Get the output compositor at the given index.
---@param index integer Zero-based index
---@return CompositorComponent
function SceneComponent:getOutputCompositorAtIndex(index) end

------------------------------------------------------------------------
-- StageComponent : TransformComponent
------------------------------------------------------------------------

---@class StageComponent : TransformComponent
---@field stageId integer Unique stage ID (read-only)
---@field trackingVolumeId integer Tracking volume ID (read/write)
---@field stageBoundsMin Vec3f Minimum bounds of the stage in mm (read-only)
---@field stageBoundsMax Vec3f Maximum bounds of the stage in mm (read-only)
local StageComponent = {}

------------------------------------------------------------------------
-- CameraComponent : TransformComponent
------------------------------------------------------------------------

---@class CameraComponent : TransformComponent
---@field ownerStageId integer ID of the owning stage (read-only)
---@field trackingMountId integer ID of the tracking mount (read-only)
---@field videoSourceId integer ID of the video source (read-only)
---@field trackingFrameDelay integer Frame delay for tracking (read/write)
---@field hasValidApertureOffset boolean Whether aperture offset is valid (read-only)
---@field aperturePositionOffset Vec3f Aperture position offset (read-only)
---@field apertureOrientationOffset Vec3f Aperture orientation offset as Euler angles (read-only)
local CameraComponent = {}

--- Trigger camera alignment.
function CameraComponent:alignCamera() end

--- Get the owning StageComponent.
---@return StageComponent
function CameraComponent:getOwnerStage() end

------------------------------------------------------------------------
-- AnchorComponent : TransformComponent
------------------------------------------------------------------------

---@class AnchorComponent : TransformComponent
local AnchorComponent = {}

--- Open the anchor editor UI.
function AnchorComponent:editAnchor() end

------------------------------------------------------------------------
-- StencilComponent : TransformComponent
------------------------------------------------------------------------

---@class StencilComponent : TransformComponent
---@field isDisabled boolean Whether the stencil is disabled (read/write)
---@field cullMode ScriptContext.CullMode Cull mode for this stencil (read/write)
local StencilComponent = {}

------------------------------------------------------------------------
-- BoxStencilComponent : StencilComponent
------------------------------------------------------------------------

---@class BoxStencilComponent : StencilComponent
---@field boxXSize number Half-extent along X axis in meters (read/write)
---@field boxYSize number Half-extent along Y axis in meters (read/write)
---@field boxZSize number Half-extent along Z axis in meters (read/write)
local BoxStencilComponent = {}

------------------------------------------------------------------------
-- QuadStencilComponent : StencilComponent
------------------------------------------------------------------------

---@class QuadStencilComponent : StencilComponent
---@field quadWidth number Width of the quad stencil in meters (read/write)
---@field quadHeight number Height of the quad stencil in meters (read/write)
---@field isDoubleSided boolean Whether the stencil is double-sided (read/write)
local QuadStencilComponent = {}

------------------------------------------------------------------------
-- ModelStencilComponent : StencilComponent
------------------------------------------------------------------------

---@class ModelStencilComponent : StencilComponent
---@field modelPath string Path to the stencil mesh file (read-only)
local ModelStencilComponent = {}

--- Trigger stencil alignment to the camera.
function ModelStencilComponent:alignStencil() end

------------------------------------------------------------------------
-- ShapeComponent : TransformComponent
------------------------------------------------------------------------

---@class ShapeComponent : TransformComponent
local ShapeComponent = {}

------------------------------------------------------------------------
-- QuadShapeComponent : ShapeComponent
------------------------------------------------------------------------

---@class QuadShapeComponent : ShapeComponent
---@field quadWidth number Width of the quad shape in meters (read/write)
---@field quadHeight number Height of the quad shape in meters (read/write)
---@field isDoubleSided boolean Whether the quad shape is double-sided (read/write)
local QuadShapeComponent = {}

------------------------------------------------------------------------
-- BoxShapeComponent : ShapeComponent
------------------------------------------------------------------------

---@class BoxShapeComponent : ShapeComponent
---@field boxXSize number Size along X axis in meters (read/write)
---@field boxYSize number Size along Y axis in meters (read/write)
---@field boxZSize number Size along Z axis in meters (read/write)
local BoxShapeComponent = {}

------------------------------------------------------------------------
-- ModelShapeComponent : ShapeComponent
------------------------------------------------------------------------

---@class ModelShapeComponent : ShapeComponent
---@field modelPath string Path to the shape mesh file (read-only)
local ModelShapeComponent = {}

------------------------------------------------------------------------
-- DMXFixtureComponent : TransformComponent
------------------------------------------------------------------------

---@class DMXFixtureComponent : TransformComponent
---@field dmxUniverse integer DMX universe number (read/write)
---@field dmxStartChannel integer DMX start channel (read/write)
---@field dmxChannelCount integer Number of DMX channels used (read-only)
---@field isDisabled boolean Whether the fixture is disabled (read/write)
local DMXFixtureComponent = {}

--- Triangulate the light position using camera data.
function DMXFixtureComponent:triangulateLight() end

--- Get the owning StageComponent.
---@return StageComponent
function DMXFixtureComponent:getOwnerStage() end

------------------------------------------------------------------------
-- RGBSpotLightComponent : DMXFixtureComponent
------------------------------------------------------------------------

---@class RGBSpotLightComponent : DMXFixtureComponent
---@field red integer Red channel value 0-255 (read/write)
---@field green integer Green channel value 0-255 (read/write)
---@field blue integer Blue channel value 0-255 (read/write)
---@field coneAngleDegrees number Cone angle in degrees (read/write)
---@field coneRangeMeters number Cone range in meters (read/write)
local RGBSpotLightComponent = {}

--- Set the RGB color of the spot light.
---@param r integer Red 0-255
---@param g integer Green 0-255
---@param b integer Blue 0-255
function RGBSpotLightComponent:setRGB(r, g, b) end

------------------------------------------------------------------------
-- RGBPixelGridComponent : DMXFixtureComponent
------------------------------------------------------------------------

---@class RGBPixelGridComponent : DMXFixtureComponent
---@field columns integer Number of pixel columns (read/write)
---@field rows integer Number of pixel rows (read/write)
local RGBPixelGridComponent = {}

--- Set a single pixel color.
---@param col integer Column index (0-based)
---@param row integer Row index (0-based)
---@param r integer Red 0-255
---@param g integer Green 0-255
---@param b integer Blue 0-255
function RGBPixelGridComponent:setPixel(col, row, r, g, b) end

--- Fill all pixels with a solid color.
---@param r integer Red 0-255
---@param g integer Green 0-255
---@param b integer Blue 0-255
function RGBPixelGridComponent:fillPixels(r, g, b) end

------------------------------------------------------------------------
-- MarkerComponent : MikanComponent
------------------------------------------------------------------------

---@class MarkerComponent : MikanComponent
---@field arucoId integer ArUco marker ID (read/write)
---@field lengthMM number Physical marker side length in millimeters (read/write)
local MarkerComponent = {}

--- Print the marker image to a file or display.
function MarkerComponent:printMarker() end

------------------------------------------------------------------------
-- CompositorComponent : MikanComponent
------------------------------------------------------------------------

---@class CompositorComponent : MikanComponent
---@field compositorId integer Unique compositor ID (read-only)
---@field cameraId integer ID of the associated camera (read-only)
---@field ownerSceneId integer ID of the owning scene (read-only)
---@field ownerStageId integer ID of the owning stage (read-only)
---@field isSpoutOutputStreaming boolean Whether Spout output streaming is active (read/write)
local CompositorComponent = {}

--- Open the compositor graph editor UI.
function CompositorComponent:editCompositorGraph() end

--- Get the owning StageComponent.
---@return StageComponent
function CompositorComponent:getOwnerStage() end

------------------------------------------------------------------------
-- QuadShapeSystem — singleton injected by ComponentScriptContext
------------------------------------------------------------------------

---@class QuadShapeSystem
local QuadShapeSystem = {}

--- Get a QuadShapeComponent by its shape ID.
---@param id integer
---@return QuadShapeComponent
function QuadShapeSystem:getQuadShapeById(id) end

--- Get a QuadShapeComponent by its name.
---@param name string
---@return QuadShapeComponent
function QuadShapeSystem:getQuadShapeByName(name) end

--- Get the total number of quad shapes in the system.
---@return integer
function QuadShapeSystem:getQuadShapeCount() end

--- Get a QuadShapeComponent at a zero-based index.
---@param index integer
---@return QuadShapeComponent
function QuadShapeSystem:getQuadShapeAtIndex(index) end

------------------------------------------------------------------------
-- BoxShapeSystem — singleton injected by ComponentScriptContext
------------------------------------------------------------------------

---@class BoxShapeSystem
local BoxShapeSystem = {}

--- Get a BoxShapeComponent by its shape ID.
---@param id integer
---@return BoxShapeComponent
function BoxShapeSystem:getBoxShapeById(id) end

--- Get a BoxShapeComponent by its name.
---@param name string
---@return BoxShapeComponent
function BoxShapeSystem:getBoxShapeByName(name) end

--- Get the total number of box shapes in the system.
---@return integer
function BoxShapeSystem:getBoxShapeCount() end

--- Get a BoxShapeComponent at a zero-based index.
---@param index integer
---@return BoxShapeComponent
function BoxShapeSystem:getBoxShapeAtIndex(index) end

------------------------------------------------------------------------
-- ModelShapeSystem — singleton injected by ComponentScriptContext
------------------------------------------------------------------------

---@class ModelShapeSystem
local ModelShapeSystem = {}

--- Get a ModelShapeComponent by its shape ID.
---@param id integer
---@return ModelShapeComponent
function ModelShapeSystem:getModelShapeById(id) end

--- Get a ModelShapeComponent by its name.
---@param name string
---@return ModelShapeComponent
function ModelShapeSystem:getModelShapeByName(name) end

--- Get the total number of model shapes in the system.
---@return integer
function ModelShapeSystem:getModelShapeCount() end

--- Get a ModelShapeComponent at a zero-based index.
---@param index integer
---@return ModelShapeComponent
function ModelShapeSystem:getModelShapeAtIndex(index) end

------------------------------------------------------------------------
-- Global injected by ComponentScriptContext
------------------------------------------------------------------------

--- The component this script is attached to.
---@type MikanComponent
ownerComponent = nil

--- The QuadShapeSystem singleton.
---@type QuadShapeSystem
QuadShapeSystem = nil

--- The BoxShapeSystem singleton.
---@type BoxShapeSystem
BoxShapeSystem = nil

--- The ModelShapeSystem singleton.
---@type ModelShapeSystem
ModelShapeSystem = nil
