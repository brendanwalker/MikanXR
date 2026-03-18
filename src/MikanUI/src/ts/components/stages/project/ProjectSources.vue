<template>
  <div class="project-panel">
    <h2>Sources Panel</h2>
    <p>Manage video and texture sources for your project.</p>

    <div class="sources-content">
      <!-- Video Source Selection -->
      <div class="selection-section">
        <div class="add-buttons-row">
          <button @click="handleAddUSBVideoSource" class="action-btn add-btn icon-btn" title="Add USB Video Source">
            <img src="/images/add_usb_camera_source_icon.png" alt="Add USB Video" class="btn-icon" />
            USB Video
          </button>
          <button @click="handleAddNetworkVideoSource" class="action-btn add-btn icon-btn" title="Add Network Video Source">
            <img src="/images/add_networked_camera_source_icon.png" alt="Add Network Video" class="btn-icon" />
            Network Video
          </button>
        </div>
        <div class="selection-row">
          <label class="selection-label">Video Source:</label>
          <select v-model="selectedVideoSourceId" class="selection-dropdown source-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="source in videoSourceComponents"
              :key="source.component_id"
              :value="source.component_id"
            >
              {{ source.component_name }}
            </option>
          </select>
          <button
            v-if="selectedVideoSourceId !== -1"
            @click="handleRemoveVideoSource"
            class="icon-only-btn remove-btn"
            title="Remove Video Source"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove" class="btn-icon-only" />
          </button>
        </div>
      </div>

      <!-- Selected Video Source Component -->
      <div v-if="selectedVideoSourceId !== -1 && selectedVideoSourceComponent" class="component-section">
        <h3>Video Source Component</h3>
        <component
          :is="videoSourceCardComponent"
          :component-id="selectedVideoSourceId"
          :component="selectedVideoSourceComponent"
          :owner-system="selectedVideoSourceSystem"
          :editable="isUSBVideoSource ? undefined : false"
        />
      </div>

      <!-- Texture Source Selection -->
      <div class="selection-section">
        <div class="add-buttons-row">
          <button @click="handleAddClientTextureSource" class="action-btn add-btn icon-btn" title="Add Client Texture Source">
            <img src="/images/add_client_texture_source_icon.png" alt="Add Client Texture" class="btn-icon" />
            Client Texture
          </button>
          <button @click="handleAddSpoutTextureSource" class="action-btn add-btn icon-btn" title="Add Spout Texture Source">
            <img src="/images/add_spout_texture_source_icon.png" alt="Add Spout Texture" class="btn-icon" />
            Spout Texture
          </button>
        </div>
        <div class="selection-row">
          <label class="selection-label">Texture Source:</label>
          <select v-model="selectedTextureSourceId" class="selection-dropdown source-select">
            <option :value="-1">&lt;None&gt;</option>
            <option
              v-for="source in textureSourceComponents"
              :key="source.component_id"
              :value="source.component_id"
            >
              {{ source.component_name }}
            </option>
          </select>
          <button
            v-if="selectedTextureSourceId !== -1"
            @click="handleRemoveTextureSource"
            class="icon-only-btn remove-btn"
            title="Remove Texture Source"
          >
            <img src="/images/delete_component_normal_icon.png" alt="Remove" class="btn-icon-only" />
          </button>
        </div>
      </div>

      <!-- Selected Texture Source Component -->
      <div v-if="selectedTextureSourceId !== -1 && selectedTextureSourceComponent" class="component-section">
        <h3>Texture Source Component</h3>
        <ComponentCard
          :component-id="selectedTextureSourceId"
          :component="selectedTextureSourceComponent"
          :owner-system="selectedTextureSourceSystem"
          :editable="false"
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
import USBVideoSourceCard from '../../shared/USBVideoSourceCard.vue'

const componentStore = useComponentStore()
const { sendRemoteControlCommand } = useRemoteControl()

// Selection state
const selectedVideoSourceId = ref<number>(-1)
const selectedTextureSourceId = ref<number>(-1)

// Get components by class
const videoSourceComponents = computed(() => {
  const usbComponents = componentStore.getComponentsByClass('USBVideoSourceComponent')
  const networkComponents = componentStore.getComponentsByClass('NetworkVideoSourceComponent')
  return [...usbComponents, ...networkComponents]
})

