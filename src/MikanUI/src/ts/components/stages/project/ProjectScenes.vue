<template>
  <div class="project-panel">
    <h2>Scenes Panel</h2>
    <p>Select a scene to manage its objects.</p>

    <div class="scenes-content">
      <!-- Scene Selection (with add/delete) -->
      <div class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Scene:</label>
          <ComponentRefSelect
            v-model="selectedSceneId"
            component-class="SceneComponent"
            class="selection-dropdown"
          />
          <button @click="handleAddScene" class="icon-only-btn add-btn" title="Add Scene">
            <img src="/images/add_component_normal_icon.png" alt="Add Scene" class="btn-icon-only" />
          </button>
          <button
            v-if="selectedSceneId !== -1"
            @click="handleRemoveScene"
            class="icon-only-btn remove-btn"
            title="Remove Scene"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove Scene" class="btn-icon-only" />
          </button>
        </div>
      </div>

      <!-- Scene Component Properties -->
      <div v-if="selectedSceneId !== -1 && selectedSceneComponent" class="component-section">
        <h3>Scene Properties</h3>
        <div class="property-grid">
          <div class="property-row">
            <label class="property-label">Name:</label>
            <PropertyField
              field-name="component_name"
              :field-value="selectedSceneComponent.component_name"
              owner-system="SceneObjectSystem"
              :component-id="selectedSceneId"
            />
          </div>
          <div class="property-row">
            <label class="property-label">Stage:</label>
            <PropertyField
              field-name="parent_stage_id"
              :field-value="selectedSceneComponent.parent_stage_id"
              field-type="component_ref"
              component-class="StageComponent"
              owner-system="SceneObjectSystem"
              :component-id="selectedSceneId"
            />
          </div>
          <div class="property-row">
            <label class="property-label">Compositor:</label>
            <PropertyField
              field-name="display_compositor_id"
              :field-value="selectedSceneComponent.display_compositor_id"
              field-type="component_ref"
              component-class="CompositorComponent"
              owner-system="SceneObjectSystem"
              :component-id="selectedSceneId"
            />
            <button @click="handleAddCompositor" class="icon-only-btn add-btn" title="Add Compositor">
              <img src="/images/add_component_normal_icon.png" alt="Add Compositor" class="btn-icon-only" />
            </button>
            <button
              v-if="selectedCompositorId !== -1"
              @click="handleRemoveCompositor"
              class="icon-only-btn remove-btn"
              title="Remove Compositor"
            >
              <img src="/images/delete_component_normal_icon.png" alt="Remove Compositor" class="btn-icon-only" />
            </button>
          </div>
          <div class="property-row">
            <label class="property-label">Script:</label>
            <div class="script-controls">
              <span class="script-path">{{ selectedSceneComponent.component_script || '&lt;None&gt;' }}</span>
              <button @click="handleReloadScript" class="icon-only-btn edit-btn" title="Reload Script">
                <img src="/images/edit_component_normal_icon.png" alt="Reload Script" class="btn-icon-only" />
              </button>
              <button @click="handleAddScript" class="icon-only-btn add-btn" title="Add Script">
                <img src="/images/add_component_normal_icon.png" alt="Add Script" class="btn-icon-only" />
              </button>
              <button @click="handleRemoveScript" class="icon-only-btn remove-btn" title="Remove Script">
                <img src="/images/delete_component_normal_icon.png" alt="Remove Script" class="btn-icon-only" />
              </button>
            </div>
          </div>
        </div>
      </div>

      <!-- Compositor Properties -->
      <div v-if="selectedCompositorId !== -1 && selectedCompositorComponent" class="component-section">
        <h3>Compositor Properties</h3>
        <div class="property-grid">
          <div class="property-row">
            <label class="property-label">Name:</label>
            <PropertyField
              field-name="component_name"
              :field-value="selectedCompositorComponent.component_name"
              owner-system="CompositorObjectSystem"
              :component-id="selectedCompositorId"
            />
          </div>
          <div class="property-row">
            <label class="property-label">Camera:</label>
            <PropertyField
              field-name="camera_id"
              :field-value="selectedCompositorComponent.camera_id"
              field-type="component_ref"
              component-class="VideoSourceComponent"
              owner-system="CompositorObjectSystem"
              :component-id="selectedCompositorId"
            />
          </div>
          <div class="property-row">
            <label class="property-label">Graph Path:</label>
            <div class="script-controls">
              <span class="script-path">{{ selectedCompositorComponent.compositor_graph_path || '&lt;None&gt;' }}</span>
              <button @click="handleEditCompositorGraph" class="icon-only-btn edit-btn" title="Reload Graph">
                <img src="/images/edit_component_normal_icon.png" alt="Reload Graph" class="btn-icon-only" />
              </button>
              <button @click="handleAddCompositorGraph" class="icon-only-btn add-btn" title="Add Graph">
                <img src="/images/add_component_normal_icon.png" alt="Add Graph" class="btn-icon-only" />
              </button>
              <button @click="handleRemoveCompositorGraph" class="icon-only-btn remove-btn" title="Remove Graph">
                <img src="/images/delete_component_normal_icon.png" alt="Remove Graph" class="btn-icon-only" />
              </button>
            </div>
          </div>
          <div class="property-row">
            <label class="property-label">Spout Output:</label>
            <PropertyField
              field-name="spout_enable_output"
              :field-value="selectedCompositorComponent.spout_enable_output"
              owner-system="CompositorObjectSystem"
              :component-id="selectedCompositorId"
            />
          </div>
          <div class="property-row">
            <label class="property-label">Spout Name:</label>
            <PropertyField
              field-name="spout_output_name"
              :field-value="selectedCompositorComponent.spout_output_name"
              owner-system="CompositorObjectSystem"
              :component-id="selectedCompositorId"
            />
          </div>
        </div>
      </div>

      <!-- Actors Panel -->
      <div v-if="selectedSceneId !== -1" class="component-section">
        <h3>Actors</h3>
        <div class="actor-buttons">
          <button @click="handleAddAnchor" class="action-btn icon-btn" title="Add Anchor">
            <img src="/images/add_anchor_icon.png" alt="Add Anchor" class="btn-icon" />
            Anchor
          </button>
          <button @click="handleAddQuadStencil" class="action-btn icon-btn" title="Add Quad Stencil">
            <img src="/images/add_quad_stencil_icon.png" alt="Add Quad" class="btn-icon" />
            Quad
          </button>
          <button @click="handleAddBoxStencil" class="action-btn icon-btn" title="Add Box Stencil">
            <img src="/images/add_box_stencil_icon.png" alt="Add Box" class="btn-icon" />
            Box
          </button>
          <button @click="handleAddModelStencil" class="action-btn icon-btn" title="Add Model Stencil">
            <img src="/images/add_model_stencil_icon.png" alt="Add Model" class="btn-icon" />
            Model
          </button>
        </div>

        <!-- Scene Outliner Tree -->
        <ul class="scene-outliner">
          <li
            v-for="item in flatSceneOutliner"
            :key="item.node.componentId"
            class="outliner-item"
            :class="{ selected: selectedSceneObjectId === item.node.componentId }"
            :style="{ paddingLeft: `${10 + item.depth * 10}px` }"
            @click="handleSelectSceneObject(item.node.componentId)"
          ><span v-if="item.depth > 0" class="tree-prefix">└ </span>{{ item.node.name }}</li>
        </ul>
      </div>

      <!-- Selected Scene Object Properties -->
      <div v-if="selectedSceneObject" class="component-section">
        <div class="section-header">
          <h3>{{ selectedSceneObject.componentType }} Properties</h3>
          <button @click="handleRemoveSelectedActor" class="icon-only-btn remove-btn" title="Remove">
            <img src="/images/delete_component_normal_icon.png" alt="Remove" class="btn-icon-only" />
          </button>
        </div>
        <ComponentCard
          :component-id="selectedSceneObject.componentId"
          :component="(selectedSceneObject.component as any)"
          :owner-system="selectedSceneObject.ownerSystem"
          :editable="true"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import ComponentRefSelect from '../../shared/ComponentRefSelect.vue'
