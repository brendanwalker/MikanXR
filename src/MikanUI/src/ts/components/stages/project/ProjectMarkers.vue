<template>
  <div class="project-panel">
    <h2>Markers Panel</h2>
    <p>Manage marker components for your project.</p>

    <div class="markers-content">
      <!-- Marker Selection -->
      <div class="selection-section">
        <div class="selection-row">
          <label class="selection-label">Marker:</label>
          <select v-model="selectedMarkerId" class="selection-dropdown marker-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="marker in markerComponents"
              :key="marker.component_id"
              :value="marker.component_id"
            >
              {{ marker.component_name }}
            </option>
          </select>
          <button @click="handleAddMarker" class="icon-only-btn add-marker-btn" title="Add Marker">
            <img src="/images/add_component_normal_icon.png" alt="Add Marker" class="btn-icon-only" />
          </button>
          <button
            v-if="selectedMarkerId !== -1"
            @click="handleRemoveMarker"
            class="icon-only-btn remove-btn"
            title="Remove Marker"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove" class="btn-icon-only" />
          </button>
        </div>
      </div>

      <!-- Selected Marker Component -->
      <div v-if="selectedMarkerId !== -1 && selectedMarkerComponent" class="component-section">
        <h3>Marker Component</h3>
        <ComponentCard
          :component-id="selectedMarkerId"
          :component="selectedMarkerComponent"
          owner-system="MarkerObjectSystem"
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
const selectedMarkerId = ref<number>(-1)

// Get components by class
const markerComponents = computed(() =>
  componentStore.getComponentsByClass('MarkerComponent')
)

// Get selected component
const selectedMarkerComponent = computed(() => {
  if (selectedMarkerId.value === -1) return null
  return componentStore.getComponent(selectedMarkerId.value, 'MarkerObjectSystem')
})

// Marker CRUD handlers
function handleAddMarker() {
  sendRemoteControlCommand('add_new_marker')
}

function handleRemoveMarker() {
  if (selectedMarkerId.value === -1) {
    console.error('[ProjectMarkers] No marker selected')
    return
  }
  sendRemoteControlCommand('remove_marker', [selectedMarkerId.value.toString()])
  selectedMarkerId.value = -1
}

// Persist selection state
watch(selectedMarkerId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectMarkers.selectedMarkerId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedMarkerId = sessionStorage.getItem('projectMarkers.selectedMarkerId')

  if (savedMarkerId) {
    const markerId = parseInt(savedMarkerId, 10)
    if (!isNaN(markerId)) {
      selectedMarkerId.value = markerId
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

.markers-content {
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

.marker-select {
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

.marker-select:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(0, 0, 0, 0.4);
}

.marker-select option {
  background: #2d2d2d;
  color: #fff;
}

.component-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
}

.icon-only-btn {
  padding: 4px;
  background: none;
  border: none;
  cursor: pointer;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  transition: transform 0.1s;
}

.icon-only-btn:hover {
  transform: scale(1.1);
}

.icon-only-btn:active {
  transform: scale(0.95);
}

.btn-icon-only {
  width: 24px;
  height: 24px;
  object-fit: contain;
  display: block;
}

/* Remove button with state-based icons */
.icon-only-btn.remove-btn .btn-icon-only {
  content: url('/images/delete_component_normal_icon.png');
}

.icon-only-btn.remove-btn:hover .btn-icon-only {
  content: url('/images/delete_component_highlight_icon.png');
}

.icon-only-btn.remove-btn:active .btn-icon-only {
  content: url('/images/delete_component_press_icon.png');
}

/* Add marker button with state-based icons */
.icon-only-btn.add-marker-btn .btn-icon-only {
  content: url('/images/add_component_normal_icon.png');
}

.icon-only-btn.add-marker-btn:hover .btn-icon-only {
  content: url('/images/add_component_highlight_icon.png');
}

.icon-only-btn.add-marker-btn:active .btn-icon-only {
  content: url('/images/add_component_press_icon.png');
}
</style>