const textureSourceComponents = computed(() => {
  const spoutComponents = componentStore.getComponentsByClass('SpoutTextureSourceComponent')
  const clientComponents = componentStore.getComponentsByClass('ClientTextureSourceComponent')
  return [...spoutComponents, ...clientComponents]
})

// Get selected components
const selectedVideoSourceComponent = computed(() => {
  if (selectedVideoSourceId.value === -1) return null

  // Try USB video source system
  let component = componentStore.getComponent(selectedVideoSourceId.value, 'USBVideoSourceSystem')
  if (component) return component

  // Try Network video source system
  component = componentStore.getComponent(selectedVideoSourceId.value, 'NetworkVideoObjectSystem')
  return component
})

const isUSBVideoSource = computed(() => {
  if (!selectedVideoSourceComponent.value) return false
  const componentClass = (selectedVideoSourceComponent.value as any).component_class
  return componentClass === 'USBVideoSourceComponent'
})

const videoSourceCardComponent = computed(() => {
  return isUSBVideoSource.value ? USBVideoSourceCard : ComponentCard
})

const selectedTextureSourceComponent = computed(() => {
  if (selectedTextureSourceId.value === -1) return null

  // Try Spout texture source system
  let component = componentStore.getComponent(selectedTextureSourceId.value, 'SpoutTextureSourceSystem')
  if (component) return component

  // Try Client texture source system
  component = componentStore.getComponent(selectedTextureSourceId.value, 'ClientTextureSourceSystem')
  return component
})

// Get system name for selected components
const selectedVideoSourceSystem = computed(() => {
  if (!selectedVideoSourceComponent.value) return ''
  const componentClass = (selectedVideoSourceComponent.value as any).component_class

  if (componentClass === 'USBVideoSourceComponent') return 'USBVideoSourceSystem'
  if (componentClass === 'NetworkVideoSourceComponent') return 'NetworkVideoObjectSystem'
  return ''
})

const selectedTextureSourceSystem = computed(() => {
  if (!selectedTextureSourceComponent.value) return ''
  const componentClass = (selectedTextureSourceComponent.value as any).component_class

  if (componentClass === 'SpoutTextureSourceComponent') return 'SpoutTextureSourceSystem'
  if (componentClass === 'ClientTextureSourceComponent') return 'ClientTextureSourceSystem'
  return ''
})

// Video Source CRUD handlers
function handleAddUSBVideoSource() {
  sendRemoteControlCommand('add_new_usb_video_source')
}

function handleAddNetworkVideoSource() {
  sendRemoteControlCommand('add_new_network_video_source')
}

function handleRemoveVideoSource() {
  if (selectedVideoSourceId.value === -1) {
    console.error('[ProjectSources] No video source selected')
    return
  }
  sendRemoteControlCommand('remove_video_source', [selectedVideoSourceId.value.toString()])
  selectedVideoSourceId.value = -1
}

// Texture Source CRUD handlers
function handleAddClientTextureSource() {
  sendRemoteControlCommand('add_new_client_texture_source')
}

function handleAddSpoutTextureSource() {
  sendRemoteControlCommand('add_new_spout_texture_source')
}

function handleRemoveTextureSource() {
  if (selectedTextureSourceId.value === -1) {
    console.error('[ProjectSources] No texture source selected')
    return
  }
  sendRemoteControlCommand('remove_texture_source', [selectedTextureSourceId.value.toString()])
  selectedTextureSourceId.value = -1
}

// Persist selection state
watch(selectedVideoSourceId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectSources.selectedVideoSourceId', newValue.toString())
  }
})

watch(selectedTextureSourceId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectSources.selectedTextureSourceId', newValue.toString())
  }
})

// Restore selection state on mount
function restoreSelectionState() {
  const savedVideoSourceId = sessionStorage.getItem('projectSources.selectedVideoSourceId')
  const savedTextureSourceId = sessionStorage.getItem('projectSources.selectedTextureSourceId')

  if (savedVideoSourceId) {
    const videoSourceId = parseInt(savedVideoSourceId, 10)
    if (!isNaN(videoSourceId)) {
      selectedVideoSourceId.value = videoSourceId
    }
  }

  if (savedTextureSourceId) {
    const textureSourceId = parseInt(savedTextureSourceId, 10)
    if (!isNaN(textureSourceId)) {
      selectedTextureSourceId.value = textureSourceId
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

.sources-content {
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

.source-select {
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

.source-select:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(0, 0, 0, 0.4);
}

.source-select option {
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
