<template>
  <div class="project-panel">
    <h2>Tracking Panel</h2>
    <p>Click on a VR device or tracking mount to edit its properties.</p>

    <div class="tracking-content">
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

      <!-- VR Device Components -->
      <ComponentList
        title="VR Device Components"
        :components="vrDeviceComponents"
        :selectable="true"
        :selected-component-id="selectedComponentId"
        sort-by="name"
        @select="handleSelectComponent($event, 'VRObjectSystem')"
      />

      <!-- Tracking Volume Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Tracking Volume Components</h3>
          <div class="section-actions">
            <button @click="handleAddSteamVRVolume" class="action-btn add-btn">+ SteamVR Volume</button>
            <button @click="handleAddMarkerVolume" class="action-btn add-btn">+ Marker Volume</button>
          </div>
        </div>
        <ComponentList
          :components="trackingVolumeComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'TrackingVolumeObjectSystem')"
        />
        <button
          v-if="selectedComponentId && isTrackingVolumeSelected"
          @click="handleRemoveTrackingVolume"
          class="action-btn remove-btn"
        >
          Remove Selected Tracking Volume
        </button>
      </div>

      <!-- Tracking Mount Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Tracking Mount Components</h3>
          <button @click="handleAddTrackingMount" class="action-btn add-btn">+ Add Tracking Mount</button>
        </div>
        <ComponentList
          :components="trackingMountComponents"
          :selectable="true"
          :selected-component-id="selectedComponentId"
          sort-by="name"
          @select="handleSelectComponent($event, 'TrackingMountObjectSystem')"
        />
        <button
          v-if="selectedComponentId && isTrackingMountSelected"
          @click="handleRemoveTrackingMount"
          class="action-btn remove-btn"
        >
          Remove Selected Tracking Mount
        </button>
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
const vrDeviceComponents = computed(() =>
  componentStore.getComponentsByClass('VRDeviceComponent')
)

const trackingVolumeComponents = computed(() => {
  const steamVRVolumes = componentStore.getComponentsByClass('SteamVRTrackingVolumeComponent')
  const markerVolumes = componentStore.getComponentsByClass('MarkerTrackingVolumeComponent')
  return [...steamVRVolumes, ...markerVolumes]
})

const trackingMountComponents = computed(() =>
  componentStore.getComponentsByClass('TrackingMountComponent')
)

// Check which type of component is selected
const isTrackingVolumeSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  const componentClass = (component as any)?.component_class
  return componentClass === 'SteamVRTrackingVolumeComponent' || componentClass === 'MarkerTrackingVolumeComponent'
})

const isTrackingMountSelected = computed(() => {
  if (!selectedComponentId.value) return false
  const component = componentStore.getComponent(selectedComponentId.value)
  return (component as any)?.component_class === 'TrackingMountComponent'
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
    console.error('[ProjectTracking] Cannot save: no component selected or not connected')
    return
  }

  const { id: componentId, system: ownerSystem } = editingComponent.value

  console.log(`[ProjectTracking] Saving ${Object.keys(changes).length} property changes for component ${componentId}`)

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
        console.log(`[ProjectTracking] Successfully updated ${fieldName}`)
      } else {
        console.error(`[ProjectTracking] Failed to update ${fieldName}: ${response.resultCode}`)
      }
    } catch (error) {
      console.error(`[ProjectTracking] Error updating ${fieldName}:`, error)
    }
  }

  closeEditor()
}

// Tracking Volume CRUD handlers
function handleAddSteamVRVolume() {
  sendRemoteControlCommand('add_new_steamvr_tracking_volume')
}

function handleAddMarkerVolume() {
  sendRemoteControlCommand('add_new_marker_tracking_volume')
}

function handleRemoveTrackingVolume() {
  if (!selectedComponentId.value || !isTrackingVolumeSelected.value) {
    console.error('[ProjectTracking] No tracking volume selected')
    return
  }
  sendRemoteControlCommand('remove_tracking_volume', [selectedComponentId.value.toString()])
  selectedComponentId.value = null
}

// Tracking Mount CRUD handlers
function handleAddTrackingMount() {
  sendRemoteControlCommand('add_new_tracking_mount')
}

function handleRemoveTrackingMount() {
  if (!selectedComponentId.value || !isTrackingMountSelected.value) {
    console.error('[ProjectTracking] No tracking mount selected')
    return
  }
  sendRemoteControlCommand('remove_tracking_mount', [selectedComponentId.value.toString()])
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

.tracking-content {
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
