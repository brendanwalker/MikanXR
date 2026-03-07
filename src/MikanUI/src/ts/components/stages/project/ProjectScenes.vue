<template>
  <div class="project-panel">
    <h2>Scenes Panel</h2>
    <p>Select a stage and scene to manage actors.</p>

    <div class="scenes-content">
      <!-- Stage Selection (no add/delete) -->
      <div class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Stage:</label>
          <ComponentRefSelect
            v-model="selectedStageId"
            component-class="StageComponent"
            class="selection-dropdown"
          />
        </div>
      </div>

      <!-- Scene Selection (with add/delete) -->
      <div v-if="selectedStageId !== -1" class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Scene:</label>
          <select v-model="selectedSceneId" class="selection-dropdown scene-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="scene in filteredScenes"
              :key="scene.component_id"
              :value="scene.component_id"
            >
              {{ scene.component_name }}
            </option>
          </select>
          <button @click="handleAddScene" class="action-btn add-btn">+ Add Scene</button>
          <button
            v-if="selectedSceneId !== -1"
            @click="handleRemoveScene"
            class="action-btn remove-btn"
          >
            Remove Scene
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
            <label class="property-label">Compositor:</label>
            <PropertyField
              field-name="display_compositor_id"
              :field-value="selectedSceneComponent.display_compositor_id"
              owner-system="SceneObjectSystem"
              :component-id="selectedSceneId"
            />
          </div>
          <div class="property-row">
            <label class="property-label">Script:</label>
            <div class="script-controls">
              <span class="script-path">{{ selectedSceneComponent.component_script || '&lt;None&gt;' }}</span>
              <button @click="handleReloadScript" class="action-btn icon-btn" title="Reload Script">
                ⟳
              </button>
              <button @click="handleAddScript" class="action-btn icon-btn" title="Add Script">
                +
              </button>
              <button @click="handleRemoveScript" class="action-btn icon-btn" title="Remove Script">
                ×
              </button>
            </div>
          </div>
        </div>
      </div>

      <!-- Actors Panel -->
      <div v-if="selectedSceneId !== -1" class="component-section">
        <h3>Actors</h3>
        <div class="actor-buttons">
          <button @click="handleAddAnchor" class="action-btn icon-btn" title="Add Anchor">
            ⚓ Anchor
          </button>
          <button @click="handleAddQuadStencil" class="action-btn icon-btn" title="Add Quad Stencil">
            ▭ Quad
          </button>
          <button @click="handleAddBoxStencil" class="action-btn icon-btn" title="Add Box Stencil">
            ▢ Box
          </button>
          <button @click="handleAddModelStencil" class="action-btn icon-btn" title="Add Model Stencil">
            ⬡ Model
          </button>
        </div>

        <!-- Scene Outliner Tree -->
        <div class="scene-outliner">
          <div
            v-for="(item, index) in sceneOutliner"
            :key="index"
            class="outliner-item"
            :class="{ selected: selectedSceneObjectIndex === index }"
            :style="{ paddingLeft: `${item.depth * 25}px` }"
            @click="handleSelectSceneObject(index)"
          >
            {{ item.name }}
          </div>
        </div>
      </div>

      <!-- Selected Scene Object Properties -->
      <div v-if="selectedSceneObject" class="component-section">
        <h3>{{ selectedSceneObject.componentType }} Properties</h3>
        <ComponentCard
          :component-id="selectedSceneObject.componentId"
          :component="selectedSceneObject.component"
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
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
import ComponentRefSelect from '../../shared/ComponentRefSelect.vue'
import PropertyField from '../../shared/PropertyField.vue'
import ComponentCard from '../../shared/ComponentCard.vue'
import {
  MikanAPIResult,
  MikanConstants,
  PropertyGetValueRequest,
  CLASS_ID_PROPERTY_GET_VALUE_REQUEST,
  PropertyGetValueResponse
} from '@mikanxr/client'

const componentStore = useComponentStore()
const mikanStore = useMikanStore()
const { sendRemoteControlCommand } = useRemoteControl()