import PropertyField from '../../shared/PropertyField.vue'
import ComponentCard from '../../shared/ComponentCard.vue'
import {
  MikanClient,
  MikanAPIResult,
  MikanConstants,
  PropertyGetValueRequest,
  PropertyGetValueResponse,
  PolymorphicObject,
  MikanVector3f,
  MikanAnchorComponentValues,
  MikanCompositorComponentValues,
  MikanQuadStencilComponentValues,
  MikanBoxStencilComponentValues,
  MikanModelStencilComponentValues
} from '@mikanxr/client'

const componentStore = useComponentStore()
const mikanStore = useMikanStore()

// Selection state
const selectedSceneId = ref<number>(-1)
const selectedSceneObjectId = ref<number>(-1)

interface SceneOutlinerNode {
  name: string
  componentId: number
  ownerSystem: string
  componentType: string
  children: SceneOutlinerNode[]
}

// Get components by system
const sceneComponents = computed(() =>
  componentStore.getComponentsByClass('SceneComponent')
)

const anchorComponents = computed(() =>
  componentStore.getComponentsByClass('AnchorComponent')
)

const quadStencilComponents = computed(() =>
  componentStore.getComponentsByClass('QuadStencilComponent')
)

const boxStencilComponents = computed(() =>
  componentStore.getComponentsByClass('BoxStencilComponent')
)

