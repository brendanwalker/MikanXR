<template>
  <div class="video-source-settings">
    <div class="settings-header">
      <h2>Video Source Settings</h2>
      <button @click="handleReturn" class="return-btn">Return</button>
    </div>

    <div v-if="isLoading" class="loading-message">
      Loading video source settings...
    </div>

    <div v-else-if="!videoSourceId || videoSourceId === -1" class="no-source-message">
      No video source selected
    </div>

    <!-- USB Video Source Settings -->
    <div v-else-if="usbVideoSourceComponent" class="settings-content">
      <div class="settings-section">
        <div class="property-row">
          <label class="property-label">Name</label>
          <span class="property-readonly">{{ usbVideoSourceComponent.component_name }}</span>
        </div>

        <div class="property-row">
          <label class="property-label">Video Device</label>
          <select
            v-model="selectedDevicePath"
            @change="handleDeviceChange"
            class="property-select"
          >
            <option value="">-- None --</option>
            <option
              v-for="device in systemValues?.usb_device_map || []"
              :key="device.key"
              :value="device.key"
            >
              {{ device.value }}
            </option>
          </select>
        </div>

        <div class="property-row">
          <label class="property-label">Resolution</label>
          <select
            v-model="selectedResolution"
            @change="handleResolutionChange"
            class="property-select"
          >
            <option
              v-for="resolution in usbVideoSourceComponent.video_resolutions"
              :key="resolution"
              :value="resolution"
            >
              {{ resolution }}
            </option>
          </select>
        </div>

        <div class="property-row">
          <label class="property-label">FPS</label>
          <select
            v-model="selectedFrameRate"
            @change="handleFrameRateChange"
            class="property-select"
          >
            <option
              v-for="frameRate in usbVideoSourceComponent.video_frame_rates"
              :key="frameRate"
              :value="frameRate"
            >
              {{ frameRate }}
            </option>
          </select>
        </div>

        <div class="property-row">
          <label class="property-label">Format</label>
          <select
            v-model="selectedFormat"
            @change="handleFormatChange"
            class="property-select"
          >
            <option
              v-for="format in usbVideoSourceComponent.video_formats"
              :key="format"
              :value="format"
            >
              {{ format }}
            </option>
          </select>
        </div>

        <!-- Video Setting Sliders -->
        <template v-for="setting in videoSettings" :key="setting.name">
          <div v-if="setting.isValid" class="property-row slider-row">
            <label class="property-label">{{ setting.label }}</label>
            <input
              type="range"
              min="0"
              max="100"
              step="1"
              :value="setting.percent"
              @input="handleSliderChange(setting.name, $event)"
              class="property-slider"
            />
            <span class="slider-value">{{ Math.round(setting.percent) }}</span>
          </div>
        </template>
      </div>
    </div>

    <!-- Network Video Source Settings (TODO) -->
    <div v-else-if="networkVideoSourceComponent" class="settings-content">
      <div class="settings-section">
        <div class="property-row">
          <label class="property-label">Name</label>
          <span class="property-readonly">{{ networkVideoSourceComponent.component_name }}</span>
        </div>
        <p class="info-message">Network video source settings will be implemented later.</p>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted, onUnmounted } from 'vue'
import { useMikanStore } from '../../stores/mikanStore.js'
import { useRemoteControl } from '../../composables/useRemoteControl.js'
import { usePropertyEditor } from '../../composables/usePropertyEditor.js'
import {
  MikanUSBVideoSourceValues,
  MikanUSBVideoSourceSystemValues,
  MikanNetworkVideoSourceValues,
  SetUSBVideoSourceDevice,
  SetUSBVideoSourceResolution,
  SetUSBVideoSourceFrameRate,
  SetUSBVideoSourceFormat,
  CLASS_ID_SET_USBVIDEO_SOURCE_DEVICE,
  CLASS_ID_SET_USBVIDEO_SOURCE_RESOLUTION,
  CLASS_ID_SET_USBVIDEO_SOURCE_FRAME_RATE,
  CLASS_ID_SET_USBVIDEO_SOURCE_FORMAT,
  PropertySetValueRequest,
  CLASS_ID_PROPERTY_SET_VALUE_REQUEST,
  MikanPropertyUpdateEvent,
  SystemGetValuesRequest,
  CLASS_ID_SYSTEM_GET_VALUES_REQUEST,
  SystemGetValuesResponse
} from '@mikanxr/client'