// Selection state
const selectedStageId = ref<number>(-1)
const selectedSceneId = ref<number>(-1)
const selectedSceneObjectIndex = ref<number>(-1)

interface SceneOutlinerItem {
  name: string
  depth: number
  componentId: number
  ownerSystem: string
  componentType: string
}

const sceneOutliner = ref<SceneOutlinerItem[]>([])

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

// Filter scenes by the selected stage
const filteredScenes = computed(() => {
  if (selectedStageId.value === -1) return []

  return sceneComponents.value.filter(scene => {
    const sceneComp = scene as any
    return sceneComp.parent_stage_id === selectedStageId.value
  })
})

// Get the selected scene component
const selectedSceneComponent = computed(() => {
  if (selectedSceneId.value === -1) return null
  return componentStore.getComponent(selectedSceneId.value) as any
})

// Get the selected scene object
const selectedSceneObject = computed(() => {
  if (selectedSceneObjectIndex.value === -1 || selectedSceneObjectIndex.value >= sceneOutliner.value.length) {
    return null
  }

  const item = sceneOutliner.value[selectedSceneObjectIndex.value]
  const component = componentStore.getComponent(item.componentId)

  return {
    componentId: item.componentId,
    component: component,
    ownerSystem: item.ownerSystem,
    componentType: item.componentType
  }
})

// Build the scene outliner tree
function buildSceneOutliner() {
  sceneOutliner.value = []

  if (selectedSceneId.value === -1) return

  // Helper to recursively add components
  function addComponentWithChildren(componentId: number, ownerSystem: string, componentType: string, depth: number) {
    const component = componentStore.getComponent(componentId) as any
    if (!component) return

    // Add this component to the outliner
    sceneOutliner.value.push({
      name: component.component_name || '<Unnamed>',
      depth,
      componentId,
      ownerSystem,
      componentType
    })

    // TODO: Add children if this component has them (for hierarchical transforms)
  }

  // Add all root anchors
  anchorComponents.value.forEach(anchor => {
    const anchorComp = anchor as any
    // TODO: Check if this anchor has no parent (is root)
    // For now, add all anchors at root level
    addComponentWithChildren(anchorComp.component_id, 'AnchorObjectSystem', 'Anchor', 0)
  })

  // Add all root quad stencils
  quadStencilComponents.value.forEach(quad => {
    const quadComp = quad as any
    // TODO: Check parent_anchor_id to determine if this is a root or child
    addComponentWithChildren(quadComp.component_id, 'QuadStencilSystem', 'Quad Stencil', 0)
  })

  // Add all root box stencils
  boxStencilComponents.value.forEach(box => {
    const boxComp = box as any
    addComponentWithChildren(boxComp.component_id, 'BoxStencilSystem', 'Box Stencil', 0)
  })

  // Add all root model stencils
  modelStencilComponents.value.forEach(model => {
    const modelComp = model as any
    addComponentWithChildren(modelComp.component_id, 'ModelStencilSystem', 'Model Stencil', 0)
  })
}

