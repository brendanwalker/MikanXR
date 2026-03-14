<template>
  <div class="project-panel">
    <h2>Tracking Panel</h2>
    <p>Manage tracking volumes and mounts for your project.</p>

    <div class="tracking-content">
      <!-- Tracking Volume Selection -->
      <div class="selection-section">
        <div class="add-buttons-row">
          <button @click="handleAddSteamVRVolume" class="action-btn add-btn icon-btn" title="Add VR Tracking Volume">
            <img src="/images/add_vr_tracking_icon.png" alt="Add VR Tracking" class="btn-icon" />
            VR Tracking
          </button>
          <button @click="handleAddMarkerVolume" class="action-btn add-btn icon-btn" title="Add Marker Tracking Volume">
            <img src="/images/add_marker_tracking_icon.png" alt="Add Marker Tracking" class="btn-icon" />
            Marker Tracking
          </button>
        </div>
        <div class="selection-row">
          <label class="selection-label">Tracking Volume:</label>
          <select v-model="selectedTrackingVolumeId" class="selection-dropdown tracking-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="volume in trackingVolumeComponents"
              :key="volume.component_id"
              :value="volume.component_id"
            >
              {{ volume.component_name }}
            </option>
          </select>
          <button
            v-if="selectedTrackingVolumeId !== -1"
            @click="handleRemoveTrackingVolume"
            class="icon-only-btn remove-btn"
            title="Remove Tracking Volume"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove" class="btn-icon-only" />
          </button>
        </div>
      </div>

      <!-- Selected Tracking Volume Component -->
      <div v-if="selectedTrackingVolumeId !== -1 && selectedTrackingVolumeComponent" class="component-section">
        <h3>Tracking Volume Component</h3>
        <ComponentCard
          :component-id="selectedTrackingVolumeId"
          :component="selectedTrackingVolumeComponent"
          :owner-system="selectedTrackingVolumeSystem"
          :editable="true"
        />
      </div>

      <!-- Tracking Mount Selection -->
      <div class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Tracking Mount:</label>
          <select v-model="selectedTrackingMountId" class="selection-dropdown tracking-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="mount in trackingMountComponents"
              :key="mount.component_id"
              :value="mount.component_id"
            >
              {{ mount.component_name }}
            </option>
          </select>
          <button @click="handleAddTrackingMount" class="icon-only-btn add-btn" title="Add Tracking Mount">
            <img src="/images/add_component_normal_icon.png" alt="Add Tracking Mount" class="btn-icon-only" />
          </button>
          <button
            v-if="selectedTrackingMountId !== -1"
            @click="handleRemoveTrackingMount"
            class="icon-only-btn remove-btn"
            title="Remove Tracking Mount"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove" class="btn-icon-only" />
          </button>
        </div>
      </div>

      <!-- Selected Tracking Mount Component -->
      <div v-if="selectedTrackingMountId !== -1 && selectedTrackingMountComponent" class="component-section">
        <h3>Tracking Mount Component</h3>
        <ComponentCard
          :component-id="selectedTrackingMountId"
          :component="selectedTrackingMountComponent"
          :owner-system="selectedTrackingMountSystem"
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
import ComponentCard from '../../shared/ComponentCard.vue'

const componentStore = useComponentStore()
const { sendRemoteControlCommand } = useRemoteControl()

// Selection state
const selectedTrackingVolumeId = ref<number>(-1)
const selectedTrackingMountId = ref<number>(-1)

// Get components by class
const trackingVolumeComponents = computed(() => {
  const vrVolumes = componentStore.getComponentsByClass('VRTrackingVolumeComponent')
  const markerVolumes = componentStore.getComponentsByClass('MarkerTrackingVolumeComponent')
  return [...vrVolumes, ...markerVolumes]
})

const trackingMountComponents = computed(() =>
  componentStore.getComponentsByClass('TrackingMountComponent')
)

// Get selected components
const selectedTrackingVolumeComponent = computed(() => {
  if (selectedTrackingVolumeId.value === -1) return null

  // Try VR tracking volume system
  let component = componentStore.getComponent(selectedTrackingVolumeId.value, 'VRTrackingVolumeSystem')
  if (component) return component

  // Try Marker tracking volume system
  component = componentStore.getComponent(selectedTrackingVolumeId.value, 'MarkerTrackingVolumeSystem')
  return component
})

