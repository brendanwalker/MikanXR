<template>
  <div class="property-editor">
    <div class="property-header">
      <h3 class="property-title">{{ title }}</h3>
      <button class="close-button" @click="$emit('close')">×</button>
    </div>

    <div class="property-content">
      <div v-for="(value, key) in editableProperties" :key="key" class="property-field">
        <label :for="`prop-${key}`" class="property-label">
          {{ formatPropertyName(key) }}
        </label>

        <div class="property-input">
          <!-- Boolean -->
          <input
            v-if="isBoolean(value)"
            :id="`prop-${key}`"
            type="checkbox"
            :checked="value"
            @change="handleChange(key, ($event.target as HTMLInputElement).checked)"
            class="checkbox-input"
          />

          <!-- Number -->
          <input
            v-else-if="isNumber(value)"
            :id="`prop-${key}`"
            type="number"
            :value="value"
            @input="handleChange(key, parseFloat(($event.target as HTMLInputElement).value))"
            :step="isInteger(value) ? 1 : 0.01"
            class="number-input"
          />

          <!-- String -->
          <input
            v-else-if="isString(value)"
            :id="`prop-${key}`"
            type="text"
            :value="value"
            @input="handleChange(key, ($event.target as HTMLInputElement).value)"
            class="text-input"
          />

          <!-- Vector3 -->
          <div v-else-if="isVector3(value)" class="vector-input">
            <input
              type="number"
              :value="value.x"
              @input="handleVectorChange(key, 'x', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="X"
            />
            <input
              type="number"
              :value="value.y"
              @input="handleVectorChange(key, 'y', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="Y"
            />
            <input
              type="number"
              :value="value.z"
              @input="handleVectorChange(key, 'z', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="Z"
            />
          </div>

          <!-- Quaternion -->
          <div v-else-if="isQuaternion(value)" class="quaternion-input">
            <input
              type="number"
              :value="value.w"
              @input="handleQuaternionChange(key, 'w', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="W"
            />
            <input
              type="number"
              :value="value.x"
              @input="handleQuaternionChange(key, 'x', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="X"
            />
            <input
              type="number"
              :value="value.y"
              @input="handleQuaternionChange(key, 'y', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="Y"
            />
            <input
              type="number"
              :value="value.z"
              @input="handleQuaternionChange(key, 'z', parseFloat(($event.target as HTMLInputElement).value), value)"
              step="0.01"
              placeholder="Z"
            />
          </div>

          <!-- Unsupported type -->
          <span v-else class="unsupported-type">
            {{ typeof value }} (not editable)
          </span>
        </div>
      </div>
    </div>

    <div class="property-footer">
      <button @click="handleSave" class="save-button" :disabled="!hasChanges">
        Save Changes
      </button>
      <button @click="handleReset" class="reset-button" :disabled="!hasChanges">
        Reset
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import type { MikanComponentValues } from '@mikanxr/client'

interface Props {
  title: string
  componentId: number
  component: MikanComponentValues
  ownerSystem: string
}

const props = defineProps<Props>()
const emit = defineEmits<{
  (e: 'close'): void
  (e: 'save', changes: Record<string, any>): void
}>()

// Track property changes
const propertyChanges = ref<Record<string, any>>({})

// Filter out internal properties
const editableProperties = computed(() => {
  const filtered: Record<string, any> = {}
  const excludeKeys = ['component_name', 'component_id']

  for (const [key, value] of Object.entries(props.component)) {
    if (!excludeKeys.includes(key) && !key.startsWith('_')) {
      filtered[key] = value
    }
  }

  return filtered
})

const hasChanges = computed(() => Object.keys(propertyChanges.value).length > 0)

// Type checking helpers
function isBoolean(value: any): boolean {
  return typeof value === 'boolean'
}

function isNumber(value: any): boolean {
  return typeof value === 'number'
}

function isInteger(value: any): boolean {
  return Number.isInteger(value)
}

function isString(value: any): boolean {
  return typeof value === 'string'
}

