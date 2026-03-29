<template>
  <div class="usb-video-source-card">
    <div class="card-header">
      <h3 class="card-name">{{ component.component_name || 'Unnamed Component' }}</h3>
      <span class="card-id">ID: {{ componentId }}</span>
    </div>
    <div class="card-properties">
      <div v-if="deviceFriendlyName" class="property-row">
        <span class="property-label">Device Name:</span>
        <span class="property-value">{{ deviceFriendlyName }}</span>
      </div>
      <div v-if="component.video_mode" class="property-row">
        <span class="property-label">Video Mode:</span>
        <span class="property-value">{{ component.video_mode }}</span>
      </div>
    </div>
    <div v-if="componentFunctions.length > 0" class="card-functions">
      <button
        v-for="func in componentFunctions"
        :key="func.functionName"
        class="function-button"
        @click.stop="handleFunctionClick(func.functionName)"
      >
        {{ func.displayName }}
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, ref, onMounted } from 'vue'
import { type MikanComponentValues, type MikanFunctionDescriptor } from '@mikanxr/client'
import { useComponentStore } from '../../stores/componentStore.js'
import { useMikanStore } from '../../stores/mikanStore.js'

interface Props {
  componentId: number
  component: MikanComponentValues
  ownerSystem: string
}

const props = defineProps<Props>()
const componentStore = useComponentStore()
const mikanStore = useMikanStore()

const componentFunctions = ref<MikanFunctionDescriptor[]>([])

const usbDeviceMap = computed(() => {
  const sysValues = componentStore.getUSBVideoSourceSystemValues()
  if (!sysValues) return []
  return Object.entries(sysValues.usb_device_map || {}).map(([key, value]) => ({ key, value }))
})

onMounted(async () => {
  await fetchFunctions()
})

async function fetchFunctions() {
  const cachedFunctions = componentStore.getComponentFunctions(props.componentId, props.ownerSystem)
  if (cachedFunctions.length > 0) {
    componentFunctions.value = cachedFunctions
    return
  }

  const client = mikanStore.client as any
  if (!client) return

  const componentClass = (props.component as any).component_class || ''
  if (!componentClass) return

  const functions = await componentStore.fetchComponentFunctions(
    client,
    props.ownerSystem,
    componentClass,
    props.componentId
  )
  componentFunctions.value = functions
}

async function handleFunctionClick(functionName: string) {
  const client = mikanStore.client as any
  if (!client) {
    console.error('[USBVideoSourceCard] Cannot invoke function: no client connection')
    return
  }

  console.log(`[USBVideoSourceCard] Invoking function "${functionName}" on component ${props.componentId}`)
  await componentStore.invokeComponentFunction(
    client,
    props.ownerSystem,
    props.componentId,
    functionName
  )
}

const deviceFriendlyName = computed(() => {
  const currentDevicePath = (props.component as any).current_device_path
  console.log('[USBVideoSourceCard] current_device_path:', currentDevicePath)

  if (!currentDevicePath || usbDeviceMap.value.length === 0) {
    console.log('[USBVideoSourceCard] Returning null - no device path or empty map')
    return null
  }

  const device = usbDeviceMap.value.find(d => d.key === currentDevicePath)
  console.log('[USBVideoSourceCard] deviceFriendlyName:', device?.value || null)
  return device?.value || null
})
</script>

<style scoped>
.usb-video-source-card {
  background: rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  padding: 10px;
  margin-bottom: 12px;
  transition: all 0.2s ease;
}

.usb-video-source-card:hover {
  background: rgba(255, 255, 255, 0.08);
  border-color: rgba(255, 255, 255, 0.2);
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
  padding-bottom: 8px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.card-name {
  font-size: 16px;
  font-weight: 600;
  margin: 0;
  color: #fff;
}

.card-id {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.6);
  font-family: monospace;
}

.card-properties {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.property-row {
  display: flex;
  flex-direction: column;
  gap: 4px;
  padding: 8px 0;
  font-size: 14px;
}

.property-label {
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
  text-align: left;
}

.property-value {
  color: #fff;
  font-family: monospace;
  text-align: left;
}

.card-functions {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 12px;
  padding-top: 12px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
}

.function-button {
  padding: 8px 16px;
  background: rgba(92, 184, 92, 0.2);
  border: 1px solid rgba(92, 184, 92, 0.5);
  border-radius: 4px;
  color: #5cb85c;
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
  white-space: nowrap;
}

.function-button:hover {
  background: rgba(92, 184, 92, 0.3);
  border-color: #5cb85c;
  color: #6cd76c;
}

.function-button:active {
  background: rgba(92, 184, 92, 0.4);
  transform: translateY(1px);
}
</style>
