<template>
  <div class="project-panel">
    <h2>Sources Panel</h2>
    <p>Click on a camera or tracking volume to edit its properties.</p>

    <div class="sources-content">
      <!-- Property Editor Modal -->
      <div v-if="editingComponent" class="editor-overlay">
        <PropertyEditor
          :title="editingComponent.component.component_name || 'Component'"
          :component-id="editingComponent.id"
          :component="editingComponent.component"
          :owner-system="editingComponent.system"
          @close="closeEditor"
          @save="handleSaveProperties"
        />
      </div>

      <!-- Camera Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Camera Components</h3>
        </div>
        <ComponentList
          :components="cameraComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'CameraObjectSystem')"
        />
        <button
          v-if="selectedComponentId && isCameraSelected"
          @click="handleCalibrateIntrinsics"
          class="action-btn"
        >
          Calibrate Intrinsics
        </button>
      </div>

      <!-- Video Source Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Video Source Components</h3>
          <div class="section-actions">
            <button @click="handleAddUSBVideoSource" class="action-btn add-btn">+ USB Video</button>
            <button @click="handleAddNetworkVideoSource" class="action-btn add-btn">+ Network Video</button>
          </div>
        </div>
        <ComponentList
          :components="videoSourceComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'VideoSourceObjectSystem')"
        />
        <button
          v-if="selectedComponentId && isUSBVideoSourceSelected"
          @click="handleRemoveVideoSource"
          class="action-btn remove-btn"
        >
          Remove USB Video Source
        </button>
        <button
          v-if="selectedComponentId && isNetworkVideoSourceSelected"
          @click="handleRemoveVideoSource"
          class="action-btn remove-btn"
        >
          Remove Network Video Source
        </button>
      </div>

      <!-- Texture Source Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Texture Source Components</h3>
          <div class="section-actions">
            <button @click="handleAddClientTextureSource" class="action-btn add-btn">+ Client Texture</button>
            <button @click="handleAddSpoutTextureSource" class="action-btn add-btn">+ Spout Texture</button>
          </div>
        </div>
        <ComponentList
          :components="textureSourceComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'TextureSourceObjectSystem')"
        />
        <button
          v-if="selectedComponentId && isClientTextureSourceSelected"
          @click="handleRemoveTextureSource"
          class="action-btn remove-btn"
        >
          Remove Client Texture Source
        </button>
        <button
          v-if="selectedComponentId && isSpoutTextureSourceSelected"
          @click="handleRemoveTextureSource"
          class="action-btn remove-btn"
        >
          Remove Spout Texture Source
        </button>
      </div>

      <!-- Compositor Components -->
      <ComponentList
        title="Compositor Components"
        :components="compositorComponents"
        :selectable="true"
        :selected-component-id="selectedComponentId"
        sort-by="name"
        @select="handleSelectComponent($event, 'CompositorObjectSystem')"
      />

      <!-- Tracking Volume Components -->
      <div class="tracking-volumes-section">
        <ComponentList
          v-if="markerTrackingVolumes.length > 0"
          title="Marker Tracking Volumes"
          :components="markerTrackingVolumes"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'MarkerTrackingVolumeObjectSystem')"
        />
        <ComponentList
          v-if="vrTrackingVolumes.length > 0"
          title="VR Tracking Volumes"
          :components="vrTrackingVolumes"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'VRTrackingVolumeObjectSystem')"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import { usePropertyEditor } from '../../../composables/usePropertyEditor.js'
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
import ComponentList from '../../shared/ComponentList.vue'
import PropertyEditor from '../../shared/PropertyEditor.vue'
import {
  MikanAPIResult,
  PropertySetValueRequest,
  CLASS_ID_PROPERTY_SET_VALUE_REQUEST
} from '@mikanxr/client'

const componentStore = useComponentStore()
const mikanStore = useMikanStore()
const { createVariantFromValue } = usePropertyEditor()
const { sendRemoteControlCommand } = useRemoteControl()

// Selection state
const selectedComponentId = ref<number | null>(null)
const editingComponent = ref<{
  id: number
  component: any
  system: string
} | null>(null)

// Get components by system
const cameraComponents = computed(() =>
  componentStore.getComponentsByClass('CameraComponent')
)

const videoSourceComponents = computed(() =>
  componentStore.getComponentsByClass('VideoSourceComponent')
)

const textureSourceComponents = computed(() =>
  componentStore.getComponentsByClass('TextureSourceComponent')
)

