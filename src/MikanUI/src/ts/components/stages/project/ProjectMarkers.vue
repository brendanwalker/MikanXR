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
          <button @click="handleAddMarker" class="icon-only-btn add-btn" title="Add Marker">
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
        <div v-if="markerImageUrl" class="marker-preview">
          <img :src="markerImageUrl" alt="AruCo Marker" class="marker-image" />
        </div>
        <ComponentCard
          :component-id="selectedMarkerId"
          :component="selectedMarkerComponent"
          owner-system="MarkerObjectSystem"
          :editable="true"
          :field-constraints="markerComponentFieldConstraints"
        />
      </div>

      <!-- ArUco Settings -->
      <div v-if="markerSystemValues" class="settings-section">
        <h3>ArUco Settings</h3>
        <div class="property-row">
          <label class="property-label">Dictionary</label>
          <select
            class="property-select"
            :value="markerSystemValues.aruco_dictionary_type"
            @change="setSystemProperty('aruco_dictionary_type', parseInt(($event.target as HTMLSelectElement).value))"
          >
            <option v-for="opt in dictionaryTypeOptions" :key="opt.value" :value="opt.value">{{ opt.label }}</option>
          </select>
        </div>
      </div>

      <!-- ChArUco Settings -->
      <div v-if="markerSystemValues" class="settings-section">
        <h3>ChArUco Settings</h3>
        <div class="property-row">
          <label class="property-label">Dictionary</label>
          <select
            class="property-select"
            :value="markerSystemValues.charuco_dictionary_type"
            @change="setSystemProperty('charuco_dictionary_type', parseInt(($event.target as HTMLSelectElement).value))"
          >
            <option v-for="opt in dictionaryTypeOptions" :key="opt.value" :value="opt.value">{{ opt.label }}</option>
          </select>
        </div>
        <div class="property-row">
          <label class="property-label">Rows</label>
          <select
            class="property-select"
            :value="markerSystemValues.charuco_rows"
            @change="setSystemProperty('charuco_rows', parseInt(($event.target as HTMLSelectElement).value))"
          >
            <option v-for="n in [4,5,6,7,8,9,10,11,12,13,14,15]" :key="n" :value="n">{{ n }}</option>
          </select>
        </div>
        <div class="property-row">
          <label class="property-label">Cols</label>
          <select
            class="property-select"
            :value="markerSystemValues.charuco_cols"
            @change="setSystemProperty('charuco_cols', parseInt(($event.target as HTMLSelectElement).value))"
          >
            <option v-for="n in [4,5,6,7,8,9,10,11,12,13,14,15]" :key="n" :value="n">{{ n }}</option>
          </select>
        </div>
        <div class="property-row">
          <label class="property-label">Square Length (mm)</label>
          <input
            type="number"
            class="property-input"
            :value="markerSystemValues.charuco_square_length_mm"
            @change="setSystemProperty('charuco_square_length_mm', parseFloat(($event.target as HTMLInputElement).value))"
          />
        </div>
        <div class="property-row">
          <label class="property-label">Marker Length (mm)</label>
          <input
            type="number"
            class="property-input"
            :value="markerSystemValues.charuco_marker_length_mm"
            @change="setSystemProperty('charuco_marker_length_mm', parseFloat(($event.target as HTMLInputElement).value))"
          />
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import { useComponentStore } from '../../../stores/componentStore.js'
import { useMikanStore } from '../../../stores/mikanStore.js'
import ComponentCard from '../../shared/ComponentCard.vue'
import { MikanClient, MikanAPIResult, MikanMarkerComponentValues, GetArucoMarkerImageRequest, ArucoMarkerImageResponse, SystemGetValuesRequest, SystemGetValuesResponse, MikanMarkerSystemValues, MikanMarkerDictionaryType, PropertySetValueRequest } from '@mikanxr/client'
import { usePropertyEditor } from '../../../composables/usePropertyEditor.js'

const { createVariantFromValue } = usePropertyEditor()

// Marker system values state
const markerSystemValues = ref<MikanMarkerSystemValues | null>(null)

const ARUCO_DICT_MAX_ID: Record<number, number> = {
  [MikanMarkerDictionaryType.DICT_4x4]: 19,
  [MikanMarkerDictionaryType.DICT_5x5]: 24,
  [MikanMarkerDictionaryType.DICT_6x6]: 35,
  [MikanMarkerDictionaryType.DICT_7x7]: 78,
}

const markerComponentFieldConstraints = computed(() => {
  const dictType = markerSystemValues.value?.aruco_dictionary_type ?? MikanMarkerDictionaryType.DICT_4x4
  const maxId = ARUCO_DICT_MAX_ID[dictType] ?? 78
  return {
    aruco_id: { min: -1, max: maxId }
  }
})