import { useComponentStore } from '../../stores/componentStore.js'

const mikanStore = useMikanStore()
const { sendRemoteControlCommand, sendRemoteControlCommandWithResults } = useRemoteControl()
const { createVariantFromValue } = usePropertyEditor()
const componentStore = useComponentStore()

// State
const isLoading = ref(true)
const videoSourceId = ref<number>(-1)
const systemValues = ref<MikanUSBVideoSourceSystemValues | null>(null)

// Selection state for dropdowns
const selectedDevicePath = ref<string>('')
const selectedResolution = ref<string>('')
const selectedFrameRate = ref<string>('')
const selectedFormat = ref<string>('')

// Get the video source component
const usbVideoSourceComponent = computed(() => {
  if (videoSourceId.value === -1) return null
  return componentStore.getUSBVideoSourceComponent(videoSourceId.value)
})

const networkVideoSourceComponent = computed(() => {
  if (videoSourceId.value === -1) return null
  return componentStore.getNetworkVideoSourceComponent(videoSourceId.value)
})

// Video settings configuration
interface VideoSetting {
  name: string
  label: string
  isValid: boolean
  percent: number
  fractionField: string
}

const videoSettings = computed((): VideoSetting[] => {
  const component = usbVideoSourceComponent.value
  if (!component) return []

  return [
    {
      name: 'brightness',
      label: 'Brightness',
      isValid: component.brightness_valid,
      percent: component.brightness_fraction * 100,
      fractionField: 'brightness_fraction'
    },
    {
      name: 'contrast',
      label: 'Contrast',
      isValid: component.contrast_valid,
      percent: component.contrast_fraction * 100,
      fractionField: 'contrast_fraction'
    },
    {
      name: 'hue',
      label: 'Hue',
      isValid: component.hue_valid,
      percent: component.hue_fraction * 100,
      fractionField: 'hue_fraction'
    },
    {
      name: 'saturation',
      label: 'Saturation',
      isValid: component.saturation_valid,
      percent: component.saturation_fraction * 100,
      fractionField: 'saturation_fraction'
    },
    {
      name: 'sharpness',
      label: 'Sharpness',
      isValid: component.sharpness_valid,
      percent: component.sharpness_fraction * 100,
      fractionField: 'sharpness_fraction'
    },
    {
      name: 'gamma',
      label: 'Gamma',
      isValid: component.gamma_valid,
      percent: component.gamma_fraction * 100,
      fractionField: 'gamma_fraction'
    },
    {
      name: 'white_balance',
      label: 'White Balance',
      isValid: component.white_balance_valid,
      percent: component.white_balance_fraction * 100,
      fractionField: 'white_balance_fraction'
    },
    {
      name: 'red_balance',
      label: 'Red Balance',
      isValid: component.red_balance_valid,
      percent: component.red_balance_fraction * 100,
      fractionField: 'red_balance_fraction'
    },
    {
      name: 'green_balance',
      label: 'Green Balance',
      isValid: component.green_balance_valid,
      percent: component.green_balance_fraction * 100,
      fractionField: 'green_balance_fraction'
    },
    {
      name: 'blue_balance',
      label: 'Blue Balance',
      isValid: component.blue_balance_valid,
      percent: component.blue_balance_fraction * 100,
      fractionField: 'blue_balance_fraction'
    },
    {
      name: 'gain',
      label: 'Gain',
      isValid: component.gain_valid,
      percent: component.gain_fraction * 100,
      fractionField: 'gain_fraction'
    },
    {
      name: 'pan',
      label: 'Pan',
      isValid: component.pan_valid,
      percent: component.pan_fraction * 100,
      fractionField: 'pan_fraction'
    },
    {
      name: 'tilt',
      label: 'Tilt',
      isValid: component.tilt_valid,
      percent: component.tilt_fraction * 100,
      fractionField: 'tilt_fraction'
    },
    {
      name: 'roll',
      label: 'Roll',
      isValid: component.roll_valid,
      percent: component.roll_fraction * 100,
      fractionField: 'roll_fraction'
    },
    {
      name: 'zoom',
      label: 'Zoom',
      isValid: component.zoom_valid,
      percent: component.zoom_fraction * 100,
      fractionField: 'zoom_fraction'
    },
    {
      name: 'exposure',
      label: 'Exposure',
      isValid: component.exposure_valid,
      percent: component.exposure_fraction * 100,
      fractionField: 'exposure_fraction'
    },
    {
      name: 'iris',
      label: 'Iris',
      isValid: component.iris_valid,
      percent: component.iris_fraction * 100,
      fractionField: 'iris_fraction'
    },
    {
      name: 'focus',
      label: 'Focus',
      isValid: component.focus_valid,
      percent: component.focus_fraction * 100,
      fractionField: 'focus_fraction'
    }
  ]
})