const modelStencilComponents = computed(() =>
  componentStore.getComponentsByClass('ModelStencilComponent')
)

// Get the selected scene component
const selectedSceneComponent = computed(() => {
  if (selectedSceneId.value === -1) return null
  return componentStore.getSceneComponent(selectedSceneId.value) as any
})

// Get the compositor currently set on the selected scene
const selectedCompositorId = computed<number>(() =>
  selectedSceneComponent.value?.display_compositor_id ?? -1
)

const selectedCompositorComponent = computed(() => {
  if (selectedCompositorId.value === -1) return null
  return componentStore.getComponent(selectedCompositorId.value, 'CompositorObjectSystem') as any
})

// parent_transform_id for newly created actors:
// use the selected outliner item if one exists, otherwise default to the scene itself
const newActorParentTransformId = computed<number>(() =>
  selectedSceneObject.value?.componentId ?? selectedSceneId.value
)

// Build nested scene outliner tree using parent_transform_id hierarchy
const sceneTree = computed<SceneOutlinerNode[]>(() => {
  if (selectedSceneId.value === -1) return []

  interface ActorEntry {
    componentId: number
    ownerSystem: string
    componentType: string
    parentTransformId: number
    name: string
  }

  const allActors: ActorEntry[] = []

  anchorComponents.value.forEach(c => {
    const comp = c as any
    allActors.push({ componentId: comp.component_id, ownerSystem: 'AnchorObjectSystem', componentType: 'Anchor', parentTransformId: comp.parent_transform_id ?? MikanConstants.InvalidMikanID, name: comp.component_name || '<Unnamed>' })
  })
  quadStencilComponents.value.forEach(c => {
    const comp = c as any
    allActors.push({ componentId: comp.component_id, ownerSystem: 'QuadStencilSystem', componentType: 'Quad Stencil', parentTransformId: comp.parent_transform_id ?? MikanConstants.InvalidMikanID, name: comp.component_name || '<Unnamed>' })
  })
  boxStencilComponents.value.forEach(c => {
    const comp = c as any
    allActors.push({ componentId: comp.component_id, ownerSystem: 'BoxStencilSystem', componentType: 'Box Stencil', parentTransformId: comp.parent_transform_id ?? MikanConstants.InvalidMikanID, name: comp.component_name || '<Unnamed>' })
  })
  modelStencilComponents.value.forEach(c => {
    const comp = c as any
    allActors.push({ componentId: comp.component_id, ownerSystem: 'ModelStencilSystem', componentType: 'Model Stencil', parentTransformId: comp.parent_transform_id ?? MikanConstants.InvalidMikanID, name: comp.component_name || '<Unnamed>' })
  })

  function buildChildren(parentId: number): SceneOutlinerNode[] {
    return allActors
      .filter(a => a.parentTransformId === parentId)
      .map(a => ({
        name: a.name,
        componentId: a.componentId,
        ownerSystem: a.ownerSystem,
        componentType: a.componentType,
        children: buildChildren(a.componentId)
      }))
  }

  return buildChildren(selectedSceneId.value)
})

// Flatten sceneTree into a depth-annotated list for flat rendering with indentation
const flatSceneOutliner = computed(() => {
  const result: { node: SceneOutlinerNode; depth: number }[] = []
  function flatten(nodes: SceneOutlinerNode[], depth: number) {
    for (const node of nodes) {
      result.push({ node, depth })
      flatten(node.children, depth + 1)
    }
  }
  flatten(sceneTree.value, 0)
  return result
})

