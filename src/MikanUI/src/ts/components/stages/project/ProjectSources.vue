<template>
  <div class="project-panel">
    <h2>Sources Panel</h2>
    <p>Manage video and texture sources for your project.</p>

    <div class="sources-content">
      <!-- Video Source Components -->
      <div class="component-section">
        <div class="section-header">
          <h3>Video Source Components</h3>
          <div class="section-actions">
            <button @click="handleAddUSBVideoSource" class="action-btn add-btn">+ USB Video</button>
            <button @click="handleAddNetworkVideoSource" class="action-btn add-btn">+ Network Video</button>
          </div>
        </div>
        <div class="component-cards">
          <div v-for="component in videoSourceComponents" :key="getComponentKey(component)" class="component-card-wrapper">
            <ComponentCard
              :component-id="component.component_id"
              :component="component"
              :owner-system="getSystemForComponent(component)"
              :editable="true"
            />
            <button
              @click="handleRemoveVideoSource(component.component_id)"
              class="action-btn remove-btn card-remove-btn"
              title="Remove Video Source"
            >
              ➖
            </button>
          </div>
          <div v-if="videoSourceComponents.length === 0" class="no-components">
            No video sources. Click + to add one.
          </div>
        </div>
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
        <div class="component-cards">
          <div v-for="component in textureSourceComponents" :key="getComponentKey(component)" class="component-card-wrapper">
            <ComponentCard
              :component-id="component.component_id"
              :component="component"
              :owner-system="getSystemForComponent(component)"
              :editable="true"
            />
            <button
              @click="handleRemoveTextureSource(component.component_id)"
              class="action-btn remove-btn card-remove-btn"
              title="Remove Texture Source"
            >
              ➖
            </button>
          </div>
          <div v-if="textureSourceComponents.length === 0" class="no-components">
            No texture sources. Click + to add one.
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useRemoteControl } from '../../../composables/useRemoteControl.js'
import ComponentCard from '../../shared/ComponentCard.vue'

const componentStore = useComponentStore()
const { sendRemoteControlCommand } = useRemoteControl()

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

// Helper to get unique key for component
function getComponentKey(component: any): string {
  return `${component.component_class}_${component.component_id}`
}

// Helper to determine system name from component class
function getSystemForComponent(component: any): string {
  const classToSystem: Record<string, string> = {
    'USBVideoSourceComponent': 'USBVideoSourceSystem',
    'NetworkVideoSourceComponent': 'NetworkVideoObjectSystem',
    'SpoutTextureSourceComponent': 'SpoutTextureSourceSystem',
    'ClientTextureSourceComponent': 'ClientTextureSourceSystem'
  }
  return classToSystem[component.component_class] || ''
}

// Video Source CRUD handlers
function handleAddUSBVideoSource() {
  sendRemoteControlCommand('add_new_usb_video_source')
}

function handleAddNetworkVideoSource() {
  sendRemoteControlCommand('add_new_network_video_source')
}

function handleRemoveVideoSource(componentId: number) {
  sendRemoteControlCommand('remove_video_source', [componentId.toString()])
}

// Texture Source CRUD handlers
function handleAddClientTextureSource() {
  sendRemoteControlCommand('add_new_client_texture_source')
}

function handleAddSpoutTextureSource() {
  sendRemoteControlCommand('add_new_spout_texture_source')
}

function handleRemoveTextureSource(componentId: number) {
  sendRemoteControlCommand('remove_texture_source', [componentId.toString()])
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

.component-cards {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.component-card-wrapper {
  position: relative;
  display: flex;
  gap: 8px;
  align-items: flex-start;
}

.component-card-wrapper > :first-child {
  flex: 1;
}

.card-remove-btn {
  flex-shrink: 0;
  padding: 6px 12px;
  font-size: 18px;
  line-height: 1;
  min-width: 40px;
}

.no-components {
  color: #888;
  font-style: italic;
  padding: 20px;
  text-align: center;
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
</style>
