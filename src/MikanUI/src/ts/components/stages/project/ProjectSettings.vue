<template>
  <div class="project-panel">
    <h2>Settings Panel</h2>
    <p>Project-wide configuration and preferences.</p>

    <div class="settings-content">
      <div class="info-message">
        <p>This panel is for future project-wide settings and preferences.</p>
        <p>Component-specific settings can be edited in the other panels (Scenes, Sources, Tracking, etc.).</p>
      </div>

      <div class="settings-section">
        <h3>Developer Settings</h3>
        <div class="developer-settings">
          <div class="setting-row">
            <label class="setting-label" for="dev-mode-toggle">Developer Mode</label>
            <div class="setting-control">
              <input
                id="dev-mode-toggle"
                type="checkbox"
                v-model="settingsStore.developerMode"
                class="toggle-checkbox"
              />
              <span class="setting-description">Show advanced properties and debug information</span>
            </div>
          </div>
        </div>
      </div>

      <div class="settings-section">
        <h3>Connection Status</h3>
        <div class="status-grid">
          <div class="status-item">
            <span class="status-label">Connection:</span>
            <span :class="['status-value', statusClass]">{{ connectionStatusText }}</span>
          </div>
          <div class="status-item">
            <span class="status-label">App Stage:</span>
            <span class="status-value">{{ appStage }}</span>
          </div>
          <div class="status-item">
            <span class="status-label">Total Components:</span>
            <span class="status-value">{{ totalComponents }}</span>
          </div>
        </div>
      </div>

      <div class="settings-section">
        <h3>System Information</h3>
        <div class="info-grid">
          <div class="info-item">
            <span class="info-label">Scenes:</span>
            <span class="info-value">{{ sceneCount }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">Stages:</span>
            <span class="info-value">{{ stageCount }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">Cameras:</span>
            <span class="info-value">{{ cameraCount }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">Markers:</span>
            <span class="info-value">{{ markerCount }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">Anchors:</span>
            <span class="info-value">{{ anchorCount }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">VR Devices:</span>
            <span class="info-value">{{ vrDeviceCount }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">Tracking Mounts:</span>
            <span class="info-value">{{ trackingMountCount }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import { useSettingsStore } from '../../../stores/settingsStore.js'

const componentStore = useComponentStore()
const mikanStore = useMikanStore()
const settingsStore = useSettingsStore()

// Connection status
const connectionStatusText = computed(() => {
  switch (mikanStore.connectionStatus) {
    case 'connected':
      return 'Connected'
    case 'connecting':
      return 'Connecting...'
    case 'disconnected':
      return 'Disconnected'
    case 'error':
      return 'Error'
    default:
      return 'Unknown'
  }
})

const statusClass = computed(() => {
  switch (mikanStore.connectionStatus) {
    case 'connected':
      return 'status-connected'
    case 'connecting':
      return 'status-connecting'
    case 'error':
      return 'status-error'
    default:
      return 'status-disconnected'
  }
})

const appStage = computed(() => mikanStore.appStage || 'Unknown')

// Component counts
const totalComponents = computed(() => componentStore.components.size)

const sceneCount = computed(() =>
  componentStore.getComponentsByClass('SceneComponent').length
)

const stageCount = computed(() =>
  componentStore.getComponentsByClass('StageComponent').length
)

const cameraCount = computed(() =>
  componentStore.getComponentsByClass('CameraComponent').length
)

const markerCount = computed(() =>
  componentStore.getComponentsByClass('MarkerComponent').length
)

const anchorCount = computed(() =>
  componentStore.getComponentsByClass('AnchorComponent').length
)

const vrDeviceCount = computed(() =>
  componentStore.getComponentsByClass('VRDeviceComponent').length
)

const trackingMountCount = computed(() =>
  componentStore.getComponentsByClass('TrackingMountComponent').length
)
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

.settings-content {
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.info-message {
  background-color: rgba(92, 184, 92, 0.1);
  border: 1px solid rgba(92, 184, 92, 0.3);
  border-radius: 4px;
  padding: 16px;
}

.info-message p {
  margin: 0;
  margin-bottom: 8px;
  color: rgba(255, 255, 255, 0.8);
}

.info-message p:last-child {
  margin-bottom: 0;
}

.settings-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
}

.settings-section h3 {
  color: #ffffff;
  margin: 0 0 16px 0;
  font-size: 18px;
  font-weight: 600;
}

.status-grid,
.info-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 16px;
}

.status-item,
.info-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px;
  background-color: rgba(255, 255, 255, 0.05);
  border-radius: 4px;
}

.status-label,
.info-label {
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
}

.status-value,
.info-value {
  color: #fff;
  font-weight: 600;
  font-family: monospace;
}

.status-connected {
  color: #5cb85c;
}

.status-connecting {
  color: #f0ad4e;
}

.status-disconnected {
  color: #999;
}

.status-error {
  color: #d9534f;
}

.developer-settings {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.setting-row {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.setting-label {
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
  font-size: 14px;
}

.setting-control {
  display: flex;
  align-items: center;
  gap: 12px;
}

.toggle-checkbox {
  width: 20px;
  height: 20px;
  cursor: pointer;
  accent-color: #5cb85c;
}

.setting-description {
  color: rgba(255, 255, 255, 0.5);
  font-size: 13px;
  font-style: italic;
}
</style>
