<template>
  <div class="project-panel">
    <h2>Settings Panel</h2>
    <p>Project-wide configuration and preferences.</p>

    <div class="settings-content">

      <!-- Editor Settings -->
      <div v-if="editorSystemValues" class="settings-section">
        <h3>Editor Settings</h3>
        <div class="property-row">
          <label class="property-label">Draw Origin</label>
          <input
            type="checkbox"
            class="toggle-checkbox"
            :checked="editorSystemValues.render_origin"
            @change="setEditorProperty('render_origin', ($event.target as HTMLInputElement).checked)"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Draw Anchors</label>
          <input
            type="checkbox"
            class="toggle-checkbox"
            :checked="editorSystemValues.render_anchors"
            @change="setEditorProperty('render_anchors', ($event.target as HTMLInputElement).checked)"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Draw Quad Stencils</label>
          <input
            type="checkbox"
            class="toggle-checkbox"
            :checked="editorSystemValues.render_quad_stencils"
            @change="setEditorProperty('render_quad_stencils', ($event.target as HTMLInputElement).checked)"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Draw Box Stencils</label>
          <input
            type="checkbox"
            class="toggle-checkbox"
            :checked="editorSystemValues.render_box_stencils"
            @change="setEditorProperty('render_box_stencils', ($event.target as HTMLInputElement).checked)"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Draw Model Stencils</label>
          <input
            type="checkbox"
            class="toggle-checkbox"
            :checked="editorSystemValues.render_model_stencils"
            @change="setEditorProperty('render_model_stencils', ($event.target as HTMLInputElement).checked)"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Camera Speed</label>
          <input
            type="number"
            class="property-input"
            :value="editorSystemValues.camera_speed"
            @change="setEditorProperty('camera_speed', parseFloat(($event.target as HTMLInputElement).value))"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Language</label>
          <select
            class="property-select"
            :value="editorSystemValues.selected_language"
            @change="setEditorProperty('selected_language', ($event.target as HTMLSelectElement).value)"
          >
            <option
              v-for="lang in editorSystemValues.available_language_list"
              :key="lang"
              :value="lang"
            >{{ lang }}</option>
          </select>
        </div>
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
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import { useSettingsStore } from '../../../stores/settingsStore.js'
import { MikanClient, MikanEditorSystemValues, PropertySetValueRequest } from '@mikanxr/client'
import { usePropertyEditor } from '../../../composables/usePropertyEditor.js'

const { createVariantFromValue } = usePropertyEditor()

const componentStore = useComponentStore()
const mikanStore = useMikanStore()
const settingsStore = useSettingsStore()

const editorSystemValues = computed(() => componentStore.getEditorSystemValues() as MikanEditorSystemValues | null)

async function setEditorProperty(fieldName: string, value: any) {
  const client = mikanStore.client as MikanClient | null
  if (!client) return

  try {
    const request = new PropertySetValueRequest()
    request.ownerSystem = 'EditorObjectSystem'
    request.fieldName = fieldName
    request.fieldValue = createVariantFromValue(value)

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error(`[ProjectSettings] Failed to set editor property "${fieldName}":`, error)
  }
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

.property-row {
  display: flex;
  align-items: center;
  gap: 12px;
}

.property-label {
  color: #ffffff;
  font-size: 14px;
  min-width: 140px;
  flex-shrink: 0;
}

.property-select {
  flex: 1;
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  color: #fff;
  font-size: 14px;
  cursor: pointer;
}

.property-select:focus {
  outline: none;
  border-color: #5cb85c;
}

.property-select option {
  background: #2d2d2d;
  color: #fff;
}

.property-input {
  flex: 1;
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  color: #fff;
  font-size: 14px;
  max-width: 100px;
}

.property-input:focus {
  outline: none;
  border-color: #5cb85c;
}
</style>