function findNodeById(nodes: SceneOutlinerNode[], id: number): SceneOutlinerNode | null {
  for (const node of nodes) {
    if (node.componentId === id) return node
    const found = findNodeById(node.children, id)
    if (found) return found
  }
  return null
}

// Get the selected scene object
const selectedSceneObject = computed(() => {
  if (selectedSceneObjectId.value === -1) return null
  const node = findNodeById(sceneTree.value, selectedSceneObjectId.value)
  if (!node) return null
  return {
    componentId: node.componentId,
    component: componentStore.getComponent(node.componentId, node.ownerSystem),
    ownerSystem: node.ownerSystem,
    componentType: node.componentType
  }
})

// Initialize selected scene from the server's current_scene_id
async function initializeFromCurrentScene() {
  if (!mikanStore.client) {
    console.log('[ProjectScenes] No client connection, skipping initialization')
    return
  }

  try {
    console.log('[ProjectScenes] Fetching current scene ID...')

    const request = new PropertyGetValueRequest()
    request.ownerSystem = 'SceneObjectSystem'
    request.componentId = MikanConstants.InvalidMikanID
    request.fieldName = 'current_scene_id'

    const future = mikanStore.client.sendRequest(request)
    const response = await future.await() as PropertyGetValueResponse

    if (response.resultCode === MikanAPIResult.Success) {
      const currentSceneId = (response.propertyValue?.fieldValue?.value_ptr?.instance as any)?.value || -1
      console.log('[ProjectScenes] Current scene ID from server:', currentSceneId)

      if (currentSceneId !== -1) {
        selectedSceneId.value = currentSceneId
      } else {
        console.log('[ProjectScenes] No current scene set on server')
      }
    } else {
      console.error('[ProjectScenes] Failed to fetch current scene ID:', response.resultCode)
    }
  } catch (error) {
    console.error('[ProjectScenes] Error fetching current scene:', error)
  }
}

// Auto-select the last scene when the list grows (e.g. after handleAddScene)
watch(sceneComponents, (newList, oldList) => {
  if (newList.length > (oldList?.length ?? 0)) {
    const last = newList[newList.length - 1] as any
    selectedSceneId.value = last.component_id
  }
})

// Watch for scene changes to clear outliner selection
watch(selectedSceneId, () => {
  selectedSceneObjectId.value = -1
})

// Watch for component store changes - initialize if no scene is selected yet
watch(
  () => componentStore.components.size,
  (newSize, oldSize) => {
    console.log('[ProjectScenes] Component store size changed:', oldSize, '->', newSize)
    if (newSize > 0 && selectedSceneId.value === -1) {
      console.log('[ProjectScenes] Components loaded, attempting initialization')
      initializeFromCurrentScene()
    }
  }
)

// Persist scene selection
watch(selectedSceneId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectScenes.selectedSceneId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedSceneId = sessionStorage.getItem('projectScenes.selectedSceneId')

  if (savedSceneId) {
    const sceneId = parseInt(savedSceneId, 10)
    if (!isNaN(sceneId)) {
      selectedSceneId.value = sceneId
    }
  }
}

// Handle scene object selection
function handleSelectSceneObject(componentId: number) {
  selectedSceneObjectId.value = componentId
}

// Remove the currently selected actor by dispatching to the right typed handler
async function handleRemoveSelectedActor() {
  const obj = selectedSceneObject.value
  if (!obj) return
  switch (obj.componentType) {
    case 'Anchor':      await handleRemoveAnchor(); break
    case 'Quad Stencil': await handleRemoveQuadStencil(); break
    case 'Box Stencil':  await handleRemoveBoxStencil(); break
    case 'Model Stencil': await handleRemoveModelStencil(); break
    default: console.warn('[ProjectScenes] Unknown component type:', obj.componentType)
  }
}

// Scene CRUD handlers
async function handleAddScene() {
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.createObject(mikanStore.client as MikanClient, 'SceneComponent')
}

async function handleRemoveScene() {
  if (selectedSceneId.value === -1) { console.error('[ProjectScenes] No scene selected'); return }
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'SceneComponent', selectedSceneId.value)
  selectedSceneId.value = -1
}

