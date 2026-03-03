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
            @update="handlePropertyUpdate"
          />
        </div>
        <span v-else class="property-value">{{ formatPropertyValue(value) }}</span>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import type { MikanComponentValues } from '@mikanxr/client'
import PropertyField from './PropertyField.vue'

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
  const filtered: Record<string, any> = {}
  const excludeKeys = ['component_name', 'component_id']

  for (const [key, value] of Object.entries(props.component)) {
    if (!excludeKeys.includes(key) && !key.startsWith('_')) {
      filtered[key] = value
    }
  }

  return filtered
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
  padding: 16px;
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
  justify-content: space-between;
  align-items: baseline;
  padding: 4px 0;
  font-size: 14px;
}

.property-label {
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
  margin-right: 12px;
  min-width: 120px;
}

.property-value {
  color: #fff;
  font-family: monospace;
  text-align: right;
  flex: 1;
}

.property-editor {
  flex: 1;
  display: flex;
  justify-content: flex-end;
}
</style>
