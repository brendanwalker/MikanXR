<template>
  <div class="project-panel">
    <h2>Stages Panel</h2>
    <p>Select a stage and scene to manage components.</p>

    <div class="stages-content">
      <!-- Stage Selection -->
      <div class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Stage:</label>
          <ComponentRefSelect
            v-model="selectedStageId"
            component-class="StageComponent"
            class="selection-dropdown"
          />
          <button @click="handleAddStage" class="action-btn add-btn">+ Add Stage</button>
          <button
            v-if="selectedStageId !== -1"
            @click="handleRemoveStage"
            class="action-btn remove-btn"
          >
            Remove Stage
          </button>
        </div>
      </div>

      <!-- Scene Selection (only shown if stage is selected) -->
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

      <!-- Stage Components List -->
      <div v-if="selectedStageId !== -1" class="component-section">
        <div class="section-header">
          <h3>Stage Components</h3>
        </div>
        <ComponentList
          :components="stageComponents"
          owner-system="StageObjectSystem"
          :selectable="true"
          :editable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent"
        />
      </div>

      <!-- Camera Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Camera Components</h3>
          <button @click="handleAddCamera" class="action-btn add-btn">+ Add Camera</button>
        </div>
        <ComponentList
          :components="cameraComponents"
          owner-system="CameraObjectSystem"
          :selectable="true"
          :editable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent"
        />
        <button
          v-if="selectedComponentId && isCameraSelected"
          @click="handleRemoveCamera"
          class="action-btn remove-btn"
        >
          Remove Selected Camera
        </button>
      </div>

      <!-- Compositor Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Compositor Components</h3>
          <button @click="handleAddCompositor" class="action-btn add-btn">+ Add Compositor</button>
        </div>
        <ComponentList
          :components="compositorComponents"
          owner-system="CompositorObjectSystem"
          :selectable="true"
          :editable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent"
        />
        <button
          v-if="selectedComponentId && isCompositorSelected"
          @click="handleRemoveCompositor"
          class="action-btn remove-btn"
        >
          Remove Selected Compositor
        </button>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
import ComponentList from '../../shared/ComponentList.vue'
import ComponentRefSelect from '../../shared/ComponentRefSelect.vue'
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
const selectedComponentId = ref<number | null>(null)

// Get components by system
const stageComponents = computed(() =>
  componentStore.getComponentsByClass('StageComponent')
)

const sceneComponents = computed(() =>
  componentStore.getComponentsByClass('SceneComponent')
)

const cameraComponents = computed(() =>
  componentStore.getComponentsByClass('CameraComponent')
)

const compositorComponents = computed(() =>
  componentStore.getComponentsByClass('CompositorComponent')
)

// Filter scenes by the selected stage
const filteredScenes = computed(() => {
  if (selectedStageId.value === -1) return []

  return sceneComponents.value.filter(scene => {
    const sceneComp = scene as any
    return sceneComp.parent_stage_id === selectedStageId.value
  })
})

// Initialize selected stage from current scene
async function initializeFromCurrentScene() {
  if (!mikanStore.client) {
    console.log('[ProjectStages] No client connection, skipping initialization')
    return
  }

  try {
    console.log('[ProjectStages] Fetching current scene ID...')

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
      console.log('[ProjectStages] Current scene ID from server:', currentSceneId)

      if (currentSceneId !== -1) {
        // Get the parent stage from the current scene
        const currentScene = componentStore.getComponent(currentSceneId) as any
        console.log('[ProjectStages] Current scene component:', currentScene)

        if (currentScene?.parent_stage_id) {
          console.log('[ProjectStages] Setting stage ID to:', currentScene.parent_stage_id)
          selectedStageId.value = currentScene.parent_stage_id
          console.log('[ProjectStages] Setting scene ID to:', currentSceneId)
          selectedSceneId.value = currentSceneId
        } else {
          console.warn('[ProjectStages] Current scene has no parent_stage_id')
        }
      } else {
        console.log('[ProjectStages] No current scene set on server')
      }
    } else {
      console.error('[ProjectStages] Failed to fetch current scene ID:', response.resultCode)
    }
  } catch (error) {
    console.error('[ProjectStages] Error fetching current scene:', error)
  }
}

// Handle component selection
function handleSelectComponent(componentId: number) {
  selectedComponentId.value = componentId
}

// Stage CRUD handlers
function handleAddStage() {
  sendRemoteControlCommand('add_new_stage')
}

function handleRemoveStage() {
  if (selectedStageId.value === -1) {
    console.error('[ProjectStages] No stage selected')
    return
  }
  sendRemoteControlCommand('remove_stage', [selectedStageId.value.toString()])
  selectedStageId.value = -1
}

// Scene CRUD handlers
function handleAddScene() {
  sendRemoteControlCommand('add_scene')
}

function handleRemoveScene() {
  if (selectedSceneId.value === -1) {
    console.error('[ProjectStages] No scene selected')
    return
  }
  sendRemoteControlCommand('remove_scene', [selectedSceneId.value.toString()])
  selectedSceneId.value = -1
}

// Camera CRUD handlers
function handleAddCamera() {
  sendRemoteControlCommand('add_new_camera')
}

function handleRemoveCamera() {
  if (!selectedComponentId.value) {
    console.error('[ProjectStages] No camera selected')
    return
  }
  sendRemoteControlCommand('remove_camera', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

// Compositor CRUD handlers
function handleAddCompositor() {
  sendRemoteControlCommand('add_new_compositor')
}

function handleRemoveCompositor() {
  if (!selectedComponentId.value) {
    console.error('[ProjectStages] No compositor selected')
    return
  }
  sendRemoteControlCommand('remove_compositor', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

// Watch for component store changes - reinitialize if we don't have a stage selected
watch(
  () => componentStore.components.size,
  (newSize, oldSize) => {
    console.log('[ProjectStages] Component store size changed:', oldSize, '->', newSize)
    // If components were just loaded and we don't have a stage selected, try to initialize
    if (newSize > 0 && selectedStageId.value === -1) {
      console.log('[ProjectStages] Components loaded, attempting initialization')
      initializeFromCurrentScene()
    }
  }
)

// Persist selection state
watch(selectedStageId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectStages.selectedStageId', newValue.toString())
  }
})

watch(selectedSceneId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectStages.selectedSceneId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedStageId = sessionStorage.getItem('projectStages.selectedStageId')
  const savedSceneId = sessionStorage.getItem('projectStages.selectedSceneId')

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

.project-panel p {
  color: #b0b0b0;
  margin-bottom: 20px;
}

.stages-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.selection-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 20px;
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
  padding: 20px;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 8px;
}

.section-header h3 {
  color: #ffffff;
  margin: 0;
  font-size: 18px;
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
  width: 100%;
  margin-top: 8px;
}

.action-btn.remove-btn:hover {
  background-color: #c9302c;
}
</style>