// Script handlers
function handleReloadScript() {
  console.warn('[ProjectScenes] handleReloadScript not yet implemented')
}

function handleAddScript() {
  console.warn('[ProjectScenes] handleAddScript not yet implemented')
}

function handleRemoveScript() {
  console.warn('[ProjectScenes] handleRemoveScript not yet implemented')
}

// Helper to create a MikanVector3f with the given components
function makeVec3(x: number, y: number, z: number): MikanVector3f {
  const v = new MikanVector3f()
  v.x = x; v.y = y; v.z = z
  return v
}

// Compositor graph path handlers
async function invokeCompositorFunction(functionName: string) {
  const client = mikanStore.client as any
  if (!client) {
    console.error('[CompositorComponent] Cannot invoke function: no client connection')
    return
  }

  console.log(`[CompositorComponent] Invoking function "${functionName}" on CompositorComponent ${selectedCompositorId.value}`)
  await componentStore.invokeComponentFunction(
    client,
    'CompositorObjectSystem',
    selectedCompositorId.value,
    functionName
  )
}

function handleEditCompositorGraph() {
  invokeCompositorFunction('edit_compositor_graph')
}

function handleAddCompositorGraph() {
  invokeCompositorFunction('add_new_compositor_graph')
}

function handleRemoveCompositorGraph() {
  invokeCompositorFunction('remove_compositor_graph')
}

// Compositor CRUD handlers
async function handleAddCompositor() {
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  if (selectedSceneId.value === -1) { console.error('[ProjectScenes] No scene selected'); return }

  const initValues = new MikanCompositorComponentValues()
  initValues.owner_scene_id = selectedSceneId.value
  initValues.camera_id = MikanConstants.InvalidMikanID

  const initParams = new PolymorphicObject(initValues, 'MikanCompositorComponentValues')
  await componentStore.createObject(mikanStore.client as MikanClient, 'CompositorComponent', initParams)
}

async function handleRemoveCompositor() {
  if (selectedCompositorId.value === -1) { console.error('[ProjectScenes] No compositor selected'); return }
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'CompositorComponent', selectedCompositorId.value)
}

// Actor CRUD handlers
async function handleAddAnchor() {
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  if (selectedSceneId.value === -1) { console.error('[ProjectScenes] No scene selected'); return }

  const initValues = new MikanAnchorComponentValues()
  initValues.relative_scale = makeVec3(1, 1, 1)
  initValues.parent_transform_id = newActorParentTransformId.value

  const initParams = new PolymorphicObject(initValues, 'MikanAnchorComponentValues')
  await componentStore.createObject(mikanStore.client as MikanClient, 'AnchorComponent', initParams)
}

async function handleRemoveAnchor() {
  if (!selectedSceneObject.value) { console.error('[ProjectScenes] No scene object selected'); return }
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'AnchorComponent', selectedSceneObject.value.componentId)
  selectedSceneObjectId.value = -1
}

async function handleAddQuadStencil() {
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }

  const initValues = new MikanQuadStencilComponentValues()
  initValues.relative_scale = makeVec3(1, 1, 1)
  initValues.parent_transform_id = newActorParentTransformId.value
  initValues.quad_width = 0.5
  initValues.quad_height = 0.5

  const initParams = new PolymorphicObject(initValues, 'MikanQuadStencilComponentValues')
  await componentStore.createObject(mikanStore.client as MikanClient, 'QuadStencilComponent', initParams)
}

async function handleRemoveQuadStencil() {
  if (!selectedSceneObject.value) { console.error('[ProjectScenes] No scene object selected'); return }
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'QuadStencilComponent', selectedSceneObject.value.componentId)
  selectedSceneObjectId.value = -1
}

async function handleAddBoxStencil() {
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }

  const initValues = new MikanBoxStencilComponentValues()
  initValues.relative_scale = makeVec3(1, 1, 1)
  initValues.parent_transform_id = newActorParentTransformId.value
  initValues.box_x_size = 0.5
  initValues.box_y_size = 0.5
  initValues.box_z_size = 0.5

  const initParams = new PolymorphicObject(initValues, 'MikanBoxStencilComponentValues')
  await componentStore.createObject(mikanStore.client as MikanClient, 'BoxStencilComponent', initParams)
}