const dictionaryTypeOptions = [
  { value: MikanMarkerDictionaryType.DICT_4x4, label: '4x4' },
  { value: MikanMarkerDictionaryType.DICT_5x5, label: '5x5' },
  { value: MikanMarkerDictionaryType.DICT_6x6, label: '6x6' },
  { value: MikanMarkerDictionaryType.DICT_7x7, label: '7x7' },
]

async function fetchMarkerSystemValues() {
  const client = mikanStore.client as MikanClient | null
  if (!client) return

  try {
    const request = new SystemGetValuesRequest()
    request.ownerSystem = 'MarkerObjectSystem'

    const future = client.sendRequest(request)
    const response = await future.await() as SystemGetValuesResponse

    if (response.resultCode === 0) {
      console.log('[ProjectMarkers] Fetched marker system values: ', response.valuesObject.instance)
      markerSystemValues.value = response.valuesObject.instance as MikanMarkerSystemValues
    } else {
      console.error('[ProjectMarkers] Failed to fetch marker system values: error code', response.resultCode)
    }
  } catch (error) {
    console.error('[ProjectMarkers] Failed to fetch marker system values:', error)
  }
}

async function setSystemProperty(fieldName: string, value: any) {
  const client = mikanStore.client as MikanClient | null
  if (!client) return

  try {
    const request = new PropertySetValueRequest()
    request.ownerSystem = 'MarkerObjectSystem'
    request.fieldName = fieldName
    request.fieldValue = createVariantFromValue(value)

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error(`[ProjectMarkers] Failed to set system property "${fieldName}":`, error)
  }
}

// Marker image state
const markerImageUrl = ref<string | null>(null)
const markerImageCache = new Map<number, string>()

async function fetchMarkerImage(arucoId: number) {
  if (!mikanStore.client) return

  const cached = markerImageCache.get(arucoId)
  if (cached) {
    markerImageUrl.value = cached
    return
  }

  try {
    const request = new GetArucoMarkerImageRequest()
    request.markerId = arucoId
    request.imageSize = 0
    const future = (mikanStore.client as MikanClient).sendRequest(request)
    const response = await future.getResponse()

    if (response.resultCode === MikanAPIResult.Success) {
      const imageResponse = response as ArucoMarkerImageResponse
      if (imageResponse.imageData) {
        const dataUrl = `data:image/png;base64,${imageResponse.imageData}`
        markerImageCache.set(arucoId, dataUrl)
        markerImageUrl.value = dataUrl
      }
    }
  } catch (err) {
    console.error('[ProjectMarkers] Failed to fetch marker image:', err)
  }
}

const componentStore = useComponentStore()
const mikanStore = useMikanStore()

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
async function handleAddMarker() {
  if (!mikanStore.client) {
    console.error('[ProjectMarkers] No client connection')
    return
  }
  await componentStore.createObject(mikanStore.client as MikanClient, 'MarkerComponent')
}

async function handleRemoveMarker() {
  if (selectedMarkerId.value === -1) {
    console.error('[ProjectMarkers] No marker selected')
    return
  }
  if (!mikanStore.client) {
    console.error('[ProjectMarkers] No client connection')
    return
  }
  await componentStore.destroyObject(mikanStore.client as MikanClient, 'MarkerComponent', selectedMarkerId.value)
  selectedMarkerId.value = -1
}

// Fetch marker image when the selected component or its aruco_id changes
watch(
  () => (selectedMarkerComponent.value as MikanMarkerComponentValues | null | undefined)?.aruco_id,
  (arucoId) => {
    if (arucoId !== undefined && arucoId >= 0) {
      fetchMarkerImage(arucoId)
    } else {
      markerImageUrl.value = null
    }
  }
)

// Persist selection state
watch(selectedMarkerId, (newValue) => {
  if (newValue !== -1) {
    sessionStorage.setItem('projectMarkers.selectedMarkerId', newValue.toString())
  }
})

// Auto-select last entry when list changes and nothing valid is selected
watch(markerComponents, (components) => {
  if (components.length === 0) {
    selectedMarkerId.value = -1
  } else if (selectedMarkerId.value === -1 || !components.some(c => c.component_id === selectedMarkerId.value)) {
    selectedMarkerId.value = components[components.length - 1].component_id
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
onMounted(async () => {
  restoreSelectionState()
  // Apply auto-select in case the store is already populated
  const components = markerComponents.value
  if (components.length === 0) {
    selectedMarkerId.value = -1
  } else if (selectedMarkerId.value === -1 || !components.some(c => c.component_id === selectedMarkerId.value)) {
    selectedMarkerId.value = components[components.length - 1].component_id
  }
  await fetchMarkerSystemValues()
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

.settings-section {
  background-color: #2d2d2d;
  border: 1px solid #404040;
  border-radius: 4px;
  padding: 12px;
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.settings-section h3 {
  color: #5cb85c;
  margin: 0 0 4px 0;
  font-size: 15px;
  font-weight: 600;
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

/* Icon button styles now imported from common-buttons.css */
</style>
