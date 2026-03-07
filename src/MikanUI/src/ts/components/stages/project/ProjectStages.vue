<template>
  <div class="project-panel">
    <h2>Stages Panel</h2>
    <p>Select a stage, camera, and compositor to manage components.</p>

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
          <button @click="handleAddStage" class="action-btn add-btn" title="Add Stage">➕</button>
          <button
            v-if="selectedStageId !== -1"
            @click="handleRemoveStage"
            class="action-btn remove-btn"
            title="Remove Stage"
          >
            ➖
          </button>
        </div>
      </div>

      <!-- Selected Stage Component -->
      <div v-if="selectedStageId !== -1 && selectedStageComponent" class="component-section">
        <h3>Stage Component</h3>
        <ComponentCard
          :component-id="selectedStageId"
          :component="selectedStageComponent"
          owner-system="StageObjectSystem"
          :editable="true"
        />
      </div>      

      <!-- Camera Selection (only shown if stage is selected) -->
      <div v-if="selectedStageId !== -1" class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Camera:</label>
          <select v-model="selectedCameraId" class="selection-dropdown scene-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="camera in filteredCameras"
              :key="camera.component_id"
              :value="camera.component_id"
            >
              {{ camera.component_name }}
            </option>
          </select>
          <button @click="handleAddCamera" class="action-btn add-btn" title="Add Camera">➕</button>
          <button
            v-if="selectedCameraId !== -1"
            @click="handleRemoveCamera"
            class="action-btn remove-btn"
            title="Remove Camera"
          >
            ➖
          </button>
        </div>
      </div>

      <!-- Selected Camera Component -->
      <div v-if="selectedStageId !== -1 && selectedCameraId !== -1 && selectedCameraComponent" class="component-section">
        <h3>Camera Component</h3>
        <ComponentCard
          :component-id="selectedCameraId"
          :component="selectedCameraComponent"
          owner-system="CameraObjectSystem"
          :editable="true"
        />
      </div>      

      <!-- Compositor Selection (only shown if stage is selected) -->
      <div v-if="selectedStageId !== -1" class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Compositor:</label>
          <select v-model="selectedCompositorId" class="selection-dropdown scene-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="compositor in filteredCompositors"
              :key="compositor.component_id"
              :value="compositor.component_id"
            >
              {{ compositor.component_name }}
            </option>
          </select>
          <button @click="handleAddCompositor" class="action-btn add-btn" title="Add Compositor">➕</button>
          <button
            v-if="selectedCompositorId !== -1"
            @click="handleRemoveCompositor"
            class="action-btn remove-btn"
            title="Remove Compositor"
          >
            ➖
          </button>
        </div>
      </div>

      <!-- Selected Compositor Component -->
      <div v-if="selectedStageId !== -1 && selectedCompositorId !== -1 && selectedCompositorComponent" class="component-section">
        <h3>Compositor Component</h3>
        <ComponentCard
          :component-id="selectedCompositorId"
          :component="selectedCompositorComponent"
          owner-system="CompositorObjectSystem"
          :editable="true"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
import ComponentRefSelect from '../../shared/ComponentRefSelect.vue'
import ComponentCard from '../../shared/ComponentCard.vue'

const componentStore = useComponentStore()
const { sendRemoteControlCommand } = useRemoteControl()

// Selection state
const selectedStageId = ref<number>(-1)
const selectedCameraId = ref<number>(-1)
const selectedCompositorId = ref<number>(-1)

// Get components by system
const stageComponents = computed(() =>
  componentStore.getComponentsByClass('StageComponent')
)

const cameraComponents = computed(() =>
  componentStore.getComponentsByClass('CameraComponent')
)

const compositorComponents = computed(() =>
  componentStore.getComponentsByClass('CompositorComponent')
)

// Filter cameras by the selected stage
const filteredCameras = computed(() => {
  if (selectedStageId.value === -1) return []

  return cameraComponents.value.filter(camera => {
    const cameraComp = camera as any
    return cameraComp.stage_id === selectedStageId.value
  })
})

// Filter compositors by the selected stage
const filteredCompositors = computed(() => {
  if (selectedStageId.value === -1) return []

  return compositorComponents.value.filter(compositor => {
    const compositorComp = compositor as any
    return compositorComp.stage_id === selectedStageId.value
  })
})

// Get the selected components
const selectedStageComponent = computed(() => {
  if (selectedStageId.value === -1) return null
  return componentStore.getComponent(selectedStageId.value)
})

const selectedCameraComponent = computed(() => {
  if (selectedCameraId.value === -1) return null
  return componentStore.getComponent(selectedCameraId.value)
})