async function handleRemoveBoxStencil() {
  if (!selectedSceneObject.value) { console.error('[ProjectScenes] No scene object selected'); return }
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'BoxStencilComponent', selectedSceneObject.value.componentId)
  selectedSceneObjectId.value = -1
}

async function handleAddModelStencil() {
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }

  const initValues = new MikanModelStencilComponentValues()
  initValues.relative_scale = makeVec3(1, 1, 1)
  initValues.parent_transform_id = newActorParentTransformId.value

  const initParams = new PolymorphicObject(initValues, 'MikanModelStencilComponentValues')
  await componentStore.createObject(mikanStore.client as MikanClient, 'ModelStencilComponent', initParams)
}

async function handleRemoveModelStencil() {
  if (!selectedSceneObject.value) { console.error('[ProjectScenes] No scene object selected'); return }
  if (!mikanStore.client) { console.error('[ProjectScenes] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'ModelStencilComponent', selectedSceneObject.value.componentId)
  selectedSceneObjectId.value = -1
}

// Initialize on mount
onMounted(() => {
  restoreSelectionState()

  if (selectedSceneId.value === -1) {
    initializeFromCurrentScene()
  }
})
</script>

<style scoped>
.project-panel {
  max-width: 1200px;
}

.project-panel h2 {
  color: #5cb85c;
  margin-bottom: 10px;
}

.project-panel h3 {
  color: #ffffff;
  margin: 0 0 16px 0;
  font-size: 18px;
  font-weight: 600;
}

.project-panel p {
  color: #b0b0b0;
  margin-bottom: 20px;
}

.scenes-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.selection-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
}

.selection-row {
  display: flex;
  align-items: center;
  gap: 12px;
}

.selection-label {
  color: #ffffff;
  font-size: 16px;
  font-weight: 600;
  min-width: 80px;
}

.selection-dropdown {
  flex: 1;
  max-width: 300px;
}

.component-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
}

.section-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 16px;
}

.section-header h3 {
  margin: 0;
}

.property-grid {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.property-row {
  display: flex;
  align-items: center;
  gap: 12px;
}

.property-label {
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
  min-width: 100px;
  font-size: 14px;
}

.script-controls {
  display: flex;
  align-items: center;
  gap: 8px;
  flex: 1;
}

.script-path {
  font-family: monospace;
  color: #fff;
  font-size: 14px;
  flex: 1;
}

.actor-buttons {
  display: flex;
  gap: 8px;
  margin-bottom: 16px;
}

.scene-outliner {
  list-style: none;
  margin: 0;
  padding: 0;
  border: 1px solid #404040;
  border-radius: 4px;
  background: rgba(0, 0, 0, 0.2);
  max-height: 400px;
  overflow-y: auto;
}

.outliner-item {
  padding: 6px 12px 6px 12px; /* left overridden per-item by depth-based inline style */
  color: #fff;
  cursor: pointer;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
  transition: background-color 0.2s;
  font-size: 13px;
  text-align: left; 
}

.tree-prefix {
  color: rgba(255, 255, 255, 0.35);
  user-select: none;
}

.outliner-item:hover {
  background-color: rgba(255, 255, 255, 0.05);
}

.outliner-item.selected {
  background-color: rgba(92, 184, 92, 0.3);
  color: #5cb85c;
  font-weight: 600;
}

.action-btn {
  padding: 8px 16px;
  font-size: 14px;
  background-color: #5cb85c;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: background-color 0.2s;
  font-weight: 500;
  display: inline-flex;
  align-items: center;
  gap: 6px;
}

.action-btn:hover:not(:disabled) {
  background-color: #4cae4c;
}

.action-btn:disabled {
  background-color: #3d3d3d;
  color: #999;
  cursor: not-allowed;
}

.action-btn.add-btn {
  background-color: #5cb85c;
}

.action-btn.add-btn:hover {
  background-color: #4cae4c;
}

.action-btn.remove-btn {
  background-color: #d9534f;
}

.action-btn.remove-btn:hover {
  background-color: #c9302c;
}

.action-btn.icon-btn {
  padding: 6px 12px;
  font-size: 13px;
}

/* Icon button styles now imported from common-buttons.css */

.scene-outliner::-webkit-scrollbar {
  width: 8px;
}

.scene-outliner::-webkit-scrollbar-track {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 4px;
}

.scene-outliner::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 4px;
}

.scene-outliner::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.3);
}
</style>
