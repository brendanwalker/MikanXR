<template>
  <div class="property-field">
    <!-- Component Reference field -->
    <ComponentRefSelect
      v-if="fieldType === 'component_ref' && componentClass"
      :model-value="fieldValue"
      :component-class="componentClass"
      @update:model-value="handleComponentRefChange"
      class="property-component-ref"
    />

    <!-- Boolean field -->
    <input
      v-else-if="fieldType === 'boolean'"
      type="checkbox"
      :checked="fieldValue"
      @change="handleBooleanChange"
      class="property-checkbox"
    />

    <!-- Number field -->
    <NumberInput
      v-else-if="fieldType === 'number'"
      :model-value="fieldValue"
      @update:model-value="handleNumberChange"
      :step="isIntegerField ? 1 : 0.01"
      :precision="isIntegerField ? 0 : 3"
      :min="min"
      :max="max"
      class="property-number"
    />

    <!-- Enum field -->
    <select
      v-else-if="fieldType === 'enum'"
      :value="normalizedEnumValue"
      @change="handleEnumChange"
      class="property-select"
    >
      <option v-for="option in enumOptions" :key="option.value" :value="option.value">
        {{ option.label }}
      </option>
    </select>

    <!-- String field -->
    <input
      v-else-if="fieldType === 'string'"
      type="text"
      :value="fieldValue"
      @blur="handleStringChange"
      class="property-text"
    />

    <!-- Vector3 field -->
    <div v-else-if="fieldType === 'vector3'" class="property-vector">
      <div class="vector-component">
        <label>X</label>
        <NumberInput
          :model-value="fieldValue.x"
          @update:model-value="(v) => handleVectorChange('x', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
      <div class="vector-component">
        <label>Y</label>
        <NumberInput
          :model-value="fieldValue.y"
          @update:model-value="(v) => handleVectorChange('y', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
      <div class="vector-component">
        <label>Z</label>
        <NumberInput
          :model-value="fieldValue.z"
          @update:model-value="(v) => handleVectorChange('z', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
    </div>

    <!-- Quaternion field -->
    <div v-else-if="fieldType === 'quaternion'" class="property-quaternion">
      <div class="quat-component">
        <label>W</label>
        <NumberInput
          :model-value="fieldValue.w"
          @update:model-value="(v) => handleQuaternionChange('w', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
      <div class="quat-component">
        <label>X</label>
        <NumberInput
          :model-value="fieldValue.x"
          @update:model-value="(v) => handleQuaternionChange('x', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
      <div class="quat-component">
        <label>Y</label>
        <NumberInput
          :model-value="fieldValue.y"
          @update:model-value="(v) => handleQuaternionChange('y', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
      <div class="quat-component">
        <label>Z</label>
        <NumberInput
          :model-value="fieldValue.z"
          @update:model-value="(v) => handleQuaternionChange('z', v)"
          :step="0.01"
          class="vector-input"
        />
      </div>
    </div>

    <!-- Fallback for unknown types -->
    <span v-else class="property-readonly">{{ JSON.stringify(fieldValue) }}</span>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import NumberInput from './NumberInput.vue'
import ComponentRefSelect from './ComponentRefSelect.vue'
import { usePropertyEditor } from '../../composables/usePropertyEditor.js'
import { useFieldMetadata } from '../../composables/useFieldMetadata.js'
import { useMikanStore } from '../../stores/mikanStore.js'
import { useComponentStore } from '../../stores/componentStore.js'
import {
  MikanAPIResult,
  PropertySetValueRequest
} from '@mikanxr/client'

interface Props {
  fieldName: string
  fieldValue: any
  ownerSystem: string
  componentId: number
  /** Serialization type string, e.g. 'enum:MikanStencilCullMode'. Optional. */
  fieldMetaType?: string
  min?: number
  max?: number
}

const props = defineProps<Props>()
const emit = defineEmits<{
  (e: 'update', fieldName: string, fieldValue: any): void
}>()

const { createVariantFromValue } = usePropertyEditor()
const { getEnumOptions } = useFieldMetadata()
const mikanStore = useMikanStore()
const componentStore = useComponentStore()

// Check if this field is a component reference
const componentClass = computed(() => {
  return componentStore.getComponentClassForField(props.fieldName)
})

// Enum options derived from the field's meta type
const enumOptions = computed(() => getEnumOptions(props.fieldMetaType ?? ''))

// Current enum value normalised to an integer (handles legacy string names like "NONE")
const normalizedEnumValue = computed(() => {
  if (typeof props.fieldValue === 'number') return props.fieldValue
  if (typeof props.fieldValue === 'string') {
    const match = enumOptions.value.find((o) => o.label === props.fieldValue)
    if (match) return match.value
  }
  return props.fieldValue
})