const selectedTrackingMountComponent = computed(() => {
  if (selectedTrackingMountId.value === -1) return null
  return componentStore.getComponent(selectedTrackingMountId.value, 'TrackingMountObjectSystem')
})

// Get system name for selected components
const selectedTrackingVolumeSystem = computed(() => {
  if (!selectedTrackingVolumeComponent.value) return ''
  const componentClass = (selectedTrackingVolumeComponent.value as any).component_class

  if (componentClass === 'VRTrackingVolumeComponent') return 'VRTrackingVolumeSystem'
  if (componentClass === 'MarkerTrackingVolumeComponent') return 'MarkerTrackingVolumeSystem'
  return ''
})

const selectedTrackingMountSystem = computed(() => {
  if (!selectedTrackingMountComponent.value) return ''
  return 'TrackingMountObjectSystem'
})

// Tracking Volume CRUD handlers
function handleAddSteamVRVolume() {
  sendRemoteControlCommand('add_new_steamvr_tracking_volume')
}

function handleAddMarkerVolume() {
  sendRemoteControlCommand('add_new_marker_tracking_volume')
}

function handleRemoveTrackingVolume() {
  if (selectedTrackingVolumeId.value === -1) {
    console.error('[ProjectTracking] No tracking volume selected')
    return
  }
  const componentClass = (selectedTrackingVolumeComponent.value as any)?.component_class
  if (componentClass === 'VRTrackingVolumeComponent') {
    sendRemoteControlCommand('remove_vr_tracking_volume', [selectedTrackingVolumeId.value.toString()])
  } else if (componentClass === 'MarkerTrackingVolumeComponent') {
    sendRemoteControlCommand('remove_marker_tracking_volume', [selectedTrackingVolumeId.value.toString()])
  }
  selectedTrackingVolumeId.value = -1
}

// Tracking Mount CRUD handlers
function handleAddTrackingMount() {
  sendRemoteControlCommand('add_new_tracking_mount')
}

function handleRemoveTrackingMount() {
  if (selectedTrackingMountId.value === -1) {
    console.error('[ProjectTracking] No tracking mount selected')
    return
  }
  sendRemoteControlCommand('remove_tracking_mount', [selectedTrackingMountId.value.toString()])
  selectedTrackingMountId.value = -1
}

// Persist selection state
watch(selectedTrackingVolumeId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectTracking.selectedTrackingVolumeId', newValue.toString())
  }
})

watch(selectedTrackingMountId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectTracking.selectedTrackingMountId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedTrackingVolumeId = sessionStorage.getItem('projectTracking.selectedTrackingVolumeId')
  const savedTrackingMountId = sessionStorage.getItem('projectTracking.selectedTrackingMountId')

  if (savedTrackingVolumeId) {
    const trackingVolumeId = parseInt(savedTrackingVolumeId, 10)
    if (!isNaN(trackingVolumeId)) {
      selectedTrackingVolumeId.value = trackingVolumeId
    }
  }

  if (savedTrackingMountId) {
    const trackingMountId = parseInt(savedTrackingMountId, 10)
    if (!isNaN(trackingMountId)) {
      selectedTrackingMountId.value = trackingMountId
    }
  }
}

// Initialize on mount
onMounted(() => {
  restoreSelectionState()
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

.tracking-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.selection-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.add-buttons-row {
  display: flex;
  gap: 8px;
  flex-wrap: wrap;
}

.selection-row {
  display: flex;
  align-items: center;
  gap: 12px;
  flex-wrap: nowrap;
}

.selection-label {
  color: #ffffff;
  font-size: 16px;
  font-weight: 600;
  min-width: 120px;
  flex-shrink: 0;
}

.selection-dropdown {
  flex: 1;
  min-width: 150px;
}

.tracking-select {
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  color: #fff;
  font-family: monospace;
  font-size: 14px;
  cursor: pointer;
  width: 100%;
}

.tracking-select:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(0, 0, 0, 0.4);
}

.tracking-select option {
  background: #2d2d2d;
  color: #fff;
}

.component-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
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
}

/* Icon button styles now imported from common-buttons.css */
</style>