// Initialize component
async function initialize() {
  isLoading.value = true

  try {
    // Get the video source component ID from the current app stage
    const results = await sendRemoteControlCommandWithResults('get_video_source_component_id')

    if (!results || results.length === 0) {
      console.error('[VideoSourceSettings] Failed to get video source component ID')
      isLoading.value = false
      return
    }

    // Parse the component ID from the first result string
    // The C++ handler returns the component ID as a string via std::to_string()
    const componentIdString = results[0]
    const componentId = parseInt(componentIdString, 10)

    if (isNaN(componentId) || componentId === -1) {
      console.error('[VideoSourceSettings] Invalid video source component ID:', componentIdString)
      isLoading.value = false
      return
    }

    videoSourceId.value = componentId
    console.log('[VideoSourceSettings] Loaded video source component ID:', componentId)

    // Fetch USB video source system values
    await fetchSystemValues()

    // Initialize dropdown values from component
    if (usbVideoSourceComponent.value) {
      selectedDevicePath.value = usbVideoSourceComponent.value.current_device_path
      selectedResolution.value = usbVideoSourceComponent.value.video_resolution
      selectedFrameRate.value = usbVideoSourceComponent.value.video_fps
      selectedFormat.value = usbVideoSourceComponent.value.video_format
    }
  } catch (error) {
    console.error('[VideoSourceSettings] Initialization error:', error)
  } finally {
    isLoading.value = false
  }
}

async function fetchSystemValues() {
  const client = mikanStore.client
  if (!client) return

  try {
    const request: SystemGetValuesRequest = {
      requestTypeId: CLASS_ID_SYSTEM_GET_VALUES_REQUEST,
      requestTypeName: 'SystemGetValuesRequest',
      requestId: 0,
      ownerSystem: 'USBVideoSourceSystem'
    }

    const future = client.sendRequest(request)
    const response = await future.await() as SystemGetValuesResponse

    if (response.resultCode === 0) {
      systemValues.value = response.valuesObject.instance as MikanUSBVideoSourceSystemValues
    }
  } catch (error) {
    console.error('[VideoSourceSettings] Failed to fetch system values:', error)
  }
}

// Event handlers
async function handleDeviceChange() {
  const client = mikanStore.client
  if (!client || videoSourceId.value === -1) return

  try {
    const request: SetUSBVideoSourceDevice = {
      requestTypeId: CLASS_ID_SET_USBVIDEO_SOURCE_DEVICE,
      requestTypeName: 'SetUSBVideoSourceDevice',
      requestId: 0,
      video_source_id: videoSourceId.value,
      device_path: selectedDevicePath.value
    }

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error('[VideoSourceSettings] Failed to set device:', error)
  }
}

async function handleResolutionChange() {
  const client = mikanStore.client
  if (!client || videoSourceId.value === -1) return

  try {
    const request: SetUSBVideoSourceResolution = {
      requestTypeId: CLASS_ID_SET_USBVIDEO_SOURCE_RESOLUTION,
      requestTypeName: 'SetUSBVideoSourceResolution',
      requestId: 0,
      video_source_id: videoSourceId.value,
      resolution: selectedResolution.value
    }

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error('[VideoSourceSettings] Failed to set resolution:', error)
  }
}

