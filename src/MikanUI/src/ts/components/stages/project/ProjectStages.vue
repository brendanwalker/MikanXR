<template>
  <div class="project-panel">
    <h2>Stages Panel</h2>
    <p>Select a stage and camera to manage components.</p>

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
          <button @click="handleAddStage" class="icon-only-btn add-btn" title="Add Stage">
            <img src="/images/add_component_normal_icon.png" alt="Add Stage" class="btn-icon-only" />
          </button>
          <button
            v-if="selectedStageId !== -1"
            @click="handleRemoveStage"
            class="icon-only-btn remove-btn"
            title="Remove Stage"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove Stage" class="btn-icon-only" />
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
          <button @click="handleAddCamera" class="icon-only-btn add-btn" title="Add Camera">
            <img src="/images/add_component_normal_icon.png" alt="Add Camera" class="btn-icon-only" />
          </button>
          <button
            v-if="selectedCameraId !== -1"
            @click="handleRemoveCamera"
            class="icon-only-btn remove-btn"
            title="Remove Camera"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove Camera" class="btn-icon-only" />
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
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import ComponentRefSelect from '../../shared/ComponentRefSelect.vue'
import ComponentCard from '../../shared/ComponentCard.vue'
import {
  MikanClient,
  MikanConstants,
  PolymorphicObject,
  MikanVector3f,
  MikanQuatd,
  MikanVector3d,
  MikanCameraComponentValues,
  CLASS_ID_MIKAN_CAMERA_COMPONENT_VALUES
} from '@mikanxr/client'

const componentStore = useComponentStore()
const mikanStore = useMikanStore()

// Selection state
const selectedStageId = ref<number>(-1)
const selectedCameraId = ref<number>(-1)

// Get components by system
const cameraComponents = computed(() =>
  componentStore.getComponentsByClass('CameraComponent')
)

// Filter cameras by the selected stage
const filteredCameras = computed(() => {
  if (selectedStageId.value === -1) return []

  return cameraComponents.value.filter(camera => {
    const cameraComp = camera as any
    return cameraComp.stage_id === selectedStageId.value
  })
})

// Get the selected components
const selectedStageComponent = computed(() => {
  if (selectedStageId.value === -1) return null
  return componentStore.getStageComponent(selectedStageId.value)
})

const selectedCameraComponent = computed(() => {
  if (selectedCameraId.value === -1) return null
  return componentStore.getCameraComponent(selectedCameraId.value)
})

// Initialize selected stage - just select the first stage if available
function initializeFromCurrentScene() {
  console.log('[ProjectStages] Initializing stage selection...')

  const stageComponents = componentStore.getComponentsByClass('StageComponent')
  if (stageComponents.length > 0) {
    const firstStage = stageComponents[0] as any
    console.log('[ProjectStages] Setting stage ID to first stage:', firstStage.component_id)
    selectedStageId.value = firstStage.component_id
  } else {
    console.log('[ProjectStages] No stages available')
  }
}

// Stage CRUD handlers
async function handleAddStage() {
  if (!mikanStore.client) { console.error('[ProjectStages] No client connection'); return }
  await componentStore.createObject(mikanStore.client as MikanClient, 'StageComponent')
}

async function handleRemoveStage() {
  if (selectedStageId.value === -1) { console.error('[ProjectStages] No stage selected'); return }
  if (!mikanStore.client) { console.error('[ProjectStages] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'StageComponent', selectedStageId.value)
  selectedStageId.value = -1
  selectedCameraId.value = -1
}

// Camera CRUD handlers
async function handleAddCamera() {
  if (!mikanStore.client) { console.error('[ProjectStages] No client connection'); return }

  const defaultScale = new MikanVector3f(); defaultScale.x = 1; defaultScale.y = 1; defaultScale.z = 1
  const identityQuat = new MikanQuatd(); identityQuat.w = 1; identityQuat.x = 0; identityQuat.y = 0; identityQuat.z = 0

  const initValues = new MikanCameraComponentValues()
  initValues.relative_scale = defaultScale
  initValues.parent_transform_id = MikanConstants.InvalidMikanID
  initValues.stage_id = selectedStageId.value
  initValues.tracking_mount_id = MikanConstants.InvalidMikanID
  initValues.video_source_id = MikanConstants.InvalidMikanID
  initValues.aperture_orientation_offset = identityQuat

  const initParams = new PolymorphicObject(initValues, CLASS_ID_MIKAN_CAMERA_COMPONENT_VALUES, 'MikanCameraComponentValues')
  await componentStore.createObject(mikanStore.client as MikanClient, 'CameraComponent', initParams)
}

async function handleRemoveCamera() {
  if (selectedCameraId.value === -1) { console.error('[ProjectStages] No camera selected'); return }
  if (!mikanStore.client) { console.error('[ProjectStages] No client connection'); return }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'CameraComponent', selectedCameraId.value)
  selectedCameraId.value = -1
}

// Watch for stage changes - clear camera if it doesn't belong to the new stage
watch(selectedStageId, (newStageId, oldStageId) => {
  console.log('[ProjectStages] Stage changed:', oldStageId, '->', newStageId)

  if (newStageId === -1) {
    selectedCameraId.value = -1
  } else if (oldStageId !== newStageId) {
    if (selectedCameraId.value !== -1) {
      const camera = componentStore.getCameraComponent(selectedCameraId.value) as any
      if (!camera || camera.stage_id !== newStageId) {
        console.log('[ProjectStages] Clearing camera - does not belong to new stage')
        selectedCameraId.value = -1
      }
    }
  }
})

// Watch for component store changes - reinitialize if we don't have a stage selected
watch(
  () => componentStore.components.size,
  (newSize, oldSize) => {
    console.log('[ProjectStages] Component store size changed:', oldSize, '->', newSize)
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

// Restore selection state on mount
function restoreSelectionState() {
  const savedStageId = sessionStorage.getItem('projectStages.selectedStageId')
  const savedCameraId = sessionStorage.getItem('projectStages.selectedCameraId')

  console.log('[ProjectStages] Restoring selection state:', { savedStageId, savedCameraId })

  if (savedStageId) {
    const stageId = parseInt(savedStageId, 10)
    if (!isNaN(stageId)) {
      selectedStageId.value = stageId
    }
  }

  if (selectedStageId.value !== -1 && savedCameraId) {
    const cameraId = parseInt(savedCameraId, 10)
    if (!isNaN(cameraId)) {
      selectedCameraId.value = cameraId
    }
  }

  console.log('[ProjectStages] Restored selections:', {
    stage: selectedStageId.value,
    camera: selectedCameraId.value
  })
}

// Initialize on mount
onMounted(() => {
  restoreSelectionState()

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

.component-section h3 {
  color: #ffffff;
  margin: 0 0 8px 0;
  font-size: 18px;
  font-weight: 600;
}

/* Icon button styles now imported from common-buttons.css */
</style>