// True for integer serialization types (int8/16/32, uint8/16/32)
const isIntegerField = computed(() => {
  const t = props.fieldMetaType
  return t === 'int8' || t === 'int16' || t === 'int32' ||
         t === 'uint8' || t === 'uint16' || t === 'uint32'
})

// Determine field type
const fieldType = computed(() => {
  // Explicit enum type from serialization metadata takes priority
  if (props.fieldMetaType?.startsWith('enum:') && enumOptions.value.length > 0) return 'enum'

  // Check if this is a component reference field
  if (componentClass.value && typeof props.fieldValue === 'number') {
    return 'component_ref'
  }

  if (typeof props.fieldValue === 'boolean') return 'boolean'
  if (typeof props.fieldValue === 'number') return 'number'
  if (typeof props.fieldValue === 'string') return 'string'
  if (typeof props.fieldValue === 'object' && props.fieldValue !== null) {
    if ('x' in props.fieldValue && 'y' in props.fieldValue && 'z' in props.fieldValue && !('w' in props.fieldValue)) {
      return 'vector3'
    }
    if ('w' in props.fieldValue && 'x' in props.fieldValue && 'y' in props.fieldValue && 'z' in props.fieldValue) {
      return 'quaternion'
    }
  }
  return 'unknown'
})

async function sendPropertyUpdate(value: any) {
  if (!mikanStore.client) {
    console.error('[PropertyField] No client connection')
    return
  }

  try {
    const variant = createVariantFromValue(value)

    const request = new PropertySetValueRequest()
    request.ownerSystem = props.ownerSystem
    request.componentId = props.componentId
    request.fieldName = props.fieldName
    request.fieldValue = variant

    const future = mikanStore.client.sendRequest(request)
    const response = await future.await()

    if (response.resultCode === MikanAPIResult.Success) {
      console.log(`[PropertyField] Successfully updated ${props.fieldName}`)
      // Update the local component store
      componentStore.updateComponentProperty(props.componentId, props.ownerSystem, props.fieldName, value)
      emit('update', props.fieldName, value)
    } else {
      console.error(`[PropertyField] Failed to update ${props.fieldName}: ${response.resultCode}`)
    }
  } catch (error) {
    console.error(`[PropertyField] Error updating ${props.fieldName}:`, error)
  }
}

function handleEnumChange(event: Event) {
  const value = parseInt((event.target as HTMLSelectElement).value, 10)
  sendPropertyUpdate(value)
}

function handleComponentRefChange(value: number) {
  sendPropertyUpdate(value)
}

function handleBooleanChange(event: Event) {
  const value = (event.target as HTMLInputElement).checked
  sendPropertyUpdate(value)
}

function handleNumberChange(value: number) {
  sendPropertyUpdate(value)
}

function handleStringChange(event: Event) {
  const value = (event.target as HTMLInputElement).value
  sendPropertyUpdate(value)
}

function handleVectorChange(axis: 'x' | 'y' | 'z', value: number) {
  const updatedVector = { ...props.fieldValue, [axis]: value }
  sendPropertyUpdate(updatedVector)
}

function handleQuaternionChange(component: 'w' | 'x' | 'y' | 'z', value: number) {
  const updatedQuat = { ...props.fieldValue, [component]: value }
  sendPropertyUpdate(updatedQuat)
}
</script>

<style scoped>
.property-field {
  display: flex;
  align-items: center;
  gap: 8px;
  max-width: 100%;
  min-width: 0;
}

.property-checkbox {
  width: 18px;
  height: 18px;
  cursor: pointer;
}

.property-component-ref {
  flex: 1;
  min-width: 150px;
}

.property-number {
  width: 100px;
  text-align: right;
}

.property-text,
.property-select {
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 4px;
  color: #fff;
  font-family: monospace;
  font-size: 14px;
  min-width: 100px;
}

.property-text:focus,
.property-select:focus {
  outline: none;
  border-color: #5cb85c;
  background: rgba(0, 0, 0, 0.4);
}

.property-select {
  cursor: pointer;
}

.property-vector,
.property-quaternion {
  display: flex;
  flex-direction: row;
  gap: 8px;
  flex: 1;
  max-width: 100%;
}

.vector-component,
.quat-component {
  display: flex;
  align-items: center;
  gap: 4px;
  flex: 1;
  min-width: 0;
}

.vector-component label,
.quat-component label {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.7);
  font-weight: 500;
  flex-shrink: 0;
  min-width: 16px;
}

.vector-input {
  flex: 1;
  min-width: 0;
}

.property-readonly {
  color: rgba(255, 255, 255, 0.6);
  font-family: monospace;
  font-size: 12px;
}
</style>