async function handleFrameRateChange() {
  const client = mikanStore.client
  if (!client || videoSourceId.value === -1) return

  try {
    const request: SetUSBVideoSourceFrameRate = {
      requestTypeId: CLASS_ID_SET_USBVIDEO_SOURCE_FRAME_RATE,
      requestTypeName: 'SetUSBVideoSourceFrameRate',
      requestId: 0,
      video_source_id: videoSourceId.value,
      frame_rate: selectedFrameRate.value
    }

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error('[VideoSourceSettings] Failed to set frame rate:', error)
  }
}

async function handleFormatChange() {
  const client = mikanStore.client
  if (!client || videoSourceId.value === -1) return

  try {
    const request: SetUSBVideoSourceFormat = {
      requestTypeId: CLASS_ID_SET_USBVIDEO_SOURCE_FORMAT,
      requestTypeName: 'SetUSBVideoSourceFormat',
      requestId: 0,
      video_source_id: videoSourceId.value,
      format: selectedFormat.value
    }

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error('[VideoSourceSettings] Failed to set format:', error)
  }
}

async function handleSliderChange(settingName: string, event: Event) {
  const client = mikanStore.client
  if (!client || videoSourceId.value === -1) return

  const target = event.target as HTMLInputElement
  const percent = parseFloat(target.value)
  const fraction = percent / 100.0

  // Find the setting to get the fraction field name
  const setting = videoSettings.value.find(s => s.name === settingName)
  if (!setting) return

  try {
    const fieldValue = createVariantFromValue(fraction)

    const request: PropertySetValueRequest = {
      requestTypeId: CLASS_ID_PROPERTY_SET_VALUE_REQUEST,
      requestTypeName: 'PropertySetValueRequest',
      requestId: 0,
      ownerSystem: 'USBVideoSourceSystem',
      componentId: videoSourceId.value,
      fieldName: setting.fractionField,
      fieldValue: fieldValue
    }

    const future = client.sendRequest(request)
    await future.await()
  } catch (error) {
    console.error(`[VideoSourceSettings] Failed to set ${settingName}:`, error)
  }
}

function handleReturn() {
  sendRemoteControlCommand('return')
}

// Watch for component updates from property events
watch(() => usbVideoSourceComponent.value, (newComponent) => {
  if (newComponent) {
    // Update dropdown values if they change from the server
    selectedDevicePath.value = newComponent.current_device_path
    selectedResolution.value = newComponent.video_resolution
    selectedFrameRate.value = newComponent.video_fps
    selectedFormat.value = newComponent.video_format
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
.video-source-settings {
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
.no-source-message,
.info-message {
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

.slider-row {
  align-items: center;
}

.property-label {
  min-width: 120px;
  font-weight: 500;
  color: #b0b0b0;
}

.property-readonly {
  color: #e0e0e0;
}

.property-select {
  flex: 1;
  padding: 6px 12px;
  background-color: #2d2d2d;
  color: #e0e0e0;
  border: 1px solid #404040;
  border-radius: 4px;
  font-size: 14px;
  cursor: pointer;
  max-width: 300px;
}

.property-select:focus {
  outline: none;
  border-color: #0078d4;
}

.property-slider {
  flex: 1;
  max-width: 200px;
  height: 6px;
  background: #404040;
  border-radius: 3px;
  outline: none;
  cursor: pointer;
}

.property-slider::-webkit-slider-thumb {
  appearance: none;
  width: 16px;
  height: 16px;
  background: #0078d4;
  border-radius: 50%;
  cursor: pointer;
}

.property-slider::-moz-range-thumb {
  width: 16px;
  height: 16px;
  background: #0078d4;
  border-radius: 50%;
  border: none;
  cursor: pointer;
}

.slider-value {
  min-width: 40px;
  text-align: right;
  color: #e0e0e0;
  font-size: 14px;
  font-family: monospace;
}
</style>
