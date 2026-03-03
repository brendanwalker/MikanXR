<template>
  <div class="project-panel">
    <h2>Stages Panel</h2>
    <p>Edit component properties inline by modifying the values directly.</p>

    <div class="stages-content">
      <!-- Stage Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Stage Components</h3>
          <button @click="handleAddStage" class="action-btn add-btn">+ Add Stage</button>
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
        <button
          v-if="selectedComponentId && isStageSelected"
          @click="handleRemoveStage"
          class="action-btn remove-btn"
        >
          Remove Selected Stage
        </button>
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
import { ref, computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
import ComponentList from '../../shared/ComponentList.vue'

const componentStore = useComponentStore()
const { sendRemoteControlCommand } = useRemoteControl()

// Selection state
const selectedComponentId = ref<number | null>(null)

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

// Check which type of component is selected
const isStageSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return (component as any)?.component_class === 'StageComponent'
})

const isCameraSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return (component as any)?.component_class === 'CameraComponent'
})

const isCompositorSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return (component as any)?.component_class === 'CompositorComponent'
})

// Handle component selection
function handleSelectComponent(componentId: number) {
  selectedComponentId.value = componentId
}

// Stage CRUD handlers
function handleAddStage() {
  sendRemoteControlCommand('add_new_stage')
}

function handleRemoveStage() {
  if (!selectedComponentId.value || !isStageSelected.value) {
    console.error('[ProjectStages] No stage selected')
    return
  }
  sendRemoteControlCommand('remove_stage', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

// Camera CRUD handlers
function handleAddCamera() {
  sendRemoteControlCommand('add_new_camera')
}

function handleRemoveCamera() {
  if (!selectedComponentId.value || !isCameraSelected.value) {
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
  if (!selectedComponentId.value || !isCompositorSelected.value) {
    console.error('[ProjectStages] No compositor selected')
    return
  }
  sendRemoteControlCommand('remove_compositor', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}
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