const selectedCompositorComponent = computed(() => {
  if (selectedCompositorId.value === -1) return null
  return componentStore.getComponent(selectedCompositorId.value)
})

// Initialize selected stage - just select the first stage if available
function initializeFromCurrentScene() {
  console.log('[ProjectStages] Initializing stage selection...')

  if (stageComponents.value.length > 0) {
    const firstStage = stageComponents.value[0] as any
    console.log('[ProjectStages] Setting stage ID to first stage:', firstStage.component_id)
    selectedStageId.value = firstStage.component_id
  } else {
    console.log('[ProjectStages] No stages available')
  }
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
  selectedCameraId.value = -1
  selectedCompositorId.value = -1
}

// Camera CRUD handlers
function handleAddCamera() {
  sendRemoteControlCommand('add_new_camera')
}

function handleRemoveCamera() {
  if (selectedCameraId.value === -1) {
    console.error('[ProjectStages] No camera selected')
    return
  }
  sendRemoteControlCommand('remove_camera', [selectedCameraId.value.toString()])
  selectedCameraId.value = -1
}

// Compositor CRUD handlers
function handleAddCompositor() {
  sendRemoteControlCommand('add_new_compositor')
}

function handleRemoveCompositor() {
  if (selectedCompositorId.value === -1) {
    console.error('[ProjectStages] No compositor selected')
    return
  }
  sendRemoteControlCommand('remove_compositor', [selectedCompositorId.value.toString()])
  selectedCompositorId.value = -1
}

// Watch for stage changes - clear camera and compositor if they don't belong to the new stage
watch(selectedStageId, (newStageId, oldStageId) => {
  console.log('[ProjectStages] Stage changed:', oldStageId, '->', newStageId)

  if (newStageId === -1) {
    // No stage selected, clear camera and compositor
    selectedCameraId.value = -1
    selectedCompositorId.value = -1
  } else if (oldStageId !== newStageId) {
    // Stage changed, check if current camera/compositor belong to new stage
    if (selectedCameraId.value !== -1) {
      const camera = componentStore.getComponent(selectedCameraId.value) as any
      if (!camera || camera.stage_id !== newStageId) {
        console.log('[ProjectStages] Clearing camera - does not belong to new stage')
        selectedCameraId.value = -1
      }
    }

    if (selectedCompositorId.value !== -1) {
      const compositor = componentStore.getComponent(selectedCompositorId.value) as any
      if (!compositor || compositor.stage_id !== newStageId) {
        console.log('[ProjectStages] Clearing compositor - does not belong to new stage')
        selectedCompositorId.value = -1
      }
    }
  }
})

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

watch(selectedCameraId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectStages.selectedCameraId', newValue.toString())
  }
})

watch(selectedCompositorId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectStages.selectedCompositorId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedStageId = sessionStorage.getItem('projectStages.selectedStageId')
  const savedCameraId = sessionStorage.getItem('projectStages.selectedCameraId')
  const savedCompositorId = sessionStorage.getItem('projectStages.selectedCompositorId')

  console.log('[ProjectStages] Restoring selection state:', { savedStageId, savedCameraId, savedCompositorId })

  // Restore stage first
  if (savedStageId) {
    const stageId = parseInt(savedStageId, 10)
    if (!isNaN(stageId)) {
      selectedStageId.value = stageId
    }
  }

  // Only restore camera if we have a stage selected
  if (selectedStageId.value !== -1 && savedCameraId) {
    const cameraId = parseInt(savedCameraId, 10)
    if (!isNaN(cameraId)) {
      selectedCameraId.value = cameraId
    }
  }

  // Only restore compositor if we have a stage selected
  if (selectedStageId.value !== -1 && savedCompositorId) {
    const compositorId = parseInt(savedCompositorId, 10)
    if (!isNaN(compositorId)) {
      selectedCompositorId.value = compositorId
    }
  }

  console.log('[ProjectStages] Restored selections:', {
    stage: selectedStageId.value,
    camera: selectedCameraId.value,
    compositor: selectedCompositorId.value
  })
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
  padding: 12px;
}

.selection-row {
  display: flex;
  align-items: center;
  gap: 8px;
  flex-wrap: nowrap;
}

.selection-label {
  color: #ffffff;
  font-size: 16px;
  font-weight: 600;
  min-width: 80px;
  flex-shrink: 0;
}

.selection-dropdown {
  flex: 1;
  min-width: 150px;
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
  padding: 6px 12px;
  font-size: 18px;
  background-color: #5cb85c;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  transition: background-color 0.2s;
  font-weight: 500;
  flex-shrink: 0;
  min-width: 40px;
  line-height: 1;
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
</style>
