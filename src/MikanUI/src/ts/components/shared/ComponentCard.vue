<template>
  <div
    class="component-card"
    :class="{ selected: isSelected, clickable: selectable && !editable }"
    @click="handleClick"
  >
    <div class="component-header">
      <h3 class="component-name">{{ component.component_name || 'Unnamed Component' }}</h3>
      <span class="component-id">ID: {{ componentId }}</span>
    </div>
    <div class="component-properties">
      <div v-for="(value, key) in displayProperties" :key="key" class="property-row">
        <span class="property-label">{{ formatPropertyName(key) }}:</span>
        <div v-if="editable" class="property-editor" @click.stop>
          <PropertyField
            :field-name="key"
            :field-value="value"
            :owner-system="ownerSystem"
            :component-id="componentId"
            :field-meta-type="getFieldMetaType((component as any).component_class || '', key)"
            @update="handlePropertyUpdate"
          />
        </div>
        <span v-else class="property-value">{{ formatPropertyValue(value) }}</span>
      </div>
    </div>
    <div v-if="componentFunctions.length > 0" class="component-functions">
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
import type { MikanComponentValues, MikanFunctionDescriptor } from '@mikanxr/client'
import PropertyField from './PropertyField.vue'
import { useDeveloperFields } from '../../composables/useDeveloperFields.js'
import { useFieldMetadata } from '../../composables/useFieldMetadata.js'
import { useComponentStore } from '../../stores/componentStore.js'
import { useMikanStore } from '../../stores/mikanStore.js'

interface Props {
  componentId: number
  component: MikanComponentValues
  ownerSystem: string
  showAllProperties?: boolean
  selectable?: boolean
  isSelected?: boolean
  editable?: boolean
}

const props = withDefaults(defineProps<Props>(), {
  showAllProperties: false,
  selectable: false,
  isSelected: false,
  editable: false
})

const emit = defineEmits<{
  (e: 'select', componentId: number): void
  (e: 'update', fieldName: string, fieldValue: any): void
}>()

const { filterComponentProperties, isDeveloperMode } = useDeveloperFields()
const { getFieldMetaType } = useFieldMetadata()
const componentStore = useComponentStore()
const mikanStore = useMikanStore()

// Function list for this component
const componentFunctions = ref<MikanFunctionDescriptor[]>([])

// Fetch functions when component is mounted
onMounted(async () => {
  await fetchFunctions()
})

async function fetchFunctions() {
  // First check if functions are already cached
  const cachedFunctions = componentStore.getComponentFunctions(props.componentId, props.ownerSystem)
  if (cachedFunctions.length > 0) {
    componentFunctions.value = cachedFunctions
    return
  }

  // Fetch from server if not cached
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
    console.error('[ComponentCard] Cannot invoke function: no client connection')
    return
  }

  console.log(`[ComponentCard] Invoking function "${functionName}" on component ${props.componentId}`)
  await componentStore.invokeComponentFunction(
    client,
    props.ownerSystem,
    props.componentId,
    functionName
  )
}

function handleClick() {
  if (props.selectable && !props.editable) {
    emit('select', props.componentId)
  }
}

function handlePropertyUpdate(fieldName: string, fieldValue: any) {
  emit('update', fieldName, fieldValue)
}

// Filter out internal properties and format for display
const displayProperties = computed(() => {
  // Access isDeveloperMode to create reactive dependency
  const devMode = isDeveloperMode.value

  const filtered: Record<string, any> = {}
  const excludeKeys = ['component_id']

  // First filter out internal/excluded properties
  for (const [key, value] of Object.entries(props.component)) {
    if (!excludeKeys.includes(key) && !key.startsWith('_')) {
      filtered[key] = value
    }
  }

  // Then filter based on developer mode settings
  const componentClass = (props.component as any).component_class || ''
  //console.log(`[ComponentCard] Component class: "${componentClass}", dev mode: ${devMode}`)
  return filterComponentProperties(componentClass, filtered)
})

// Format property names from snake_case to Title Case
function formatPropertyName(name: string): string {
  return name
    .split('_')
    .map(word => word.charAt(0).toUpperCase() + word.slice(1))
    .join(' ')
}

// Format property values for display
function formatPropertyValue(value: any): string {
  if (value === null || value === undefined) {
    return 'N/A'
  }

  if (typeof value === 'boolean') {
    return value ? 'Yes' : 'No'
  }

  if (typeof value === 'object') {
    // Handle nested objects (like vectors, quaternions)
    if ('x' in value && 'y' in value && 'z' in value) {
      return `(${value.x.toFixed(2)}, ${value.y.toFixed(2)}, ${value.z.toFixed(2)})`
    }
    if ('w' in value && 'x' in value && 'y' in value && 'z' in value) {
      return `(${value.w.toFixed(2)}, ${value.x.toFixed(2)}, ${value.y.toFixed(2)}, ${value.z.toFixed(2)})`
    }
    return JSON.stringify(value)
  }

  if (typeof value === 'number') {
    return Number.isInteger(value) ? value.toString() : value.toFixed(2)
  }

  return String(value)
}
</script>

<style scoped>
.component-card {
  background: rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  padding: 10px;
  margin-bottom: 12px;
  transition: all 0.2s ease;
}

.component-card.clickable {
  cursor: pointer;
}

.component-card.clickable:hover {
  background: rgba(255, 255, 255, 0.08);
  border-color: rgba(255, 255, 255, 0.2);
}

.component-card.selected {
  background: rgba(92, 184, 92, 0.2);
  border-color: #5cb85c;
}

.component-card.selected:hover {
  background: rgba(92, 184, 92, 0.25);
  border-color: #5cb85c;
}

.component-card:not(.clickable):hover {
  background: rgba(255, 255, 255, 0.08);
  border-color: rgba(255, 255, 255, 0.2);
}

.component-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
  padding-bottom: 8px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.component-name {
  font-size: 16px;
  font-weight: 600;
  margin: 0;
  color: #fff;
}

.component-id {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.6);
  font-family: monospace;
}

.component-properties {
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

.property-editor {
  display: flex;
  width: 100%;
  min-width: 0;
}

.component-functions {
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