const compositorComponents = computed(() =>
  componentStore.getComponentsByClass('CompositorComponent')
)

const markerTrackingVolumes = computed(() =>
  componentStore.getComponentsByClass('MarkerTrackingVolumeComponent')
)

const vrTrackingVolumes = computed(() =>
  componentStore.getComponentsByClass('VRTrackingVolumeComponent')
)

// Check which type of component is selected
const isCameraSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'CameraComponent'
})

const isVideoSourceSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'USBVideoSourceComponent' ||
         component?.component_class === 'NetworkVideoSourceComponent'
})

const isUSBVideoSourceSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'USBVideoSourceComponent'
})

const isNetworkVideoSourceSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'NetworkVideoSourceComponent'
})

const isTextureSourceSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'ClientTextureSourceComponent' ||
         component?.component_class === 'SpoutTextureSourceComponent'
})

const isClientTextureSourceSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'ClientTextureSourceComponent'
})

const isSpoutTextureSourceSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return component?.component_class === 'SpoutTextureSourceComponent'
})

// Handle component selection
function handleSelectComponent(componentId: number, ownerSystem: string) {
  selectedComponentId.value = componentId
  const component = componentStore.getComponent(componentId)

  if (component) {
    editingComponent.value = {
      id: componentId,
      component,
      system: ownerSystem
    }
  }
}

function closeEditor() {
  editingComponent.value = null
  selectedComponentId.value = null
}

// Handle property save
async function handleSaveProperties(changes: Record<string, any>) {
  if (!editingComponent.value || !mikanStore.client) {
    console.error('[ProjectSources] Cannot save: no component selected or not connected')
    return
  }

  const { id: componentId, system: ownerSystem } = editingComponent.value

  console.log(`[ProjectSources] Saving ${Object.keys(changes).length} property changes for component ${componentId}`)

  for (const [fieldName, fieldValue] of Object.entries(changes)) {
    try {
      const variant = createVariantFromValue(fieldValue)

      const request: PropertySetValueRequest = {
        requestTypeId: CLASS_ID_PROPERTY_SET_VALUE_REQUEST,
        requestTypeName: 'PropertySetValueRequest',
        requestId: 0,
        ownerSystem,
        componentId,
        fieldName,
        fieldValue: variant
      }

      const future = mikanStore.client.sendRequest(request)
      const response = await future.await()

      if (response.resultCode === MikanAPIResult.Success) {
        console.log(`[ProjectSources] Successfully updated ${fieldName}`)
      } else {
        console.error(`[ProjectSources] Failed to update ${fieldName}: ${response.resultCode}`)
      }
    } catch (error) {
      console.error(`[ProjectSources] Error updating ${fieldName}:`, error)
    }
  }

  closeEditor()
}

// Camera action handlers
function handleCalibrateIntrinsics() {
  if (!selectedComponentId.value || !isCameraSelected.value) {
    console.error('[ProjectSources] No camera selected')
    return
  }
  sendRemoteControlCommand('calibrate_intrinsics', [selectedComponentId.value.toString()])
}

// Video Source CRUD handlers
function handleAddUSBVideoSource() {
  sendRemoteControlCommand('add_new_usb_video_source')
}

function handleAddNetworkVideoSource() {
  sendRemoteControlCommand('add_new_network_video_source')
}

function handleRemoveVideoSource() {
  if (!selectedComponentId.value || !isVideoSourceSelected.value) {
    console.error('[ProjectSources] No video source selected')
    return
  }
  sendRemoteControlCommand('remove_video_source', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

// Texture Source CRUD handlers
function handleAddClientTextureSource() {
  sendRemoteControlCommand('add_new_client_texture_source')
}

function handleAddSpoutTextureSource() {
  sendRemoteControlCommand('add_new_spout_texture_source')
}

function handleRemoveTextureSource() {
  if (!selectedComponentId.value || !isTextureSourceSelected.value) {
    console.error('[ProjectSources] No texture source selected')
    return
  }
  sendRemoteControlCommand('remove_texture_source', [selectedComponentId.value.toString()])
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

.sources-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
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

.section-actions {
  display: flex;
  gap: 8px;
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
  width: 100%;
  margin-top: 8px;
}

.action-btn:hover:not(:disabled) {
  background-color: #4cae4c;
}

.action-btn:disabled {
  background-color: #3d3d3d;
  color: #999;
  cursor: not-allowed;
}

.tracking-volumes-section {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.editor-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.8);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  padding: 20px;
}
</style>