// Initialize selected stage from current scene
async function initializeFromCurrentScene() {
  if (!mikanStore.client) {
    console.log('[ProjectScenes] No client connection, skipping initialization')
    return
  }

  try {
    console.log('[ProjectScenes] Fetching current scene ID...')

    // Fetch current_scene_id from SceneObjectSystem
    const request: PropertyGetValueRequest = {
      requestTypeId: CLASS_ID_PROPERTY_GET_VALUE_REQUEST,
      requestTypeName: 'PropertyGetValueRequest',
      requestId: 0,
      ownerSystem: 'SceneObjectSystem',
      componentId: MikanConstants.InvalidMikanID, // System property
      fieldName: 'current_scene_id'
    }

    const future = mikanStore.client.sendRequest(request)
    const response = await future.await() as PropertyGetValueResponse

    if (response.resultCode === MikanAPIResult.Success) {
      const currentSceneId = response.propertyValue?.fieldValue?.value_ptr?.instance?.value || -1
      console.log('[ProjectScenes] Current scene ID from server:', currentSceneId)

      if (currentSceneId !== -1) {
        // Get the parent stage from the current scene
        const currentScene = componentStore.getComponent(currentSceneId) as any
        console.log('[ProjectScenes] Current scene component:', currentScene)

        if (currentScene?.parent_stage_id) {
          console.log('[ProjectScenes] Setting stage ID to:', currentScene.parent_stage_id)
          selectedStageId.value = currentScene.parent_stage_id
          console.log('[ProjectScenes] Setting scene ID to:', currentSceneId)
          selectedSceneId.value = currentSceneId
        } else {
          console.warn('[ProjectScenes] Current scene has no parent_stage_id')
        }
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

// Watch for scene changes to rebuild outliner
watch(selectedSceneId, () => {
  buildSceneOutliner()
  selectedSceneObjectIndex.value = -1
})

// Watch for component store changes - reinitialize if we don't have a stage selected
watch(
  () => componentStore.components.size,
  (newSize, oldSize) => {
    console.log('[ProjectScenes] Component store size changed:', oldSize, '->', newSize)
    // If components were just loaded and we don't have a stage selected, try to initialize
    if (newSize > 0 && selectedStageId.value === -1) {
      console.log('[ProjectScenes] Components loaded, attempting initialization')
      initializeFromCurrentScene()
    }
  }
)

// Persist selection state
watch(selectedStageId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectScenes.selectedStageId', newValue.toString())
  }
})

watch(selectedSceneId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectScenes.selectedSceneId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedStageId = sessionStorage.getItem('projectScenes.selectedStageId')
  const savedSceneId = sessionStorage.getItem('projectScenes.selectedSceneId')

  if (savedStageId) {
    const stageId = parseInt(savedStageId, 10)
    if (!isNaN(stageId)) {
      selectedStageId.value = stageId
    }
  }

  if (savedSceneId) {
    const sceneId = parseInt(savedSceneId, 10)
    if (!isNaN(sceneId)) {
      selectedSceneId.value = sceneId
    }
  }
}

// Handle scene object selection
function handleSelectSceneObject(index: number) {
  selectedSceneObjectIndex.value = index
}

// Scene CRUD handlers
function handleAddScene() {
  sendRemoteControlCommand('add_scene')
}

function handleRemoveScene() {
  if (selectedSceneId.value === -1) {
    console.error('[ProjectScenes] No scene selected')
    return
  }
  sendRemoteControlCommand('remove_scene', [selectedSceneId.value.toString()])
  selectedSceneId.value = -1
}

// Script handlers
function handleReloadScript() {
  sendRemoteControlCommand('reload_script')
}

function handleAddScript() {
  sendRemoteControlCommand('add_new_script')
}

function handleRemoveScript() {
  sendRemoteControlCommand('remove_script')
}

// Actor CRUD handlers
function handleAddAnchor() {
  sendRemoteControlCommand('add_new_anchor')
}

function handleAddQuadStencil() {
  sendRemoteControlCommand('add_new_quad')
}

function handleAddBoxStencil() {
  sendRemoteControlCommand('add_new_box')
}

function handleAddModelStencil() {
  sendRemoteControlCommand('add_new_model')
}

// Initialize on mount
onMounted(() => {
  // First try to restore previous selection state
  restoreSelectionState()

  // If no saved state, try to initialize from current scene
  if (selectedStageId.value === -1) {
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

.scene-select {
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  color: #fff;
  font-family: monospace;
  font-size: 14px;
  cursor: pointer;
}

.scene-select:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(0, 0, 0, 0.4);
}

.scene-select option {
  background: #2d2d2d;
  color: #fff;
}

.component-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
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
  border: 1px solid #404040;
  border-radius: 4px;
  background: rgba(0, 0, 0, 0.2);
  max-height: 400px;
  overflow-y: auto;
}

.outliner-item {
  padding: 8px 12px;
  color: #fff;
  cursor: pointer;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
  transition: background-color 0.2s;
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
