<template>
  <div class="texture-source-settings">
    <div class="settings-header">
      <h2>Texture Source Settings</h2>
      <button @click="handleReturn" class="return-btn">Return</button>
    </div>

    <div v-if="isLoading" class="loading-message">
      Loading texture source settings...
    </div>

    <div v-else-if="!textureSourceId || textureSourceId === -1" class="no-source-message">
      No texture source selected
    </div>

    <!-- Spout Texture Source Settings -->
    <div v-else-if="spoutTextureSourceComponent" class="settings-content">
      <div class="settings-section">
        <div class="property-row">
          <label class="property-label">Name</label>
          <span class="property-readonly">{{ spoutTextureSourceComponent.component_name }}</span>
        </div>

        <div class="property-row">
          <label class="property-label">Spout Source</label>
          <input
            type="text"
            v-model="spoutSourceName"
            @blur="handleSpoutSourceChange"
            @keyup.enter="handleSpoutSourceChange"
            class="property-input"
            placeholder="Spout sender name"
          />
        </div>
      </div>
    </div>

    <!-- Client Texture Source Settings -->
    <div v-else-if="clientTextureSourceComponent" class="settings-content">
      <div class="settings-section">
        <div class="property-row">
          <label class="property-label">Name</label>
          <span class="property-readonly">{{ clientTextureSourceComponent.component_name }}</span>
        </div>

        <div class="property-row">
          <label class="property-label">Client Source</label>
          <input
            type="text"
            v-model="clientSourceName"
            @blur="handleClientSourceChange"
            @keyup.enter="handleClientSourceChange"
            class="property-input"
            placeholder="Client source name"
          />
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted, onUnmounted } from 'vue'
import { useMikanStore } from '../../stores/mikanStore.js'
import { useComponentStore } from '../../stores/componentStore.js'
import { useRemoteControl } from '../../composables/useRemoteControl.js'
import { usePropertyEditor } from '../../composables/usePropertyEditor.js'
import {
  PropertySetValueRequest
} from '@mikanxr/client'

const mikanStore = useMikanStore()
const componentStore = useComponentStore()
const { sendRemoteControlCommand, sendRemoteControlCommandWithResults } = useRemoteControl()
const { createVariantFromValue } = usePropertyEditor()

// State
const isLoading = ref(true)
const textureSourceId = ref<number>(-1)

// Editable source name fields
const spoutSourceName = ref<string>('')
const clientSourceName = ref<string>('')

// Get the texture source component (one will be non-null, the other null)
const spoutTextureSourceComponent = computed(() => {
  if (textureSourceId.value === -1) return null
  return componentStore.getSpoutTextureSourceComponent(textureSourceId.value)
})

const clientTextureSourceComponent = computed(() => {
  if (textureSourceId.value === -1) return null
  return componentStore.getClientTextureSourceComponent(textureSourceId.value)
})

// Initialize component
async function initialize() {
  isLoading.value = true

  try {
    const results = await sendRemoteControlCommandWithResults('get_texture_source_component_id')

    if (!results || results.length === 0) {
      console.error('[TextureSourceSettings] Failed to get texture source component ID')
      isLoading.value = false
      return
    }

    const componentId = parseInt(results[0], 10)

    if (isNaN(componentId) || componentId === -1) {
      console.error('[TextureSourceSettings] Invalid texture source component ID:', results[0])
      isLoading.value = false
      return
    }

    textureSourceId.value = componentId
    console.log('[TextureSourceSettings] Loaded texture source component ID:', componentId)

    // Initialize editable fields from component
    if (spoutTextureSourceComponent.value) {
      spoutSourceName.value = spoutTextureSourceComponent.value.spout_source
    } else if (clientTextureSourceComponent.value) {
      clientSourceName.value = clientTextureSourceComponent.value.client_source
    }
  } catch (error) {
    console.error('[TextureSourceSettings] Initialization error:', error)
  } finally {
    isLoading.value = false
  }
}

// Send a property update for the given system/field
async function setTextureSourceProperty(ownerSystem: string, fieldName: string, value: any) {
  const client = mikanStore.client
  if (!client || textureSourceId.value === -1) return

  try {
    const request: PropertySetValueRequest = {
      requestTypeName: 'PropertySetValueRequest',
      requestId: 0,
      ownerSystem,
      componentId: textureSourceId.value,
      fieldName,
      fieldValue: createVariantFromValue(value)
    }

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error(`[TextureSourceSettings] Failed to set ${fieldName}:`, error)
  }
}

async function handleSpoutSourceChange() {
  await setTextureSourceProperty('SpoutTextureSourceSystem', 'spout_source', spoutSourceName.value)
}

async function handleClientSourceChange() {
  await setTextureSourceProperty('ClientTextureSourceSystem', 'client_source', clientSourceName.value)
}

function handleReturn() {
  sendRemoteControlCommand('return')
}

// Sync editable fields when the component updates from property events
watch(spoutTextureSourceComponent, (component) => {
  if (component) {
    spoutSourceName.value = component.spout_source
  }
}, { deep: true })

watch(clientTextureSourceComponent, (component) => {
  if (component) {
    clientSourceName.value = component.client_source
  }
}, { deep: true })

// Lifecycle
onMounted(() => {
  initialize()
})

onUnmounted(() => {
  // Cleanup if needed
})
</script>

<style scoped>
.texture-source-settings {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 100%;
  background-color: #1e1e1e;
  color: #e0e0e0;
  padding: 20px;
  overflow-y: auto;
}

.settings-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding-bottom: 10px;
  border-bottom: 2px solid #404040;
}

.settings-header h2 {
  margin: 0;
  font-size: 24px;
  font-weight: 600;
}

.return-btn {
  padding: 8px 16px;
  background-color: #404040;
  color: #e0e0e0;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  transition: background-color 0.2s;
}

.return-btn:hover {
  background-color: #4a4a4a;
}

.loading-message,
.no-source-message {
  padding: 20px;
  text-align: center;
  color: #b0b0b0;
  font-size: 16px;
}

.settings-content {
  flex: 1;
}

.settings-section {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 8px;
  padding: 20px;
}

.property-row {
  display: flex;
  align-items: center;
  margin-bottom: 16px;
  gap: 12px;
}

.property-label {
  min-width: 120px;
  font-weight: 500;
  color: #b0b0b0;
}

.property-readonly {
  color: #e0e0e0;
}

.property-input {
  flex: 1;
  max-width: 300px;
  padding: 6px 12px;
  background-color: #2d2d2d;
  color: #e0e0e0;
  border: 1px solid #404040;
  border-radius: 4px;
  font-size: 14px;
  font-family: monospace;
}

.property-input:focus {
  outline: none;
  border-color: #0078d4;
}
</style>