function isVector3(value: any): boolean {
  return value && typeof value === 'object' && 'x' in value && 'y' in value && 'z' in value && !('w' in value)
}

function isQuaternion(value: any): boolean {
  return value && typeof value === 'object' && 'w' in value && 'x' in value && 'y' in value && 'z' in value
}

// Format property names
function formatPropertyName(name: string): string {
  return name
    .split('_')
    .map(word => word.charAt(0).toUpperCase() + word.slice(1))
    .join(' ')
}

// Handle property changes
function handleChange(key: string, value: any) {
  propertyChanges.value[key] = value
}

function handleVectorChange(key: string, axis: 'x' | 'y' | 'z', value: number, originalValue: any) {
  const updatedVector = { ...originalValue, [axis]: value }
  propertyChanges.value[key] = updatedVector
}

function handleQuaternionChange(key: string, component: 'w' | 'x' | 'y' | 'z', value: number, originalValue: any) {
  const updatedQuat = { ...originalValue, [component]: value }
  propertyChanges.value[key] = updatedQuat
}

function handleSave() {
  emit('save', propertyChanges.value)
  propertyChanges.value = {}
}

function handleReset() {
  propertyChanges.value = {}
}
</script>

<style scoped>
.property-editor {
  background: rgba(0, 0, 0, 0.9);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 8px;
  padding: 20px;
  max-width: 600px;
  margin: 0 auto;
}

.property-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding-bottom: 12px;
  border-bottom: 2px solid rgba(255, 255, 255, 0.2);
}

.property-title {
  font-size: 20px;
  font-weight: 700;
  margin: 0;
  color: #fff;
}

.close-button {
  background: none;
  border: none;
  color: rgba(255, 255, 255, 0.6);
  font-size: 32px;
  line-height: 1;
  cursor: pointer;
  padding: 0;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: color 0.2s;
}

.close-button:hover {
  color: #fff;
}

.property-content {
  display: flex;
  flex-direction: column;
  gap: 16px;
  max-height: 60vh;
  overflow-y: auto;
  padding-right: 8px;
}

.property-field {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.property-label {
  font-size: 14px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.8);
}

.property-input {
  display: flex;
  align-items: center;
}

input[type="text"],
input[type="number"] {
  background: rgba(255, 255, 255, 0.1);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  padding: 8px 12px;
  color: #fff;
  font-size: 14px;
  font-family: monospace;
  width: 100%;
}

input[type="text"]:focus,
input[type="number"]:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(255, 255, 255, 0.15);
}

input[type="checkbox"] {
  width: 20px;
  height: 20px;
  cursor: pointer;
}

.vector-input,
.quaternion-input {
  display: flex;
  gap: 8px;
}

.vector-input input,
.quaternion-input input {
  flex: 1;
}

.unsupported-type {
  color: rgba(255, 255, 255, 0.5);
  font-style: italic;
  font-size: 14px;
}

.property-footer {
  display: flex;
  gap: 12px;
  margin-top: 20px;
  padding-top: 16px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
}

.save-button,
.reset-button {
  flex: 1;
  padding: 10px 20px;
  border: none;
  border-radius: 4px;
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
}

.save-button {
  background: #5cb85c;
  color: white;
}

.save-button:hover:not(:disabled) {
  background: #4cae4c;
}

.save-button:disabled {
  background: rgba(92, 184, 92, 0.3);
  cursor: not-allowed;
}

.reset-button {
  background: rgba(255, 255, 255, 0.1);
  color: #fff;
}

.reset-button:hover:not(:disabled) {
  background: rgba(255, 255, 255, 0.2);
}

.reset-button:disabled {
  opacity: 0.3;
  cursor: not-allowed;
}

/* Scrollbar styling */
.property-content::-webkit-scrollbar {
  width: 8px;
}

.property-content::-webkit-scrollbar-track {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 4px;
}

.property-content::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 4px;
}

.property-content::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.3);
}
</style>
